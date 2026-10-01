from __future__ import annotations

import json
import urllib.parse
import urllib.request
import urllib.error
from dataclasses import dataclass
from typing import Any, Dict, Optional
import time

try:
    from .build_info import load_build_info
except ImportError:
    from build_info import load_build_info  # type: ignore

_BUILD_INFO = load_build_info()
_BACKEND_PORT = int(_BUILD_INFO.get("backend_port", 39061))


class ApiError(RuntimeError):
    pass


@dataclass
class ApiClient:
    base_url: str = f"http://127.0.0.1:{_BACKEND_PORT}"
    timeout_seconds: float = 60.0

    def get(self, path: str) -> Dict[str, Any]:
        request = urllib.request.Request(f"{self.base_url}{path}", method="GET")
        return self._execute(request)

    def post(self, path: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        encoded = urllib.parse.urlencode({k: v for k, v in (params or {}).items()}).encode("utf-8")
        request = urllib.request.Request(
            f"{self.base_url}{path}",
            data=encoded,
            method="POST",
            headers={"Content-Type": "application/x-www-form-urlencoded"},
        )
        return self._execute(request)

    def _execute(self, request: urllib.request.Request) -> Dict[str, Any]:
        try:
            with urllib.request.urlopen(request, timeout=self.timeout_seconds) as response:
                payload = response.read().decode("utf-8")
        except urllib.error.HTTPError as exc:
            try:
                detail = json.loads(exc.read().decode("utf-8")).get("error", str(exc))
            except (ValueError, AttributeError):
                detail = str(exc)
            raise ApiError(detail) from exc
        except Exception as exc:  # pragma: no cover - runtime error surfacing only
            raise ApiError(str(exc)) from exc

        try:
            decoded = json.loads(payload)
        except json.JSONDecodeError as exc:  # pragma: no cover
            raise ApiError(f"Invalid backend response: {payload}") from exc

        if not isinstance(decoded, dict):
            raise ApiError("Invalid backend response: expected an object")
        if not decoded.get("ok"):
            raise ApiError(decoded.get("error", "Unknown backend error"))
        return decoded.get("data", {})


    def wait_for_health(self, timeout_seconds: float = 20.0, poll_interval: float = 0.25) -> bool:
        deadline = time.monotonic() + timeout_seconds
        while time.monotonic() < deadline:
            try:
                self.get("/api/health")
                return True
            except ApiError:
                time.sleep(poll_interval)
        return False
