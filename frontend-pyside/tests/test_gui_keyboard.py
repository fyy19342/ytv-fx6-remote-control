from __future__ import annotations

import pathlib
import unittest
from unittest.mock import patch

from PySide6.QtCore import Qt, QEvent
from PySide6.QtGui import QKeyEvent
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication, QLineEdit, QDialog
from fx6_operator.api import ApiError
from fx6_operator.main_window import MainWindow
from fx6_operator.keyboard_controls import ControlMode


class FakeApi:
    def __init__(self):
        self.calls = []
        self.fail = False
        self.state = {"connected": True, "cameraModel": "FX6 test double",
                      "iris": {"label": "F4"}, "iso": {"label": "ISO 800"},
                      "ndFilter": {"label": "OFF"}, "ndOpticalDensity": {"label": "1/~4 (OD 0.6)"}}

    def get(self, path):
        return self.state.copy()

    def post(self, path, params=None):
        self.calls.append((path, params or {}))
        if self.fail:
            self.state["ndFilter"] = {"label": "OFF"}
            raise ApiError("ND ON aborted; ND OFF confirmed")
        if path == "/api/nd/on": self.state["ndFilter"] = {"label": "ON"}
        if path == "/api/nd/off": self.state["ndFilter"] = {"label": "OFF"}
        return self.state.copy()


class GuiKeyboardTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])
        cls.app.setQuitOnLastWindowClosed(False)

    def setUp(self):
        with patch.object(MainWindow, "_ensure_backend_started", lambda self: None), patch.object(MainWindow, "quit_app", lambda self: None):
            self.window = MainWindow(pathlib.Path(__file__).resolve().parents[2])
        self.api = FakeApi()
        self.window.api = self.api
        self.window.show()
        self.window.activateWindow()
        self.window.raise_()
        QTest.qWait(40)

    def tearDown(self):
        self.window.refresh_timer.stop()
        with patch.object(MainWindow, "quit_app", lambda self: None):
            self.window.close()
        self.app.processEvents()

    def operation(self):
        self.window.backend_ready = True
        self.window.usage_consent.setChecked(True)
        self.window.camera_combo.addItem("Test camera", "test-camera")
        self.window.connect_camera()
        self.window.refresh_timer.stop()
        self.api.calls.clear()
        self.app.processEvents()
        self.assertTrue(self.window.isActiveWindow())
        self.assertIs(self.window.stack.currentWidget(), self.window.operation_page)

    def key(self, text, target=None):
        QTest.keyClicks(target or self.window.operation_page, text)
        self.app.processEvents()

    def test_login_text_is_not_a_camera_command(self):
        self.window.user_input.setText("")
        self.window.user_input.setFocus()
        self.key("ignudbm", self.window.user_input)
        self.assertEqual(self.window.user_input.text(), "ignudbm")
        self.assertEqual(self.api.calls, [])
        self.assertIsNone(self.window.control_mode)

    def test_camera_connection_requires_explicit_consent(self):
        self.assertFalse(self.window.usage_consent.isChecked())
        self.assertFalse(self.window.connect_button.isEnabled())
        self.window.backend_ready = True
        self.window.camera_combo.addItem("Test", "test-camera")
        self.window.connect_camera()
        self.assertEqual(self.api.calls, [])
        self.window.usage_consent.setChecked(True)
        self.assertTrue(self.window.connect_button.isEnabled())
        self.window.usage_consent.setChecked(False)
        self.assertFalse(self.window.connect_button.isEnabled())

    def test_all_modes_and_directions_via_real_shortcuts(self):
        self.operation()
        self.key("u")
        self.assertEqual(self.api.calls, [])
        self.key("iudgudnbudm")
        self.assertEqual(self.api.calls, [
            ("/api/iris/step", {"delta": 1}), ("/api/iris/step", {"delta": -1}),
            ("/api/iso/step", {"delta": 1}), ("/api/iso/step", {"delta": -1}),
            ("/api/nd/on", {}), ("/api/nd/step", {"delta": -1}),
            ("/api/nd/step", {"delta": 1}), ("/api/nd/off", {})])
        self.assertEqual(self.window.card_iso.title_label.text(), "Gain (ISO)")
        self.assertEqual(self.window.card_nd.value_label.text(), "OFF")

    def test_nd_off_blocks_steps_and_buttons_preserve_mode(self):
        self.operation()
        self.key("nud")
        self.assertEqual(self.api.calls, [])
        self.key("gbm")
        self.assertEqual(self.window.control_mode, ControlMode.GAIN)
        self.assertEqual([p for p, _ in self.api.calls], ["/api/nd/on", "/api/nd/off"])

    def test_child_button_focus_and_no_repeat_or_modifiers(self):
        self.operation()
        self.window.disconnect_button.setFocus()
        self.key("iu", self.window.disconnect_button)
        self.assertEqual(self.api.calls, [("/api/iris/step", {"delta": 1})])
        QTest.keyClick(self.window.disconnect_button, Qt.Key_U, Qt.ControlModifier)
        self.app.sendEvent(self.window.disconnect_button,
                           QKeyEvent(QEvent.KeyPress, Qt.Key_U, Qt.NoModifier, "u", True, 2))
        self.assertEqual(len(self.api.calls), 1)

    def test_inactive_window_modal_and_disconnect_do_not_operate(self):
        self.operation()
        other = QLineEdit()
        other.show()
        other.activateWindow()
        other.setFocus()
        QTest.qWait(40)
        self.assertFalse(self.window.isActiveWindow())
        self.key("iubm", other)
        self.assertEqual(self.api.calls, [])
        other.close()
        self.window.activateWindow()
        QTest.qWait(40)
        dialog = QDialog(self.window)
        dialog.setModal(True)
        dialog.show()
        QTest.qWait(40)
        self.key("iubm", dialog)
        self.assertEqual(self.api.calls, [])
        dialog.close()
        self.window.activateWindow()
        self.window.operation_page.setFocus()
        QTest.qWait(40)
        self.window._render_state({"connected": False})
        self.key("iubm")
        self.assertEqual(self.api.calls, [])

    def test_nd_failure_shows_reason_and_refreshes_off_state(self):
        self.operation()
        self.key("b")
        self.api.fail = True
        self.key("b")
        self.assertIn("ND OFF confirmed", self.window.control_feedback.text())
        self.assertEqual(self.window.card_nd.value_label.text(), "OFF")

    def test_quit_disconnects_only_once(self):
        self.operation()
        with patch("fx6_operator.main_window.QApplication.instance"):
            self.window.quit_app()
            self.window.quit_app()  # Button quit can also cause closeEvent.
        self.assertEqual(self.api.calls, [("/api/disconnect", {})])
        self.assertFalse(self.window.camera_connected)


if __name__ == "__main__":
    unittest.main()
