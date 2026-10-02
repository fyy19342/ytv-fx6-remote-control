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
                      "shutterSpeed": {"label": "1/60 s"}, "shutterMode": {"label": "Speed"},
                      "whiteBalanceMode": {"label": "Manual"}, "colorTemperature": {"label": "5600 K"},
                      "awb": {"status": "idle", "message": ""},
                      "ndFilter": {"label": "OFF"}, "ndOpticalDensity": {"label": "1/~4 (OD 0.6)"}}

    def get(self, path):
        return self.state.copy()

    def post(self, path, params=None):
        self.calls.append((path, params or {}))
        if self.fail:
            self.state["ndFilter"] = {"label": "OFF"}
            raise ApiError("ND ON aborted; ND OFF confirmed")
        if path == "/api/nd/toggle":
            self.state["ndFilter"] = {"label": "OFF" if self.state["ndFilter"]["label"] == "ON" else "ON"}
        if path == "/api/nd/off": self.state["ndFilter"] = {"label": "OFF"}
        if path == "/api/white-balance/awb":
            self.state["awb"] = {"status": "running", "message": "AWB 実行中", "retryAfterMs": 3000}
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
        self.key("ignsudbma", self.window.user_input)
        self.assertEqual(self.window.user_input.text(), "ignsudbma")
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

    def test_direct_ip_probe_selects_target_without_authentication(self):
        camera = {"id": "ip:192.168.0.5", "ipAddress": "192.168.0.5", "fingerprint": "test-fingerprint"}
        self.window.backend_ready = True
        self.window.ip_input.setText("192.168.0.5")
        with patch.object(self.api, "post", return_value=camera) as post:
            self.window.probe_camera_ip()
            post.assert_called_once_with("/api/cameras/ip", {"ipAddress": "192.168.0.5"})
        self.assertEqual(self.window.camera_combo.currentData(), camera["id"])
        self.assertEqual(self.window.fingerprint_label.text(), camera["fingerprint"])
        self.assertFalse(self.window.camera_connected)
        self.assertFalse(self.window.usage_consent.isChecked())
        self.window.usage_consent.setChecked(True)
        self.window.connect_camera()
        self.window.refresh_timer.stop()
        self.assertEqual(self.api.calls[0][1]["fingerprint"], camera["fingerprint"])

    def test_failed_ip_probe_does_not_leave_stale_target_selected(self):
        self.window.backend_ready = True
        self.window.camera_combo.addItem("Old camera", "ip:192.168.0.5")
        self.window.ip_input.setText("bad address")
        with patch.object(self.api, "post", side_effect=ApiError("Invalid IPv4 address")):
            self.window.probe_camera_ip()
        self.assertIsNone(self.window.camera_combo.currentData())
        self.assertIn("Invalid IPv4", self.window.login_status.text())
        self.assertTrue(self.window.ip_probe_button.isEnabled())

    def test_empty_ip_does_not_call_backend(self):
        self.window.probe_camera_ip()
        self.assertEqual(self.api.calls, [])

    def test_all_modes_and_directions_via_real_shortcuts(self):
        self.operation()
        self.key("u")
        self.assertEqual(self.api.calls, [])
        self.key("iudgudsudnbudm")
        self.assertEqual(self.api.calls, [
            ("/api/iris/step", {"delta": 1}), ("/api/iris/step", {"delta": -1}),
            ("/api/iso/step", {"delta": 1}), ("/api/iso/step", {"delta": -1}),
            ("/api/shutter/step", {"delta": 1}), ("/api/shutter/step", {"delta": -1}),
            ("/api/nd/toggle", {}), ("/api/nd/step", {"delta": -1}),
            ("/api/nd/step", {"delta": 1}), ("/api/nd/off", {})])
        self.assertEqual(self.window.card_iso.title_label.text(), "Gain (ISO)")
        self.assertEqual(self.window.card_shutter.value_label.text(), "1/60 s")
        self.assertEqual(self.window.card_nd.value_label.text(), "OFF")

    def test_nd_off_blocks_steps_and_buttons_preserve_mode(self):
        self.operation()
        self.key("nud")
        self.assertEqual(self.api.calls, [])
        self.key("gbm")
        self.assertEqual(self.window.control_mode, ControlMode.GAIN)
        self.assertEqual([p for p, _ in self.api.calls], ["/api/nd/toggle", "/api/nd/off"])

    def test_b_toggles_from_live_state_and_retains_shutter_mode(self):
        self.operation()
        self.key("sb")
        self.assertEqual(self.window.control_mode, ControlMode.SHUTTER)
        self.assertIn("最も明るい", self.window.control_feedback.text())
        self.key("b")
        self.assertEqual(self.window.card_nd.value_label.text(), "OFF")
        self.assertIn("OFF", self.window.control_feedback.text())
        self.key("b")
        self.assertNotEqual(self.window.card_nd.value_label.text(), "OFF")
        self.api.state["ndFilter"] = {"label": "OFF"}  # Camera body changed after the last GUI poll.
        self.key("b")
        self.assertNotEqual(self.window.card_nd.value_label.text(), "OFF")
        self.assertEqual(self.window.control_mode, ControlMode.SHUTTER)
        self.assertEqual(self.api.calls, [("/api/nd/toggle", {})] * 4)

    def test_shutter_mode_selection_does_not_write_and_non_speed_is_labelled(self):
        self.operation()
        self.key("s")
        self.assertEqual(self.api.calls, [])
        for mode in ("OFF", "Auto", "ECS", "Angle"):
            self.window._render_state({**self.api.state, "shutterMode": {"label": mode}})
            self.assertEqual(self.window.card_shutter.value_label.text(), mode)

    def test_nd_uses_fx6_transmittance_readback(self):
        self.operation()
        state = {**self.api.state, "ndFilter": {"label": "ON"}, "ndValue": {"label": "1/4.8"},
                 "ndOpticalDensity": {"label": "different-model-property"}}
        self.window._render_state(state)
        self.assertEqual(self.window.card_nd.value_label.text(), "1/4.8")

    @patch("fx6_operator.main_window.monotonic", return_value=1000.0)
    def test_awb_action_preserves_mode_blocks_reentry_and_renders_result(self, clock):
        self.operation()
        self.key("sa")
        self.assertEqual(self.api.calls, [("/api/white-balance/awb", {})])
        self.assertEqual(self.window.control_mode, ControlMode.SHUTTER)
        self.assertIn("実行中", self.window.white_balance_label.text())
        self.assertNotIn("完了", self.window.white_balance_label.text())
        self.key("aa")
        self.assertEqual(len(self.api.calls), 1)
        clock.return_value = 1002.0
        for status, message in [("completed", "AWB 完了"), ("failed", "白い領域が不足"),
                                ("unconfirmed", "結果を確認できません")]:
            self.api.state["awb"] = {"status": status, "message": message, "retryAfterMs": 1000}
            self.window.refresh_state()
            self.assertIn(message, self.window.white_balance_label.text())
            self.assertFalse(self.window.awb_running)
            self.key("a")
            self.assertEqual(len(self.api.calls), 1, "Result arrival must not shorten the cooldown")
        clock.return_value = 1003.0
        self.key("a")
        self.assertEqual(len(self.api.calls), 2)

    @patch("fx6_operator.main_window.monotonic", return_value=1000.0)
    def test_awb_retries_at_three_seconds_without_result_or_poll(self, clock):
        self.operation()
        self.key("a")
        clock.return_value = 1002.999
        self.key("aa")
        self.assertEqual(len(self.api.calls), 1)
        self.assertIn("3秒", self.window.control_feedback.text())
        self.assertTrue(self.window.awb_running)
        clock.return_value = 1003.0
        self.key("a")
        self.assertEqual(len(self.api.calls), 2)
        clock.return_value = 1005.999
        self.key("a")
        self.assertEqual(len(self.api.calls), 2)
        clock.return_value = 1006.0
        self.key("a")
        self.assertEqual(len(self.api.calls), 3)

    def test_awb_needs_no_exposure_mode_and_no_auto_repeat(self):
        self.operation()
        self.window.disconnect_button.setFocus()
        QTest.keyClick(self.window.disconnect_button, Qt.Key_A, Qt.ControlModifier)
        self.app.sendEvent(self.window.disconnect_button,
                           QKeyEvent(QEvent.KeyPress, Qt.Key_A, Qt.NoModifier, "a", True, 2))
        self.assertEqual(self.api.calls, [])
        self.key("a", self.window.disconnect_button)
        self.assertIsNone(self.window.control_mode)
        self.assertEqual(self.api.calls, [("/api/white-balance/awb", {})])

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
        self.key("isubma", other)
        self.assertEqual(self.api.calls, [])
        other.close()
        self.window.activateWindow()
        QTest.qWait(40)
        dialog = QDialog(self.window)
        dialog.setModal(True)
        dialog.show()
        QTest.qWait(40)
        self.key("isubma", dialog)
        self.assertEqual(self.api.calls, [])
        dialog.close()
        self.window.activateWindow()
        self.window.operation_page.setFocus()
        QTest.qWait(40)
        self.window._render_state({"connected": False})
        self.key("isubma")
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
