from __future__ import annotations

from enum import Enum


class ControlMode(str, Enum):
    IRIS = "Iris"
    GAIN = "Gain (ISO)"
    ND = "ND"


MODE_KEYS = {
    "i": ControlMode.IRIS,
    "g": ControlMode.GAIN,
    "n": ControlMode.ND,
}


def step_command(mode: ControlMode, key: str) -> tuple[str, int]:
    if key not in ("u", "d"):
        raise ValueError(f"Unsupported step key: {key}")
    if mode == ControlMode.IRIS:
        return "/api/iris/step", 1 if key == "u" else -1
    if mode == ControlMode.GAIN:
        return "/api/iso/step", 1 if key == "u" else -1
    if mode == ControlMode.ND:
        return "/api/nd/step", -1 if key == "u" else 1
    raise ValueError(f"Unsupported control mode: {mode}")


ND_KEYS = {"b": "/api/nd/on", "m": "/api/nd/off"}
