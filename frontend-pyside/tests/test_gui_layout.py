"""Layout regressions with real Qt widgets; optional captures for visual review."""
from __future__ import annotations

import os
from pathlib import Path
import unittest
from unittest.mock import patch

from PySide6.QtCore import QPoint
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication, QFrame, QStyle, QStyleOptionButton, QStyleOptionComboBox

from fx6_operator.main_window import MainWindow


class GuiLayoutTest(unittest.TestCase):
    SIZES = ((800, 600), (1040, 800), (1280, 900))

    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])
        cls.app.setQuitOnLastWindowClosed(False)

    def setUp(self):
        with patch.object(MainWindow, "_ensure_backend_started", lambda self: None):
            self.window = MainWindow(Path(__file__).resolve().parents[2])
        self.window.camera_combo.addItem("検出されたカメラがありません", "")
        self.window.login_status.setText("0 台のカメラを検出しました。検出ボタンで再試行してください。")
        self.window.login_backend_label.setText("Runtime build: " + self.window.build_info["build_id"])
        self.window.show()
        QTest.qWait(30)

    def tearDown(self):
        with patch.object(MainWindow, "quit_app", lambda self: None):
            self.window.close()
        self.window.deleteLater()
        self.app.processEvents()

    def capture(self, name):
        output = os.environ.get("FX6_GUI_SCREENSHOT_DIR")
        if output:
            path = Path(output)
            path.mkdir(parents=True, exist_ok=True)
            self.assertTrue(self.window.grab().save(str(path / f"{name}.png")))

    def button_text_fits(self, button):
        option = QStyleOptionButton()
        button.initStyleOption(option)
        content = button.style().subElementRect(QStyle.SE_PushButtonContents, option, button)
        self.assertGreaterEqual(content.height(), button.fontMetrics().height())
        self.assertGreaterEqual(content.width(), button.fontMetrics().horizontalAdvance(button.text()))

    def test_login_fields_and_japanese_buttons_fit_at_supported_sizes(self):
        for width, height in self.SIZES:
            with self.subTest(size=(width, height)):
                self.window.resize(width, height)
                QTest.qWait(30)
                self.assertEqual(self.window.width(), width, "Content must not force the window wider")
                card = self.window.findChild(QFrame, "loginCard")
                self.assertGreater(self.window.user_input.width(), card.width() * .60)
                for field in (self.window.user_input, self.window.password_input, self.window.ip_input):
                    self.assertGreaterEqual(field.height(), field.fontMetrics().height() + 16)
                for button in (self.window.refresh_button, self.window.connect_button, self.window.ip_probe_button):
                    self.button_text_fits(button)
                combo = self.window.camera_combo
                option = QStyleOptionComboBox()
                combo.initStyleOption(option)
                content = combo.style().subControlRect(QStyle.CC_ComboBox, option, QStyle.SC_ComboBoxEditField, combo)
                self.assertGreaterEqual(content.height(), combo.fontMetrics().height())
                self.assertGreaterEqual(content.width(), combo.fontMetrics().horizontalAdvance(combo.currentText()))
                page = self.window.login_page
                page.verticalScrollBar().setValue(0)
                self.capture(f"login-{width}x{height}")
                point = self.window.login_backend_label.mapTo(page.viewport(), QPoint(0, 0))
                self.assertGreaterEqual(point.y(), 0)
                self.assertLessEqual(point.y() + self.window.login_backend_label.height(), page.viewport().height())
                self.assertEqual(page.verticalScrollBar().maximum(), 0, "Compact login must fit without scrolling")

    def test_operation_cards_and_long_paths_stay_inside_window(self):
        long_path = "/Users/test/Downloads/" + "long-folder-name/" * 12 + "fx6d"
        self.window.backend_runtime = {"buildId": self.window.build_info["build_id"], "executablePath": long_path}
        self.window._render_state({"connected": True, "cameraModel": "ILME-FX6V",
            "cameraId": "AA:BB:CC:DD:EE:FF", "logPath": long_path + ".log",
            "iris": {"label": "F4.0"}, "iso": {"label": "ISO 12800"},
            "shutterSpeed": {"label": "1/59.94 s"}, "shutterMode": {"label": "Speed"},
            "whiteBalanceMode": {"label": "Manual"}, "colorTemperature": {"label": "15000 K"},
            "awb": {"status": "unconfirmed", "message": "AWB の結果通知を確認できません。カメラ本体の結果を確認してください。"},
            "ndFilter": {"label": "ON"}, "ndOpticalDensity": {"label": "1/~128 (OD 2.1)"}})
        self.window.stack.setCurrentWidget(self.window.operation_page)
        for width, height in self.SIZES:
            with self.subTest(size=(width, height)):
                self.window.resize(width, height)
                QTest.qWait(30)
                self.assertEqual(self.window.width(), width)
                page = self.window.operation_page
                cards = (self.window.card_iris, self.window.card_iso, self.window.card_shutter, self.window.card_nd,
                         self.window.card_camera, self.window.card_backend)
                for card in cards:
                    point = card.mapTo(page.widget(), QPoint(0, 0))
                    self.assertLessEqual(point.x() + card.width(), page.viewport().width())
                    for label in (card.title_label, card.value_label, card.note_label):
                        self.assertEqual(label.frameWidth(), 0, "Card borders must not apply to labels")
                self.assertLessEqual(abs(cards[0].width() - cards[1].width()), 1)
                for button in (self.window.disconnect_button, self.window.quit_button):
                    self.button_text_fits(button)
                self.assertEqual(self.window.card_backend.toolTip(), long_path)
                page.verticalScrollBar().setValue(0)
                self.capture(f"operation-{width}x{height}")


if __name__ == "__main__":
    unittest.main()
