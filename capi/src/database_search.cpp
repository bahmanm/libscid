#include "scid/database.h"
#include "scid/database_search.h"

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
        append_text_search_pair(out_arguments, "-player", criteria.player.c_str());
        append_text_search_pair(out_arguments, "-white", criteria.white.c_str());
        append_text_search_pair(out_arguments, "-black", criteria.black.c_str());
        append_text_search_pair(out_arguments, "-event", criteria.event.c_str());
        append_text_search_pair(out_arguments, "-site", criteria.site.c_str());
        append_text_search_pair(out_arguments, "-sitecountry", criteria.site_country.c_str());
        append_text_search_pair(out_arguments, "-round", criteria.round.c_str());
        append_text_range_search_pair(
            out_arguments, "-date", criteria.date_min.c_str(), criteria.date_max.c_str(),
            "1800.01.01");
        append_text_range_search_pair(
            out_arguments, "-eventdate", criteria.event_date_min.c_str(),
            criteria.event_date_max.c_str(), "1800.01.01");
        append_text_range_search_pair(
            out_arguments, "-eco", criteria.eco_min.c_str(), criteria.eco_max.c_str(), "A00");

        const auto result_filter = result_search_filter(criteria.result.c_str());
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
    if (database == nullptr || criteria == nullptr || !criteria->position.has_value() ||
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
        scid::core::Position     search_position = *criteria->position;
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
                criteria->include_variations, true, criteria->include_flipped, search_type, ply);
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


scid_error
scid_search_header_criteria_create(scid_search_header_criteria** out_criteria)
{
    if (out_criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_criteria = new scid_search_header_criteria;
        return SCID_OK;
    }
    catch (...)
    {
        *out_criteria = nullptr;
        return SCID_ERROR;
    }
}


void
scid_search_header_criteria_free(scid_search_header_criteria* criteria)
{
    delete criteria;
}


