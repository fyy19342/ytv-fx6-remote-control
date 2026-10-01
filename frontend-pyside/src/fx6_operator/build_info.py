from __future__ import annotations

import json
import pathlib
import sys
from typing import Any, Dict


DEFAULT_BUILD_INFO: Dict[str, Any] = {
    "app_name": "FX6 Operation App",
    "repo_name": "ytv-fx6-remote-control",
    "bundle_name": "FX6OperationApp",
    "build_id": "unknown",
    "version": "0.0.0",
    "backend_port": 39061,
    "release_zip_prefix": "ytv-fx6-remote-control-full-replacement",
}


def _candidate_paths() -> list[pathlib.Path]:
    candidates: list[pathlib.Path] = []

    module_path = pathlib.Path(__file__).resolve()
    for parent in [module_path.parent, *module_path.parents]:
        candidates.append(parent / "build_info.json")

    if getattr(sys, "frozen", False):
        executable = pathlib.Path(sys.executable).resolve()
        candidates.extend(
            [
                executable.parent / "build_info.json",
                executable.parent.parent / "Resources" / "build_info.json",
            ]
        )
        meipass = getattr(sys, "_MEIPASS", None)
        if meipass:
            candidates.append(pathlib.Path(meipass) / "build_info.json")

    ordered: list[pathlib.Path] = []
    seen: set[str] = set()
    for candidate in candidates:
        key = str(candidate)
        if key not in seen:
            seen.add(key)
            ordered.append(candidate)
    return ordered


def load_build_info() -> Dict[str, Any]:
    for candidate in _candidate_paths():
        if not candidate.exists():
            continue
        try:
            payload = json.loads(candidate.read_text(encoding="utf-8"))
        except Exception:
            continue
        return {**DEFAULT_BUILD_INFO, **payload}
    return dict(DEFAULT_BUILD_INFO)
