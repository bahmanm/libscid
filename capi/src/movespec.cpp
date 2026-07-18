#include "scid/movespec.h"

#include "scid/libscid/handles.h"
#include "scid/libscid/support.h"

#include "scid/core/date.h"
#include "scid/core/game.h"
#include "scid/core/game_cursor.h"
#include "scid/core/game_result.h"
#include "scid/core/movetext_cursor.h"
#include "scid/core/nags.h"
#include "scid/core/notation.h"
#include "scid/core/pgn/decode.h"
#include "scid/core/pgn/encode.h"
#include "scid/core/position.h"
#include "scid/core/primitives.h"
#include "scid/database/scidbase.h"
#include "scid/eco/book.h"
#include "scid/eco/code.h"

#include <array>
#include <cctype>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace scid::libscid;

scid_error
scid_movespec_create(
    scid_square    from,
    scid_square    to,
    scid_piece     promotion,
    int            is_castling,
    scid_movespec* out_move)
{
    if (out_move == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_move = {from, to, promotion, is_castling != 0};

    return SCID_OK;
}


scid_error
scid_movespec_create_from_uci(
    const char*    text,
    scid_movespec* out_move)
{
    if (text == nullptr || out_move == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (std::strcmp(text, "0000") == 0)
    {
        *out_move = {0, 0, SCID_PIECE_NONE, 0};
        return SCID_OK;
    }

    const bool has_promotion = text[0] != '\0' && text[1] != '\0' && text[2] != '\0' &&
                               text[3] != '\0' && text[4] != '\0' && text[5] == '\0';

    const bool is_quiet =
        text[0] != '\0' && text[1] != '\0' && text[2] != '\0' && text[3] != '\0' && text[4] == '\0';

    if (!is_quiet && !has_promotion)
    {
        return SCID_ERROR_BAD_ARG;
    }

    scid_square from = 0;
    scid_square to = 0;
    scid_piece  promotion = SCID_PIECE_NONE;

    if (parse_square_chars(text[0], text[1], &from) != SCID_OK ||
        parse_square_chars(text[2], text[3], &to) != SCID_OK)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (has_promotion && promotion_from_char(text[4], &promotion) != SCID_OK)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_move = {from, to, promotion, 0};
    return SCID_OK;
}


scid_error
scid_movespec_to_uci(
    scid_movespec move,
    char*         out_text,
    size_t        out_text_capacity,
    size_t*       out_text_size)
{
    if (!promotion_is_valid(move.promotion))
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (move.from == move.to && move.is_castling == 0)
    {
        return write_text("0000", out_text, out_text_capacity, out_text_size);
    }

    if (!square_is_valid(move.from) || !square_is_valid(move.to))
    {
        return SCID_ERROR_BAD_ARG;
    }

    char text[6] = {
        static_cast<char>('a' + scid::core::square_Fyle(move.from)),
        static_cast<char>('1' + scid::core::square_Rank(move.from)),
        static_cast<char>('a' + scid::core::square_Fyle(move.to)),
        static_cast<char>('1' + scid::core::square_Rank(move.to)),
        '\0',
        '\0'};

    if (move.promotion != SCID_PIECE_NONE)
    {
        text[4] = promotion_to_char(move.promotion);
    }

    return write_text(text, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_movespec_create_from_san(
    const scid_position* position,
    const char*          text,
    scid_movespec*       out_move)
{
    if (position == nullptr || text == nullptr || out_move == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::core::MoveSpec move;
        const scid_error     error =
            const_cast<scid::core::Position&>(position->value).parseMoveSpec(move, text);
        if (error != SCID_OK)
        {
            return error;
        }

        *out_move = movespec_from_core(move);
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_movespec_to_san(
    const scid_position* position,
    scid_movespec        move,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::core::MoveSpec core_move;
        if (const scid_error error = movespec_to_core(move, &core_move); error != SCID_OK)
        {
            return error;
        }

        const std::string text = const_cast<scid::core::Position&>(position->value)
                                     .makeSan(core_move, scid::core::SAN_MATETEST);
        if (text.empty())
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(text, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}