scid_error
scid_search_header_criteria_player_set(
    scid_search_header_criteria* criteria,
    const char*                  player)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->player = player == nullptr ? "" : player;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_player_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->player, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_white_set(
    scid_search_header_criteria* criteria,
    const char*                  white)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->white = white == nullptr ? "" : white;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_white_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->white, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_black_set(
    scid_search_header_criteria* criteria,
    const char*                  black)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->black = black == nullptr ? "" : black;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_black_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->black, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_event_set(
    scid_search_header_criteria* criteria,
    const char*                  event)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->event = event == nullptr ? "" : event;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_event_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->event, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_site_set(
    scid_search_header_criteria* criteria,
    const char*                  site)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->site = site == nullptr ? "" : site;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_site_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->site, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_site_country_set(
    scid_search_header_criteria* criteria,
    const char*                  site_country)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->site_country = site_country == nullptr ? "" : site_country;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_site_country_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->site_country, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_round_set(
    scid_search_header_criteria* criteria,
    const char*                  round)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->round = round == nullptr ? "" : round;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_round_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->round, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_date_range_set(
    scid_search_header_criteria* criteria,
    const char*                  date_min,
    const char*                  date_max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->date_min = date_min == nullptr ? "" : date_min;
    criteria->date_max = date_max == nullptr ? "" : date_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_date_range_get(
    const scid_search_header_criteria* criteria,
    char*                              out_date_min,
    size_t                             out_date_min_capacity,
    size_t*                            out_date_min_size,
    char*                              out_date_max,
    size_t                             out_date_max_capacity,
    size_t*                            out_date_max_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (const scid_error error =
            write_text(criteria->date_min, out_date_min, out_date_min_capacity, out_date_min_size);
        error != SCID_OK)
    {
        return error;
    }

    return write_text(criteria->date_max, out_date_max, out_date_max_capacity, out_date_max_size);
}


scid_error
scid_search_header_criteria_event_date_range_set(
    scid_search_header_criteria* criteria,
    const char*                  event_date_min,
    const char*                  event_date_max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->event_date_min = event_date_min == nullptr ? "" : event_date_min;
    criteria->event_date_max = event_date_max == nullptr ? "" : event_date_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_event_date_range_get(
    const scid_search_header_criteria* criteria,
    char*                              out_event_date_min,
    size_t                             out_event_date_min_capacity,
    size_t*                            out_event_date_min_size,
    char*                              out_event_date_max,
    size_t                             out_event_date_max_capacity,
    size_t*                            out_event_date_max_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (const scid_error error = write_text(
            criteria->event_date_min, out_event_date_min, out_event_date_min_capacity,
            out_event_date_min_size);
        error != SCID_OK)
    {
        return error;
    }

    return write_text(
        criteria->event_date_max, out_event_date_max, out_event_date_max_capacity,
        out_event_date_max_size);
}


scid_error
scid_search_header_criteria_eco_range_set(
    scid_search_header_criteria* criteria,
    const char*                  eco_min,
    const char*                  eco_max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->eco_min = eco_min == nullptr ? "" : eco_min;
    criteria->eco_max = eco_max == nullptr ? "" : eco_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_eco_range_get(
    const scid_search_header_criteria* criteria,
    char*                              out_eco_min,
    size_t                             out_eco_min_capacity,
    size_t*                            out_eco_min_size,
    char*                              out_eco_max,
    size_t                             out_eco_max_capacity,
    size_t*                            out_eco_max_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (const scid_error error =
            write_text(criteria->eco_min, out_eco_min, out_eco_min_capacity, out_eco_min_size);
        error != SCID_OK)
    {
        return error;
    }

    return write_text(criteria->eco_max, out_eco_max, out_eco_max_capacity, out_eco_max_size);
}


scid_error
scid_search_header_criteria_result_set(
    scid_search_header_criteria* criteria,
    const char*                  result)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->result = result == nullptr ? "" : result;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_result_get(
    const scid_search_header_criteria* criteria,
    char*                              out_text,
    size_t                             out_text_capacity,
    size_t*                            out_text_size)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_text(criteria->result, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_search_header_criteria_game_number_range_set(
    scid_search_header_criteria* criteria,
    size_t                       min,
    size_t                       max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->game_number_min = min;
    criteria->game_number_max = max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_game_number_range_get(
    const scid_search_header_criteria* criteria,
    size_t*                            out_min,
    size_t*                            out_max)
{
    if (criteria == nullptr || out_min == nullptr || out_max == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_min = criteria->game_number_min;
    *out_max = criteria->game_number_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_halfmove_count_range_set(
    scid_search_header_criteria* criteria,
    size_t                       min,
    size_t                       max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->halfmove_count_min = min;
    criteria->halfmove_count_max = max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_halfmove_count_range_get(
    const scid_search_header_criteria* criteria,
    size_t*                            out_min,
    size_t*                            out_max)
{
    if (criteria == nullptr || out_min == nullptr || out_max == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_min = criteria->halfmove_count_min;
    *out_max = criteria->halfmove_count_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_white_elo_range_set(
    scid_search_header_criteria* criteria,
    size_t                       min,
    size_t                       max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->white_elo_min = min;
    criteria->white_elo_max = max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_white_elo_range_get(
    const scid_search_header_criteria* criteria,
    size_t*                            out_min,
    size_t*                            out_max)
{
    if (criteria == nullptr || out_min == nullptr || out_max == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_min = criteria->white_elo_min;
    *out_max = criteria->white_elo_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_black_elo_range_set(
    scid_search_header_criteria* criteria,
    size_t                       min,
    size_t                       max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->black_elo_min = min;
    criteria->black_elo_max = max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_black_elo_range_get(
    const scid_search_header_criteria* criteria,
    size_t*                            out_min,
    size_t*                            out_max)
{
    if (criteria == nullptr || out_min == nullptr || out_max == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_min = criteria->black_elo_min;
    *out_max = criteria->black_elo_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_elo_difference_range_set(
    scid_search_header_criteria* criteria,
    int                          min,
    int                          max)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->elo_difference_min = min;
    criteria->elo_difference_max = max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_elo_difference_range_get(
    const scid_search_header_criteria* criteria,
    int*                               out_min,
    int*                               out_max)
{
    if (criteria == nullptr || out_min == nullptr || out_max == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_min = criteria->elo_difference_min;
    *out_max = criteria->elo_difference_max;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_has_variations_set(
    scid_search_header_criteria* criteria,
    int                          enabled)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->has_variations = (enabled != 0);
    return SCID_OK;
}


scid_error
scid_search_header_criteria_has_variations_get(
    const scid_search_header_criteria* criteria,
    int*                               out_enabled)
{
    if (criteria == nullptr || out_enabled == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_enabled = criteria->has_variations ? 1 : 0;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_has_comments_set(
    scid_search_header_criteria* criteria,
    int                          enabled)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->has_comments = (enabled != 0);
    return SCID_OK;
}


scid_error
scid_search_header_criteria_has_comments_get(
    const scid_search_header_criteria* criteria,
    int*                               out_enabled)
{
    if (criteria == nullptr || out_enabled == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_enabled = criteria->has_comments ? 1 : 0;
    return SCID_OK;
}


scid_error
scid_search_header_criteria_has_nags_set(
    scid_search_header_criteria* criteria,
    int                          enabled)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->has_nags = (enabled != 0);
    return SCID_OK;
}


scid_error
scid_search_header_criteria_has_nags_get(
    const scid_search_header_criteria* criteria,
    int*                               out_enabled)
{
    if (criteria == nullptr || out_enabled == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_enabled = criteria->has_nags ? 1 : 0;
    return SCID_OK;
}


scid_error
scid_search_board_criteria_create(scid_search_board_criteria** out_criteria)
{
    if (out_criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_criteria = new scid_search_board_criteria;
        return SCID_OK;
    }
    catch (...)
    {
        *out_criteria = nullptr;
        return SCID_ERROR;
    }
}


void
scid_search_board_criteria_free(scid_search_board_criteria* criteria)
{
    delete criteria;
}


scid_error
scid_search_board_criteria_position_set(
    scid_search_board_criteria* criteria,
    const scid_position*        position)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (position == nullptr)
    {
        criteria->position.reset();
    }
    else
    {
        criteria->position = position->value;
    }
    return SCID_OK;
}


scid_error
scid_search_board_criteria_position_get(
    const scid_search_board_criteria* criteria,
    scid_position*                    out_position)
{
    if (criteria == nullptr || out_position == nullptr || !criteria->position.has_value())
    {
        return SCID_ERROR_BAD_ARG;
    }

    return write_position(*criteria->position, out_position);
}


scid_error
scid_search_board_criteria_match_set(
    scid_search_board_criteria* criteria,
    scid_board_search_match     match)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (match != SCID_BOARD_SEARCH_MATCH_EXACT && match != SCID_BOARD_SEARCH_MATCH_PAWNS &&
        match != SCID_BOARD_SEARCH_MATCH_FILES)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->match = match;
    return SCID_OK;
}


scid_error
scid_search_board_criteria_match_get(
    const scid_search_board_criteria* criteria,
    scid_board_search_match*          out_match)
{
    if (criteria == nullptr || out_match == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_match = criteria->match;
    return SCID_OK;
}


scid_error
scid_search_board_criteria_include_variations_set(
    scid_search_board_criteria* criteria,
    int                         enabled)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->include_variations = (enabled != 0);
    return SCID_OK;
}


scid_error
scid_search_board_criteria_include_variations_get(
    const scid_search_board_criteria* criteria,
    int*                              out_enabled)
{
    if (criteria == nullptr || out_enabled == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_enabled = criteria->include_variations ? 1 : 0;
    return SCID_OK;
}


scid_error
scid_search_board_criteria_include_flipped_set(
    scid_search_board_criteria* criteria,
    int                         enabled)
{
    if (criteria == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    criteria->include_flipped = (enabled != 0);
    return SCID_OK;
}


scid_error
scid_search_board_criteria_include_flipped_get(
    const scid_search_board_criteria* criteria,
    int*                              out_enabled)
{
    if (criteria == nullptr || out_enabled == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_enabled = criteria->include_flipped ? 1 : 0;
    return SCID_OK;
}
