/**
 * @file database_filter.h
 * @brief Database game filters, subset views, and sorted index-to-row mapping.
 */

#ifndef SCID_DATABASE_FILTER_H
#define SCID_DATABASE_FILTER_H

#include "scid/_platform.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup database_filter Database Filters & Sorting
     * @brief Dynamic game filtering, selection bitsets, multi-criteria sorting, and paginated row mapping.
     * @{
     */

    /**
     * @name Filter Identifiers & Special Constants
     * @brief Handle identifiers and predefined filter IDs for database game subsets.
     * @{
     */

    /**
     * @brief Forward declaration of database handle.
     * @see scid_database
     */
    typedef struct scid_database scid_database;

    /**
     * @brief Numeric identifier for a database game filter / subset view.
     *
     * User-created filters have positive integer identifiers (`> 0`). Predefined pseudo-filters
     * use negative sentinel values (@ref SCID_FILTER_ALL_GAMES, @ref SCID_FILTER_PRIMARY).
     */
    typedef int scid_filter_id;

    /**
     * @brief Special filter constants for universal and primary database views.
     */
    enum
    {
        /** @brief Pseudo-filter matching all games in the database without exclusion. */
        SCID_FILTER_ALL_GAMES = -1,

        /** @brief Primary default filter used for interactive search results and active selections. */
        SCID_FILTER_PRIMARY = -2
    };

    /** @} */


    /**
     * @name Filter Lifecycle
     * @brief Allocation and destruction of custom user filters.
     * @{
     */

    /**
     * @brief Allocates and registers a new, empty user filter within the database.
     *
     * @param[in,out] database      Pointer to the open database. Must not be NULL.
     * @param[out]    out_filter_id Pointer receiving the newly assigned positive filter ID. Must not be NULL.
     *
     * @retval SCID_OK           Filter created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_filter_id is NULL, or @p database is closed.
     *
     * @see scid_database_filter_delete()
     */
    SCID_API scid_error
    scid_database_filter_create(
        scid_database*  database,
        scid_filter_id* out_filter_id);


    /**
     * @brief Deletes a previously created user filter and releases its resources.
     *
     * Predefined filters (@ref SCID_FILTER_ALL_GAMES, @ref SCID_FILTER_PRIMARY) cannot be deleted.
     *
     * @param[in,out] database  Pointer to the open database. Must not be NULL.
     * @param[in]     filter_id Identifier of the user filter to delete (`> 0`).
     *
     * @retval SCID_OK           Filter deleted successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database is NULL, @p filter_id is invalid (<= 0), or filter is not found.
     *
     * @see scid_database_filter_create()
     */
    SCID_API scid_error
    scid_database_filter_delete(
        scid_database* database,
        scid_filter_id filter_id);

    /** @} */


    /**
     * @name Filter Membership & Sorted Game Retrieval
     * @brief Filter match counting, sorted pagination, and row-to-game coordinate translation.
     * @{
     */

    /**
     * @brief Retrieves the number of games currently matched by the filter.
     *
     * @param[in]  database  Pointer to the database. Must not be NULL.
     * @param[in]  filter_id Filter identifier to query (@ref scid_filter_id).
     * @param[out] out_count Pointer receiving the matched game count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_count is NULL, or @p filter_id is invalid.
     */
    SCID_API scid_error
    scid_database_filter_game_count_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        size_t*              out_count);


    /**
     * @brief Retrieves a contiguous window of sorted game indices matching a filter.
     *
     * Allows paginated display of database views according to custom sort criteria
     * (e.g. `"Date"`, `"White"`, `"Black"`, `"ECO"`, `"Rating"`, `"Result"`).
     *
     * @param[in]  database                  Pointer to the database. Must not be NULL.
     * @param[in]  filter_id                 Filter identifier to query.
     * @param[in]  sort_criteria             Sort criteria string (e.g. `""`, `"Date"`, `"White"`, `"ECO"`). Must not be NULL.
     * @param[in]  start_row                 Zero-based starting row in the sorted filter view.
     * @param[in]  row_count                 Requested number of rows to retrieve.
     * @param[out] out_game_indices          Caller-allocated array receiving the 0-based database game indices.
     *                                       Must hold at least @p row_count elements.
     * @param[in]  out_game_indices_capacity Capacity of @p out_game_indices in elements.
     * @param[out] out_game_indices_count    Pointer receiving the actual number of indices written. Must not be NULL.
     *
     * @retval SCID_OK               Game indices retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If any mandatory pointer is NULL, or @p filter_id is invalid.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_game_indices_capacity is less than @p row_count.
     */
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


    /**
     * @brief Translates a sorted display row index to its underlying database game index.
     *
     * @param[in]  database       Pointer to the database. Must not be NULL.
     * @param[in]  filter_id      Filter identifier to query.
     * @param[in]  sort_criteria  Sort criteria string. Must not be NULL.
     * @param[in]  row            Zero-based sorted row position to look up.
     * @param[out] out_game_index Pointer receiving the 0-based database game index. Must not be NULL.
     *
     * @retval SCID_OK           Translation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL, or @p row is out of range.
     *
     * @see scid_database_filter_game_row_for_index_get()
     */
    SCID_API scid_error
    scid_database_filter_game_index_at_row_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        const char*          sort_criteria,
        size_t               row,
        size_t*              out_game_index);


    /**
     * @brief Translates a database game index to its display row position under the sorted filter.
     *
     * @param[in]  database     Pointer to the database. Must not be NULL.
     * @param[in]  filter_id    Filter identifier to query.
     * @param[in]  sort_criteria Sort criteria string. Must not be NULL.
     * @param[in]  game_index   Zero-based database game index to look up.
     * @param[out] out_row      Pointer receiving the 0-based sorted display row position. Must not be NULL.
     *
     * @retval SCID_OK           Translation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any mandatory pointer is NULL, or @p game_index is not in the filter.
     *
     * @see scid_database_filter_game_index_at_row_get()
     */
    SCID_API scid_error
    scid_database_filter_game_row_for_index_get(
        const scid_database* database,
        scid_filter_id       filter_id,
        const char*          sort_criteria,
        size_t               game_index,
        size_t*              out_row);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
