#include "scid/game.h"
#include "scid/game_cursor.h"
#include "scid/game_pgn.h"

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
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace scid::libscid;

namespace
{

    scid_error
    scid_game_merge_moves_transactional(
        scid_game*                          target_game,
        const scid::core::MovetextLocation& target_location,
        const scid_game*                    source_game,
        scid_game_merge_moves_mode          mode,
        scid_game_cursor**                  out_cursor)
    {
        scid::core::Game staging = target_game->value;
        scid::core::MovetextCursor edit_cursor(staging);
        if (!edit_cursor.restore(target_location))
        {
            return SCID_ERROR;
        }

        switch (mode)
        {
            case SCID_GAME_MERGE_MOVES_APPEND:
                if (edit_cursor.nextMove() != nullptr)
                {
                    return SCID_ERROR_BAD_ARG;
                }
                if (const scid_error error = maybe_set_line_start_comment(
                        edit_cursor, source_game->value.initialComment());
                    error != SCID_OK)
                {
                    return error;
                }
                break;

            case SCID_GAME_MERGE_MOVES_INSERT_VARIATION:
                if (edit_cursor.nextMove() == nullptr)
                {
                    return SCID_ERROR_BAD_ARG;
                }
                if (edit_cursor.addVariation(source_game->value.initialComment()) == nullptr)
                {
                    return SCID_ERROR_BAD_ARG;
                }
                break;

            case SCID_GAME_MERGE_MOVES_REPLACE:
                edit_cursor.truncate();
                if (const scid_error error = maybe_set_line_start_comment(
                        edit_cursor, source_game->value.initialComment());
                    error != SCID_OK)
                {
                    return error;
                }
                break;
        }

        const auto& source_movetext = source_game->value.movetext();
        if (const scid_error error =
                append_move_sequence(edit_cursor, source_movetext.mainline);
            error != SCID_OK)
        {
            return error;
        }

        const auto location = edit_cursor.location();
        auto staged_cursor = std::make_unique<scid_game_cursor>(target_game);

        target_game->value = std::move(staging);

        if (!staged_cursor->value.restore(location))
        {
            return SCID_ERROR;
        }

        *out_cursor = staged_cursor.release();
        return SCID_OK;
    }

} // namespace


scid_error
scid_game_create_blank(
    const scid_position* position,
    scid_game**          out_game)
{
    if (any_null(position, out_game))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_game = nullptr;

    return abi_guard([&]() -> scid_error {
        auto game = std::make_unique<scid_game>();
        if (!position->value.IsStdStart())
        {
            game->value.setStartPosition(position->value);
        }

        *out_game = game.release();
        return SCID_OK;
    });
}


scid_error
scid_game_create(
    const scid_position* position,
    const char*          pgn,
    size_t               pgn_size,
    scid_game**          out_game,
    char*                out_diagnostic,
    size_t               out_diagnostic_capacity,
    size_t*              out_diagnostic_size)
{
    if (any_null(position, pgn, out_game))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_game = nullptr;

    return abi_guard([&]() -> scid_error {
        auto game = std::make_unique<scid_game>();
        if (!position->value.IsStdStart())
        {
            game->value.setStartPosition(position->value);
        }

        scid::core::pgn::ParseLog log;
        const bool                ok = scid::core::pgn::parseGame(pgn, pgn_size, game->value, log);

        const scid_error diagnostic_error = write_optional_diagnostic(
            log.log, out_diagnostic, out_diagnostic_capacity, out_diagnostic_size);
        if (diagnostic_error != SCID_OK)
        {
            return diagnostic_error;
        }

        if (!ok)
        {
            return SCID_ERROR_CORRUPT;
        }

        *out_game = game.release();
        return SCID_OK;
    });
}


void
scid_game_free(scid_game* game)
{
    abi_guard_void([&] { delete game; });
}


