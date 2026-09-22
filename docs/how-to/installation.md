# How to Build and Install libscid

This guide explains how to install prebuilt release archives or compile and install libscid from source.

---

## 1. System Requirements

- Consumer projects: A C11-compliant compiler.
- Source builds: A C++23 compiler and CMake 3.28 or newer.

### Toolchain Setup by Operating System

Ubuntu 24.04:
```sh
sudo apt-get update
sudo apt-get install --yes clang-20 clang-tools-20
```

macOS (Homebrew):
```sh
brew install llvm@20

export LLVM20=/opt/homebrew/opt/llvm@20
export PATH="$LLVM20/bin:$PATH"
export SDKROOT="$(xcrun --show-sdk-path)"
```

Windows (Chocolatey):
```powershell
choco install llvm --version=20.1.4 --yes --no-progress
choco install ninja --yes --no-progress
```

---

## 2. Installing from a Release Archive

Prebuilt archives are available on the GitHub releases page for Linux, macOS (ARM), and Windows:

```sh
version=vX.Y.Z
platform=linux # linux, macos-arm, or windows
asset_name="libscid__${version}__${platform}.tar.gz"

mkdir -p downloads install/libscid
curl -L \
    "https://github.com/bahmanm/libscid/releases/download/${version}/${asset_name}" \
    -o "downloads/${asset_name}"

tar -xzf "downloads/${asset_name}" \
    -C install/libscid \
    --strip-components=1
```

Point `CMAKE_PREFIX_PATH` at the extracted directory when configuring dependent projects:

```sh
cmake -S your-project -B _build/your-project -DCMAKE_PREFIX_PATH="$PWD/install/libscid"
cmake --build _build/your-project
```

### Archive Directory Layout

- `include/`: Public C ABI headers.
- `lib/`: Compiled shared library and CMake package configuration files under `lib/cmake/libscid`.
- `share/doc/libscid/COPYING`: GNU GPL v2 licence text.
- `share/doc/libscid/README.md`: Release README.
- `share/doc/libscid/examples/`: Standalone C ABI example projects and test fixtures.
- `share/doc/libscid/html/`: Generated HTML reference documentation when packaged with docs enabled.

The package publishes the CMake target `LibScid::LibScid`.

---

## 3. Building and Installing from Source

Configure, build, and install libscid with a specified installation prefix:

```sh
cmake -S capi -B _staging/build/capi/release \
    -DCMAKE_BUILD_TYPE=Release \
    -DLIBSCID_INSTALL=ON \
    -DLIBSCID_SOURCE_ROOT="$PWD" \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DCMAKE_INSTALL_PREFIX="$PWD/install/libscid"

cmake --build _staging/build/capi/release
cmake --install _staging/build/capi/release
```

To include generated API documentation in the package:

```sh
cmake -S capi -B _staging/build/capi/package \
    -DCMAKE_BUILD_TYPE=Release \
    -DLIBSCID_INSTALL=ON \
    -DLIBSCID_BUILD_DOCS=ON \
    -DLIBSCID_SOURCE_ROOT="$PWD" \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DCMAKE_INSTALL_PREFIX="$PWD/install/libscid"

cmake --build _staging/build/capi/package
cmake --install _staging/build/capi/package
```

Using repository presets:

```sh
cd capi

cmake --preset release
cmake --build --preset release

cmake --preset package
cmake --build --preset package
cpack --preset portable-tgz
```

### Installing via Make

You can also use the top-level Makefile to build and install components into a custom prefix:

```sh
make install PREFIX=/path/to/install
```

Or for individual components:

```sh
make libscid.capi.install PREFIX=/path/to/install
make libscid.python.install PREFIX=/path/to/install
```

When staging package installations into a temporary root (e.g. for package managers), specify `DESTDIR`:

```sh
make install DESTDIR=/tmp/stage PREFIX=/usr/local
```

---

## 4. Building and Running the Test Suite

Enable tests explicitly during configuration:

```sh
cmake -S capi -B _staging/build/capi/debug \
    -DBUILD_TESTING=ON \
    -DLIBSCID_INSTALL=OFF \
    -DLIBSCID_SOURCE_ROOT="$PWD" \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20
cmake --build _staging/build/capi/debug
ctest --test-dir _staging/build/capi/debug --output-on-failure
```

Run focused subsystem test suites:

```sh
ctest --test-dir _staging/build/capi/debug -L core --output-on-failure
ctest --test-dir _staging/build/capi/debug -L database --output-on-failure
ctest --test-dir _staging/build/capi/debug -L eco --output-on-failure
ctest --test-dir _staging/build/capi/debug -L capi --output-on-failure
```

Or run all test suites through Make:

```sh
make libscid.capi.test
make libscid.python.test
make libscid.test-examples
```

---

## 5. Building Documentation Locally

Build the MkDocs documentation site locally using Doxygen and uv:

```sh
cd capi
cmake --preset docs
cmake --build --preset docs
```

The compiled C API static site will be generated in `_staging/build/capi/docs/site`.

To build the unified documentation site combining both the C ABI and Python API references:

```sh
make libscid.docs
```

The unified static site will be generated in `_staging/build/docs/public`.
