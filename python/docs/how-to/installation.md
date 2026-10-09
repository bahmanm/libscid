# Install libscid Python

Install the package in a virtual environment with Python 3.10 or newer:

```sh
python -m pip install libscid
```

## Published Binary Wheels

PyPI has prebuilt wheels for these platforms:

| Wheel target | Supported platform | Runtime requirements |
|---|---|---|
| `manylinux_2_39_x86_64` | Linux x86-64 with glibc 2.39 or newer | The system dynamic loader, libc, libm, `libstdc++.so.6`, and `libgcc_s.so.1`. The library requires `GLIBC_2.38` and `GLIBCXX_3.4.31` symbols; the wheel tag sets the stricter glibc 2.39 installation floor. |
| `macosx_15_0_arm64` | Apple Silicon, macOS 15 or newer | System `libc++` and `libSystem`, provided by macOS. |
| `win_amd64` | Windows x64 | The Microsoft Visual C++ 2015–2022 Redistributable (x64), including `MSVCP140.dll`, `VCRUNTIME140.dll`, and `VCRUNTIME140_1.dll`. The wheel bundles `scid.dll`, not these runtime libraries. |

There are no prebuilt wheels yet for Intel Macs, Linux ARM, or musl-based Linux
distributions such as Alpine.

## Build a Wheel for Another Platform

It's straightforward to build a wheel for another platform. Download the
`libscid__<version>__source.tar.gz` archive from the [GitHub Releases
page](https://github.com/bahmanm/libscid/releases). Build and install the C
library using the [C installation guide](https://libscid.bahmanm.com/capi/how-to/installation/),
then build the Python wheel from the archive's `python/` directory. Point the
build at the C library installation prefix and it will bundle the native
library in the wheel:

```sh
cd python
LIBSCID_LIBRARY_PREFIX=/path/to/libscid python -m pip wheel . --wheel-dir dist
```