scid_error
scid_game_pgn_options_create(scid_game_pgn_options** out_options)
{
    if (any_null(out_options))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_options = nullptr;

    return abi_guard([&]() -> scid_error {
        auto options = std::make_unique<scid_game_pgn_options>();
        *out_options = options.release();
        return SCID_OK;
    });
}


void
scid_game_pgn_options_free(scid_game_pgn_options* options)
{
    abi_guard_void([&] { delete options; });
}


scid_error
scid_game_pgn_options_symbolic_nags_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (any_null(options))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        options->value.symbolicNags = enabled != 0;
        return SCID_OK;
    });
}


scid_error
scid_game_pgn_options_supplemental_tags_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (any_null(options))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        options->value.includeSupplementalTags = enabled != 0;
        return SCID_OK;
    });
}


scid_error
scid_game_pgn_options_comments_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (any_null(options))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        options->value.includeComments = enabled != 0;
        return SCID_OK;
    });
}


scid_error
scid_game_pgn_options_variations_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (any_null(options))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        options->value.includeVariations = enabled != 0;
        return SCID_OK;
    });
}


scid_error
scid_game_pgn_options_line_width_set(
    scid_game_pgn_options* options,
    unsigned               line_width)
{
    if (any_null(options))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        if (line_width == 0)
        {
            options->value.lineWidth = std::nullopt;
        }
        else
        {
            options->value.lineWidth = line_width;
        }
        return SCID_OK;
    });
}


