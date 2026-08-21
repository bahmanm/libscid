#ifndef SCID_DATABASE_SEARCH_H
#define SCID_DATABASE_SEARCH_H

#include "scid/_platform.h"
#include "scid/database_filter.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_database scid_database;

    typedef int scid_board_search_match;

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

    typedef struct scid_search_header_criteria scid_search_header_criteria;

    typedef struct scid_search_board_criteria scid_search_board_criteria;


    SCID_API scid_error
    scid_search_header_criteria_create(scid_search_header_criteria** out_criteria);


    SCID_API void
    scid_search_header_criteria_free(scid_search_header_criteria* criteria);


    SCID_API scid_error
    scid_search_header_criteria_player_set(
        scid_search_header_criteria* criteria,
        const char*                  player);


    SCID_API scid_error
    scid_search_header_criteria_player_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_white_set(
        scid_search_header_criteria* criteria,
        const char*                  white);


    SCID_API scid_error
    scid_search_header_criteria_white_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_black_set(
        scid_search_header_criteria* criteria,
        const char*                  black);


    SCID_API scid_error
    scid_search_header_criteria_black_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_event_set(
        scid_search_header_criteria* criteria,
        const char*                  event);


    SCID_API scid_error
    scid_search_header_criteria_event_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_site_set(
        scid_search_header_criteria* criteria,
        const char*                  site);


    SCID_API scid_error
    scid_search_header_criteria_site_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_site_country_set(
        scid_search_header_criteria* criteria,
        const char*                  site_country);


    SCID_API scid_error
    scid_search_header_criteria_site_country_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_round_set(
        scid_search_header_criteria* criteria,
        const char*                  round);


    SCID_API scid_error
    scid_search_header_criteria_round_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_date_range_set(
        scid_search_header_criteria* criteria,
        const char*                  date_min,
        const char*                  date_max);


    SCID_API scid_error
    scid_search_header_criteria_date_range_get(
        const scid_search_header_criteria* criteria,
        char*                              out_date_min,
        size_t                             out_date_min_capacity,
        size_t*                            out_date_min_size,
        char*                              out_date_max,
        size_t                             out_date_max_capacity,
        size_t*                            out_date_max_size);


    SCID_API scid_error
    scid_search_header_criteria_event_date_range_set(
        scid_search_header_criteria* criteria,
        const char*                  event_date_min,
        const char*                  event_date_max);


    SCID_API scid_error
    scid_search_header_criteria_event_date_range_get(
        const scid_search_header_criteria* criteria,
        char*                              out_event_date_min,
        size_t                             out_event_date_min_capacity,
        size_t*                            out_event_date_min_size,
        char*                              out_event_date_max,
        size_t                             out_event_date_max_capacity,
        size_t*                            out_event_date_max_size);


    SCID_API scid_error
    scid_search_header_criteria_eco_range_set(
        scid_search_header_criteria* criteria,
        const char*                  eco_min,
        const char*                  eco_max);


    SCID_API scid_error
    scid_search_header_criteria_eco_range_get(
        const scid_search_header_criteria* criteria,
        char*                              out_eco_min,
        size_t                             out_eco_min_capacity,
        size_t*                            out_eco_min_size,
        char*                              out_eco_max,
        size_t                             out_eco_max_capacity,
        size_t*                            out_eco_max_size);


    SCID_API scid_error
    scid_search_header_criteria_result_set(
        scid_search_header_criteria* criteria,
        const char*                  result);


    SCID_API scid_error
    scid_search_header_criteria_result_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    SCID_API scid_error
    scid_search_header_criteria_game_number_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    SCID_API scid_error
    scid_search_header_criteria_game_number_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    SCID_API scid_error
    scid_search_header_criteria_halfmove_count_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    SCID_API scid_error
    scid_search_header_criteria_halfmove_count_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    SCID_API scid_error
    scid_search_header_criteria_white_elo_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    SCID_API scid_error
    scid_search_header_criteria_white_elo_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    SCID_API scid_error
    scid_search_header_criteria_black_elo_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    SCID_API scid_error
    scid_search_header_criteria_black_elo_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    SCID_API scid_error
    scid_search_header_criteria_elo_difference_range_set(
        scid_search_header_criteria* criteria,
        int                          min,
        int                          max);


    SCID_API scid_error
    scid_search_header_criteria_elo_difference_range_get(
        const scid_search_header_criteria* criteria,
        int*                               out_min,
        int*                               out_max);


    SCID_API scid_error
    scid_search_header_criteria_has_variations_set(
        scid_search_header_criteria* criteria,
        int                          enabled);


    SCID_API scid_error
    scid_search_header_criteria_has_variations_get(
        const scid_search_header_criteria* criteria,
        int*                               out_enabled);


    SCID_API scid_error
    scid_search_header_criteria_has_comments_set(
        scid_search_header_criteria* criteria,
        int                          enabled);


    SCID_API scid_error
    scid_search_header_criteria_has_comments_get(
        const scid_search_header_criteria* criteria,
        int*                               out_enabled);


    SCID_API scid_error
    scid_search_header_criteria_has_nags_set(
        scid_search_header_criteria* criteria,
        int                          enabled);


    SCID_API scid_error
    scid_search_header_criteria_has_nags_get(
        const scid_search_header_criteria* criteria,
        int*                               out_enabled);


    SCID_API scid_error
    scid_search_board_criteria_create(scid_search_board_criteria** out_criteria);


    SCID_API void
    scid_search_board_criteria_free(scid_search_board_criteria* criteria);


    SCID_API scid_error
    scid_search_board_criteria_position_set(
        scid_search_board_criteria* criteria,
        const scid_position*        position);


    SCID_API scid_error
    scid_search_board_criteria_position_get(
        const scid_search_board_criteria* criteria,
        scid_position*                    out_position);


    SCID_API scid_error
    scid_search_board_criteria_match_set(
        scid_search_board_criteria* criteria,
        scid_board_search_match     match);


    SCID_API scid_error
    scid_search_board_criteria_match_get(
        const scid_search_board_criteria* criteria,
        scid_board_search_match*          out_match);


    SCID_API scid_error
    scid_search_board_criteria_include_variations_set(
        scid_search_board_criteria* criteria,
        int                         enabled);


    SCID_API scid_error
    scid_search_board_criteria_include_variations_get(
        const scid_search_board_criteria* criteria,
        int*                              out_enabled);


    SCID_API scid_error
    scid_search_board_criteria_include_flipped_set(
        scid_search_board_criteria* criteria,
        int                         enabled);


    SCID_API scid_error
    scid_search_board_criteria_include_flipped_get(
        const scid_search_board_criteria* criteria,
        int*                              out_enabled);


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

#ifdef __cplusplus
}
#endif

#endif
