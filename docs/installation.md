# Installation {#installation}

libscid can be used from a release archive or installed from a source checkout.
The release archives are the easiest way to consume the library in another
project; source installation is useful when developing libscid itself or when a
prebuilt archive is not available for the target platform.

## Requirements

- A C++20 compiler.
- CMake 3.23 or newer for source builds.
- CMake 3.25 or newer when using the repository's CMake presets.

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

- `include/`: installed public headers.
- `lib/`: compiled libscid libraries and CMake package files under `lib/cmake/LibScid`.
- `share/doc/libscid/COPYING`: the GNU GPL v2 licence text.
- `share/doc/libscid/README.md`: the release README.
- `share/doc/libscid/examples/`: standalone example projects and fixtures.
- `share/doc/libscid/html/`: generated API documentation when built into the package archive.

The package exports these CMake targets:

- `LibScid::Core`
- `LibScid::Database`
- `LibScid::Eco`
- `LibScid::Spelling`

## Build And Install From Source

Configure, build and install with an explicit prefix:

```sh
cmake -S . -B _build/release \
    -DCMAKE_BUILD_TYPE=Release \
    -DLIBSCID_INSTALL=ON \
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
cmake -S . -B _build -DBUILD_TESTING=ON
cmake --build _build
ctest --test-dir _build --output-on-failure
```

Focused module test runs are also available after configuring with tests:

```sh
ctest --test-dir _build -L core --output-on-failure
ctest --test-dir _build -L database --output-on-failure
ctest --test-dir _build -L eco --output-on-failure
```

## Build Documentation For GitHub Pages Locally

The documentation site is generated with Doxygen. If Doxygen, Graphviz and
PlantUML are installed:

```sh
cmake --preset docs
cmake --build --preset docs
```

The generated HTML is written under `_build/docs/doxygen/html`.
