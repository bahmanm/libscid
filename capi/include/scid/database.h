#ifndef SCID_DATABASE_H
#define SCID_DATABASE_H

#include "scid/_platform.h"
#include "scid/eco.h"
#include "scid/game.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_database scid_database;

    typedef int scid_filter_id;

    typedef int scid_board_search_match;

    enum
    {
        SCID_FILTER_ALL_GAMES = -1,
        SCID_FILTER_PRIMARY = -2
    };

    enum
    {
        SCID_BOARD_SEARCH_MATCH_EXACT = 0,
        SCID_BOARD_SEARCH_MATCH_PAWNS = 1,
        SCID_BOARD_SEARCH_MATCH_FILES = 2
    };

    typedef void (*scid_progress_report_callback)(
        size_t      done,
        size_t      total,
        const char* message,
        void*       user_data);

    typedef int (*scid_should_cancel_fn)(void* user_data);

    typedef struct scid_search_header_criteria
    {
            const char* player;
            const char* white;
            const char* black;
            const char* event;
            const char* site;
            const char* site_country;
            const char* round;

            const char* date_min;
            const char* date_max;
            const char* event_date_min;
            const char* event_date_max;

            const char* eco_min;
            const char* eco_max;

            const char* result;

            size_t game_number_min;
            size_t game_number_max;
            size_t halfmove_count_min;
            size_t halfmove_count_max;

            size_t white_elo_min;
            size_t white_elo_max;
            size_t black_elo_min;
            size_t black_elo_max;
            int    elo_difference_min;
            int    elo_difference_max;

            int has_variations;
            int has_comments;
            int has_nags;
    } scid_search_header_criteria;

    typedef struct scid_search_board_criteria
    {
            const scid_position*    position;
            scid_board_search_match match;
            int                     include_variations;
            int                     include_flipped;
    } scid_search_board_criteria;


    SCID_API scid_error
    scid_database_create_memory(
        const char*     name,
        scid_database** out_database);


    SCID_API scid_error
    scid_database_create_scid5(
        const char*     path,
        scid_database** out_database);


    SCID_API scid_error
    scid_database_open_scid5(
        const char*     path,
        scid_database** out_database);


    SCID_API scid_error
    scid_database_open_scid5_read_only(
        const char*     path,
        scid_database** out_database);


    SCID_API scid_error
    scid_database_open_pgn_read_only(
        const char*                   path,
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data,
        scid_database**               out_database);


    SCID_API scid_error
    scid_database_close(scid_database* database);


    SCID_API void
    scid_database_free(scid_database* database);


    SCID_API scid_error
    scid_database_is_open(
        const scid_database* database,
        int*                 out_is_open);


    SCID_API scid_error
    scid_database_filename_get(
        const scid_database* database,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    SCID_API scid_error
    scid_database_type_get(
        const scid_database* database,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    SCID_API scid_error
    scid_database_read_only_get(
        const scid_database* database,
        int*                 out_read_only);


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


    SCID_API scid_error
    scid_database_search_headers(
        scid_database*                     database,
        scid_filter_id                     source_filter_id,
        scid_filter_id                     destination_filter_id,
        const scid_search_header_criteria* criteria,
        scid_progress_report_callback      progress_report,
        void*                              progress_report_user_data,
        scid_should_cancel_fn              should_cancel,
        void*                              should_cancel_user_data);


    SCID_API scid_error
    scid_database_search_position(
        scid_database*                database,
        scid_filter_id                source_filter_id,
        scid_filter_id                destination_filter_id,
        const scid_position*          position,
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data);


    SCID_API scid_error
    scid_database_search_board(
        scid_database*                    database,
        scid_filter_id                    source_filter_id,
        scid_filter_id                    destination_filter_id,
        const scid_search_board_criteria* criteria,
        scid_progress_report_callback     progress_report,
        void*                             progress_report_user_data,
        scid_should_cancel_fn             should_cancel,
        void*                             should_cancel_user_data);


    SCID_API scid_error
    scid_database_save(scid_database* database);


    SCID_API scid_error
    scid_database_metadata_get(
        const scid_database* database,
        const char*          key,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    SCID_API scid_error
    scid_database_metadata_set(
        scid_database* database,
        const char*    key,
        const char*    value);


    SCID_API scid_error
    scid_database_metadata_count_get(
        const scid_database* database,
        size_t*              out_count);


    SCID_API scid_error
    scid_database_metadata_at_get(
        const scid_database* database,
        size_t               index,
        char*                out_key,
        size_t               out_key_capacity,
        size_t*              out_key_size,
        char*                out_value,
        size_t               out_value_capacity,
        size_t*              out_value_size);


    SCID_API scid_error
    scid_database_stats_date_range_get(
        const scid_database* database,
        char*                out_min_date,
        size_t               out_min_date_capacity,
        size_t*              out_min_date_size,
        char*                out_max_date,
        size_t               out_max_date_capacity,
        size_t*              out_max_date_size);


    SCID_API scid_error
    scid_database_stats_result_count_get(
        const scid_database* database,
        const char*          result,
        size_t*              out_count);


    SCID_API scid_error
    scid_database_game_count_get(
        const scid_database* database,
        size_t*              out_count);


    SCID_API scid_error
    scid_database_import_pgn(
        scid_database* database,
        const char*    pgn,
        size_t         pgn_size,
        char*          out_diagnostic,
        size_t         out_diagnostic_capacity,
        size_t*        out_diagnostic_size,
        size_t*        out_imported_count);


    SCID_API scid_error
    scid_database_game_add(
        scid_database*   database,
        const scid_game* game,
        const char*      flags);


    SCID_API scid_error
    scid_database_game_replace(
        scid_database*   database,
        size_t           index,
        const scid_game* game,
        const char*      flags);


    SCID_API scid_error
    scid_database_game_delete(
        scid_database* database,
        size_t         index);


    SCID_API scid_error
    scid_database_game_undelete(
        scid_database* database,
        size_t         index);


    SCID_API scid_error
    scid_database_game_get(
        const scid_database* database,
        size_t               index,
        scid_game**          out_game,
        char*                out_flags,
        size_t               out_flags_capacity,
        size_t*              out_flags_size);


    SCID_API scid_error
    scid_database_game_export_pgn(
        const scid_database* database,
        size_t               index,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    SCID_API scid_error
    scid_database_game_tag_get(
        const scid_database* database,
        size_t               index,
        const char*          name,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    SCID_API scid_error
    scid_database_game_halfmove_count_get(
        const scid_database* database,
        size_t               index,
        size_t*              out_count);


    SCID_API scid_error
    scid_database_game_number_get(
        const scid_database* database,
        size_t               index,
        size_t*              out_number);


    SCID_API scid_error
    scid_database_game_deleted_get(
        const scid_database* database,
        size_t               index,
        int*                 out_deleted);


    SCID_API scid_error
    scid_database_game_result_get(
        const scid_database* database,
        size_t               index,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    SCID_API scid_error
    scid_database_game_eco_get(
        const scid_database* database,
        size_t               index,
        scid_eco_code*       out_code);


    SCID_API scid_error
    scid_database_game_date_get(
        const scid_database* database,
        size_t               index,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);

#ifdef __cplusplus
}
#endif

#endif
