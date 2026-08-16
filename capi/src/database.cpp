#include "scid/database.h"

#include "scid/libscid/handles.h"
#include "scid/libscid/progress.h"
#include "scid/libscid/support.h"

#include "scid/core/game.h"
#include "scid/core/game_result.h"
#include "scid/core/notation.h"
#include "scid/core/pgn/decode.h"
#include "scid/core/pgn/encode.h"
#include "scid/core/primitives.h"
#include "scid/database/scidbase.h"

#include <array>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

using namespace scid::libscid;

scid_error
scid_database_create_memory(
    const char*     name,
    scid_database** out_database)
{
    return database_open("MEMORY", scid::database::FMODE_Create, name, out_database);
}


scid_error
scid_database_create_scid5(
    const char*     path,
    scid_database** out_database)
{
    return database_open("SCID5", scid::database::FMODE_Create, path, out_database);
}


scid_error
scid_database_open_scid5(
    const char*     path,
    scid_database** out_database)
{
    return database_open("SCID5", scid::database::FMODE_Both, path, out_database);
}


scid_error
scid_database_open_scid5_read_only(
    const char*     path,
    scid_database** out_database)
{
    return database_open("SCID5", scid::database::FMODE_ReadOnly, path, out_database);
}


scid_error
scid_database_open_pgn_read_only(
    const char*                   path,
    scid_progress_report_callback progress_report,
    void*                         progress_report_user_data,
    scid_should_cancel_fn         should_cancel,
    void*                         should_cancel_user_data,
    scid_database**               out_database)
{
    scid::database::Progress progress(new CallbackProgress(
        progress_report, progress_report_user_data, should_cancel, should_cancel_user_data));
    return database_open("PGN", scid::database::FMODE_ReadOnly, path, out_database, &progress);
}


scid_error
scid_database_close(scid_database* database)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        if (database->value.isOpen())
        {
            database->value.Close();
        }
        database->type.clear();
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


void
scid_database_free(scid_database* database)
{
    delete database;
}


