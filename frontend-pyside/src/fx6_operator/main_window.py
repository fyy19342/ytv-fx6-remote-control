from __future__ import annotations

import pathlib
from typing import Any, Dict, List

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QCloseEvent, QFont, QKeySequence, QShortcut
from PySide6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFormLayout,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QLayout,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QScrollArea,
    QSizePolicy,
    QStackedWidget,
    QVBoxLayout,
    QWidget,
)

try:
    from .api import ApiClient, ApiError
    from .backend_process import BackendProcess
    from .build_info import load_build_info
    from .keyboard_controls import ControlMode, MODE_KEYS, ND_KEYS, step_command
    from .usage_terms import USAGE_NOTICE_HTML, CONSENT_LABEL
except ImportError:
    from api import ApiClient, ApiError  # type: ignore
    from backend_process import BackendProcess  # type: ignore
    from build_info import load_build_info  # type: ignore
    from keyboard_controls import ControlMode, MODE_KEYS, ND_KEYS, step_command  # type: ignore
    from usage_terms import USAGE_NOTICE_HTML, CONSENT_LABEL  # type: ignore


CARD_STYLE = """
QFrame#metricCard, QFrame#loginCard {
    background-color: #172030;
    border-radius: 14px;
    border: 1px solid #31445f;
}
QLabel#cardTitle {
    color: #9cb2cf;
    font-size: 13px;
}
QLabel#cardValue {
    color: white;
    font-size: 28px;
    font-weight: 700;
}
"""

WINDOW_STYLE = """
QWidget {
    color: #edf2f7;
    font-family: -apple-system, BlinkMacSystemFont, 'Helvetica Neue', sans-serif;
}
QMainWindow, QStackedWidget, QScrollArea, QWidget#pageSurface {
    background-color: #0c1522;
}
QLabel, QCheckBox { background: transparent; }
QLineEdit, QComboBox {
    background-color: #132033;
    border: 1px solid #304866;
    border-radius: 10px;
    padding: 8px 10px;
    min-height: 24px;
    font-size: 14px;
}
QPushButton {
    background-color: #1570ef;
    border: none;
    border-radius: 10px;
    padding: 10px 16px;
    font-weight: 700;
    font-size: 14px;
    min-height: 24px;
}
QPushButton#secondaryButton {
    background-color: #30384a;
}
QPushButton#dangerButton {
    background-color: #b91c1c;
}
QPushButton:disabled { background-color: #263345; color: #8090a5; }
QLineEdit:focus, QComboBox:focus { border-color: #60a5fa; }
"""


class MetricCard(QFrame):
    def __init__(self, title: str, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("metricCard")
        self.setStyleSheet(CARD_STYLE)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(18, 16, 18, 16)
        layout.setSpacing(6)

        self.title_label = QLabel(title)
        self.title_label.setObjectName("cardTitle")
        self.value_label = QLabel("—")
        self.value_label.setObjectName("cardValue")
        self.value_label.setWordWrap(True)
        self.value_label.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)
        self.note_label = QLabel("")
        self.note_label.setStyleSheet("color:#7f8da3;font-size:12px;")
        self.note_label.setWordWrap(True)

        layout.addWidget(self.title_label)
        layout.addWidget(self.value_label)
        layout.addWidget(self.note_label)

    def set_value(self, value: str, note: str = "") -> None:
        self.value_label.setText(value)
        self.note_label.setText(note)


