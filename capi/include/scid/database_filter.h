#ifndef SCID_DATABASE_FILTER_H
#define SCID_DATABASE_FILTER_H

#include "scid/_platform.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_database scid_database;

    typedef int scid_filter_id;

    enum
    {
        SCID_FILTER_ALL_GAMES = -1,
        SCID_FILTER_PRIMARY = -2
    };


    SCID_API scid_error
    scid_database_filter_create(
        scid_database*  database,
        scid_filter_id* out_filter_id);


    SCID_API scid_error
    scid_database_filter_delete(
        scid_database* database,
        scid_filter_id filter_id);


    SCID_API scid_error
    scid_database_filter_game_count_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        size_t*              out_count);


    SCID_API scid_error
    scid_database_filter_game_indices_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        const char*          sort_criteria,
        size_t               start_row,
        size_t               row_count,
        size_t*              out_game_indices,
        size_t               out_game_indices_capacity,
        size_t*              out_game_indices_count);


    SCID_API scid_error
    scid_database_filter_game_index_at_row_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        const char*          sort_criteria,
        size_t               row,
        size_t*              out_game_index);


    SCID_API scid_error
    scid_database_filter_game_row_for_index_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        const char*          sort_criteria,
        size_t               game_index,
        size_t*              out_row);

#ifdef __cplusplus
}
#endif

#endif