scid_error
scid_game_to_pgn(
    const scid_game*             game,
    const scid_game_pgn_options* options,
    char*                        out_text,
    size_t                       out_text_capacity,
    size_t*                      out_text_size)
{
    if (any_null(game, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        std::string pgn;
        const auto  encode_options =
            options == nullptr ? scid::core::pgn::EncodeOptions{} : options->value;
        scid::core::pgn::encode(game->value, pgn, encode_options);
        return write_text(pgn, out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_mainline_halfmove_count_get(
    const scid_game* game,
    size_t*          out_count)
{
    if (any_null(game, out_count))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard(
        [&]() -> scid_error { return write_size(game->value.mainlineHalfMoveCount(), out_count); });
}


scid_error
scid_game_initial_comment_get(
    const scid_game* game,
    char*            out_text,
    size_t           out_text_capacity,
    size_t*          out_text_size)
{
    if (any_null(game, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        return write_text(game->value.initialComment(), out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_tag_get(
    const scid_game* game,
    const char*      name,
    char*            out_text,
    size_t           out_text_capacity,
    size_t*          out_text_size)
{
    if (any_null(game, name, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        return write_text(
            game_tag_value(game->value, name), out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_tag_set(
    scid_game*  game,
    const char* name,
    const char* value)
{
    if (any_null(game, name, value))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error { return game_set_tag(game->value, name, value); });
}


scid_error
scid_game_tag_count_get(
    const scid_game* game,
    size_t*          out_count)
{
    if (any_null(game, out_count))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard(
        [&]() -> scid_error { return write_size(game_tag_count(game->value), out_count); });
}


scid_error
scid_game_tag_at_get(
    const scid_game* game,
    size_t           index,
    char*            out_name,
    size_t           out_name_capacity,
    size_t*          out_name_size,
    char*            out_value,
    size_t           out_value_capacity,
    size_t*          out_value_size)
{
    if (any_null(game, out_name_size, out_value_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        std::string_view name;
        std::string      value;
        if (!game_tag_at(game->value, index, &name, &value))
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (const scid_error error = write_text(name, out_name, out_name_capacity, out_name_size);
            error != SCID_OK)
        {
            return error;
        }

        return write_text(value, out_value, out_value_capacity, out_value_size);
    });
}


scid_error
scid_game_tag_remove(
    scid_game*  game,
    const char* name,
    int*        out_removed)
{
    if (any_null(game, name, out_removed))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        const std::string_view tag_name(name);
        if (tag_name == "ECO")
        {
            const bool found = !game->value.eco().empty();
            game->value.setEco({});
            *out_removed = found ? 1 : 0;
            return SCID_OK;
        }

        if (tag_name == "EventDate")
        {
            const bool found = game->value.eventDate() != scid::core::ZERO_DATE;
            game->value.setEventDate(scid::core::ZERO_DATE);
            *out_removed = found ? 1 : 0;
            return SCID_OK;
        }

        if (tag_name == "FEN")
        {
            *out_removed = 0;
            return SCID_OK;
        }

        const bool found = game->value.findExtraTag(name) != nullptr;
        if (found)
        {
            game->value.removeExtraTag(name);
        }

        *out_removed = found ? 1 : 0;
        return SCID_OK;
    });
}


scid_error
scid_game_start_position_get(
    const scid_game* game,
    scid_position*   out_position)
{
    if (any_null(game, out_position))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        if (const scid::core::Position* position = game->value.startPosition())
        {
            return write_position(*position, out_position);
        }

        return write_position(scid::core::Position::getStdStart(), out_position);
    });
}


scid_error
scid_game_final_position_get(
    const scid_game* game,
    scid_position*   out_position)
{
    if (any_null(game, out_position))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        scid::core::GameCursor cursor(game->value);
        cursor.toEnd();
        const auto position = cursor.currentPosition();
        if (!position)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_position(*position, out_position);
    });
}




scid_error
scid_game_merge_moves(
    scid_game*                 target_game,
    const scid_game_cursor*    target_cursor,
    const scid_game*           source_game,
    scid_game_merge_moves_mode mode,
    scid_game_cursor**         out_cursor)
{
    if (any_null(out_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }
    *out_cursor = nullptr;

    if (any_null(source_game))
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (mode != SCID_GAME_MERGE_MOVES_APPEND && mode != SCID_GAME_MERGE_MOVES_INSERT_VARIATION &&
        mode != SCID_GAME_MERGE_MOVES_REPLACE)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(target_game, target_cursor);
            error != SCID_OK)
        {
            return error;
        }

        scid::core::GameCursor read_cursor(target_game->value);
        if (!read_cursor.restore(target_cursor->value.location()))
        {
            return SCID_ERROR;
        }

        const auto target_position = read_cursor.currentPosition();
        if (!target_position)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        const auto source_start_position = game_start_position(source_game->value);
        if (!positions_match(*target_position, source_start_position))
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        const auto& source_movetext = source_game->value.movetext();
        if (const scid_error error =
                validate_move_sequence(source_movetext.mainline, *target_position);
            error != SCID_OK)
        {
            return error;
        }

        return scid_game_merge_moves_transactional(
            target_game, target_cursor->value.location(), source_game, mode, out_cursor);
    });
}


scid_error
scid_game_cursor_create(
    scid_game*         game,
    scid_game_cursor** out_cursor)
{
    if (any_null(game, out_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto cursor = std::make_unique<scid_game_cursor>(game);
        *out_cursor = cursor.release();
        return SCID_OK;
    });
}


scid_error
scid_game_cursor_clone(
    scid_game*              game,
    const scid_game_cursor* source_cursor,
    scid_game_cursor**      out_cursor)
{
    if (any_null(out_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, source_cursor); error != SCID_OK)
        {
            return error;
        }

        return create_cursor_copy(source_cursor, out_cursor);
    });
}


void
scid_game_cursor_free(scid_game_cursor* cursor)
{
    abi_guard_void([&] { delete cursor; });
}


scid_error
scid_game_cursor_position_get(
    const scid_game_cursor* cursor,
    scid_position*          out_position)
{
    if (any_null(cursor, out_position))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        scid::core::GameCursor read_cursor(cursor->game->value);
        if (!read_cursor.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        const auto position = read_cursor.currentPosition();
        if (!position)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_position(*position, out_position);
    });
}


