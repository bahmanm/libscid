#ifndef SCID_LIBSCID_HANDLES_H
#define SCID_LIBSCID_HANDLES_H

#include "scid/core/game.h"
#include "scid/core/movetext_cursor.h"
#include "scid/core/pgn/encode.h"
#include "scid/core/position.h"
#include "scid/database/scidbase.h"
#include "scid/eco/book.h"

#include <string>

struct scid_position
{
        scid::core::Position value;
};

struct scid_game
{
        scid::core::Game value;
};

struct scid_game_pgn_options
{
        scid::core::pgn::EncodeOptions value;
};

struct scid_game_cursor
{
        scid_game*                 game = nullptr;
        scid::core::MovetextCursor value;

        explicit scid_game_cursor(scid_game* source_game)
            : game(source_game),
              value(source_game->value)
        {}
};

struct scid_eco_book
{
        scid::eco::Book value;
};

struct scid_database
{
        scid::database::scidBaseT value;
        std::string               type;
};

#endif
