import io
import json
import pathlib
import unittest
import urllib.error
from unittest.mock import patch, MagicMock
from fx6_operator.backend_process import BackendProcess, _BUILD_INFO
from fx6_operator.api import ApiClient, ApiError

class BackendProcessTest(unittest.TestCase):
    def test_old_and_missing_build_are_rejected_without_launch(self):
        for payload in [{}, {"buildId": "20260327e"}]:
            with patch("fx6_operator.backend_process._health_payload", return_value=payload), patch("subprocess.Popen") as popen:
                with self.assertRaisesRegex(RuntimeError, "different backend build"):
                    BackendProcess(pathlib.Path("/missing")).start()
                popen.assert_not_called()

    def test_matching_backend_is_reused_not_stopped(self):
        with patch("fx6_operator.backend_process._health_payload", return_value={"buildId": _BUILD_INFO["build_id"], "executablePath": "/test/fx6d"}), patch("subprocess.Popen") as popen:
            backend = BackendProcess(pathlib.Path("/missing"))
            backend.start()
            backend.stop()
            self.assertEqual(backend.selected_source, "reused-running-backend")
            popen.assert_not_called()

    def test_unknown_port_occupant_is_rejected(self):
        with patch("fx6_operator.backend_process._health_payload", return_value=None), patch("socket.create_connection", return_value=MagicMock()), patch("subprocess.Popen") as popen:
            with self.assertRaisesRegex(RuntimeError, "occupied"):
                BackendProcess(pathlib.Path("/missing")).start()
            popen.assert_not_called()

    def test_http_error_keeps_backend_reason(self):
        error = urllib.error.HTTPError("http://localhost", 400, "Bad Request", {}, io.BytesIO(json.dumps({"ok": False, "error": "ND OFF rollback FAILED"}).encode()))
        with patch("urllib.request.urlopen", side_effect=error):
            with self.assertRaisesRegex(ApiError, "ND OFF rollback FAILED"):
                ApiClient().post("/api/nd/on")
