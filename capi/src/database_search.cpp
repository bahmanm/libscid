#include "scid/database.h"

#include "scid/libscid/handles.h"
#include "scid/libscid/progress.h"
#include "scid/libscid/support.h"

#include "scid/core/board.h"
#include "scid/core/game.h"
#include "scid/core/primitives.h"
#include "scid/database/scidbase.h"

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace scid::libscid;

namespace scid::database
{

    scid::core::errorT
    search_index(
        const scid::database::scidBaseT* base,
        scid::database::HFilter&         filter,
        int                              argc,
        const char**                     argv,
        const scid::database::Progress&  progress);

} // namespace scid::database

namespace
{

    bool
    text_has_value(const char* text)
    {
        return text != nullptr && text[0] != '\0';
    }


    void
    append_search_pair(
        std::vector<std::pair<
            std::string,
            std::string>>& arguments,
        std::string        name,
        std::string        value)
    {
        if (!value.empty())
        {
            arguments.emplace_back(std::move(name), std::move(value));
        }
    }


    void
    append_text_search_pair(
        std::vector<std::pair<
            std::string,
            std::string>>& arguments,
        const char*        name,
        const char*        value)
    {
        if (text_has_value(value))
        {
            arguments.emplace_back(name, value);
        }
    }


    void
    append_text_range_search_pair(
        std::vector<std::pair<
            std::string,
            std::string>>& arguments,
        const char*        name,
        const char*        min_value,
        const char*        max_value,
        const char*        default_min)
    {
        if (!text_has_value(min_value) && !text_has_value(max_value))
        {
            return;
        }

        std::string value;
        if (text_has_value(min_value))
        {
            value = min_value;
        }
        else
        {
            value = default_min;
        }

        if (text_has_value(max_value))
        {
            value += ' ';
            value += max_value;
        }

        arguments.emplace_back(name, std::move(value));
    }


    void
    append_size_range_search_pair(
        std::vector<std::pair<
            std::string,
            std::string>>& arguments,
        const char*        name,
        size_t             min_value,
        size_t             max_value,
        size_t             default_min,
        size_t             default_max)
    {
        if (min_value == 0 && max_value == 0)
        {
            return;
        }

        const size_t effective_min = min_value == 0 ? default_min : min_value;
        const size_t effective_max = max_value == 0 ? default_max : max_value;
        arguments.emplace_back(
            name, std::to_string(effective_min) + " " + std::to_string(effective_max));
    }


    void
    append_int_range_search_pair(
        std::vector<std::pair<
            std::string,
            std::string>>& arguments,
        const char*        name,
        int                min_value,
        int                max_value,
        int                default_min,
        int                default_max)
    {
        if (min_value == 0 && max_value == 0)
        {
            return;
        }

        const int effective_min = min_value == 0 ? default_min : min_value;
        const int effective_max = max_value == 0 ? default_max : max_value;
        arguments.emplace_back(
            name, std::to_string(effective_min) + " " + std::to_string(effective_max));
    }


    bool
    result_search_token_to_char(
        std::string_view token,
        char*            out_result)
    {
        if (out_result == nullptr)
        {
            return false;
        }

        if (token == "1-0" || token == "1")
        {
            *out_result = '1';
            return true;
        }
        if (token == "0-1" || token == "0")
        {
            *out_result = '0';
            return true;
        }
        if (token == "1/2-1/2" || token == "=" || token == "=-=")
        {
            *out_result = '=';
            return true;
        }
        if (token == "*")
        {
            *out_result = '*';
            return true;
        }

        return false;
    }


    bool
    append_result_token(
        std::string_view token,
        std::string&     out_filter)
    {
        if (token.empty())
        {
            return true;
        }

        char result = '\0';
        if (result_search_token_to_char(token, &result))
        {
            out_filter.push_back(result);
            return true;
        }

        return false;
    }


    std::optional<std::string>
    result_search_filter(const char* text)
    {
        if (!text_has_value(text))
        {
            return std::string();
        }

        std::string_view source(text);
        std::string      result;
        size_t           token_start = 0;
        bool             saw_separator = false;

        for (size_t i = 0; i < source.size(); ++i)
        {
            const bool separator = source[i] == ',' || source[i] == ';' || source[i] == '|' ||
                                   std::isspace(static_cast<unsigned char>(source[i]));
            if (!separator)
            {
                continue;
            }

            saw_separator = true;
            if (!append_result_token(source.substr(token_start, i - token_start), result))
            {
                return std::nullopt;
            }
            token_start = i + 1;
        }

        if (saw_separator)
        {
            if (!append_result_token(source.substr(token_start), result))
            {
                return std::nullopt;
            }
            return result;
        }

        char parsed_result = '\0';
        if (result_search_token_to_char(source, &parsed_result))
        {
            result.push_back(parsed_result);
            return result;
        }

        for (const char ch : source)
        {
            if (ch == '1' || ch == '0' || ch == '=' || ch == '*')
            {
                result.push_back(ch);
            }
            else
            {
                return std::nullopt;
            }
        }

        return result;
    }


