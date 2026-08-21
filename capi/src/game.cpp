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
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace scid::libscid;

scid_error
scid_game_create_blank(
    const scid_position* position,
    scid_game**          out_game)
{
    if (position == nullptr || out_game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* game = new scid_game;
        if (!position->value.IsStdStart())
        {
            game->value.setStartPosition(position->value);
        }

        *out_game = game;
        return SCID_OK;
    }
    catch (...)
    {
        *out_game = nullptr;
        return SCID_ERROR;
    }
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
    if (position == nullptr || pgn == nullptr || out_game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* game = new scid_game;
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
            delete game;
            *out_game = nullptr;
            return diagnostic_error;
        }

        if (!ok)
        {
            delete game;
            *out_game = nullptr;
            return SCID_ERROR_CORRUPT;
        }

        *out_game = game;
        return SCID_OK;
    }
    catch (...)
    {
        *out_game = nullptr;
        return SCID_ERROR;
    }
}


void
scid_game_free(scid_game* game)
{
    delete game;
}


scid_error
scid_game_pgn_options_create(scid_game_pgn_options** out_options)
{
    if (out_options == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_options = new scid_game_pgn_options;
        return SCID_OK;
    }
    catch (...)
    {
        *out_options = nullptr;
        return SCID_ERROR;
    }
}


void
scid_game_pgn_options_free(scid_game_pgn_options* options)
{
    delete options;
}


scid_error
scid_game_pgn_options_symbolic_nags_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (options == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    options->value.symbolicNags = enabled != 0;
    return SCID_OK;
}


scid_error
scid_game_pgn_options_supplemental_tags_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (options == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    options->value.includeSupplementalTags = enabled != 0;
    return SCID_OK;
}


scid_error
scid_game_pgn_options_comments_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (options == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    options->value.includeComments = enabled != 0;
    return SCID_OK;
}


