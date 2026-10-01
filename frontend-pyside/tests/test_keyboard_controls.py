from __future__ import annotations

import unittest

from fx6_operator.keyboard_controls import ControlMode, MODE_KEYS, ND_KEYS, step_command


class KeyboardControlsTest(unittest.TestCase):
    def test_mode_keys(self) -> None:
        self.assertEqual(MODE_KEYS["i"], ControlMode.IRIS)
        self.assertEqual(MODE_KEYS["g"], ControlMode.GAIN)
        self.assertEqual(MODE_KEYS["n"], ControlMode.ND)

    def test_step_direction(self) -> None:
        self.assertEqual(step_command(ControlMode.IRIS, "u"), ("/api/iris/step", 1))
        self.assertEqual(step_command(ControlMode.IRIS, "d"), ("/api/iris/step", -1))
        self.assertEqual(step_command(ControlMode.GAIN, "u"), ("/api/iso/step", 1))
        self.assertEqual(step_command(ControlMode.GAIN, "d"), ("/api/iso/step", -1))
        self.assertEqual(step_command(ControlMode.ND, "u"), ("/api/nd/step", -1))
        self.assertEqual(step_command(ControlMode.ND, "d"), ("/api/nd/step", 1))

    def test_nd_keys(self) -> None:
        self.assertEqual(ND_KEYS, {"b": "/api/nd/on", "m": "/api/nd/off"})


if __name__ == "__main__":
    unittest.main()