class MainWindow(QMainWindow):
    def __init__(self, repo_root: pathlib.Path) -> None:
        super().__init__()
        self.repo_root = repo_root
        self.build_info = load_build_info()
        self.api = ApiClient()
        self.backend = BackendProcess(repo_root)
        self.current_camera_map: Dict[str, Dict[str, Any]] = {}
        self.backend_runtime: Dict[str, Any] = {}
        self.backend_ready = False
        self.camera_connected = False
        self.control_mode: ControlMode | None = None
        self.nd_on = False
        self._closing = False

        self.setWindowTitle(f"FX6 Operation App [{self.build_info['build_id']}]")
        self.resize(1040, 800)
        self.setMinimumSize(800, 600)
        self.setStyleSheet(WINDOW_STYLE)

        self.stack = QStackedWidget()
        self.setCentralWidget(self.stack)

        self.login_page = self._build_login_page()
        self.operation_page = self._build_operation_page()
        self.stack.addWidget(self.login_page)
        self.stack.addWidget(self.operation_page)
        self.stack.setCurrentWidget(self.login_page)

        self.refresh_timer = QTimer(self)
        self.refresh_timer.setInterval(1000)
        self.refresh_timer.timeout.connect(self.refresh_state)

        self._ensure_backend_started()
        if self.backend_ready:
            QTimer.singleShot(500, self.refresh_camera_list)

    def _set_login_status(self, text: str, level: str = "info") -> None:
        color = {
            "info": "#93c5fd",
            "ok": "#86efac",
            "warn": "#fcd34d",
            "error": "#fca5a5",
        }.get(level, "#93c5fd")
        self.login_status.setStyleSheet(f"color:{color};")
        self.login_status.setText(text)

    def _ensure_backend_started(self) -> None:
        try:
            self.backend.start()
        except Exception as exc:
            self._set_login_status(f"Backend 起動失敗: {exc}", "error")
            self.login_backend_label.setText(str(exc))
            self.backend_ready = False
            return

        self._set_login_status("バックエンド起動確認中...", "info")
        QApplication.processEvents()
        if not self.api.wait_for_health(timeout_seconds=20.0):
            self._set_login_status(
                "バックエンドの起動待ちでタイムアウトしました。backend/build/fx6d を単体起動して確認してください。",
                "error",
            )
            checked = self.backend.checked_paths or []
            checked_text = "\n".join(str(path) for path in checked) if checked else "候補パスなし"
            self.login_backend_label.setText(f"Backend: 起動失敗\n{checked_text}")
            self.backend_ready = False
            return
        if not self._refresh_backend_runtime():
            self.backend.stop()
            return
        self.backend_ready = True
        self._set_login_status("バックエンド起動確認完了。カメラ一覧を取得します。", "ok")

    def _refresh_backend_runtime(self) -> bool:
        try:
            runtime = self.api.get("/api/health")
        except ApiError:
            runtime = {}
        self.backend_runtime = runtime
        runtime_build = runtime.get("buildId", "—")
        runtime_executable = runtime.get("executablePath", "—")
        summary = self.backend.summary()
        details = (
            "Backend: "
            + summary
            + "\nRuntime build: "
            + str(runtime_build)
            + "\nExecutable: "
            + str(runtime_executable)
        )
        self.login_backend_label.setText(f"Runtime build: {runtime_build}")
        self.login_backend_label.setToolTip(details)
        self.card_backend.set_value(str(runtime_build), "バックエンド稼働中")
        self.card_backend.setToolTip(details)
        if runtime_build != self.build_info["build_id"] or (
            self.backend.selected_source == "launched-local-binary" and not self.backend.is_running()
        ):
            self.backend_ready = False
            self.camera_connected = False
            self._set_login_status(f"Backend の build ID / 起動プロセスが不一致です: {runtime_build}", "error")
            return False
        return True

    @staticmethod
    def _scroll_page(content: QWidget) -> QScrollArea:
        # Preserve the controls' natural height instead of compressing their text.
        content.setObjectName("pageSurface")
        content.layout().setSizeConstraint(QLayout.SetMinimumSize)
        scroll = QScrollArea()
        scroll.setFrameShape(QFrame.NoFrame)
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        scroll.setWidget(content)
        return scroll

    def _build_login_page(self) -> QWidget:
        root = QWidget()
        layout = QVBoxLayout(root)
        layout.setContentsMargins(28, 28, 28, 28)
        layout.setSpacing(16)

        heading = QLabel("FX6 Operation App")
        heading_font = QFont()
        heading_font.setPointSize(28)
        heading_font.setBold(True)
        heading.setFont(heading_font)

        sub = QLabel("Sony FX6 を Camera Remote SDK 経由で接続し、キーボードで Iris・Gain (ISO)・ND を操作します。")
        sub.setStyleSheet("color:#9aa6b2;font-size:14px;")
        sub.setWordWrap(True)

        build_badge = QLabel(f"Build {self.build_info['build_id']} / v{self.build_info['version']}")
        build_badge.setStyleSheet(
            "background:#123154;color:#cde5ff;border:1px solid #2b5a8f;"
            "padding:6px 10px;border-radius:8px;font-weight:700;"
        )

        form_card = QFrame()
        form_card.setObjectName("loginCard")
        form_card.setStyleSheet(CARD_STYLE)
        form = QFormLayout(form_card)
        form.setContentsMargins(24, 24, 24, 24)
        form.setSpacing(18)
        form.setFieldGrowthPolicy(QFormLayout.AllNonFixedFieldsGrow)
        form.setFormAlignment(Qt.AlignLeft | Qt.AlignTop)
        form.setLabelAlignment(Qt.AlignLeft | Qt.AlignVCenter)

        self.camera_combo = QComboBox()
        self.camera_combo.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.camera_combo.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLengthWithIcon)
        self.camera_combo.setMinimumContentsLength(24)
        self.refresh_button = QPushButton("カメラ一覧を更新")
        self.refresh_button.setObjectName("secondaryButton")
        self.refresh_button.clicked.connect(self.refresh_camera_list)

        camera_row = QWidget()
        camera_row_layout = QHBoxLayout(camera_row)
        camera_row_layout.setContentsMargins(0, 0, 0, 0)
        camera_row_layout.addWidget(self.camera_combo, 1)
        camera_row_layout.addWidget(self.refresh_button)

        self.fingerprint_label = QLabel("—")
        self.fingerprint_label.setStyleSheet("color:#9aa6b2;")
        self.fingerprint_label.setWordWrap(True)
        self.fingerprint_label.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)
        self.camera_combo.currentIndexChanged.connect(self._update_fingerprint_label)

        self.user_input = QLineEdit("admin")
        self.password_input = QLineEdit()
        self.password_input.setEchoMode(QLineEdit.Password)
        for field in (self.user_input, self.password_input):
            field.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        self.connect_button = QPushButton("接続")
        self.connect_button.setEnabled(False)
        self.connect_button.clicked.connect(self.connect_camera)
        self.connect_button.setMinimumWidth(120)
        self.connect_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Fixed)

        usage_notice = QLabel(USAGE_NOTICE_HTML)
        usage_notice.setWordWrap(True)
        usage_notice.setOpenExternalLinks(True)
        usage_notice.setStyleSheet("color:#9aa6b2;font-size:12px;")
        self.usage_consent = QCheckBox(CONSENT_LABEL)
        self.usage_consent.setChecked(False)
        self.usage_consent.toggled.connect(self.connect_button.setEnabled)

        form.addRow("Camera", camera_row)
        form.addRow("Fingerprint", self.fingerprint_label)
        form.addRow("User", self.user_input)
        form.addRow("Password", self.password_input)
        form.addRow("", self.connect_button)

        self.login_status = QLabel("")
        self.login_status.setWordWrap(True)
        self._set_login_status("待機中", "info")
        self.login_backend_label = QLabel("Backend: 確認前")
        self.login_backend_label.setWordWrap(True)
        self.login_backend_label.setStyleSheet("color:#9aa6b2;")
        self.login_backend_label.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)

        layout.addWidget(heading)
        layout.addWidget(sub)
        layout.addWidget(build_badge, 0, Qt.AlignLeft)
        layout.addWidget(form_card)
        layout.addWidget(usage_notice)
        layout.addWidget(self.usage_consent)
        layout.addWidget(self.login_status)
        layout.addWidget(self.login_backend_label)
        layout.addStretch(1)
        return self._scroll_page(root)

    def _build_operation_page(self) -> QWidget:
        root = QWidget()
        layout = QVBoxLayout(root)
        layout.setContentsMargins(24, 24, 24, 24)
        layout.setSpacing(18)

        header = QHBoxLayout()
        title = QLabel("Operation")
        title_font = QFont()
        title_font.setPointSize(24)
        title_font.setBold(True)
        title.setFont(title_font)

        self.connection_label = QLabel("Disconnected")
        self.connection_label.setStyleSheet("color:#f59e0b;font-weight:700;")
        self.version_label = QLabel(f"Build {self.build_info['build_id']} / v{self.build_info['version']}")
        self.version_label.setStyleSheet("color:#9aa6b2;font-weight:700;")

        header.addWidget(title)
        header.addStretch(1)
        header.addWidget(self.version_label)
        header.addWidget(self.connection_label)

        grid = QGridLayout()
        grid.setHorizontalSpacing(16)
        grid.setVerticalSpacing(16)
        for column in range(3):
            grid.setColumnStretch(column, 1)

        self.card_iris = MetricCard("Iris")
        self.card_iso = MetricCard("Gain (ISO)")
        self.card_nd = MetricCard("ND")
        self.card_camera = MetricCard("Camera")
        self.card_backend = MetricCard("Backend")

        grid.addWidget(self.card_iris, 0, 0)
        grid.addWidget(self.card_iso, 0, 1)
        grid.addWidget(self.card_nd, 0, 2)
        grid.addWidget(self.card_camera, 1, 0)
        grid.addWidget(self.card_backend, 1, 1, 1, 2)

        self.mode_label = QLabel("操作モード: 未選択")
        self.mode_label.setStyleSheet(
            "background:#123154;color:#d8ecff;border:1px solid #2b5a8f;"
            "padding:12px 16px;border-radius:12px;font-size:18px;font-weight:700;"
        )
        self.mode_label.setWordWrap(True)
        self.control_feedback = QLabel("i / g / n で操作モードを選択してください。")
        self.control_feedback.setWordWrap(True)
        self.control_feedback.setStyleSheet("color:#9cb2cf;")

        footer = QHBoxLayout()
        self.log_path_label = QLabel("Log: —")
        self.log_path_label.setStyleSheet("color:#9aa6b2;")
        self.log_path_label.setWordWrap(True)
        self.log_path_label.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)
        footer.addWidget(self.log_path_label, 1)

        self.disconnect_button = QPushButton("切断")
        self.disconnect_button.setObjectName("secondaryButton")
        self.disconnect_button.clicked.connect(self.disconnect_camera)

        self.quit_button = QPushButton("終了")
        self.quit_button.setObjectName("dangerButton")
        self.quit_button.clicked.connect(self.quit_app)
        footer.addWidget(self.disconnect_button)
        footer.addWidget(self.quit_button)

        guide = QLabel(
            "モード  i: Iris   g: Gain (ISO)   n: ND\n"
            "調整  u: 明るく   d: 暗く  (選択モードを1段ずつ)\n"
            "ND     b: ON + 最小濃度   m: OFF"
        )
        guide.setStyleSheet("background:#172030;color:#d7e4f7;padding:16px;border-radius:12px;")
        guide.setWordWrap(True)

        layout.addLayout(header)
        layout.addWidget(self.mode_label)
        layout.addLayout(grid)
        layout.addWidget(guide)
        layout.addWidget(self.control_feedback)
        layout.addLayout(footer)
        layout.addStretch(1)

        page = self._scroll_page(root)
        page.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        self.shortcuts: list[QShortcut] = []
        for key in (*MODE_KEYS, "u", "d", *ND_KEYS):
            shortcut = QShortcut(QKeySequence(key.upper()), page)
            shortcut.setContext(Qt.ShortcutContext.WidgetWithChildrenShortcut)
            shortcut.setAutoRepeat(False)
            shortcut.activated.connect(lambda pressed=key: self._handle_hotkey(pressed))
            self.shortcuts.append(shortcut)
        return page

    def refresh_camera_list(self) -> None:
        if not self.backend_ready:
            self._ensure_backend_started()
            if not self.backend_ready:
                return
        self.refresh_button.setEnabled(False)
        self.refresh_button.setText("検出中...")
        self._set_login_status("カメラを検出しています...", "info")
        QApplication.processEvents()
        try:
            payload = self.api.get("/api/cameras")
        except ApiError as exc:
            self._set_login_status(f"カメラ一覧取得失敗: {exc}", "error")
            self.camera_combo.blockSignals(True)
            self.camera_combo.clear()
            self.camera_combo.addItem("検出されたカメラがありません", "")
            self.camera_combo.blockSignals(False)
            self.fingerprint_label.setText("—")
            return
        finally:
            self.refresh_button.setEnabled(True)
            self.refresh_button.setText("カメラ一覧を更新")

        cameras: List[Dict[str, Any]] = list(payload)
        self.current_camera_map = {camera["id"]: camera for camera in cameras}
        self.camera_combo.blockSignals(True)
        self.camera_combo.clear()
        if not cameras:
            self.camera_combo.addItem("検出されたカメラがありません", "")
        else:
            for camera in cameras:
                label = f"{camera['model']} ({camera['id']})"
                self.camera_combo.addItem(label, camera["id"])
        self.camera_combo.blockSignals(False)
        self._update_fingerprint_label()
        if cameras:
            self._set_login_status(f"{len(cameras)} 台のカメラを検出しました。", "ok")
        else:
            self._set_login_status("0 台のカメラを検出しました。検出ボタンで再試行してください。", "warn")
        self._refresh_backend_runtime()

    def _update_fingerprint_label(self) -> None:
        camera_id = self.camera_combo.currentData()
        camera = self.current_camera_map.get(camera_id, {})
        self.fingerprint_label.setText(camera.get("fingerprint") or "—")

    def connect_camera(self) -> None:
        if not self.usage_consent.isChecked():
            self._set_login_status("利用条件への同意後に接続してください。", "warn")
            return
        if not self.backend_ready:
            self._set_login_status("一致する backend が起動していません。", "error")
            return
        camera_id = self.camera_combo.currentData()
        if not camera_id:
            QMessageBox.warning(self, "Connect", "接続対象カメラを選択してください。")
            return
        self._set_login_status("カメラへ接続中...", "info")
        QApplication.processEvents()
        try:
            state = self.api.post(
                "/api/connect",
                {
                    "cameraId": camera_id,
                    "userId": self.user_input.text(),
                    "password": self.password_input.text(),
                },
            )
        except ApiError as exc:
            self._set_login_status(f"接続失敗: {exc}", "error")
            return

        self.camera_connected = bool(state.get("connected"))
        if not self.camera_connected:
            self._set_login_status("接続応答に connected=true がありません。", "error")
            return
        self.control_mode = None
        self.mode_label.setText("操作モード: 未選択")
        self._set_control_feedback("i / g / n で操作モードを選択してください。")
        self.stack.setCurrentWidget(self.operation_page)
        self.operation_page.setFocus()
        self.refresh_timer.start()
        self._render_state(state)

    def _set_control_feedback(self, message: str, error: bool = False) -> None:
        self.control_feedback.setText(message)
        self.control_feedback.setStyleSheet("color:#fca5a5;" if error else "color:#9cb2cf;")

    def _handle_hotkey(self, key: str) -> None:
        if (not self.isActiveWindow() or QApplication.activeModalWidget() is not None
                or self.stack.currentWidget() is not self.operation_page or not self.camera_connected):
            return
        if key in MODE_KEYS:
            self.control_mode = MODE_KEYS[key]
            self.mode_label.setText(f"操作モード: {self.control_mode.value}")
            self._set_control_feedback(f"{self.control_mode.value} を選択しました。u / d で1段ずつ調整します。")
            return

        if key in ("u", "d"):
            if self.control_mode is None:
                self._set_control_feedback("先に i / g / n で操作モードを選択してください。", error=True)
                return
            if self.control_mode == ControlMode.ND and not self.nd_on:
                self._set_control_feedback("ND は OFF または状態不明です。b で最小濃度の ON にしてください。", error=True)
                return
            path, delta = step_command(self.control_mode, key)
            params: Dict[str, Any] = {"delta": delta}
        elif key in ND_KEYS:
            path = ND_KEYS[key]
            params = {}
        else:
            return

        try:
            state = self.api.post(path, params)
        except ApiError as exc:
            self.refresh_state()
            self._set_control_feedback(f"操作失敗: {exc}", error=True)
            return

        self._render_state(state)
        if key == "b":
            self._set_control_feedback("ND を ON にし、最も明るい濃度にしました。")
        elif key == "m":
            self._set_control_feedback("ND を OFF にしました。")
        else:
            self._set_control_feedback(f"{self.control_mode.value} を1段調整しました。")

    def refresh_state(self) -> None:
        try:
            state = self.api.get("/api/state")
        except ApiError as exc:
            self.camera_connected = False
            self.connection_label.setText(f"Error: {exc}")
            self.connection_label.setStyleSheet("color:#fca5a5;font-weight:700;")
            return

        self._render_state(state)

    def _render_state(self, state: Dict[str, Any]) -> None:

        connected = bool(state.get("connected"))
        self.camera_connected = connected
        self.connection_label.setText("Connected" if connected else "Disconnected")
        self.connection_label.setStyleSheet(
            "color:#34d399;font-weight:700;" if connected else "color:#f59e0b;font-weight:700;"
        )

        iris = state.get("iris", {})
        iso = state.get("iso", {})
        nd_filter = state.get("ndFilter", {})
        nd_density = state.get("ndOpticalDensity", {})

        self.card_iris.set_value(iris.get("label", "—"), "i + u/d")
        self.card_iso.set_value(iso.get("label", "—"), "g + u/d")
        self.nd_on = nd_filter.get("label") == "ON"
        nd_label = nd_density.get("label", "—") if self.nd_on else "OFF" if nd_filter.get("label") == "OFF" else "—"
        self.card_nd.set_value(nd_label, "n + u/d / b: ON / m: OFF")
        self.card_camera.set_value(state.get("cameraModel", "—"), state.get("cameraId", ""))
        backend_build = str(self.backend_runtime.get("buildId", "—"))
        backend_path = str(self.backend_runtime.get("executablePath", self.backend.summary()))
        self.card_backend.set_value(backend_build, "バックエンド稼働中")
        self.card_backend.setToolTip(backend_path)
        log_path = str(state.get('logPath', '—'))
        self.log_path_label.setText(f"Log: {pathlib.Path(log_path).name}")
        self.log_path_label.setToolTip(log_path)

    def disconnect_camera(self) -> None:
        try:
            self.api.post("/api/disconnect")
        except ApiError as exc:
            QMessageBox.warning(self, "Disconnect", str(exc))
            return
        self.refresh_timer.stop()
        self.camera_connected = False
        self.control_mode = None
        self.stack.setCurrentWidget(self.login_page)
        self._set_login_status("切断しました。", "ok")

    def quit_app(self) -> None:
        if self._closing:
            return
        self._closing = True
        self.refresh_timer.stop()
        try:
            if self.camera_connected:
                self.api.post("/api/disconnect")
            if self.backend.selected_source == "launched-local-binary":
                self.api.post("/api/quit")
        except ApiError:
            pass
        self.camera_connected = False
        self.backend.stop()
        QApplication.instance().quit()

    def closeEvent(self, event: QCloseEvent) -> None:
        self.quit_app()
        event.accept()