scid_error
scid_database_is_open(
    const scid_database* database,
    int*                 out_is_open)
{
    if (database == nullptr || out_is_open == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_bool(database->value.isOpen(), out_is_open);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_filename_get(
    const scid_database* database,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_text(
            database->value.getFileName(), out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_type_get(
    const scid_database* database,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_text(database->type, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_read_only_get(
    const scid_database* database,
    int*                 out_read_only)
{
    if (database == nullptr || out_read_only == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_bool(database->value.isReadOnly(), out_read_only);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_save(scid_database* database)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return database_error_to_c(database->value.flush());
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_metadata_get(
    const scid_database* database,
    const char*          key,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr || key == nullptr || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        for (const auto& [name, value] : database->value.getExtraInfo())
        {
            if (std::strcmp(name, key) == 0)
            {
                return write_text(value, out_text, out_text_capacity, out_text_size);
            }
        }

        return write_text("", out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_metadata_set(
    scid_database* database,
    const char*    key,
    const char*    value)
{
    if (database == nullptr || key == nullptr || value == nullptr || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return database_error_to_c(database->value.setExtraInfo(key, value));
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_metadata_count_get(
    const scid_database* database,
    size_t*              out_count)
{
    if (database == nullptr || out_count == nullptr || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_size(database->value.getExtraInfo().size(), out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_metadata_at_get(
    const scid_database* database,
    size_t               index,
    char*                out_key,
    size_t               out_key_capacity,
    size_t*              out_key_size,
    char*                out_value,
    size_t               out_value_capacity,
    size_t*              out_value_size)
{
    if (database == nullptr || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const auto metadata = database->value.getExtraInfo();
        if (index >= metadata.size())
        {
            return SCID_ERROR_BAD_ARG;
        }

        const auto& [key, value] = metadata[index];
        if (const scid_error error = write_text(key, out_key, out_key_capacity, out_key_size);
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
scid_database_stats_date_range_get(
    const scid_database* database,
    char*                out_min_date,
    size_t               out_min_date_capacity,
    size_t*              out_min_date_size,
    char*                out_max_date,
    size_t               out_max_date_capacity,
    size_t*              out_max_date_size)
{
    if (database == nullptr || out_min_date_size == nullptr || out_max_date_size == nullptr ||
        !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const auto& stats = database->value.getStats();
        if (const scid_error error = write_text(
                date_to_string(stats.minDate), out_min_date, out_min_date_capacity,
                out_min_date_size);
            error != SCID_OK)
        {
            return error;
        }


        return write_text(
            date_to_string(stats.maxDate), out_max_date, out_max_date_capacity, out_max_date_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_stats_result_count_get(
    const scid_database* database,
    const char*          result,
    size_t*              out_count)
{
    if (database == nullptr || result == nullptr || out_count == nullptr ||
        !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::core::resultT core_result = scid::core::RESULT_None;
        if (const scid_error error = result_from_string(result, &core_result); error != SCID_OK)
        {
            return error;
        }

        return write_size(database->value.getStats().nResults[core_result], out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_count_get(
    const scid_database* database,
    size_t*              out_count)
{
    if (database == nullptr || out_count == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_size(database->value.numGames(), out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_import_pgn(
    scid_database* database,
    const char*    pgn,
    size_t         pgn_size,
    char*          out_diagnostic,
    size_t         out_diagnostic_capacity,
    size_t*        out_diagnostic_size,
    size_t*        out_imported_count)
{
    if (database == nullptr || pgn == nullptr || out_imported_count == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_imported_count = 0;

    try
    {
        std::vector<scid::core::Game> games;
        scid::core::pgn::ParseLog     log;
        size_t                        parsed_size = 0;

        while (parsed_size < pgn_size)
        {
            while (parsed_size < pgn_size &&
                   std::isspace(static_cast<unsigned char>(pgn[parsed_size])))
            {
                ++parsed_size;
            }
            if (parsed_size >= pgn_size)
            {
                break;
            }

            scid::core::Game game;
            const auto       start_size = log.n_bytes;
            const bool       ok =
                scid::core::pgn::parseGame(pgn + parsed_size, pgn_size - parsed_size, game, log);
            const auto consumed_size = static_cast<size_t>(log.n_bytes - start_size);

            if (!ok || consumed_size == 0)
            {
                const scid_error diagnostic_error = write_optional_diagnostic(
                    log.log, out_diagnostic, out_diagnostic_capacity, out_diagnostic_size);
                return diagnostic_error == SCID_OK ? SCID_ERROR_CORRUPT : diagnostic_error;
            }

            parsed_size += consumed_size;
            games.push_back(std::move(game));
        }

        const scid_error diagnostic_error = write_optional_diagnostic(
            log.log, out_diagnostic, out_diagnostic_capacity, out_diagnostic_size);
        if (diagnostic_error != SCID_OK)
        {
            return diagnostic_error;
        }

        for (const auto& game : games)
        {
            const scid_error error = database_error_to_c(database->value.addGame(game, ""));
            if (error != SCID_OK)
            {
                return error;
            }
            ++*out_imported_count;
        }

        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_add(
    scid_database*   database,
    const scid_game* game,
    const char*      flags)
{
    if (database == nullptr || game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const auto error = database->value.addGame(game->value, flags == nullptr ? "" : flags);
        return database_error_to_c(error);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_replace(
    scid_database*   database,
    size_t           index,
    const scid_game* game,
    const char*      flags)
{
    if (database == nullptr || game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_is_valid(database->value, index, &game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }


        return database_error_to_c(
            database->value.saveGame(game->value, flags == nullptr ? "" : flags, game_index));
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_delete(
    scid_database* database,
    size_t         index)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_is_valid(database->value, index, &game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }


        return database_error_to_c(
            database->value.setFlag(true, 1u << scid::database::GAME_FLAG_DELETE, game_index));
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_undelete(
    scid_database* database,
    size_t         index)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_is_valid(database->value, index, &game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }


        return database_error_to_c(
            database->value.setFlag(false, 1u << scid::database::GAME_FLAG_DELETE, game_index));
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_get(
    const scid_database* database,
    size_t               index,
    scid_game**          out_game,
    char*                out_flags,
    size_t               out_flags_capacity,
    size_t*              out_flags_size)
{
    if (database == nullptr || out_game == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto*                game = new scid_game;
        std::array<char, 22> flags = {};
        const auto           error = database->value.loadGame(
            static_cast<scid::database::gamenumT>(index), game->value, flags.data(), flags.size());
        if (error != scid::core::OK)
        {
            delete game;
            *out_game = nullptr;
            return error == scid::core::ERROR_BadArg ? SCID_ERROR_BAD_ARG : SCID_ERROR;
        }

        if (const scid_error flags_error =
                write_optional_text(flags.data(), out_flags, out_flags_capacity, out_flags_size);
            flags_error != SCID_OK)
        {
            delete game;
            *out_game = nullptr;
            return flags_error;
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
scid_database_game_export_pgn(
    const scid_database* database,
    size_t               index,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }

        scid::core::Game game;
        const auto       error = database->value.loadGame(game_index, game, nullptr, 0);
        if (error != scid::core::OK)
        {
            return database_error_to_c(error);
        }

        std::string pgn;
        scid::core::pgn::encode(game, pgn);
        return write_text(pgn, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_tag_get(
    const scid_database* database,
    size_t               index,
    const char*          name,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr || name == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index) ||
            !database->value.gameInfoBounds(game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }


        return write_text(
            database_game_tag_value(database->value, game_index, name), out_text, out_text_capacity,
            out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_halfmove_count_get(
    const scid_database* database,
    size_t               index,
    size_t*              out_count)
{
    if (database == nullptr || out_count == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }

        const auto info = database->value.gameInfoBounds(game_index);
        if (!info)
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(info->halfMoveCount, out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_number_get(
    const scid_database* database,
    size_t               index,
    size_t*              out_number)
{
    if (database == nullptr || out_number == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info))
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(index + 1, out_number);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_deleted_get(
    const scid_database* database,
    size_t               index,
    int*                 out_deleted)
{
    if (database == nullptr || out_deleted == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info))
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_bool(info.hasDeleteFlag(), out_deleted);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_result_get(
    const scid_database* database,
    size_t               index,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info))
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_text(
            scid::core::RESULT_LONGSTR[info.result], out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_eco_get(
    const scid_database* database,
    size_t               index,
    scid_eco_code*       out_code)
{
    if (database == nullptr || out_code == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info))
        {
            return SCID_ERROR_BAD_ARG;
        }

        *out_code = static_cast<scid_eco_code>(info.ecoCode);
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_game_date_get(
    const scid_database* database,
    size_t               index,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (database == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info))
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_text(date_to_string(info.date), out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}