    scid_error
    build_header_search_arguments(
        const scid_search_header_criteria& criteria,
        size_t                             game_count,
        std::vector<std::pair<
            std::string,
            std::string>>&                 out_arguments)
    {
        append_text_search_pair(out_arguments, "-player", criteria.player);
        append_text_search_pair(out_arguments, "-white", criteria.white);
        append_text_search_pair(out_arguments, "-black", criteria.black);
        append_text_search_pair(out_arguments, "-event", criteria.event);
        append_text_search_pair(out_arguments, "-site", criteria.site);
        append_text_search_pair(out_arguments, "-sitecountry", criteria.site_country);
        append_text_search_pair(out_arguments, "-round", criteria.round);
        append_text_range_search_pair(
            out_arguments, "-date", criteria.date_min, criteria.date_max, "1800.01.01");
        append_text_range_search_pair(
            out_arguments, "-eventdate", criteria.event_date_min, criteria.event_date_max,
            "1800.01.01");
        append_text_range_search_pair(
            out_arguments, "-eco", criteria.eco_min, criteria.eco_max, "A00");

        const auto result_filter = result_search_filter(criteria.result);
        if (!result_filter)
        {
            return SCID_ERROR_BAD_ARG;
        }
        append_search_pair(out_arguments, "-result", *result_filter);

        append_size_range_search_pair(
            out_arguments, "-gnum", criteria.game_number_min, criteria.game_number_max, 1,
            game_count);
        append_size_range_search_pair(
            out_arguments, "-length", criteria.halfmove_count_min, criteria.halfmove_count_max, 0,
            9999);
        append_size_range_search_pair(
            out_arguments, "-welo", criteria.white_elo_min, criteria.white_elo_max, 0, 4095);
        append_size_range_search_pair(
            out_arguments, "-belo", criteria.black_elo_min, criteria.black_elo_max, 0, 4095);
        append_int_range_search_pair(
            out_arguments, "-delo", criteria.elo_difference_min, criteria.elo_difference_max, -4095,
            4095);

        if (criteria.has_variations)
        {
            out_arguments.emplace_back("-n_variations", "1 999999");
        }
        if (criteria.has_comments)
        {
            out_arguments.emplace_back("-n_comments", "1 999999");
        }
        if (criteria.has_nags)
        {
            out_arguments.emplace_back("-n_nags", "1 999999");
        }

        return SCID_OK;
    }


    void
    copy_filter(
        const scid::database::scidBaseT& database,
        const scid::database::HFilter&   source,
        scid::database::HFilter&         destination)
    {
        for (scid::database::gamenumT i = 0, n = database.numGames(); i < n; ++i)
        {
            destination.set(i, source.get(i));
        }
    }


    std::vector<unsigned char>
    included_games_snapshot(
        const scid::database::scidBaseT& database,
        const scid::database::HFilter&   filter)
    {
        std::vector<unsigned char> included(database.numGames());
        for (scid::database::gamenumT i = 0, n = database.numGames(); i < n; ++i)
        {
            included[i] = filter.get(i) != 0;
        }
        return included;
    }


    void
    intersect_with_snapshot(
        const scid::database::scidBaseT&  database,
        const std::vector<unsigned char>& included,
        scid::database::HFilter&          filter)
    {
        for (scid::database::gamenumT i = 0, n = database.numGames(); i < n; ++i)
        {
            if (!included[i])
            {
                filter.erase(i);
            }
        }
    }


    bool
    board_search_match_to_core(
        scid_board_search_match          match,
        scid::database::gameExactMatchT* out_match)
    {
        if (out_match == nullptr)
        {
            return false;
        }

        switch (match)
        {
            case SCID_BOARD_SEARCH_MATCH_EXACT:
                *out_match = scid::database::GAME_EXACT_MATCH_Exact;
                return true;
            case SCID_BOARD_SEARCH_MATCH_PAWNS:
                *out_match = scid::database::GAME_EXACT_MATCH_Pawns;
                return true;
            case SCID_BOARD_SEARCH_MATCH_FILES:
                *out_match = scid::database::GAME_EXACT_MATCH_Fyles;
                return true;
            default:
                return false;
        }
    }


    scid::core::Position
    color_flipped_position(const scid::core::Position& position)
    {
        scid::core::Position result;
        result.Clear();
        result.SetToMove(scid::core::color_Flip(position.GetToMove()));
        result.SetPlyCounter(position.GetPlyCounter());

        const scid::core::pieceT* board = position.GetBoard();
        for (scid::core::squareT square = scid::core::A1; square <= scid::core::H8; ++square)
        {
            const scid::core::pieceT piece = board[square];
            if (piece == scid::core::EMPTY)
            {
                continue;
            }

            const scid::core::pieceT  flipped_piece = scid::core::PIECE_FLIP[piece];
            const scid::core::squareT flipped_square = scid::core::square_FlipRank(square);
            (void)result.AddPiece(flipped_piece, flipped_square);
        }

        return result;
    }

} // namespace

