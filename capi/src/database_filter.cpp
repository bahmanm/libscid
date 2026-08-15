#include "scid/database.h"

#include "scid/libscid/handles.h"
#include "scid/libscid/support.h"

#include "scid/database/scidbase.h"

#include <algorithm>
#include <vector>

using namespace scid::libscid;

scid_error
scid_database_filter_create(
    scid_database*  database,
    scid_filter_id* out_filter_id)
{
    if (database == nullptr || out_filter_id == nullptr || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const scid_filter_id filter_id = database->next_filter_id++;
        database->filters.emplace_back(filter_id, database->value.newFilter());
        *out_filter_id = filter_id;
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_filter_delete(
    scid_database* database,
    scid_filter_id filter_id)
{
    if (database == nullptr || filter_id <= 0 || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        const auto it = std::find_if(
            database->filters.begin(), database->filters.end(),
            [filter_id](const auto& entry) { return entry.first == filter_id; });
        if (it == database->filters.end())
        {
            return SCID_ERROR_BAD_ARG;
        }

        database->value.deleteFilter(it->second.c_str());
        database->filters.erase(it);
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_filter_game_count_get(
    const scid_database* database,
    scid_filter_id       filter_id,
    size_t*              out_count)
{
    if (database == nullptr || out_count == nullptr || !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter))
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(filter->size(), out_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_filter_game_indices_get(
    const scid_database* database,
    scid_filter_id       filter_id,
    const char*          sort_criteria,
    size_t               start_row,
    size_t               row_count,
    size_t*              out_game_indices,
    size_t               out_game_indices_capacity,
    size_t*              out_game_indices_count)
{
    if (database == nullptr || sort_criteria == nullptr || out_game_indices_count == nullptr ||
        !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (out_game_indices == nullptr || out_game_indices_capacity < row_count)
    {
        *out_game_indices_count = row_count;
        return SCID_ERROR_BUFFER_FULL;
    }

    try
    {
        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter))
        {
            return SCID_ERROR_BAD_ARG;
        }

        auto& mutable_database = const_cast<scid::database::scidBaseT&>(database->value);
        std::vector<scid::database::gamenumT> game_indices(row_count);
        const size_t                          listed = mutable_database.listGames(
            sort_criteria, start_row, row_count, filter, game_indices.data());

        for (size_t i = 0; i < listed; ++i)
        {
            out_game_indices[i] = game_indices[i];
        }

        return write_size(listed, out_game_indices_count);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_database_filter_game_index_at_row_get(
    const scid_database* database,
    scid_filter_id       filter_id,
    const char*          sort_criteria,
    size_t               row,
    size_t*              out_game_index)
{
    if (out_game_index == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    size_t           count = 0;
    const scid_error error = scid_database_filter_game_indices_get(
        database, filter_id, sort_criteria, row, 1, out_game_index, 1, &count);
    if (error != SCID_OK)
    {
        return error;
    }
    return count == 1 ? SCID_OK : SCID_ERROR_BAD_ARG;
}


scid_error
scid_database_filter_game_row_for_index_get(
    const scid_database* database,
    scid_filter_id       filter_id,
    const char*          sort_criteria,
    size_t               game_index,
    size_t*              out_row)
{
    if (database == nullptr || sort_criteria == nullptr || out_row == nullptr ||
        !database->value.isOpen())
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::database::gamenumT core_game_index = 0;
        if (!database_game_index_is_valid(database->value, game_index, &core_game_index))
        {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter))
        {
            return SCID_ERROR_BAD_ARG;
        }

        auto&        mutable_database = const_cast<scid::database::scidBaseT&>(database->value);
        const size_t row = mutable_database.sortedPosition(sort_criteria, filter, core_game_index);
        if (row == scid::database::INVALID_GAMEID)
        {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(row, out_row);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}
