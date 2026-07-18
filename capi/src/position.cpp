#include "scid/position.h"

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
scid_position_create_from_fen(
    const char*     fen,
    scid_position** out_position)
{
    if (fen == nullptr || out_position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto*            position = new scid_position;
        const scid_error error = position->value.ReadFromFEN(fen);
        if (error != SCID_OK)
        {
            delete position;
            *out_position = nullptr;
            return error;
        }

        *out_position = position;
        return SCID_OK;
    }
    catch (...)
    {
        *out_position = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_position_create_with_san(
    const scid_position* position,
    const char*          san,
    scid_position**      out_position)
{
    if (position == nullptr || san == nullptr || out_position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* next_position = new scid_position;
        next_position->value = position->value;

        scid::core::MoveSpec move;
        if (const scid_error error = next_position->value.parseMoveSpec(move, san);
            error != SCID_OK)
        {
            delete next_position;
            *out_position = nullptr;
            return error;
        }

        if (const scid_error error = next_position->value.applyMove(move); error != SCID_OK)
        {
            delete next_position;
            *out_position = nullptr;
            return error;
        }

        *out_position = next_position;
        return SCID_OK;
    }
    catch (...)
    {
        *out_position = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_position_create_with_uci(
    const scid_position* position,
    const char*          uci,
    scid_position**      out_position)
{
    if (position == nullptr || uci == nullptr || out_position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* next_position = new scid_position;
        next_position->value = position->value;

        scid::core::MoveSpec move;
        if (const scid_error error = next_position->value.readCoordinateMoveSpec(move, uci, false);
            error != SCID_OK)
        {
            delete next_position;
            *out_position = nullptr;
            return error;
        }

        if (const scid_error error = next_position->value.applyMove(move); error != SCID_OK)
        {
            delete next_position;
            *out_position = nullptr;
            return error;
        }

        *out_position = next_position;
        return SCID_OK;
    }
    catch (...)
    {
        *out_position = nullptr;
        return SCID_ERROR;
    }
}


void
scid_position_free(scid_position* position)
{
    delete position;
}


scid_error
scid_position_to_fen(
    const scid_position* position,
    char*                out_fen,
    size_t               out_fen_capacity,
    size_t*              out_fen_size)
{
    if (position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        char fen[256];
        position->value.PrintFEN(fen, sizeof(fen));
        return write_text(fen, out_fen, out_fen_capacity, out_fen_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_position_apply_san(
    scid_position* position,
    const char*    san)
{
    if (position == nullptr || san == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::core::MoveSpec move;
        if (const scid_error error = position->value.parseMoveSpec(move, san); error != SCID_OK)
        {
            return error;
        }

        return position->value.applyMove(move);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_position_apply_uci(
    scid_position* position,
    const char*    uci)
{
    if (position == nullptr || uci == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::core::MoveSpec move;
        if (const scid_error error = position->value.readCoordinateMoveSpec(move, uci, false);
            error != SCID_OK)
        {
            return error;
        }

        return position->value.applyMove(move);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_position_is_start(
    const scid_position* position,
    int*                 out_is_start)
{
    if (position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(position->value.IsStdStart(), out_is_start);
}


scid_error
scid_position_is_check(
    const scid_position* position,
    int*                 out_is_check)
{
    if (position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_bool(
            const_cast<scid::core::Position&>(position->value).IsKingInCheck(), out_is_check);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_position_is_checkmate(
    const scid_position* position,
    int*                 out_is_checkmate)
{
    if (position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_bool(
            const_cast<scid::core::Position&>(position->value).IsKingInMate(), out_is_checkmate);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_position_is_legal(
    const scid_position* position,
    int*                 out_is_legal)
{
    if (position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_bool(
            const_cast<scid::core::Position&>(position->value).IsLegal(), out_is_legal);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_position_side_to_move_get(
    const scid_position* position,
    scid_colour*         out_side_to_move)
{
    if (position == nullptr || out_side_to_move == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_side_to_move = position->value.GetToMove() == scid::core::WHITE ? SCID_WHITE : SCID_BLACK;
    return SCID_OK;
}


scid_error
scid_position_fullmove_number_get(
    const scid_position* position,
    unsigned*            out_fullmove_number)
{
    if (position == nullptr || out_fullmove_number == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_fullmove_number = position->value.GetFullMoveCount();
    return SCID_OK;
}


scid_error
scid_position_halfmove_clock_get(
    const scid_position* position,
    unsigned*            out_halfmove_clock)
{
    if (position == nullptr || out_halfmove_clock == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_halfmove_clock = position->value.GetHalfMoveClock();
    return SCID_OK;
}


scid_error
scid_position_piece_at_get(
    const scid_position* position,
    scid_square          square,
    scid_piece*          out_piece)
{
    if (position == nullptr || out_piece == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (!square_is_valid(square))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_piece = piece_to_c(position->value.GetPiece(square));
    return SCID_OK;
}
