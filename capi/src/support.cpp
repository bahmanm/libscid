#include "scid/libscid/support.h"

#include "scid/libscid/handles.h"

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

namespace scid::libscid
{

    constexpr std::string_view pgn_roster_tags[] = {
        "Event", "Site", "Date", "Round", "White", "Black", "Result",
    };

    constexpr size_t pgn_roster_tag_count = sizeof(pgn_roster_tags) / sizeof(pgn_roster_tags[0]);

    constexpr std::string_view pgn_special_tags[] = {
        "ECO",
        "EventDate",
    };

    bool
    square_is_valid(scid_square square)
    {
        return square <= scid::core::H8;
    }


    scid_error
    parse_square_chars(
        char         file,
        char         rank_text,
        scid_square* out_square)
    {
        if (out_square == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (file >= 'A' && file <= 'H')
        {
            file = static_cast<char>(file - 'A' + 'a');
        }

        const auto fyle = scid::core::fyle_FromChar(file);
        const auto rank = scid::core::rank_FromChar(rank_text);
        if (fyle == scid::core::NO_FYLE || rank == scid::core::NO_RANK)
        {
            return SCID_ERROR_BAD_ARG;
        }

        *out_square = scid::core::square_Make(fyle, rank);
        return SCID_OK;
    }


    scid_error
    parse_square(
        const char*  text,
        scid_square* out_square)
    {
        if (text == nullptr || out_square == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (text[0] == '\0' || text[1] == '\0' || text[2] != '\0')
        {
            return SCID_ERROR_BAD_ARG;
        }

        return parse_square_chars(text[0], text[1], out_square);
    }


    bool
    promotion_is_valid(scid_piece promotion)
    {
        return promotion == SCID_PIECE_NONE || promotion == SCID_PIECE_QUEEN ||
               promotion == SCID_PIECE_ROOK || promotion == SCID_PIECE_BISHOP ||
               promotion == SCID_PIECE_KNIGHT;
    }


    char
    promotion_to_char(scid_piece promotion)
    {
        switch (promotion)
        {
            case SCID_PIECE_QUEEN:
                return 'q';
            case SCID_PIECE_ROOK:
                return 'r';
            case SCID_PIECE_BISHOP:
                return 'b';
            case SCID_PIECE_KNIGHT:
                return 'n';
            default:
                return '\0';
        }
    }


    scid_error
    promotion_from_char(
        char        text,
        scid_piece* out_piece)
    {
        if (out_piece == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        switch (text)
        {
            case 'Q':
            case 'q':
                *out_piece = SCID_PIECE_QUEEN;
                return SCID_OK;
            case 'R':
            case 'r':
                *out_piece = SCID_PIECE_ROOK;
                return SCID_OK;
            case 'B':
            case 'b':
                *out_piece = SCID_PIECE_BISHOP;
                return SCID_OK;
            case 'N':
            case 'n':
                *out_piece = SCID_PIECE_KNIGHT;
                return SCID_OK;
            default:
                return SCID_ERROR_BAD_ARG;
        }
    }


    scid_error
    write_text(
        std::string_view text,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size)
    {
        if (out_text_size == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        *out_text_size = text.size();

        if (text.empty())
        {
            if (out_text != nullptr && out_text_capacity > 0)
            {
                out_text[0] = '\0';
            }

            return SCID_OK;
        }

        if (out_text == nullptr || out_text_capacity <= text.size())
        {
            return SCID_ERROR_BUFFER_FULL;
        }

        std::memcpy(out_text, text.data(), text.size());
        out_text[text.size()] = '\0';
        return SCID_OK;
    }


    scid_error
    write_bool(
        bool value,
        int* out_value)
    {
        if (out_value == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        *out_value = value ? 1 : 0;
        return SCID_OK;
    }


    scid_error
    write_size(
        size_t  value,
        size_t* out_value)
    {
        if (out_value == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        *out_value = value;
        return SCID_OK;
    }


    scid_piece
    piece_to_c(scid::core::pieceT piece)
    {
        return piece == scid::core::EMPTY ? SCID_PIECE_NONE : piece;
    }


    scid_error
    movespec_to_core(
        scid_movespec         move,
        scid::core::MoveSpec* out_move)
    {
        if (out_move == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (!promotion_is_valid(move.promotion))
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (!square_is_valid(move.from) || !square_is_valid(move.to))
        {
            return SCID_ERROR_BAD_ARG;
        }

        out_move->from = static_cast<scid::core::squareT>(move.from);
        out_move->to = static_cast<scid::core::squareT>(move.to);
        out_move->promotion = move.promotion == SCID_PIECE_NONE
                                  ? scid::core::EMPTY
                                  : static_cast<scid::core::pieceT>(move.promotion);
        out_move->castling = move.is_castling != 0;
        return SCID_OK;
    }


    scid_movespec
    movespec_from_core(const scid::core::MoveSpec& move)
    {
        return {
            move.from, move.to,
            move.promotion == scid::core::EMPTY ? SCID_PIECE_NONE
                                                : static_cast<scid_piece>(move.promotion),
            move.castling ? 1 : 0};
    }


    scid_error
    write_move_spec(
        const scid::core::Move* move,
        scid_movespec*          out_move)
    {
        if (out_move == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (move == nullptr)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        *out_move = movespec_from_core(move->spec);
        return SCID_OK;
    }


    scid_error
    write_move_comment(
        const scid::core::Move* move,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size)
    {
        if (out_text_size == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (move == nullptr)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(move->metadata.comment, out_text, out_text_capacity, out_text_size);
    }


    scid_error
    write_move_nag_count(
        const scid::core::Move* move,
        size_t*                 out_count)
    {
        if (out_count == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (move == nullptr)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_size(move->metadata.nags.size(), out_count);
    }


    scid_error
    write_move_nag_at(
        const scid::core::Move* move,
        size_t                  index,
        scid_nag*               out_nag)
    {
        if (out_nag == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (move == nullptr)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        if (index >= move->metadata.nags.size())
        {
            return SCID_ERROR_BAD_ARG;
        }

        *out_nag = scid::core::nagCode(move->metadata.nags[index]);
        return SCID_OK;
    }


    bool
    nag_is_move_annotation(scid::core::Nag nag)
    {
        const auto value = scid::core::nagCode(nag);
        return value >= 1 && value <= 6;
    }


    bool
    nag_is_position_annotation(scid::core::Nag nag)
    {
        const auto value = scid::core::nagCode(nag);
        return value >= 10 && value <= 21;
    }


    scid_error
    validate_move_at_cursor(
        const scid_game_cursor*     cursor,
        const scid::core::MoveSpec& move)
    {
        scid::core::GameCursor read_cursor(cursor->game->value);
        if (!read_cursor.restore(cursor->value.location()))
        {
            return SCID_ERROR;
        }

        auto position = read_cursor.currentPosition();
        if (!position)
        {
            return SCID_ERROR_INVALID_MOVE;
        }

        return position->applyMove(move);
    }


    scid_error
    validate_cursor_game(
        scid_game*              game,
        const scid_game_cursor* cursor)
    {
        if (game == nullptr || cursor == nullptr || cursor->game != game)
        {
            return SCID_ERROR_BAD_ARG;
        }

        return SCID_OK;
    }


    scid_error
    create_cursor_at(
        scid_game*                          game,
        const scid::core::MovetextLocation& location,
        scid_game_cursor**                  out_cursor)
    {
        if (game == nullptr || out_cursor == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        try
        {
            auto* cursor = new scid_game_cursor(game);
            if (!cursor->value.restore(location))
            {
                delete cursor;
                *out_cursor = nullptr;
                return SCID_ERROR_BAD_ARG;
            }

            *out_cursor = cursor;
            return SCID_OK;
        }
        catch (...)
        {
            *out_cursor = nullptr;
            return SCID_ERROR;
        }
    }


    scid_error
    create_cursor_copy(
        const scid_game_cursor* source_cursor,
        scid_game_cursor**      out_cursor)
    {
        if (source_cursor == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        return create_cursor_at(source_cursor->game, source_cursor->value.location(), out_cursor);
    }


    std::string_view
    eco_name_from_line(std::string_view line)
    {
        const auto name_start = line.find('[');
        const auto name_end = line.rfind(']');
        if (name_start == std::string_view::npos || name_end == std::string_view::npos ||
            name_start >= name_end)
        {
            return {};
        }

        return line.substr(name_start + 1, name_end - name_start - 1);
    }


    std::string
    date_to_string(scid::core::dateT date)
    {
        char text[16] = {};
        scid::core::date_DecodeToString(date, text);
        return text;
    }


    bool
    database_game_index_to_core(
        size_t                    index,
        scid::database::gamenumT* out_index)
    {
        if (out_index == nullptr || index > std::numeric_limits<scid::database::gamenumT>::max())
        {
            return false;
        }

        *out_index = static_cast<scid::database::gamenumT>(index);
        return true;
    }


    bool
    database_game_info_get(
        const scid::database::scidBaseT& database,
        size_t                           index,
        scid::database::GameInfo*        out_info)
    {
        if (out_info == nullptr)
        {
            return false;
        }

        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index))
        {
            return false;
        }

        const auto info = database.gameInfoBounds(game_index);
        if (!info)
        {
            return false;
        }

        *out_info = *info;
        return true;
    }


    bool
    database_game_index_is_valid(
        const scid::database::scidBaseT& database,
        size_t                           index,
        scid::database::gamenumT*        out_index)
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index) ||
            !database.gameInfoBounds(game_index))
        {
            return false;
        }

        if (out_index != nullptr)
        {
            *out_index = game_index;
        }

        return true;
    }


    scid_error
    database_error_to_c(scid::core::errorT error)
    {
        switch (error)
        {
            case scid::core::OK:
                return SCID_OK;
            case scid::core::ERROR_UserCancel:
                return SCID_ERROR_USER_CANCEL;
            case scid::core::ERROR_BadArg:
                return SCID_ERROR_BAD_ARG;
            case scid::core::ERROR_FileOpen:
                return SCID_ERROR_FILE_OPEN;
            case scid::core::ERROR_FileReadOnly:
                return SCID_ERROR_FILE_READ_ONLY;
            case scid::core::ERROR_Corrupt:
                return SCID_ERROR_CORRUPT;
            default:
                return SCID_ERROR;
        }
    }


    bool
    filter_value_is_valid(unsigned value)
    {
        return value <= std::numeric_limits<scid::core::byte>::max();
    }


    bool
    database_filter_id_is_builtin(std::string_view filter_id)
    {
        return filter_id == "all" || filter_id == "dbfilter";
    }


    bool
    database_filter_id_is_mutable(std::string_view filter_id)
    {
        return filter_id != "all";
    }


    bool
    database_filter_get(
        const scid_database*     database,
        const char*              filter_id,
        scid::database::HFilter* out_filter)
    {
        if (database == nullptr || filter_id == nullptr || out_filter == nullptr ||
            !database->value.isOpen())
        {
            return false;
        }

        auto filter = database->value.getFilter(filter_id);
        if (filter == nullptr)
        {
            return false;
        }

        *out_filter = filter;
        return true;
    }


    scid_error
    database_open(
        std::string_view                db_type,
        scid::database::fileModeT       mode,
        const char*                     path,
        scid_database**                 out_database,
        const scid::database::Progress* progress)
    {
        if (path == nullptr || out_database == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        try
        {
            const scid::database::Progress default_progress;
            const auto& selected_progress = progress == nullptr ? default_progress : *progress;
            auto*       database = new scid_database;
            const auto  error = database->value.open(db_type, mode, path, selected_progress);
            if (error != scid::core::OK)
            {
                delete database;
                *out_database = nullptr;
                return database_error_to_c(error);
            }

            if (db_type == "MEMORY")
            {
                database->type = "memory";
            }
            else if (db_type == "SCID5")
            {
                database->type = "scid5";
            }
            else
            {
                database->type.assign(db_type);
            }

            *out_database = database;
            return SCID_OK;
        }
        catch (...)
        {
            *out_database = nullptr;
            return SCID_ERROR;
        }
    }


    scid_error
    result_from_string(
        std::string_view     text,
        scid::core::resultT* out_result)
    {
        if (out_result == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        if (text == "*")
        {
            *out_result = scid::core::RESULT_None;
            return SCID_OK;
        }

        if (text == "1-0")
        {
            *out_result = scid::core::RESULT_White;
            return SCID_OK;
        }

        if (text == "0-1")
        {
            *out_result = scid::core::RESULT_Black;
            return SCID_OK;
        }

        if (text == "1/2-1/2")
        {
            *out_result = scid::core::RESULT_Draw;
            return SCID_OK;
        }

        return SCID_ERROR_BAD_ARG;
    }


    std::string
    position_fen(const scid::core::Position& position)
    {
        char fen[256];
        position.PrintFEN(fen, sizeof(fen));
        return fen;
    }


    std::string
    game_tag_value(
        const scid::core::Game& game,
        std::string_view        name)
    {
        if (name == "Event")
        {
            return game.event();
        }
        if (name == "Site")
        {
            return game.site();
        }
        if (name == "Date")
        {
            return date_to_string(game.date());
        }
        if (name == "Round")
        {
            return game.round();
        }
        if (name == "White")
        {
            return game.white().name;
        }
        if (name == "Black")
        {
            return game.black().name;
        }
        if (name == "Result")
        {
            return std::string(game.resultString());
        }
        if (name == "ECO")
        {
            return game.eco();
        }
        if (name == "EventDate")
        {
            if (game.eventDate() == scid::core::ZERO_DATE)
            {
                return {};
            }
            return date_to_string(game.eventDate());
        }
        if (name == "FEN")
        {
            if (const scid::core::Position* position = game.startPosition())
            {
                return position_fen(*position);
            }
            return {};
        }

        if (const std::string* value = game.findExtraTag(name))
        {
            return *value;
        }

        return {};
    }


    bool
    game_has_special_tag(
        const scid::core::Game& game,
        std::string_view        name)
    {
        if (name == "ECO")
        {
            return !game.eco().empty();
        }
        if (name == "EventDate")
        {
            return game.eventDate() != scid::core::ZERO_DATE;
        }
        if (name == "FEN")
        {
            return game.startPosition() != nullptr;
        }

        return false;
    }


    size_t
    game_tag_count(const scid::core::Game& game)
    {
        size_t count = pgn_roster_tag_count + game.extraTags().size();
        for (const auto tag : pgn_special_tags)
        {
            if (game_has_special_tag(game, tag))
            {
                ++count;
            }
        }
        if (game_has_special_tag(game, "FEN"))
        {
            ++count;
        }

        return count;
    }


    bool
    game_tag_at(
        const scid::core::Game& game,
        size_t                  index,
        std::string_view*       out_name,
        std::string*            out_value)
    {
        if (out_name == nullptr || out_value == nullptr)
        {
            return false;
        }

        if (index < pgn_roster_tag_count)
        {
            *out_name = pgn_roster_tags[index];
            *out_value = game_tag_value(game, *out_name);
            return true;
        }

        index -= pgn_roster_tag_count;

        for (const auto tag : pgn_special_tags)
        {
            if (!game_has_special_tag(game, tag))
            {
                continue;
            }

            if (index == 0)
            {
                *out_name = tag;
                *out_value = game_tag_value(game, tag);
                return true;
            }

            --index;
        }

        const auto& extra_tags = game.extraTags();
        if (index < extra_tags.size())
        {
            *out_name = extra_tags[index].first;
            *out_value = extra_tags[index].second;
            return true;
        }

        index -= extra_tags.size();
        if (game_has_special_tag(game, "FEN") && index == 0)
        {
            *out_name = "FEN";
            *out_value = game_tag_value(game, "FEN");
            return true;
        }

        return false;
    }


    std::string
    database_game_tag_value(
        const scid::database::scidBaseT& database,
        scid::database::gamenumT         index,
        std::string_view                 name)
    {
        const auto info = database.gameInfo(index);
        const auto tags = database.tagRoster(index);

        if (name == "Event")
        {
            return tags.event;
        }
        if (name == "Site")
        {
            return tags.site;
        }
        if (name == "Date")
        {
            return date_to_string(info.date);
        }
        if (name == "Round")
        {
            return tags.round;
        }
        if (name == "White")
        {
            return tags.white;
        }
        if (name == "Black")
        {
            return tags.black;
        }
        if (name == "Result")
        {
            return scid::core::RESULT_LONGSTR[info.result];
        }
        if (name == "ECO")
        {
            scid::eco::String text = {};
            scid::eco::toExtendedString(info.ecoCode, text);
            return text;
        }
        if (name == "EventDate")
        {
            return date_to_string(info.eventDate);
        }

        return {};
    }


    scid_error
    game_set_tag(
        scid::core::Game& game,
        std::string_view  name,
        std::string_view  value)
    {
        if (name == "Date")
        {
            game.setDate(scid::core::date_parsePGNTag(value.data(), value.size()));
            return SCID_OK;
        }
        if (name == "EventDate")
        {
            game.setEventDate(scid::core::date_parsePGNTag(value.data(), value.size()));
            return SCID_OK;
        }
        if (name == "Result")
        {
            scid::core::resultT result = scid::core::RESULT_None;
            if (const scid_error error = result_from_string(value, &result); error != SCID_OK)
            {
                return error;
            }
            game.setResult(result);
            return SCID_OK;
        }
        if (name == "ECO")
        {
            game.setEco(value);
            return SCID_OK;
        }

        game.addTag(name, value);
        return SCID_OK;
    }


    scid_error
    write_optional_diagnostic(
        std::string_view text,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size)
    {
        if (out_text == nullptr && out_text_capacity == 0 && out_text_size == nullptr)
        {
            return SCID_OK;
        }

        return write_text(text, out_text, out_text_capacity, out_text_size);
    }


    scid_error
    write_optional_text(
        std::string_view text,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size)
    {
        if (out_text == nullptr && out_text_capacity == 0 && out_text_size == nullptr)
        {
            return SCID_OK;
        }

        return write_text(text, out_text, out_text_capacity, out_text_size);
    }


    scid_error
    write_position(
        const scid::core::Position& source,
        scid_position*              out_position)
    {
        if (out_position == nullptr)
        {
            return SCID_ERROR_BAD_ARG;
        }

        out_position->value = source;
        return SCID_OK;
    }


    scid::core::Position
    game_start_position(const scid::core::Game& game)
    {
        return game.startPosition() ? *game.startPosition() : scid::core::Position::getStdStart();
    }


    bool
    positions_match(
        const scid::core::Position& lhs,
        const scid::core::Position& rhs)
    {
        return position_fen(lhs) == position_fen(rhs);
    }


    scid_error
    validate_move_sequence(
        const scid::core::MoveSequence& sequence,
        const scid::core::Position&     start_position)
    {
        auto position = start_position;
        for (const auto& move : sequence.moves)
        {
            for (const auto& variation : move.childVariations)
            {
                if (const scid_error error = validate_move_sequence(variation.line, position);
                    error != SCID_OK)
                {
                    return error;
                }
            }

            if (const scid_error error = position.applyMove(move.spec); error != SCID_OK)
            {
                return error;
            }
        }

        return SCID_OK;
    }


    scid_error
    append_move_sequence(
        scid::core::MovetextCursor&     cursor,
        const scid::core::MoveSequence& sequence)
    {
        for (const auto& source_move : sequence.moves)
        {
            auto& target_move = cursor.addMove(source_move.spec);
            target_move.san = source_move.san;
            target_move.metadata = source_move.metadata;
            target_move.childVariations = source_move.childVariations;
        }

        return SCID_OK;
    }


    scid_error
    maybe_set_line_start_comment(
        scid::core::MovetextCursor& cursor,
        std::string_view            comment)
    {
        if (comment.empty() || !cursor.isAtLineStart())
        {
            return SCID_OK;
        }

        return cursor.setComment(comment) ? SCID_OK : SCID_ERROR;
    }

}
