/**
 * @file database_search.h
 * @brief High-performance chess database query engine, header criteria, board matching, and search execution.
 */

#ifndef SCID_DATABASE_SEARCH_H
#define SCID_DATABASE_SEARCH_H

#include "scid/_platform.h"
#include "scid/database_filter.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup database_search Database Search Engine
     * @brief Multi-criteria header filters, position and material pattern queries, board configuration matching, and search execution.
     * @{
     */

    /**
     * @name Board Search Match Modes & Criteria Types
     * @brief Match algorithms and criteria handles for database searches.
     * @{
     */

    /**
     * @brief Forward declaration of database handle.
     * @see scid_database
     */
    typedef struct scid_database scid_database;

    /**
     * @brief Board search matching mode specifying the strictness of position comparison.
     */
    typedef int scid_board_search_match;

    /**
     * @brief Matching algorithms for board configuration search.
     */
    enum
    {
        /** @brief Exact piece placement: all pieces and pawns on identical squares. */
        SCID_BOARD_SEARCH_MATCH_EXACT = 0,

        /** @brief Pawn structure and material: exact pawn placement and matching overall piece balance. */
        SCID_BOARD_SEARCH_MATCH_PAWNS = 1,

        /** @brief File configurations: matching piece count per vertical file. */
        SCID_BOARD_SEARCH_MATCH_FILES = 2
    };

    /**
     * @brief Function pointer type for long-running asynchronous progress notifications.
     *
     * @param[in] done      Count of items completed so far (e.g. games examined).
     * @param[in] total     Total expected item count, or `0` if unknown.
     * @param[in] message   Optional descriptive phase message (may be NULL).
     * @param[in] user_data User context pointer passed to the search function.
     */
    typedef void (*scid_progress_report_callback)(
        size_t      done,
        size_t      total,
        const char* message,
        void*       user_data);

    /**
     * @brief Function pointer type for querying early cooperative cancellation of search execution.
     *
     * @param[in] user_data User context pointer passed to the search function.
     *
     * @return Non-zero (`1`) to request immediate cooperative abort; zero (`0`) to proceed.
     */
    typedef int (*scid_should_cancel_fn)(void* user_data);

    /**
     * @brief Opaque handle representing game header search criteria.
     *
     * Encapsulates text matchers (player names, events, sites), date/ECO ranges, rating limits,
     * game lengths, results, and structural movetext flags (has comments, variations, NAGs).
     *
     * @see scid_search_header_criteria_create()
     * @see scid_search_header_criteria_free()
     */
    typedef struct scid_search_header_criteria scid_search_header_criteria;

    /**
     * @brief Opaque handle representing board pattern and position search criteria.
     *
     * Encapsulates target board layout, match strictness, variation traversal, and color-flipped matching.
     *
     * @see scid_search_board_criteria_create()
     * @see scid_search_board_criteria_free()
     */
    typedef struct scid_search_board_criteria scid_search_board_criteria;

    /** @} */


    /**
     * @name Header Search Criteria
     * @brief Configuration getters and setters for header metadata queries.
     * @{
     */

    /**
     * @brief Allocates and initialises a new header search criteria handle with default (unrestricted) values.
     *
     * @param[out] out_criteria Pointer receiving the newly allocated criteria handle. Must not be NULL.
     *
     * @retval SCID_OK           Handle allocated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p out_criteria is NULL.
     *
     * @note The caller acquires ownership of @p out_criteria and must release it with @ref scid_search_header_criteria_free().
     */
    SCID_API scid_error
    scid_search_header_criteria_create(scid_search_header_criteria** out_criteria);


    /**
     * @brief Releases a header search criteria handle.
     *
     * @param[in,out] criteria Pointer to the criteria handle to release. If NULL, this function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_search_header_criteria_free(scid_search_header_criteria* criteria);


    /**
     * @brief Sets a player name filter matching either White or Black.
     *
     * @param[in,out] criteria Pointer to the header criteria handle. Must not be NULL.
     * @param[in]     player   Player substring pattern (or empty string/NULL to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_player_set(
        scid_search_header_criteria* criteria,
        const char*                  player);


    /**
     * @brief Retrieves the current player name filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the player filter string.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_player_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the White player name substring filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     white    White player substring (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_white_set(
        scid_search_header_criteria* criteria,
        const char*                  white);


    /**
     * @brief Retrieves the current White player name filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the filter string.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_white_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the Black player name substring filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     black    Black player substring (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_black_set(
        scid_search_header_criteria* criteria,
        const char*                  black);


    /**
     * @brief Retrieves the current Black player name filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the filter string.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_black_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the Event name substring filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     event    Event substring (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_event_set(
        scid_search_header_criteria* criteria,
        const char*                  event);


    /**
     * @brief Retrieves the current Event name filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the filter string.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_event_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the Site location substring filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     site     Site substring (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_site_set(
        scid_search_header_criteria* criteria,
        const char*                  site);


    /**
     * @brief Retrieves the current Site location filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the filter string.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_site_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the Site country code filter.
     *
     * @param[in,out] criteria     Pointer to the header criteria. Must not be NULL.
     * @param[in]     site_country ISO/IOC country code string (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_site_country_set(
        scid_search_header_criteria* criteria,
        const char*                  site_country);


    /**
     * @brief Retrieves the current Site country code filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the country filter string.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_site_country_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the Round number/identifier filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     round    Round string (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_round_set(
        scid_search_header_criteria* criteria,
        const char*                  round);


    /**
     * @brief Retrieves the current Round filter.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the round filter string.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_round_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the played date range filter (`"YYYY.MM.DD"`).
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     date_min Minimum date (or NULL/`""` for open lower bound).
     * @param[in]     date_max Maximum date (or NULL/`""` for open upper bound).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_date_range_set(
        scid_search_header_criteria* criteria,
        const char*                  date_min,
        const char*                  date_max);


    /**
     * @brief Retrieves the current played date range filter.
     *
     * @param[in]  criteria               Pointer to the header criteria. Must not be NULL.
     * @param[out] out_date_min           Caller-allocated buffer receiving the minimum date string.
     * @param[in]  out_date_min_capacity  Capacity of @p out_date_min in bytes.
     * @param[out] out_date_min_size      Pointer receiving min date bytes written. Must not be NULL.
     * @param[out] out_date_max           Caller-allocated buffer receiving the maximum date string.
     * @param[in]  out_date_max_capacity  Capacity of @p out_date_max in bytes.
     * @param[out] out_date_max_size      Pointer receiving max date bytes written. Must not be NULL.
     *
     * @retval SCID_OK               Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If any mandatory pointer is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If either buffer capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_date_range_get(
        const scid_search_header_criteria* criteria,
        char*                              out_date_min,
        size_t                             out_date_min_capacity,
        size_t*                            out_date_min_size,
        char*                              out_date_max,
        size_t                             out_date_max_capacity,
        size_t*                            out_date_max_size);


    /**
     * @brief Sets the event date range filter (`"YYYY.MM.DD"`).
     *
     * @param[in,out] criteria       Pointer to the header criteria. Must not be NULL.
     * @param[in]     event_date_min Minimum event date (or NULL/`""` for open lower bound).
     * @param[in]     event_date_max Maximum event date (or NULL/`""` for open upper bound).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_event_date_range_set(
        scid_search_header_criteria* criteria,
        const char*                  event_date_min,
        const char*                  event_date_max);


    /**
     * @brief Retrieves the current event date range filter.
     *
     * @param[in]  criteria                     Pointer to the header criteria. Must not be NULL.
     * @param[out] out_event_date_min           Caller-allocated buffer receiving the minimum event date.
     * @param[in]  out_event_date_min_capacity  Capacity of @p out_event_date_min in bytes.
     * @param[out] out_event_date_min_size      Pointer receiving min event date bytes written. Must not be NULL.
     * @param[out] out_event_date_max           Caller-allocated buffer receiving the maximum event date.
     * @param[in]  out_event_date_max_capacity  Capacity of @p out_event_date_max in bytes.
     * @param[out] out_event_date_max_size      Pointer receiving max event date bytes written. Must not be NULL.
     *
     * @retval SCID_OK               Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If any mandatory pointer is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If either buffer capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_event_date_range_get(
        const scid_search_header_criteria* criteria,
        char*                              out_event_date_min,
        size_t                             out_event_date_min_capacity,
        size_t*                            out_event_date_min_size,
        char*                              out_event_date_max,
        size_t                             out_event_date_max_capacity,
        size_t*                            out_event_date_max_size);


    /**
     * @brief Sets the ECO code classification range filter (e.g. `"B00"` to `"B99"`).
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     eco_min  Minimum ECO code string (or NULL/`""` for `"A00"`).
     * @param[in]     eco_max  Maximum ECO code string (or NULL/`""` for `"E99"`).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_eco_range_set(
        scid_search_header_criteria* criteria,
        const char*                  eco_min,
        const char*                  eco_max);


    /**
     * @brief Retrieves the current ECO code range filter.
     *
     * @param[in]  criteria             Pointer to the header criteria. Must not be NULL.
     * @param[out] out_eco_min          Caller-allocated buffer receiving the minimum ECO code.
     * @param[in]  out_eco_min_capacity Capacity of @p out_eco_min in bytes.
     * @param[out] out_eco_min_size     Pointer receiving min ECO bytes written. Must not be NULL.
     * @param[out] out_eco_max          Caller-allocated buffer receiving the maximum ECO code.
     * @param[in]  out_eco_max_capacity Capacity of @p out_eco_max in bytes.
     * @param[out] out_eco_max_size     Pointer receiving max ECO bytes written. Must not be NULL.
     *
     * @retval SCID_OK               Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If any mandatory pointer is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If either buffer capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_eco_range_get(
        const scid_search_header_criteria* criteria,
        char*                              out_eco_min,
        size_t                             out_eco_min_capacity,
        size_t*                            out_eco_min_size,
        char*                              out_eco_max,
        size_t                             out_eco_max_capacity,
        size_t*                            out_eco_max_size);


    /**
     * @brief Sets the outcome result filter string (`"1-0"`, `"0-1"`, `"1/2-1/2"`, `"="`, `"*"`, or combinations).
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     result   Result pattern string (or NULL/`""` to clear).
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_result_set(
        scid_search_header_criteria* criteria,
        const char*                  result);


    /**
     * @brief Retrieves the current outcome result filter string.
     *
     * @param[in]  criteria          Pointer to the header criteria. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the result filter.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Filter string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p criteria or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_search_header_criteria_result_get(
        const scid_search_header_criteria* criteria,
        char*                              out_text,
        size_t                             out_text_capacity,
        size_t*                            out_text_size);


    /**
     * @brief Sets the 1-based sequential game number range filter (`min..max`).
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     min      Minimum 1-based game number (`0` or `1` for unrestricted start).
     * @param[in]     max      Maximum 1-based game number (`0` for unrestricted end).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_game_number_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    /**
     * @brief Retrieves the current game number range filter.
     *
     * @param[in]  criteria Pointer to the header criteria. Must not be NULL.
     * @param[out] out_min  Pointer receiving the minimum game number. Must not be NULL.
     * @param[out] out_max  Pointer receiving the maximum game number. Must not be NULL.
     *
     * @retval SCID_OK           Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_game_number_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    /**
     * @brief Sets the game length (halfmove/ply count) range filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     min      Minimum halfmove count (`0` for unrestricted start).
     * @param[in]     max      Maximum halfmove count (`0` for unrestricted end).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_halfmove_count_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    /**
     * @brief Retrieves the current game length range filter.
     *
     * @param[in]  criteria Pointer to the header criteria. Must not be NULL.
     * @param[out] out_min  Pointer receiving the minimum halfmove count. Must not be NULL.
     * @param[out] out_max  Pointer receiving the maximum halfmove count. Must not be NULL.
     *
     * @retval SCID_OK           Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_halfmove_count_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    /**
     * @brief Sets the White player Elo rating range filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     min      Minimum Elo rating (`0` for unrestricted start).
     * @param[in]     max      Maximum Elo rating (`0` for unrestricted end).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_white_elo_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    /**
     * @brief Retrieves the current White Elo rating range filter.
     *
     * @param[in]  criteria Pointer to the header criteria. Must not be NULL.
     * @param[out] out_min  Pointer receiving the minimum White Elo. Must not be NULL.
     * @param[out] out_max  Pointer receiving the maximum White Elo. Must not be NULL.
     *
     * @retval SCID_OK           Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_white_elo_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    /**
     * @brief Sets the Black player Elo rating range filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     min      Minimum Elo rating (`0` for unrestricted start).
     * @param[in]     max      Maximum Elo rating (`0` for unrestricted end).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_black_elo_range_set(
        scid_search_header_criteria* criteria,
        size_t                       min,
        size_t                       max);


    /**
     * @brief Retrieves the current Black Elo rating range filter.
     *
     * @param[in]  criteria Pointer to the header criteria. Must not be NULL.
     * @param[out] out_min  Pointer receiving the minimum Black Elo. Must not be NULL.
     * @param[out] out_max  Pointer receiving the maximum Black Elo. Must not be NULL.
     *
     * @retval SCID_OK           Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_black_elo_range_get(
        const scid_search_header_criteria* criteria,
        size_t*                            out_min,
        size_t*                            out_max);


    /**
     * @brief Sets the Elo difference (`White Elo - Black Elo`) range filter.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     min      Minimum Elo difference (signed integer, e.g. `-400`).
     * @param[in]     max      Maximum Elo difference (signed integer, e.g. `+400`).
     *
     * @retval SCID_OK           Range updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_elo_difference_range_set(
        scid_search_header_criteria* criteria,
        int                          min,
        int                          max);


    /**
     * @brief Retrieves the current Elo difference range filter.
     *
     * @param[in]  criteria Pointer to the header criteria. Must not be NULL.
     * @param[out] out_min  Pointer receiving the minimum Elo difference. Must not be NULL.
     * @param[out] out_max  Pointer receiving the maximum Elo difference. Must not be NULL.
     *
     * @retval SCID_OK           Range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_elo_difference_range_get(
        const scid_search_header_criteria* criteria,
        int*                               out_min,
        int*                               out_max);


    /**
     * @brief Configures the requirement for games containing alternative move variations.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     enabled  `1` to require variations; `0` to exclude this requirement.
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_has_variations_set(
        scid_search_header_criteria* criteria,
        int                          enabled);


    /**
     * @brief Checks if games containing variations are required.
     *
     * @param[in]  criteria    Pointer to the header criteria. Must not be NULL.
     * @param[out] out_enabled Pointer receiving non-zero (`1`) if required; zero (`0`) otherwise.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_enabled is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_has_variations_get(
        const scid_search_header_criteria* criteria,
        int*                               out_enabled);


    /**
     * @brief Configures the requirement for games containing text commentary / annotations.
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     enabled  `1` to require comments; `0` to exclude this requirement.
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_has_comments_set(
        scid_search_header_criteria* criteria,
        int                          enabled);


    /**
     * @brief Checks if games containing text commentary are required.
     *
     * @param[in]  criteria    Pointer to the header criteria. Must not be NULL.
     * @param[out] out_enabled Pointer receiving non-zero (`1`) if required; zero (`0`) otherwise.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_enabled is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_has_comments_get(
        const scid_search_header_criteria* criteria,
        int*                               out_enabled);


    /**
     * @brief Configures the requirement for games containing Numeric Annotation Glyphs (NAGs).
     *
     * @param[in,out] criteria Pointer to the header criteria. Must not be NULL.
     * @param[in]     enabled  `1` to require NAGs; `0` to exclude this requirement.
     *
     * @retval SCID_OK           Filter updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_has_nags_set(
        scid_search_header_criteria* criteria,
        int                          enabled);


    /**
     * @brief Checks if games containing NAG annotations are required.
     *
     * @param[in]  criteria    Pointer to the header criteria. Must not be NULL.
     * @param[out] out_enabled Pointer receiving non-zero (`1`) if required; zero (`0`) otherwise.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_enabled is NULL.
     */
    SCID_API scid_error
    scid_search_header_criteria_has_nags_get(
        const scid_search_header_criteria* criteria,
        int*                               out_enabled);

    /** @} */


    /**
     * @name Board & Position Search Criteria
     * @brief Configuration getters and setters for piece placement, pawn structure, and material queries.
     * @{
     */

    /**
     * @brief Allocates and initialises a new board search criteria handle with default values.
     *
     * @param[out] out_criteria Pointer receiving the newly allocated criteria handle. Must not be NULL.
     *
     * @retval SCID_OK           Handle allocated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p out_criteria is NULL.
     *
     * @note The caller acquires ownership of @p out_criteria and must release it with @ref scid_search_board_criteria_free().
     */
    SCID_API scid_error
    scid_search_board_criteria_create(scid_search_board_criteria** out_criteria);


    /**
     * @brief Releases a board search criteria handle.
     *
     * @param[in,out] criteria Pointer to the criteria handle to release. If NULL, this function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_search_board_criteria_free(scid_search_board_criteria* criteria);


    /**
     * @brief Sets the target chess position pattern to search for.
     *
     * @param[in,out] criteria Pointer to the board criteria. Must not be NULL.
     * @param[in]     position Pointer to the target position entity. Must not be NULL.
     *
     * @retval SCID_OK           Position pattern updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p position is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_position_set(
        scid_search_board_criteria* criteria,
        const scid_position*        position);


    /**
     * @brief Retrieves the target position pattern configured in the board criteria.
     *
     * @param[in]  criteria     Pointer to the board criteria. Must not be NULL.
     * @param[out] out_position Pointer to an allocated @ref scid_position receiving the copied state.
     *                          Must not be NULL.
     *
     * @retval SCID_OK           Position copied successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_position is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_position_get(
        const scid_search_board_criteria* criteria,
        scid_position*                    out_position);


    /**
     * @brief Configures the matching algorithm strictness (@ref scid_board_search_match).
     *
     * @param[in,out] criteria Pointer to the board criteria. Must not be NULL.
     * @param[in]     match    Matching mode (@ref SCID_BOARD_SEARCH_MATCH_EXACT,
     *                         @ref SCID_BOARD_SEARCH_MATCH_PAWNS, or @ref SCID_BOARD_SEARCH_MATCH_FILES).
     *
     * @retval SCID_OK           Match mode updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL or @p match is invalid.
     */
    SCID_API scid_error
    scid_search_board_criteria_match_set(
        scid_search_board_criteria* criteria,
        scid_board_search_match     match);


    /**
     * @brief Retrieves the current board matching mode.
     *
     * @param[in]  criteria  Pointer to the board criteria. Must not be NULL.
     * @param[out] out_match Pointer receiving the @ref scid_board_search_match value. Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_match is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_match_get(
        const scid_search_board_criteria* criteria,
        scid_board_search_match*          out_match);


    /**
     * @brief Configures whether position searches traverse sub-variations in addition to the mainline.
     *
     * @param[in,out] criteria Pointer to the board criteria. Must not be NULL.
     * @param[in]     enabled  `1` to search inside variations; `0` for mainline only.
     *
     * @retval SCID_OK           Setting updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_include_variations_set(
        scid_search_board_criteria* criteria,
        int                         enabled);


    /**
     * @brief Checks if variation traversal is enabled for board search.
     *
     * @param[in]  criteria    Pointer to the board criteria. Must not be NULL.
     * @param[out] out_enabled Pointer receiving non-zero (`1`) if enabled; zero (`0`) otherwise.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_enabled is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_include_variations_get(
        const scid_search_board_criteria* criteria,
        int*                              out_enabled);


    /**
     * @brief Configures whether board search matches horizontally/vertically flipped mirror configurations.
     *
     * @param[in,out] criteria Pointer to the board criteria. Must not be NULL.
     * @param[in]     enabled  `1` to match flipped board configurations; `0` for standard orientation only.
     *
     * @retval SCID_OK           Setting updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_include_flipped_set(
        scid_search_board_criteria* criteria,
        int                         enabled);


    /**
     * @brief Checks if flipped mirror matching is enabled for board search.
     *
     * @param[in]  criteria    Pointer to the board criteria. Must not be NULL.
     * @param[out] out_enabled Pointer receiving non-zero (`1`) if enabled; zero (`0`) otherwise.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p criteria or @p out_enabled is NULL.
     */
    SCID_API scid_error
    scid_search_board_criteria_include_flipped_get(
        const scid_search_board_criteria* criteria,
        int*                              out_enabled);

    /** @} */


    /**
     * @name Search Execution
     * @brief Running database search queries across header indices and game movetexts.
     * @{
     */

    /**
     * @brief Executes a header metadata search over games in a source filter, populating a destination filter.
     *
     * @param[in,out] database                  Pointer to the open database. Must not be NULL.
     * @param[in]     source_filter_id          Filter ID restricting games to scan (@ref scid_filter_id).
     * @param[in]     destination_filter_id     Filter ID to receive matching game indices.
     * @param[in]     criteria                  Pointer to configured header search criteria. Must not be NULL.
     * @param[in]     progress_report           Optional callback for progress notifications (may be NULL).
     * @param[in]     progress_report_user_data Context pointer passed to @p progress_report.
     * @param[in]     should_cancel             Optional cancellation callback (may be NULL).
     * @param[in]     should_cancel_user_data  Context pointer passed to @p should_cancel.
     *
     * @retval SCID_OK           Search executed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p criteria is NULL, or filter IDs are invalid.
     * @retval SCID_ERROR_CANCEL If search was aborted early by @p should_cancel.
     */
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


    /**
     * @brief Searches the database for occurrences of an exact board position state.
     *
     * @param[in,out] database                  Pointer to the open database. Must not be NULL.
     * @param[in]     source_filter_id          Filter ID restricting games to scan.
     * @param[in]     destination_filter_id     Filter ID to receive matching game indices.
     * @param[in]     position                  Target chess position to match. Must not be NULL.
     * @param[in]     progress_report           Optional progress callback (may be NULL).
     * @param[in]     progress_report_user_data Context pointer passed to @p progress_report.
     * @param[in]     should_cancel             Optional cancellation callback (may be NULL).
     * @param[in]     should_cancel_user_data  Context pointer passed to @p should_cancel.
     *
     * @retval SCID_OK           Search executed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p position is NULL, or filter IDs are invalid.
     * @retval SCID_ERROR_CANCEL If search was aborted early by @p should_cancel.
     */
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


    /**
     * @brief Executes a board pattern / pawn / file configuration search across games in the database.
     *
     * @param[in,out] database                  Pointer to the open database. Must not be NULL.
     * @param[in]     source_filter_id          Filter ID restricting games to scan.
     * @param[in]     destination_filter_id     Filter ID to receive matching game indices.
     * @param[in]     criteria                  Configured board search criteria. Must not be NULL.
     * @param[in]     progress_report           Optional progress callback (may be NULL).
     * @param[in]     progress_report_user_data Context pointer passed to @p progress_report.
     * @param[in]     should_cancel             Optional cancellation callback (may be NULL).
     * @param[in]     should_cancel_user_data  Context pointer passed to @p should_cancel.
     *
     * @retval SCID_OK           Search executed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p criteria is NULL, or filter IDs are invalid.
     * @retval SCID_ERROR_CANCEL If search was aborted early by @p should_cancel.
     */
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

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
