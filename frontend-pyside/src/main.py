from __future__ import annotations

import pathlib
import sys


def _bootstrap_import_path() -> None:
    base = pathlib.Path(__file__).resolve().parent
    if str(base) not in sys.path:
        sys.path.insert(0, str(base))

    if getattr(sys, "frozen", False):
        meipass = getattr(sys, "_MEIPASS", None)
        if meipass:
            mp = pathlib.Path(meipass)
            for candidate in [mp, mp / "fx6_operator"]:
                if candidate.exists() and str(candidate) not in sys.path:
                    sys.path.insert(0, str(candidate))


_bootstrap_import_path()

from PySide6.QtWidgets import QApplication
from fx6_operator.main_window import MainWindow


def repo_root() -> pathlib.Path:
    return pathlib.Path(__file__).resolve().parents[2]


def main() -> int:
    app = QApplication(sys.argv)
    window = MainWindow(repo_root())
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
