# -*- mode: python ; coding: utf-8 -*-

from pathlib import Path
import json

project_dir = Path.cwd()
build_info = json.loads((project_dir / "../build_info.json").read_text(encoding="utf-8"))
bundle_name = build_info["bundle_name"]
version = build_info["version"]
build_id = build_info["build_id"]


a = Analysis(
    ["src/main.py"],
    pathex=[str(project_dir / "src")],
    binaries=[],
    datas=[
        (str(project_dir / "../build_info.json"), "."),
    ],
    hiddenimports=[
        "fx6_operator.main_window",
        "fx6_operator.api",
        "fx6_operator.backend_process",
        "fx6_operator.build_info",
        "fx6_operator.keyboard_controls",
        "fx6_operator.usage_terms",
    ],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=["PySide6.QtQml", "PySide6.QtQuick", "PySide6.QtVirtualKeyboard", "PySide6.QtPdf"],
    noarchive=False,
    optimize=0,
)
# Widget-only UI: keep QtBase and omit plugins pulling in unrelated modules.
qt_frameworks = {"QtCore", "QtGui", "QtWidgets", "QtDBus", "QtNetwork"}
qt_plugins = {"platforms/libqcocoa.dylib", "platforms/libqoffscreen.dylib",
              "platforms/libqminimal.dylib", "styles/libqmacstyle.dylib"}

def used_qt_file(entry):
    # Analysis also adds root-level aliases to framework binaries.
    names = [entry[0], entry[1]] if entry[2] == "SYMLINK" else [entry[0]]
    for name in names:
        if name.startswith("PySide6/Qt/lib/"):
            if name.split("/")[3].removesuffix(".framework") not in qt_frameworks:
                return False
        if name.startswith("PySide6/Qt/plugins/"):
            if name.removeprefix("PySide6/Qt/plugins/") not in qt_plugins:
                return False
        if name.startswith("PySide6/Qt/qml/"):
            return False
    return True

a.binaries = [entry for entry in a.binaries if used_qt_file(entry)]
a.datas = [entry for entry in a.datas if used_qt_file(entry)]
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    name=bundle_name,
    debug=False,
    bootloader_ignore_signals=False,
    exclude_binaries=True,
    strip=False,
    upx=False,
    console=False,
)

coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=False,
    name=bundle_name,
)

app = BUNDLE(
    coll,
    name=f"{bundle_name}.app",
    icon=None,
    bundle_identifier="com.ytv.fx6operationapp",
    info_plist={
        "CFBundleDisplayName": build_info["app_name"],
        "CFBundleName": bundle_name,
        "CFBundleShortVersionString": version,
        "CFBundleVersion": build_id,
        "LSMinimumSystemVersion": "15.0",
    },
)
