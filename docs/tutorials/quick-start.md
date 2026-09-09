# Quick Start

This tutorial walks through creating and building your first program with the libscid C ABI. You will verify that the library is installed properly, initialise a standard chess board position, and check its start-state using `cc`, Make, or CMake.

The instructions assume libscid has been installed to `install/libscid`. Refer to the [Installation Guide](../how-to/installation.md) if you have not yet installed the library.

---

## 1. Minimal Program

Create a source file named `main.c`:

```c
#include <scid/scid.h>

#include <stdio.h>

int
main(void)
{
    const char* standard_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/"
                               "RNBQKBNR w KQkq - 0 1";
    scid_position* position = NULL;
    int is_start = 0;

    if (scid_position_create_from_fen(standard_fen, &position) != SCID_OK)
    {
        return 1;
    }

    if (scid_position_is_start(position, &is_start) != SCID_OK)
    {
        scid_position_free(position);
        return 1;
    }

    scid_position_free(position);

    if (!is_start)
    {
        return 1;
    }

    puts("libscid is available");
    return 0;
}
```

The primary umbrella header is `scid/scid.h`, and the exported CMake target name is `LibScid::LibScid`.

---

## 2. Compiling with cc

You can compile directly using your system C compiler:

```sh
cc -std=c11 \
    -I install/libscid/include \
    main.c \
    -L install/libscid/lib \
    -Wl,-rpath,install/libscid/lib \
    -lscid \
    -o libscid-hello

./libscid-hello
```

If you installed libscid to a custom path, substitute `install/libscid` with your chosen prefix directory.

---

## 3. Compiling with Make

Create a `Makefile` alongside `main.c`:

```make
CC ?= cc
LIBSCID_PREFIX ?= install/libscid

CFLAGS += -std=c11 -I$(LIBSCID_PREFIX)/include
LDFLAGS += -L$(LIBSCID_PREFIX)/lib -Wl,-rpath,$(LIBSCID_PREFIX)/lib
LDLIBS += -lscid

libscid-hello: main.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f libscid-hello
```

Compile and execute:

```sh
make
./libscid-hello
```

---

## 4. Compiling with CMake

Create a `CMakeLists.txt` file:

```cmake
cmake_minimum_required( VERSION 3.23 )

project( libscid-hello LANGUAGES C )

find_package( libscid CONFIG REQUIRED )

add_executable( libscid-hello main.c )
set_target_properties(
    libscid-hello
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )
target_link_libraries( libscid-hello PRIVATE LibScid::LibScid )
```

Configure, build, and run:

```sh
cmake -S . -B _build -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build
./_build/libscid-hello
```

---

## 5. Next Steps

Now that you have verified your installation and compiled your first program, explore:
- [How-To Guides](../how-to/index.md): Practical task-oriented recipes for parsing PGN, navigating moves, and querying databases.
- [C ABI Architecture](../explanation/architecture.md): Conceptual models and subsystem designs.
- [C ABI Reference](../reference/index_modules.md): Authoritative documentation for every public function and type.
