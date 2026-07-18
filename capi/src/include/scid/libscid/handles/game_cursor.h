#ifndef SCID_LIBSCID_HANDLES_GAME_CURSOR_H
#define SCID_LIBSCID_HANDLES_GAME_CURSOR_H

#include "scid/libscid/handles/game.h"

#include "scid/core/movetext_cursor.h"

struct scid_game_cursor
{
        scid_game*                 game = nullptr;
        scid::core::MovetextCursor value;

        explicit scid_game_cursor(scid_game* source_game)
            : game(source_game),
              value(source_game->value)
        {}
};

#endif
