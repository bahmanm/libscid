# Installation {#installation}

libscid can be used from a release archive or installed from a source checkout.
The release archives are the easiest way to consume the library in another
project; source installation is useful when developing libscid itself or when a
prebuilt archive is not available for the target platform.

## Requirements

- A C11 compiler for public C ABI consumers.
- A C++23 compiler for source builds.
- CMake 3.28 or newer for source builds, CMake consumers and the repository's
  CMake presets.

On Ubuntu 24.04, install Clang 20 and the matching Clang dependency scanner for
source builds:

```sh
sudo apt-get update
sudo apt-get install --yes clang-20 clang-tools-20
```

On local macOS development machines, use the Homebrew LLVM 20 toolchain from
`/opt/homebrew/opt/llvm@20/bin` so the compiler major version stays aligned
with CI:

```sh
brew install llvm@20

export LLVM20=/opt/homebrew/opt/llvm@20
export PATH="$LLVM20/bin:$PATH"
export SDKROOT="$(xcrun --show-sdk-path)"
```

Install cppcheck separately when running local static analysis. The Homebrew
LLVM package includes `clang-tidy` and `run-clang-tidy`.

```sh
sudo apt-get install --yes cppcheck clang-tidy-20 clang-tools-20 # Ubuntu
brew install cppcheck                                           # macOS
```

On Windows, the CI pipeline uses LLVM 20 through Chocolatey and builds with
`clang-cl` so the toolchain keeps the MSVC ABI and runtime model:

```powershell
choco install llvm --version=20.1.4 --yes --no-progress
choco install ninja --yes --no-progress
```

## Install From A Release Archive

Pick the archive for your platform from the GitHub release assets:

- `libscid__<version>__linux.tar.gz`
- `libscid__<version>__macos-arm.tar.gz`
- `libscid__<version>__windows.tar.gz`

For example:

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

The installation is relocatable for normal CMake package discovery. Point
`CMAKE_PREFIX_PATH` at the extracted directory:

```sh
cmake -S your-project -B _build/your-project -DCMAKE_PREFIX_PATH="$PWD/install/libscid"
cmake --build _build/your-project
```

## Release Archive Contents

A release archive contains:

- `include/`: installed public C ABI headers.
- `lib/`: the compiled libscid library and CMake package files under `lib/cmake/libscid`.
- `share/doc/libscid/COPYING`: the GNU GPL v2 licence text.
- `share/doc/libscid/README.md`: the release README.
- `share/doc/libscid/examples/`: standalone C ABI example projects and fixtures.
- `share/doc/libscid/html/`: generated API documentation when built into the package archive.

The package exports one CMake target:

- `LibScid::LibScid`

## Build And Install From Source

Configure, build and install with an explicit prefix:

```sh
cmake -S . -B _build/release \
    -DCMAKE_BUILD_TYPE=Release \
    -DLIBSCID_INSTALL=ON \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DCMAKE_INSTALL_PREFIX="$PWD/install/libscid"

cmake --build _build/release
cmake --install _build/release
```

To include generated API documentation in the install tree, enable docs and
provide the documentation tools required by the build:

```sh
cmake -S . -B _build/package \
    -DCMAKE_BUILD_TYPE=Release \
    -DLIBSCID_INSTALL=ON \
    -DLIBSCID_BUILD_DOCS=ON \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DCMAKE_INSTALL_PREFIX="$PWD/install/libscid"

cmake --build _build/package
cmake --install _build/package
```

The repository also provides presets for local release and package builds:

```sh
cmake --preset release
cmake --build --preset release

cmake --preset package
cmake --build --preset package
cpack --preset portable-tgz
```

## Build With Tests

Tests are disabled by default in top-level builds. Enable them explicitly:

```sh
cmake -S . -B _build \
    -DBUILD_TESTING=ON \
    -DLIBSCID_INSTALL=OFF \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20
cmake --build _build
ctest --test-dir _build --output-on-failure
```

Top-level source builds generate a compilation database at
`<build-dir>/compile_commands.json` for static analysis tools.

Run the relaxed cppcheck pass with the analysis preset:

```sh
cmake --preset analysis \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20
cmake --build --preset cppcheck
```

Run the relaxed clang-tidy pass with the same analysis preset:

```sh
cmake --build --preset clang-tidy
```

On macOS, replace the compiler settings in the source-build commands with the
Homebrew LLVM 20 paths:

```sh
-DCMAKE_C_COMPILER="$LLVM20/bin/clang" \
-DCMAKE_CXX_COMPILER="$LLVM20/bin/clang++" \
-DCMAKE_OSX_SYSROOT="$SDKROOT"
```

On Windows, configure with Ninja and `clang-cl` from the LLVM install:

```powershell
cmake -S . -B _build -G Ninja `
    -DBUILD_TESTING=ON `
    "-DCMAKE_C_COMPILER=C:\Program Files\LLVM\bin\clang-cl.exe" `
    "-DCMAKE_CXX_COMPILER=C:\Program Files\LLVM\bin\clang-cl.exe"
```

Focused module test runs are also available after configuring with tests:

```sh
ctest --test-dir _build -L core --output-on-failure
ctest --test-dir _build -L database --output-on-failure
ctest --test-dir _build -L eco --output-on-failure
ctest --test-dir _build -L libscid --output-on-failure
```

## Build Documentation For GitHub Pages Locally

The documentation site is generated with Doxygen. If Doxygen, Graphviz and
PlantUML are installed:

```sh
cmake --preset docs
cmake --build --preset docs
```

The generated HTML is written under `_build/docs/doxygen/html`.