scid_error
scid_game_pgn_options_variations_set(
    scid_game_pgn_options* options,
    int                    enabled)
{
    if (options == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    options->value.includeVariations = enabled != 0;
    return SCID_OK;
}


scid_error
scid_game_pgn_options_line_width_set(
    scid_game_pgn_options* options,
    unsigned               line_width)
{
    if (options == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (line_width == 0)
    {
        options->value.lineWidth = std::nullopt;
    }
    else
    {
        options->value.lineWidth = line_width;
    }
    return SCID_OK;
}


scid_error
scid_game_to_pgn(
    const scid_game*             game,
    const scid_game_pgn_options* options,
    char*                        out_text,
    size_t                       out_text_capacity,
    size_t*                      out_text_size)
{
    if (game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        std::string pgn;
        const auto  encode_options =
            options == nullptr ? scid::core::pgn::EncodeOptions{} : options->value;
        scid::core::pgn::encode(game->value, pgn, encode_options);
        return write_text(pgn, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_mainline_halfmove_count_get(
    const scid_game* game,
    size_t*          out_count)
{
    if (game == nullptr || out_count == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_size(game->value.mainlineHalfMoveCount(), out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_initial_comment_get(
    const scid_game* game,
    char*            out_text,
    size_t           out_text_capacity,
    size_t*          out_text_size)
{
    if (game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_text(game->value.initialComment(), out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_tag_get(
    const scid_game* game,
    const char*      name,
    char*            out_text,
    size_t           out_text_capacity,
    size_t*          out_text_size)
{
    if (game == nullptr || name == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_text(
            game_tag_value(game->value, name), out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_tag_set(
    scid_game*  game,
    const char* name,
    const char* value)
{
    if (game == nullptr || name == nullptr || value == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return game_set_tag(game->value, name, value);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_tag_count_get(
    const scid_game* game,
    size_t*          out_count)
{
    if (game == nullptr || out_count == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_size(game_tag_count(game->value), out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
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
    if (game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_tag_remove(
    scid_game*  game,
    const char* name,
    int*        out_removed)
{
    if (game == nullptr || name == nullptr || out_removed == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_start_position_get(
    const scid_game* game,
    scid_position*   out_position)
{
    if (game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid::core::Position* position = game->value.startPosition())
        {
            return write_position(*position, out_position);
        }

        return write_position(scid::core::Position::getStdStart(), out_position);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_final_position_get(
    const scid_game* game,
    scid_position*   out_position)
{
    if (game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::core::GameCursor cursor(game->value);
        cursor.toEnd();
        const auto position = cursor.currentPosition();
        if (!position)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_position(*position, out_position);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_merge_moves(
    scid_game*                 target_game,
    const scid_game_cursor*    target_cursor,
    const scid_game*           source_game,
    scid_game_merge_moves_mode mode,
    scid_game_cursor**         out_cursor)
{
    if (out_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }
    *out_cursor = nullptr;

    if (source_game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (mode != SCID_GAME_MERGE_MOVES_APPEND && mode != SCID_GAME_MERGE_MOVES_INSERT_VARIATION &&
        mode != SCID_GAME_MERGE_MOVES_REPLACE)
    {
        return SCID_ERROR_BAD_ARG;
    }

    scid::core::Game backup;
    bool             has_backup = false;

    try
    {
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

        backup = target_game->value;
        has_backup = true;
        auto restore_and_return = [&](scid_error error) {
            target_game->value = backup;
            *out_cursor = nullptr;
            return error;
        };

        scid::core::MovetextCursor edit_cursor(target_game->value);
        if (!edit_cursor.restore(target_cursor->value.location()))
        {
            return restore_and_return(SCID_ERROR);
        }

        switch (mode)
        {
            case SCID_GAME_MERGE_MOVES_APPEND:
                if (edit_cursor.nextMove() != nullptr)
                {
                    return restore_and_return(SCID_ERROR_BAD_ARG);
                }
                if (const scid_error error = maybe_set_line_start_comment(
                        edit_cursor, source_game->value.initialComment());
                    error != SCID_OK)
                {
                    return restore_and_return(error);
                }
                break;

            case SCID_GAME_MERGE_MOVES_INSERT_VARIATION:
                if (edit_cursor.nextMove() == nullptr)
                {
                    return restore_and_return(SCID_ERROR_BAD_ARG);
                }
                if (edit_cursor.addVariation(source_game->value.initialComment()) == nullptr)
                {
                    return restore_and_return(SCID_ERROR_BAD_ARG);
                }
                break;

            case SCID_GAME_MERGE_MOVES_REPLACE:
                edit_cursor.truncate();
                if (const scid_error error = maybe_set_line_start_comment(
                        edit_cursor, source_game->value.initialComment());
                    error != SCID_OK)
                {
                    return restore_and_return(error);
                }
                break;
        }

        if (const scid_error error = append_move_sequence(edit_cursor, source_movetext.mainline);
            error != SCID_OK)
        {
            return restore_and_return(error);
        }

        const auto location = edit_cursor.location();
        if (const scid_error error = create_cursor_at(target_game, location, out_cursor);
            error != SCID_OK)
        {
            return restore_and_return(error);
        }

        return SCID_OK;
    }
    catch (...)
    {
        if (has_backup && target_game != nullptr)
        {
            target_game->value = backup;
        }
        *out_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_create(
    scid_game*         game,
    scid_game_cursor** out_cursor)
{
    if (game == nullptr || out_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_cursor = new scid_game_cursor(game);
        return SCID_OK;
    }
    catch (...)
    {
        *out_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_clone(
    scid_game*              game,
    const scid_game_cursor* source_cursor,
    scid_game_cursor**      out_cursor)
{
    if (out_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (const scid_error error = validate_cursor_game(game, source_cursor); error != SCID_OK)
    {
        return error;
    }

    return create_cursor_copy(source_cursor, out_cursor);
}


void
scid_game_cursor_free(scid_game_cursor* cursor)
{
    delete cursor;
}


scid_error
scid_game_cursor_position_get(
    const scid_game_cursor* cursor,
    scid_position*          out_position)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_ply_get(
    const scid_game_cursor* cursor,
    size_t*                 out_ply)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.ply(), out_ply);
}


scid_error
scid_game_cursor_variation_count_get(
    const scid_game_cursor* cursor,
    size_t*                 out_count)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.variationCount(), out_count);
}


scid_error
scid_game_cursor_variation_depth_get(
    const scid_game_cursor* cursor,
    size_t*                 out_depth)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.variationDepth(), out_depth);
}


scid_error
scid_game_cursor_variation_index_get(
    const scid_game_cursor* cursor,
    size_t*                 out_index)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.variationIndex(), out_index);
}


scid_error
scid_game_cursor_is_line_start(
    const scid_game_cursor* cursor,
    int*                    out_is_line_start)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtLineStart(), out_is_line_start);
}


scid_error
scid_game_cursor_is_line_end(
    const scid_game_cursor* cursor,
    int*                    out_is_line_end)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtLineEnd(), out_is_line_end);
}


scid_error
scid_game_cursor_is_game_start(
    const scid_game_cursor* cursor,
    int*                    out_is_game_start)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtGameStart(), out_is_game_start);
}


scid_error
scid_game_cursor_is_game_end(
    const scid_game_cursor* cursor,
    int*                    out_is_game_end)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtGameEnd(), out_is_game_end);
}


scid_error
scid_game_cursor_is_variation_start(
    const scid_game_cursor* cursor,
    int*                    out_is_variation_start)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtVariationStart(), out_is_variation_start);
}


scid_error
scid_game_cursor_is_variation_end(
    const scid_game_cursor* cursor,
    int*                    out_is_variation_end)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtVariationEnd(), out_is_variation_end);
}


scid_error
scid_game_cursor_is_variation_empty(
    const scid_game_cursor* cursor,
    int*                    out_is_variation_empty)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtEmptyVariation(), out_is_variation_empty);
}


scid_error
scid_game_cursor_comment_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_comment_set(
    scid_game*              game,
    const scid_game_cursor* cursor,
    const char*             comment)
{
    if (comment == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_previous_movespec_get(
    const scid_game_cursor* cursor,
    scid_movespec*          out_move)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_spec(cursor->value.previousMove(), out_move);
}


scid_error
scid_game_cursor_previous_move_san_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (cursor == nullptr || out_text_size == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const auto san =
            scid::core::notation::previousSan(cursor->game->value, cursor->value.location());
        if (san.empty())
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(san, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_previous_move_comment_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }


    return write_move_comment(
        cursor->value.previousMove(), out_text, out_text_capacity, out_text_size);
}


scid_error
scid_game_cursor_previous_move_nag_count_get(
    const scid_game_cursor* cursor,
    size_t*                 out_count)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_count(cursor->value.previousMove(), out_count);
}


scid_error
scid_game_cursor_previous_move_nag_at_get(
    const scid_game_cursor* cursor,
    size_t                  index,
    scid_nag*               out_nag)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_at(cursor->value.previousMove(), index, out_nag);
}


scid_error
scid_game_cursor_next_movespec_get(
    const scid_game_cursor* cursor,
    scid_movespec*          out_move)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_spec(cursor->value.nextMove(), out_move);
}


scid_error
scid_game_cursor_next_move_san_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (cursor == nullptr || out_text_size == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const auto san =
            scid::core::notation::nextSan(cursor->game->value, cursor->value.location());
        if (san.empty())
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(san, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_next_move_comment_get(
    const scid_game_cursor* cursor,
    char*                   out_text,
    size_t                  out_text_capacity,
    size_t*                 out_text_size)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_comment(cursor->value.nextMove(), out_text, out_text_capacity, out_text_size);
}


scid_error
scid_game_cursor_next_move_nag_count_get(
    const scid_game_cursor* cursor,
    size_t*                 out_count)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_count(cursor->value.nextMove(), out_count);
}


scid_error
scid_game_cursor_next_move_nag_at_get(
    const scid_game_cursor* cursor,
    size_t                  index,
    scid_nag*               out_nag)
{
    if (cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_at(cursor->value.nextMove(), index, out_nag);
}


scid_error
scid_game_cursor_to_start(
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_start_cursor)
{
    if (cursor == nullptr || out_start_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* start_cursor = new scid_game_cursor(cursor->game);
        start_cursor->value.toStart();
        *out_start_cursor = start_cursor;
        return SCID_OK;
    }
    catch (...)
    {
        *out_start_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_to_end(
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_end_cursor)
{
    if (cursor == nullptr || out_end_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* end_cursor = new scid_game_cursor(cursor->game);
        end_cursor->value.toEnd();
        *out_end_cursor = end_cursor;
        return SCID_OK;
    }
    catch (...)
    {
        *out_end_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_to_ply(
    const scid_game_cursor* cursor,
    size_t                  ply,
    int*                    out_moved,
    scid_game_cursor**      out_ply_cursor)
{
    if (cursor == nullptr || out_moved == nullptr || out_ply_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto*      ply_cursor = new scid_game_cursor(cursor->game);
        const bool moved = ply_cursor->value.toPly(ply);
        if (!moved)
        {
            delete ply_cursor;
            *out_ply_cursor = nullptr;
            return write_bool(false, out_moved);
        }

        *out_ply_cursor = ply_cursor;
        return write_bool(true, out_moved);
    }
    catch (...)
    {
        *out_ply_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_next(
    const scid_game_cursor* cursor,
    int*                    out_moved,
    scid_game_cursor**      out_next_cursor)
{
    if (cursor == nullptr || out_moved == nullptr || out_next_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* next_cursor = new scid_game_cursor(cursor->game);
        if (!next_cursor->value.restore(cursor->value.location()))
        {
            delete next_cursor;
            *out_next_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!next_cursor->value.next())
        {
            delete next_cursor;
            *out_next_cursor = nullptr;
            return write_bool(false, out_moved);
        }

        *out_next_cursor = next_cursor;
        return write_bool(true, out_moved);
    }
    catch (...)
    {
        *out_next_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_previous(
    const scid_game_cursor* cursor,
    int*                    out_moved,
    scid_game_cursor**      out_previous_cursor)
{
    if (cursor == nullptr || out_moved == nullptr || out_previous_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* previous_cursor = new scid_game_cursor(cursor->game);
        if (!previous_cursor->value.restore(cursor->value.location()))
        {
            delete previous_cursor;
            *out_previous_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!previous_cursor->value.previous())
        {
            delete previous_cursor;
            *out_previous_cursor = nullptr;
            return write_bool(false, out_moved);
        }

        *out_previous_cursor = previous_cursor;
        return write_bool(true, out_moved);
    }
    catch (...)
    {
        *out_previous_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_variation_enter(
    const scid_game_cursor* cursor,
    size_t                  index,
    int*                    out_entered,
    scid_game_cursor**      out_variation_cursor)
{
    if (cursor == nullptr || out_entered == nullptr || out_variation_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* variation_cursor = new scid_game_cursor(cursor->game);
        if (!variation_cursor->value.restore(cursor->value.location()))
        {
            delete variation_cursor;
            *out_variation_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!variation_cursor->value.enterVariation(index))
        {
            delete variation_cursor;
            *out_variation_cursor = nullptr;
            return write_bool(false, out_entered);
        }

        *out_variation_cursor = variation_cursor;
        return write_bool(true, out_entered);
    }
    catch (...)
    {
        *out_variation_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_variation_exit(
    const scid_game_cursor* cursor,
    int*                    out_exited,
    scid_game_cursor**      out_parent_cursor)
{
    if (cursor == nullptr || out_exited == nullptr || out_parent_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto* parent_cursor = new scid_game_cursor(cursor->game);
        if (!parent_cursor->value.restore(cursor->value.location()))
        {
            delete parent_cursor;
            *out_parent_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!parent_cursor->value.exitVariation())
        {
            delete parent_cursor;
            *out_parent_cursor = nullptr;
            return write_bool(false, out_exited);
        }

        *out_parent_cursor = parent_cursor;
        return write_bool(true, out_exited);
    }
    catch (...)
    {
        *out_parent_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_move_add(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_movespec           move,
    scid_game_cursor**      out_next_cursor)
{
    if (out_next_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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

        auto* next_cursor = new scid_game_cursor(game);
        if (!next_cursor->value.restore(cursor->value.location()))
        {
            delete next_cursor;
            *out_next_cursor = nullptr;
            return SCID_ERROR;
        }

        next_cursor->value.addMove(core_move);
        *out_next_cursor = next_cursor;
        return SCID_OK;
    }
    catch (...)
    {
        *out_next_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_variation_add(
    scid_game*              game,
    const scid_game_cursor* cursor,
    const char*             initial_comment,
    int*                    out_added,
    scid_game_cursor**      out_variation_cursor)
{
    if (out_added == nullptr || out_variation_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto* variation_cursor = new scid_game_cursor(game);
        if (!variation_cursor->value.restore(cursor->value.location()))
        {
            delete variation_cursor;
            *out_variation_cursor = nullptr;
            return SCID_ERROR;
        }

        const auto* comment = initial_comment == nullptr ? "" : initial_comment;
        if (variation_cursor->value.addVariation(comment) == nullptr)
        {
            delete variation_cursor;
            *out_variation_cursor = nullptr;
            return write_bool(false, out_added);
        }

        *out_variation_cursor = variation_cursor;
        return write_bool(true, out_added);
    }
    catch (...)
    {
        *out_variation_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_nag_add(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_nag                nag,
    int*                    out_added)
{
    if (out_added == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_nag_remove(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int                     is_move_nag,
    int*                    out_removed)
{
    if (out_removed == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_nag_clear(
    scid_game*              game,
    const scid_game_cursor* cursor)
{
    try
    {
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
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_variation_promote_to_first(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int*                    out_promoted,
    scid_game_cursor**      out_promoted_cursor)
{
    if (out_promoted == nullptr || out_promoted_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto* promoted_cursor = new scid_game_cursor(game);
        if (!promoted_cursor->value.restore(cursor->value.location()))
        {
            delete promoted_cursor;
            *out_promoted_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!promoted_cursor->value.promoteVariationToFirst())
        {
            delete promoted_cursor;
            *out_promoted_cursor = nullptr;
            return write_bool(false, out_promoted);
        }

        *out_promoted_cursor = promoted_cursor;
        return write_bool(true, out_promoted);
    }
    catch (...)
    {
        *out_promoted_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_variation_promote_to_mainline(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int*                    out_promoted,
    scid_game_cursor**      out_mainline_cursor)
{
    if (out_promoted == nullptr || out_mainline_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto* mainline_cursor = new scid_game_cursor(game);
        if (!mainline_cursor->value.restore(cursor->value.location()))
        {
            delete mainline_cursor;
            *out_mainline_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!mainline_cursor->value.promoteVariationToMainline())
        {
            delete mainline_cursor;
            *out_mainline_cursor = nullptr;
            return write_bool(false, out_promoted);
        }

        *out_mainline_cursor = mainline_cursor;
        return write_bool(true, out_promoted);
    }
    catch (...)
    {
        *out_mainline_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_variation_delete(
    scid_game*              game,
    const scid_game_cursor* cursor,
    int*                    out_deleted,
    scid_game_cursor**      out_parent_cursor)
{
    if (out_deleted == nullptr || out_parent_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto* parent_cursor = new scid_game_cursor(game);
        if (!parent_cursor->value.restore(cursor->value.location()))
        {
            delete parent_cursor;
            *out_parent_cursor = nullptr;
            return SCID_ERROR;
        }

        if (!parent_cursor->value.deleteVariation())
        {
            delete parent_cursor;
            *out_parent_cursor = nullptr;
            return write_bool(false, out_deleted);
        }

        *out_parent_cursor = parent_cursor;
        return write_bool(true, out_deleted);
    }
    catch (...)
    {
        *out_parent_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_truncate(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_cursor)
{
    if (out_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto* result_cursor = new scid_game_cursor(game);
        if (!result_cursor->value.restore(cursor->value.location()))
        {
            delete result_cursor;
            *out_cursor = nullptr;
            return SCID_ERROR;
        }

        result_cursor->value.truncate();
        *out_cursor = result_cursor;
        return SCID_OK;
    }
    catch (...)
    {
        *out_cursor = nullptr;
        return SCID_ERROR;
    }
}


scid_error
scid_game_cursor_truncate_before_cursor(
    scid_game*              game,
    const scid_game_cursor* cursor,
    scid_game_cursor**      out_cursor)
{
    if (out_cursor == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (const scid_error error = validate_cursor_game(game, cursor); error != SCID_OK)
        {
            return error;
        }

        auto* result_cursor = new scid_game_cursor(game);
        if (!result_cursor->value.restore(cursor->value.location()))
        {
            delete result_cursor;
            *out_cursor = nullptr;
            return SCID_ERROR;
        }

        result_cursor->value.truncateBeforeCursor();
        *out_cursor = result_cursor;
        return SCID_OK;
    }
    catch (...)
    {
        *out_cursor = nullptr;
        return SCID_ERROR;
    }
}
