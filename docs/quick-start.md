# Quick Start {#quick_start}

This guide shows three small ways to consume an installed libscid release:
direct `cc`, a simple Makefile, and CMake.

The examples use a local installation directory named `install/libscid`.
See @ref installation "Installation" if libscid is not installed yet.

## Example Program

Create `main.c`:

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

The public header is `scid/scid.h`, and the installed CMake target is
`LibScid::LibScid`.

## PGN Workflow

The PGN-facing game API is built from a few foundational calls:

- Create a start position with `scid_position_create_from_fen()`.
- Create a blank game with `scid_game_create_blank()` or parse moves and tags
  with `scid_game_create()`.
- Create a `scid_game_cursor` from the game. Cursor navigation is immutable:
  functions such as `scid_game_cursor_next()` return a new cursor through an
  `out_...` parameter.
- Edit the game at a cursor, or merge another game's moves with
  `scid_game_merge_moves()`.
- Export with `scid_game_to_pgn(game, options, ...)`. Pass `NULL` for the
  default complete PGN export, or pass a `scid_game_pgn_options` object to
  control symbolic NAGs, supplemental tags, comments, variations and line
  width.

See @ref examples_recipes "Examples and Recipes" for complete programs that
parse, author, navigate, mutate and export PGN.

## With cc

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

For a non-standard install prefix, replace `install/libscid` with the directory
where the release archive was extracted or where `cmake --install` installed
the library.

## With Make

Create `Makefile`:

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

Configure, build and run:

```sh
cmake -S . -B _build -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build
./_build/libscid-hello
```
