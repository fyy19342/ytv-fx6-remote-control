# Third-party software

FX6 Operation App uses the components below. Their licenses and copyright notices are in `licenses/` and in the app's `Contents/Resources/licenses/`. These licenses apply to their respective components; they do not transfer ownership of the Sony SDK or the project code.

| Component | Version / license | Source |
| --- | --- | --- |
| Sony Camera Remote SDK | 2.01.00 / Sony proprietary terms | [Official SDK terms](https://support.d-imaging.sony.co.jp/app/sdk/licenseagreement/en-US.html) |
| Qt Core, GUI, Widgets, Network, D-Bus | 6.11.0 / LGPLv3, with separately licensed third-party code | [QtBase v6.11.0](https://github.com/qt/qtbase/tree/v6.11.0) |
| PySide6 / Shiboken6 | 6.11.0 / LGPLv3 | [Qt for Python v6.11.0](https://github.com/pyside/pyside-setup/tree/v6.11.0) |
| Python | 3.11.9 / PSF and included notices | [CPython v3.11.9](https://github.com/python/cpython/tree/v3.11.9) |
| PyInstaller bootloader | 6.19.0 / GPL with bootloader exception | [PyInstaller v6.19.0](https://github.com/pyinstaller/pyinstaller/tree/v6.19.0) |
| cpp-httplib | 0.19.0 / MIT | [cpp-httplib v0.19.0](https://github.com/yhirose/cpp-httplib/tree/v0.19.0) |
| libusb (SDK dependency) | 1.0.26 / LGPLv2.1 | [libusb v1.0.26](https://github.com/libusb/libusb/tree/v1.0.26) |
| libssh2 (SDK dependency) | 1.11.0 / BSD | [libssh2 1.11.0](https://github.com/libssh2/libssh2/tree/libssh2-1.11.0) |
| gettext libintl (Python dependency) | 1.0 / LGPLv2.1 | [GNU gettext 1.0](https://ftp.gnu.org/gnu/gettext/gettext-1.0.tar.gz) |
| OpenSSL (Python dependency) | 3.6.1 / Apache 2.0 | [OpenSSL 3.6.1](https://github.com/openssl/openssl/tree/openssl-3.6.1) |
| XZ / liblzma (Python dependency) | 5.8.2 / 0BSD for library | [XZ 5.8.2](https://github.com/tukaani-project/xz/tree/v5.8.2) |

Qt is dynamically linked. Qt Virtual Keyboard, Qt Quick/QML, Qt PDF and their optional plugins are not included. QtBase and PySide's upstream license directories and third-party copyright/attribution files are preserved under `licenses/QtBase-6.11.0/` and `licenses/PySide-6.11.0/`. System libraries provided by macOS are not redistributed.

## Source access and replacing LGPL libraries

The same [GitHub Release](https://github.com/fyy19342/ytv-fx6-remote-control/releases/tag/v0.4.2-20261001c) as the app provides `FX6OperationApp-20261001c-open-source-dependencies.tar`, at no charge. It contains the upstream source archives for QtBase, PySide/Shiboken, gettext and libusb, their build files and a SHA256 source manifest. The app package contains this project's source and build scripts. The project has not modified the LGPL library source. Source URL/checksum pins are in `scripts/open_source_dependencies.json`.

Sony supplies the SDK's libusb binary. Its reported version is 1.0.26; the archive includes that upstream source. Sony's source offer and any SDK-specific source/changes remain available via the [Sony open-source portal](https://oss.sony.net/Products/Linux/) as described in the SDK agreement.

You may modify/relink the LGPL components for your own use and reverse engineer the combined work for debugging those modifications. No application term limits rights granted by the applicable open-source licenses. This permission does not authorize reverse engineering Sony's proprietary components.

Make a copy of the app before replacing libraries. Compatible Qt frameworks live under `Contents/Frameworks/PySide6/Qt/lib/`, plugins under `Contents/Frameworks/PySide6/Qt/plugins/`, PySide/Shiboken under `Contents/Frameworks/PySide6/` and `Contents/Frameworks/shiboken6/`. `libintl.8.dylib` is in `Contents/Frameworks/`. The SDK's LGPL libusb is in `Contents/Resources/backend/build/Contents/Frameworks/CrAdapter/`. Use compatible arm64 libraries and preserve their install names and symlinks. Re-sign the modified copy with `codesign --force --deep --sign - /path/to/FX6OperationApp.app` so macOS can load it. An Apple developer account is not required for this local ad-hoc signature. Alternatively rebuild using `docs/BUILD.md` and your replacement dependencies. No integrity lock prevents these replacements.

## Sony SDK distribution

The Git repository and source export omit SDK headers, sample sources and standalone SDK libraries. The application embeds only the runtime needed to operate Sony cameras. Do not extract and redistribute the proprietary SDK as a development kit. Build contributors obtain the SDK directly from Sony under its terms. [End-user notice](docs/TERMS.md) and the login confirmation explain the warranty condition, prohibited uses and independent support.