scid_error
scid_game_cursor_ply_get(
    const scid_game_cursor* cursor,
    size_t*                 out_ply)
{
    if (any_null(cursor, out_ply))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_ply = 0;

    return abi_guard([&]() -> scid_error { return write_size(cursor->value.ply(), out_ply); });
}


scid_error
scid_game_cursor_variation_count_get(
    const scid_game_cursor* cursor,
    size_t*                 out_count)
{
    if (any_null(cursor, out_count))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_count = 0;

    return abi_guard(
        [&]() -> scid_error { return write_size(cursor->value.variationCount(), out_count); });
}


scid_error
scid_game_cursor_variation_depth_get(
    const scid_game_cursor* cursor,
    size_t*                 out_depth)
{
    if (any_null(cursor, out_depth))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_depth = 0;

    return abi_guard(
        [&]() -> scid_error { return write_size(cursor->value.variationDepth(), out_depth); });
}


scid_error
scid_game_cursor_variation_index_get(
    const scid_game_cursor* cursor,
    size_t*                 out_index)
{
    if (any_null(cursor, out_index))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_index = 0;

    return abi_guard(
        [&]() -> scid_error { return write_size(cursor->value.variationIndex(), out_index); });
}


scid_error
scid_game_cursor_is_line_start(
    const scid_game_cursor* cursor,
    int*                    out_is_line_start)
{
    if (any_null(cursor, out_is_line_start))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_line_start = 0;

    return abi_guard([&]() -> scid_error {
        return write_bool(cursor->value.isAtLineStart(), out_is_line_start);
    });
}


scid_error
scid_game_cursor_is_line_end(
    const scid_game_cursor* cursor,
    int*                    out_is_line_end)
{
    if (any_null(cursor, out_is_line_end))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_line_end = 0;

    return abi_guard(
        [&]() -> scid_error { return write_bool(cursor->value.isAtLineEnd(), out_is_line_end); });
}


scid_error
scid_game_cursor_is_game_start(
    const scid_game_cursor* cursor,
    int*                    out_is_game_start)
{
    if (any_null(cursor, out_is_game_start))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_game_start = 0;

    return abi_guard([&]() -> scid_error {
        return write_bool(cursor->value.isAtGameStart(), out_is_game_start);
    });
}


scid_error
scid_game_cursor_is_game_end(
    const scid_game_cursor* cursor,
    int*                    out_is_game_end)
{
    if (any_null(cursor, out_is_game_end))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_game_end = 0;

    return abi_guard(
        [&]() -> scid_error { return write_bool(cursor->value.isAtGameEnd(), out_is_game_end); });
}


scid_error
scid_game_cursor_is_variation_start(
    const scid_game_cursor* cursor,
    int*                    out_is_variation_start)
{
    if (any_null(cursor, out_is_variation_start))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_variation_start = 0;

    return abi_guard([&]() -> scid_error {
        return write_bool(cursor->value.isAtVariationStart(), out_is_variation_start);
    });
}


scid_error
scid_game_cursor_is_variation_end(
    const scid_game_cursor* cursor,
    int*                    out_is_variation_end)
{
    if (any_null(cursor, out_is_variation_end))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_variation_end = 0;

    return abi_guard([&]() -> scid_error {
        return write_bool(cursor->value.isAtVariationEnd(), out_is_variation_end);
    });
}


scid_error
scid_game_cursor_is_variation_empty(
    const scid_game_cursor* cursor,
    int*                    out_is_variation_empty)
{
    if (any_null(cursor, out_is_variation_empty))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_is_variation_empty = 0;

    return abi_guard([&]() -> scid_error {
        return write_bool(cursor->value.isAtEmptyVariation(), out_is_variation_empty);
    });
}


