from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import urllib.request
import json
import socket
from dataclasses import dataclass
from typing import Optional

try:
    from .build_info import load_build_info
except ImportError:
    from build_info import load_build_info  # type: ignore

_BUILD_INFO = load_build_info()
_BACKEND_PORT = int(_BUILD_INFO.get("backend_port", 39061))
HEALTH_URL = f"http://127.0.0.1:{_BACKEND_PORT}/api/health"


def _health_payload(timeout: float = 1.5) -> Optional[dict]:
    try:
        req = urllib.request.Request(HEALTH_URL, method="GET")
        with urllib.request.urlopen(req, timeout=timeout) as response:
            payload = response.read().decode("utf-8")
        decoded = json.loads(payload)
        if not decoded.get("ok"):
            return None
        data = decoded.get("data", {})
        return data if isinstance(data, dict) else {}
    except Exception:
        return None


@dataclass
class BackendProcess:
    repo_root: pathlib.Path
    process: Optional[subprocess.Popen] = None
    selected_binary: Optional[pathlib.Path] = None
    selected_source: str = "unknown"
    checked_paths: list[pathlib.Path] | None = None
    last_health_payload: dict | None = None

    def _frozen_candidates(self) -> list[pathlib.Path]:
        if not getattr(sys, "frozen", False):
            return []
        exe = pathlib.Path(sys.executable).resolve()
        candidates: list[pathlib.Path] = []
        if ".app/Contents/MacOS" in str(exe):
            contents = exe.parents[1]
            resources = contents / "Resources"
            candidates.extend(
                [
                    resources / "backend" / "build" / "fx6d",
                    resources / "fx6d",
                    exe.parent / "backend" / "build" / "fx6d",
                    exe.parent / "fx6d",
                ]
            )
        else:
            candidates.extend(
                [
                    exe.parent / "backend" / "build" / "fx6d",
                    exe.parent / "fx6d",
                ]
            )

        meipass = getattr(sys, "_MEIPASS", None)
        if meipass:
            mp = pathlib.Path(meipass)
            candidates.extend(
                [
                    mp / "backend" / "build" / "fx6d",
                    mp / "Resources" / "backend" / "build" / "fx6d",
                    mp / "fx6d",
                ]
            )
        return candidates

    def candidate_binaries(self) -> list[pathlib.Path]:
        exe_name = "fx6d"
        candidates: list[pathlib.Path] = []

        env_override = os.environ.get("FX6_BACKEND_BINARY")
        if env_override:
            candidates.append(pathlib.Path(env_override).expanduser())

        candidates.extend(self._frozen_candidates())
        candidates.append(self.repo_root / "backend" / "build" / exe_name)

        # unique order-preserving
        ordered: list[pathlib.Path] = []
        seen: set[str] = set()
        for c in candidates:
            s = str(c)
            if s not in seen:
                seen.add(s)
                ordered.append(c)
        return ordered

    def backend_binary(self) -> Optional[pathlib.Path]:
        self.checked_paths = self.candidate_binaries()
        for candidate in self.checked_paths:
            if candidate.exists():
                return candidate
        return None

    def is_running(self) -> bool:
        return self.process is not None and self.process.poll() is None

    def start(self) -> None:
        self.last_health_payload = _health_payload()
        if self.last_health_payload is not None:
            running_build = self.last_health_payload.get("buildId")
            expected_build = _BUILD_INFO.get("build_id")
            if running_build != expected_build:
                raise RuntimeError(
                    f"Port {_BACKEND_PORT} has a different backend build "
                    f"({running_build or 'unknown'}). Expected {expected_build}. "
                    "Stop the old backend before launching this app."
                )
            self.selected_source = "reused-running-backend"
            executable = self.last_health_payload.get("executablePath")
            self.selected_binary = pathlib.Path(executable) if executable else None
            return
        if self.is_running():
            return

        try:
            with socket.create_connection(("127.0.0.1", _BACKEND_PORT), timeout=0.5):
                pass
        except OSError:
            pass
        else:
            raise RuntimeError(f"Port {_BACKEND_PORT} is occupied by an unknown/unhealthy service. Check its PID before restarting.")

        binary = self.backend_binary()
        if binary is None:
            checked = "\n".join(str(p) for p in self.candidate_binaries())
            raise RuntimeError(
                "Backend binary was not found, and no running backend was detected.\n"
                f"Checked:\n{checked}"
            )

        env = os.environ.copy()
        env.setdefault("FX6_FRONTEND_BUILD_ID", str(_BUILD_INFO.get("build_id", "unknown")))
        self.process = subprocess.Popen(
            [str(binary)],
            cwd=str(binary.parent),
            env=env,
        )
        self.selected_binary = binary
        self.selected_source = "launched-local-binary"

    def stop(self) -> None:
        if not self.is_running():
            return
        assert self.process is not None
        self.process.terminate()
        try:
            self.process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait(timeout=2)
        finally:
            self.process = None

    def summary(self) -> str:
        binary = str(self.selected_binary) if self.selected_binary else "—"
        source = self.selected_source
        return f"{source}: {binary}"