scid_error
scid_database_search_headers(
    scid_database*                     database,
    scid_filter_id                     source_filter_id,
    scid_filter_id                     destination_filter_id,
    const scid_search_header_criteria* criteria,
    scid_progress_report_callback      progress_report,
    void*                              progress_report_user_data,
    scid_should_cancel_fn              should_cancel,
    void*                              should_cancel_user_data)
{
    if (database == nullptr || criteria == nullptr ||
        destination_filter_id == SCID_FILTER_ALL_GAMES || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::HFilter source(nullptr);
        if (!database_filter_get(database, source_filter_id, &source))
        {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter destination(nullptr);
        if (!database_filter_get(database, destination_filter_id, &destination))
        {
            return SCID_ERROR_BAD_ARG;
        }

        std::vector<std::pair<std::string, std::string>> arguments;
        arguments.emplace_back("-filter", "AND");
        if (const scid_error error =
                build_header_search_arguments(*criteria, database->value.numGames(), arguments);
            error != SCID_OK)
        {
            return error;
        }

        std::vector<const char*> argv;
        argv.reserve(arguments.size() * 2);
        for (const auto& [name, value] : arguments)
        {
            argv.push_back(name.c_str());
            argv.push_back(value.c_str());
        }

        copy_filter(database->value, source, destination);
        scid::database::Progress progress(new CallbackProgress(
            progress_report, progress_report_user_data, should_cancel, should_cancel_user_data));
        return database_error_to_c(
            scid::database::search_index(
                &database->value, destination, static_cast<int>(argv.size()), argv.data(),
                progress));
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_search_position(
    scid_database*                database,
    scid_filter_id                source_filter_id,
    scid_filter_id                destination_filter_id,
    const scid_position*          position,
    scid_progress_report_callback progress_report,
    void*                         progress_report_user_data,
    scid_should_cancel_fn         should_cancel,
    void*                         should_cancel_user_data)
{
    if (database == nullptr || position == nullptr ||
        destination_filter_id == SCID_FILTER_ALL_GAMES || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::HFilter source(nullptr);
        if (!database_filter_get(database, source_filter_id, &source))
        {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter destination(nullptr);
        if (!database_filter_get(database, destination_filter_id, &destination))
        {
            return SCID_ERROR_BAD_ARG;
        }

        const auto               source_included = included_games_snapshot(database->value, source);
        scid::database::Progress progress(new CallbackProgress(
            progress_report, progress_report_user_data, should_cancel, should_cancel_user_data));
        if (!database->value.setPositionSearchFilter(position->value, destination, progress))
        {
            return SCID_ERROR_USER_CANCEL;
        }
        intersect_with_snapshot(database->value, source_included, destination);
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_search_board(
    scid_database*                    database,
    scid_filter_id                    source_filter_id,
    scid_filter_id                    destination_filter_id,
    const scid_search_board_criteria* criteria,
    scid_progress_report_callback     progress_report,
    void*                             progress_report_user_data,
    scid_should_cancel_fn             should_cancel,
    void*                             should_cancel_user_data)
{
    if (database == nullptr || criteria == nullptr || criteria->position == nullptr ||
        destination_filter_id == SCID_FILTER_ALL_GAMES || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gameExactMatchT search_type = scid::database::GAME_EXACT_MATCH_Exact;
        if (!board_search_match_to_core(criteria->match, &search_type))
        {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter source(nullptr);
        if (!database_filter_get(database, source_filter_id, &source))
        {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter destination(nullptr);
        if (!database_filter_get(database, destination_filter_id, &destination))
        {
            return SCID_ERROR_BAD_ARG;
        }

        const auto source_included = included_games_snapshot(database->value, source);
        destination.clear();

        scid::core::Game         scratch_game;
        scid::core::Position     search_position = criteria->position->value;
        scid::core::Position     flipped_position = color_flipped_position(search_position);
        scid::database::Progress progress(new CallbackProgress(
            progress_report, progress_report_user_data, should_cancel, should_cancel_user_data));

        const auto game_count = database->value.numGames();
        for (scid::database::gamenumT index = 0; index < game_count; ++index)
        {
            if (!source_included[index])
            {
                continue;
            }

            if (!progress(index, game_count, "Searching board"))
            {
                return SCID_ERROR_USER_CANCEL;
            }

            scid::core::uint         ply = 0;
            const scid::core::errorT error = database->value.searchBoard(
                index, scratch_game, &search_position, &flipped_position,
                criteria->include_variations != 0, true, criteria->include_flipped != 0,
                search_type, ply);
            if (error != scid::core::OK)
            {
                return database_error_to_c(error);
            }
            if (ply != 0)
            {
                destination.set(index, static_cast<scid::core::byte>(ply));
            }
        }

        return progress(game_count, game_count, "Searching board") ? SCID_OK
                                                                   : SCID_ERROR_USER_CANCEL;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}