scid_error
scid_game_cursor_comment_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (any_null(cursor, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        if (cursor->value.isAtLineStart())
        {
            if (cursor->value.variationDepth() == 0)
            {
                return write_text(
                    cursor->game->value.initialComment(), out_text, out_text_capacity,
                    out_text_size);
            }

            const auto* variation = cursor->value.currentVariation();
            if (variation == nullptr)
            {
                return SCID_ERROR;
            }

            return write_text(
                variation->initialComment, out_text, out_text_capacity, out_text_size);
        }

        return write_move_comment(
            cursor->value.previousMove(), out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_cursor_comment_set(
    scid_game*              game,
    const scid_game_cursor* cursor,
    const char*             comment)
{
    if (any_null(comment))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        scid::core::MovetextCursor edit_cursor(game->value);
        if (!edit_cursor.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        return edit_cursor.setComment(comment) ? SCID_OK : SCID_ERROR;
    });
}


scid_error
scid_game_cursor_previous_movespec_get(
    const scid_game_cursor* cursor,
    scid_movespec*          out_move)
{
    if (any_null(cursor, out_move))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_move = {0, 0, SCID_PIECE_NONE, 0};

    return abi_guard(
        [&]() -> scid_error { return write_move_spec(cursor->value.previousMove(), out_move); });
}


scid_error
scid_game_cursor_previous_move_san_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (any_null(cursor, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        const auto san =
            scid::core::notation::previousSan(cursor->game->value, cursor->value.location());
        if (san.empty())
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(san, out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_cursor_previous_move_comment_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (any_null(cursor, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        return write_move_comment(
            cursor->value.previousMove(), out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_cursor_previous_move_nag_count_get(
    const scid_game_cursor* cursor,
    size_t*                 out_count)
{
    if (any_null(cursor, out_count))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_count = 0;

    return abi_guard([&]() -> scid_error {
        return write_move_nag_count(cursor->value.previousMove(), out_count);
    });
}


scid_error
scid_game_cursor_previous_move_nag_at_get(
    const scid_game_cursor* cursor,
    size_t                  index,
    scid_nag*               out_nag)
{
    if (any_null(cursor, out_nag))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_nag = 0;

    return abi_guard([&]() -> scid_error {
        return write_move_nag_at(cursor->value.previousMove(), index, out_nag);
    });
}


scid_error
scid_game_cursor_next_movespec_get(
    const scid_game_cursor* cursor,
    scid_movespec*          out_move)
{
    if (any_null(cursor, out_move))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_move = {0, 0, SCID_PIECE_NONE, 0};

    return abi_guard(
        [&]() -> scid_error { return write_move_spec(cursor->value.nextMove(), out_move); });
}


scid_error
scid_game_cursor_next_move_san_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (any_null(cursor, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        const auto san =
            scid::core::notation::nextSan(cursor->game->value, cursor->value.location());
        if (san.empty())
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(san, out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_cursor_next_move_comment_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (any_null(cursor, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        return write_move_comment(
            cursor->value.nextMove(), out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_game_cursor_next_move_nag_count_get(
    const scid_game_cursor* cursor,
    size_t*                 out_count)
{
    if (any_null(cursor, out_count))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_count = 0;

    return abi_guard(
        [&]() -> scid_error { return write_move_nag_count(cursor->value.nextMove(), out_count); });
}


scid_error
scid_game_cursor_next_move_nag_at_get(
    const scid_game_cursor* cursor,
    size_t                  index,
    scid_nag*               out_nag)
{
    if (any_null(cursor, out_nag))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_nag = 0;

    return abi_guard([&]() -> scid_error {
        return write_move_nag_at(cursor->value.nextMove(), index, out_nag);
    });
}


scid_error
scid_game_cursor_to_start(
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_start_cursor)
{
    if (any_null(cursor, out_start_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_start_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto start_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        start_cursor->value.toStart();
        *out_start_cursor = start_cursor.release();
        return SCID_OK;
    });
}


scid_error
scid_game_cursor_to_end(
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_end_cursor)
{
    if (any_null(cursor, out_end_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_end_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto end_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        end_cursor->value.toEnd();
        *out_end_cursor = end_cursor.release();
        return SCID_OK;
    });
}


scid_error
scid_game_cursor_to_ply(
    const scid_game_cursor* cursor,
    size_t                  ply,
    int*                    out_moved,
    scid_game_cursor**      out_ply_cursor)
{
    if (any_null(cursor, out_moved, out_ply_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_ply_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto       ply_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        const bool moved = ply_cursor->value.toPly(ply);
        if (!moved)
        {
            return write_bool(false, out_moved);
        }

        *out_ply_cursor = ply_cursor.release();
        return write_bool(true, out_moved);
    });
}


scid_error
scid_game_cursor_next(
    const scid_game_cursor* cursor,
    int*                    out_moved,
    scid_game_cursor**      out_next_cursor)
{
    if (any_null(cursor, out_moved, out_next_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_next_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto next_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        if (!next_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!next_cursor->value.next())
        {
            return write_bool(false, out_moved);
        }

        *out_next_cursor = next_cursor.release();
        return write_bool(true, out_moved);
    });
}


scid_error
scid_game_cursor_previous(
    const scid_game_cursor* cursor,
    int*                    out_moved,
    scid_game_cursor**      out_previous_cursor)
{
    if (any_null(cursor, out_moved, out_previous_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_previous_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto previous_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        if (!previous_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!previous_cursor->value.previous())
        {
            return write_bool(false, out_moved);
        }

        *out_previous_cursor = previous_cursor.release();
        return write_bool(true, out_moved);
    });
}


scid_error
scid_game_cursor_variation_enter(
    const scid_game_cursor* cursor,
    size_t                  index,
    int*                    out_entered,
    scid_game_cursor**      out_variation_cursor)
{
    if (any_null(cursor, out_entered, out_variation_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_variation_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto variation_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        if (!variation_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!variation_cursor->value.enterVariation(index))
        {
            return write_bool(false, out_entered);
        }

        *out_variation_cursor = variation_cursor.release();
        return write_bool(true, out_entered);
    });
}


scid_error
scid_game_cursor_variation_exit(
    const scid_game_cursor* cursor,
    int*                    out_exited,
    scid_game_cursor**      out_parent_cursor)
{
    if (any_null(cursor, out_exited, out_parent_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_parent_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        auto parent_cursor = std::make_unique<scid_game_cursor>(cursor->game);
        if (!parent_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!parent_cursor->value.exitVariation())
        {
            return write_bool(false, out_exited);
        }

        *out_parent_cursor = parent_cursor.release();
        return write_bool(true, out_exited);
    });
}


scid_error
scid_game_cursor_move_add(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_movespec           move,
    scid_game_cursor**      out_next_cursor)
{
    if (any_null(out_next_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_next_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        scid::core::MoveSpec core_move;
        if (const scid_error error = movespec_to_core(move, &core_move); error != SCID_OK)
        {
            return error;
        }

        if (const scid_error error = validate_move_at_cursor(cursor, core_move); error != SCID_OK)
        {
            return error;
        }

        auto next_cursor = std::make_unique<scid_game_cursor>(game);
        if (!next_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        next_cursor->value.addMove(core_move);
        *out_next_cursor = next_cursor.release();
        return SCID_OK;
    });
}


scid_error
scid_game_cursor_variation_add(
    scid_game*              game,
    const scid_game_cursor* cursor,
    const char*             initial_comment,
    int*                    out_added,
    scid_game_cursor**      out_variation_cursor)
{
    if (any_null(out_added, out_variation_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_added = 0;
    *out_variation_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto variation_cursor = std::make_unique<scid_game_cursor>(game);
        if (!variation_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        const auto* comment = initial_comment == nullptr ? "" : initial_comment;
        if (variation_cursor->value.addVariation(comment) == nullptr)
        {
            return write_bool(false, out_added);
        }

        *out_variation_cursor = variation_cursor.release();
        return write_bool(true, out_added);
    });
}


scid_error
scid_game_cursor_nag_add(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_nag                nag,
    int*                    out_added)
{
    if (any_null(out_added))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_added = 0;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        scid::core::MovetextCursor edit_cursor(game->value);
        if (!edit_cursor.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (edit_cursor.previousMove() == nullptr || nag == 0)
        {
            return write_bool(false, out_added);
        }

        return write_bool(edit_cursor.addPreviousMoveNag(scid::core::nagFromCode(nag)), out_added);
    });
}


scid_error
scid_game_cursor_nag_remove(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int                     is_move_nag,
    int*                    out_removed)
{
    if (any_null(out_removed))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_removed = 0;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        scid::core::MovetextCursor edit_cursor(game->value);
        if (!edit_cursor.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        auto* move = edit_cursor.previousMove();
        if (move == nullptr)
        {
            return write_bool(false, out_removed);
        }

        auto& nags = move->metadata.nags;
        for (auto it = nags.begin(); it != nags.end(); ++it)
        {
            const bool matches =
                is_move_nag != 0 ? nag_is_move_annotation(*it) : nag_is_position_annotation(*it);
            if (matches)
            {
                nags.erase(it);
                return write_bool(true, out_removed);
            }
        }

        return write_bool(false, out_removed);
    });
}


scid_error
scid_game_cursor_nag_clear(
    scid_game*              game,
    const scid_game_cursor* cursor)
{
    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        scid::core::MovetextCursor edit_cursor(game->value);
        if (!edit_cursor.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        edit_cursor.clearPreviousMoveNags();
        return SCID_OK;
    });
}


scid_error
scid_game_cursor_variation_promote_to_first(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int*                    out_promoted,
    scid_game_cursor**      out_promoted_cursor)
{
    if (any_null(out_promoted, out_promoted_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_promoted = 0;
    *out_promoted_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto promoted_cursor = std::make_unique<scid_game_cursor>(game);
        if (!promoted_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!promoted_cursor->value.promoteVariationToFirst())
        {
            return write_bool(false, out_promoted);
        }

        *out_promoted_cursor = promoted_cursor.release();
        return write_bool(true, out_promoted);
    });
}


scid_error
scid_game_cursor_variation_promote_to_mainline(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int*                    out_promoted,
    scid_game_cursor**      out_mainline_cursor)
{
    if (any_null(out_promoted, out_mainline_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_promoted = 0;
    *out_mainline_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto mainline_cursor = std::make_unique<scid_game_cursor>(game);
        if (!mainline_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!mainline_cursor->value.promoteVariationToMainline())
        {
            return write_bool(false, out_promoted);
        }

        *out_mainline_cursor = mainline_cursor.release();
        return write_bool(true, out_promoted);
    });
}


scid_error
scid_game_cursor_variation_delete(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int*                    out_deleted,
    scid_game_cursor**      out_parent_cursor)
{
    if (any_null(out_deleted, out_parent_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_deleted = 0;
    *out_parent_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto parent_cursor = std::make_unique<scid_game_cursor>(game);
        if (!parent_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        if (!parent_cursor->value.deleteVariation())
        {
            return write_bool(false, out_deleted);
        }

        *out_parent_cursor = parent_cursor.release();
        return write_bool(true, out_deleted);
    });
}


scid_error
scid_game_cursor_truncate(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_cursor)
{
    if (any_null(out_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto result_cursor = std::make_unique<scid_game_cursor>(game);
        if (!result_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        result_cursor->value.truncate();
        *out_cursor = result_cursor.release();
        return SCID_OK;
    });
}


scid_error
scid_game_cursor_truncate_before_cursor(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_cursor)
{
    if (any_null(out_cursor))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_cursor = nullptr;

    return abi_guard([&]() -> scid_error {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto result_cursor = std::make_unique<scid_game_cursor>(game);
        if (!result_cursor->value.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        result_cursor->value.truncateBeforeCursor();
        *out_cursor = result_cursor.release();
        return SCID_OK;
    });
}
