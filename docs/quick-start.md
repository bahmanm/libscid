# Quick Start {#quick_start}

This guide shows three small ways to consume an installed libscid release:
direct `g++`, a simple Makefile, and CMake.

The examples use a local installation directory named `install/libscid`.
See @ref installation "Installation" if libscid is not installed yet.

## Example Program

Create `main.cpp`:

```cpp
#include <scid/core/position.h>

#include <iostream>

int
main()
{
    const auto& position = scid::core::Position::getStdStart();

    if (!position.IsStdStart())
    {
        return 1;
    }

    std::cout << "libscid is available\n";
    return 0;
}
```

This program uses only Core, so it links only `LibScidCpp::Core` in CMake or
`libscid_core` when compiling directly.

## With g++

```sh
g++ -std=c++20 \
    -I install/libscid/include \
    main.cpp \
    -L install/libscid/lib \
    -lscid_core \
    -o libscid-hello

./libscid-hello
```

For a non-standard install prefix, replace `install/libscid` with the directory
where the release archive was extracted or where `cmake --install` installed
the library.

When linking additional libraries directly, keep dependent libraries before
their dependencies:

```sh
# Database also needs Core and threads.
g++ -std=c++20 -I install/libscid/include main.cpp \
    -L install/libscid/lib \
    -lscid_database -lscid_core -pthread \
    -o app

# ECO needs Core.
g++ -std=c++20 -I install/libscid/include main.cpp \
    -L install/libscid/lib \
    -lscid_eco -lscid_core \
    -o app

# Spelling needs Database, Core and threads.
g++ -std=c++20 -I install/libscid/include main.cpp \
    -L install/libscid/lib \
    -lscid_spelling -lscid_database -lscid_core -pthread \
    -o app
```

Prefer CMake for larger consumers: the exported targets carry include
directories, transitive libscid dependencies and thread linkage.

## With Make

Create `Makefile`:

```make
CXX ?= g++
LIBSCID_PREFIX ?= install/libscid

CXXFLAGS += -std=c++20 -I$(LIBSCID_PREFIX)/include
LDFLAGS += -L$(LIBSCID_PREFIX)/lib
LDLIBS += -lscid_core

libscid-hello: main.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f libscid-hello
```

Build and run:

```sh
make
./libscid-hello
```

Use `make LIBSCID_PREFIX=/path/to/libscid` if libscid is installed somewhere
else.

## With CMake

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required( VERSION 3.23 )

project( libscid-hello LANGUAGES CXX )

find_package( libscid-cpp CONFIG REQUIRED )

add_executable( libscid-hello main.cpp )
target_compile_features( libscid-hello PRIVATE cxx_std_20 )
set_target_properties( libscid-hello PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( libscid-hello PRIVATE LibScidCpp::Core )
```

Configure, build and run:

```sh
cmake -S . -B _build -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build
./_build/libscid-hello
```

Link the target that matches the API surface you use:

```cmake
target_link_libraries( app PRIVATE LibScidCpp::Core )
target_link_libraries( app PRIVATE LibScidCpp::Database )
target_link_libraries( app PRIVATE LibScidCpp::Eco )
target_link_libraries( app PRIVATE LibScidCpp::Spelling )
```

`LibScidCpp::Database`, `LibScidCpp::Eco` and `LibScidCpp::Spelling` bring their libscid
dependencies transitively.
