/**
 * @file database.h
 * @brief Chess database management, persistent Scid5/PGN/memory backends, header indexing, and game
 * storage.
 */

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

    /**
     * @defgroup database Database Management
     * @brief Persistent and in-memory chess database storage, fast header indexing, game
     * serialization, and metadata.
     * @{
     */

    /**
     * @name Database Lifecycle & Storage Factories
     * @brief Creation, opening, closing, and persistence of Scid5, PGN, and in-memory database
     * handles.
     * @{
     */

    /**
     * @brief Opaque handle representing an open chess database.
     *
     * Manages a collection of indexed chess games, header caches, name dictionaries,
     * and custom metadata. Supports multiple underlying storage engines:
     * - **In-Memory**: Volatile, high-speed RAM-backed storage.
     * - **Scid 5 (.si5, .sg5, .sn5)**: High-performance binary database with fast header indices.
     * - **PGN (.pgn)**: Read-only plain-text Portable Game Notation archive with in-memory index.
     *
     * Database handles are created via factory functions (@ref scid_database_create_memory(),
     * @ref scid_database_create_scid5(), @ref scid_database_open_scid5(), etc.) and must be
     * released using @ref scid_database_free().
     *
     * @see scid_database_free()
     * @see scid_database_close()
     */
    typedef struct scid_database scid_database;


    /**
     * @brief Function pointer type for long-running asynchronous progress notifications.
     *
     * @param[in] done      Count of items completed so far (e.g. bytes parsed, games scanned).
     * @param[in] total     Total expected item count, or `0` if unknown.
     * @param[in] message   Optional descriptive phase message (may be NULL).
     * @param[in] user_data User context pointer passed to the initiating function.
     */
    typedef void (*scid_progress_report_callback)(
        size_t      done,
        size_t      total,
        const char* message,
        void*       user_data);


    /**
     * @brief Function pointer type for querying early cancellation of long-running operations.
     *
     * @param[in] user_data User context pointer passed to the initiating function.
     *
     * @return Non-zero (`1`) to request immediate cooperative cancellation; zero (`0`) to proceed.
     */
    typedef int (*scid_should_cancel_fn)(void* user_data);


    /**
     * @brief Creates a new empty in-memory chess database.
     *
     * In-memory databases provide fast transient game storage and search capabilities
     * without writing files to disk.
     *
     * @param[in]  name         Descriptive identifier for the database. Must not be NULL.
     * @param[out] out_database Pointer to a handle pointer receiving the newly allocated database.
     *                          Must not be NULL.
     *
     * @retval SCID_OK           Database created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p name or @p out_database is NULL.
     *
     * @note The caller acquires ownership of @p out_database and must release it with @ref
     * scid_database_free().
     *
     * @see scid_database_free()
     */
    SCID_API scid_error
    scid_database_create_memory(
        const char*     name,
        scid_database** out_database);


    /**
     * @brief Creates a new, empty persistent Scid 5 binary database on disk.
     *
     * Creates the triplet of Scid 5 files: `<path>.si5` (index), `<path>.sg5` (gamedata),
     * and `<path>.sn5` (name dictionary).
     *
     * @param[in]  path         Base filesystem path without extension (or with `.si5`). Must not be
     * NULL.
     * @param[out] out_database Pointer to a handle pointer receiving the newly created database
     * handle. Must not be NULL.
     *
     * @retval SCID_OK           Database created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p path or @p out_database is NULL.
     * @retval SCID_ERROR        If the files cannot be created or already exist.
     *
     * @note The caller acquires ownership of @p out_database and must release it with @ref
     * scid_database_free().
     *
     * @see scid_database_open_scid5()
     */
    SCID_API scid_error
    scid_database_create_scid5(
        const char*     path,
        scid_database** out_database);


    /**
     * @brief Opens an existing Scid 5 database for read-write operations.
     *
     * @param[in]  path                      Base filesystem path without extension (or with
     *                                       `.si5`). Must not be NULL.
     * @param[in]  progress_report           Optional callback for opening progress notifications
     *                                       (may be NULL).
     * @param[in]  progress_report_user_data Context pointer passed to @p progress_report.
     * @param[in]  should_cancel             Optional cancellation callback (may be NULL).
     * @param[in]  should_cancel_user_data   Context pointer passed to @p should_cancel.
     * @param[out] out_database              Pointer to a handle pointer receiving the opened
     *                                       database handle. Must not be NULL.
     *
     * @retval SCID_OK                Database opened successfully.
     * @retval SCID_ERROR_BAD_ARG     If @p path or @p out_database is NULL.
     * @retval SCID_ERROR             If the database files cannot be found or read.
     * @retval SCID_ERROR_USER_CANCEL If opening was cancelled by @p should_cancel.
     *
     * @note The caller acquires ownership of @p out_database and must release it with @ref
     * scid_database_free().
     *
     * @see scid_database_open_scid5_read_only()
     */
    SCID_API scid_error
    scid_database_open_scid5(
        const char*                   path,
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data,
        scid_database**               out_database);


    /**
     * @brief Opens an existing Scid 5 database in read-only mode.
     *
     * Prevents accidental modification and allows concurrent read access from multiple processes.
     *
     * @param[in]  path                      Base filesystem path without extension (or with
     *                                       `.si5`). Must not be NULL.
     * @param[in]  progress_report           Optional callback for opening progress notifications
     *                                       (may be NULL).
     * @param[in]  progress_report_user_data Context pointer passed to @p progress_report.
     * @param[in]  should_cancel             Optional cancellation callback (may be NULL).
     * @param[in]  should_cancel_user_data   Context pointer passed to @p should_cancel.
     * @param[out] out_database              Pointer to a handle pointer receiving the opened
     *                                       database handle. Must not be NULL.
     *
     * @retval SCID_OK                Database opened successfully.
     * @retval SCID_ERROR_BAD_ARG     If @p path or @p out_database is NULL.
     * @retval SCID_ERROR             If the database files cannot be found or read.
     * @retval SCID_ERROR_USER_CANCEL If opening was cancelled by @p should_cancel.
     *
     * @note The caller acquires ownership of @p out_database and must release it with @ref
     * scid_database_free().
     *
     * @see scid_database_open_scid5()
     */
    SCID_API scid_error
    scid_database_open_scid5_read_only(
        const char*                   path,
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data,
        scid_database**               out_database);


    /**
     * @brief Opens and indexes a plain-text PGN file in read-only mode.
     *
     * Parses all PGN game headers in the file to construct an in-memory index for fast
     * random access without pre-loading full movetexts into memory.
     *
     * @param[in]  path                      Filesystem path to the `.pgn` file. Must not be NULL.
     * @param[in]  progress_report           Optional callback for indexing progress notifications
     * (may be NULL).
     * @param[in]  progress_report_user_data Context pointer passed to @p progress_report.
     * @param[in]  should_cancel             Optional cancellation callback (may be NULL).
     * @param[in]  should_cancel_user_data  Context pointer passed to @p should_cancel.
     * @param[out] out_database              Pointer to a handle pointer receiving the opened
     * database handle. Must not be NULL.
     *
     * @retval SCID_OK           Database opened and indexed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p path or @p out_database is NULL.
     * @retval SCID_ERROR        If the file cannot be opened or parsed.
     * @retval SCID_ERROR_CANCEL If indexing was cancelled by @p should_cancel.
     *
     * @note The caller acquires ownership of @p out_database and must release it with @ref
     * scid_database_free().
     */
    SCID_API scid_error
    scid_database_open_pgn_read_only(
        const char*                   path,
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data,
        scid_database**               out_database);


    /**
     * @brief Flushes unwritten changes and closes all underlying storage files.
     *
     * @param[in,out] database Pointer to the database handle. Must not be NULL.
     *
     * @retval SCID_OK           Database closed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database is NULL.
     *
     * @see scid_database_free()
     * @see scid_database_is_open()
     */
    SCID_API scid_error
    scid_database_close(scid_database* database);


    /**
     * @brief Releases a database handle, closing underlying files if still open.
     *
     * @param[in,out] database Pointer to the database handle to release. If NULL, this
     *                         function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_database_free(scid_database* database);


    /**
     * @brief Checks whether the database is currently open and accessible.
     *
     * @param[in]  database    Pointer to the database handle. Must not be NULL.
     * @param[out] out_is_open Pointer receiving non-zero (`1`) if open; zero (`0`) if closed.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_is_open is NULL.
     */
    SCID_API scid_error
    scid_database_is_open(
        const scid_database* database,
        int*                 out_is_open);


    /**
     * @brief Explicitly persists pending database modifications, header index updates, and name
     * caches to disk.
     *
     * @param[in,out] database Pointer to the database to save. Must not be NULL.
     *
     * @retval SCID_OK           Database saved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database is NULL.
     * @retval SCID_ERROR        If the database is read-only or an I/O error occurs.
     */
    SCID_API scid_error
    scid_database_save(scid_database* database);

    /** @} */


    /**
     * @name Database Properties & Metadata
     * @brief Inspection of database backend type, file paths, read-only status, and custom
     * key-value metadata.
     * @{
     */

    /**
     * @brief Retrieves the filesystem path or name of the database.
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated filename
     * string. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               Filename retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_database_filename_get(
        const scid_database* database,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    /**
     * @brief Retrieves the database storage backend type string (e.g. `"memory"`, `"scid5"`,
     * `"pgn"`).
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated type
     * string. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Type string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_database_type_get(
        const scid_database* database,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    /**
     * @brief Checks if the database is opened in read-only mode.
     *
     * @param[in]  database      Pointer to the database. Must not be NULL.
     * @param[out] out_read_only Pointer receiving non-zero (`1`) if read-only; zero (`0`) if
     * read-write. Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_read_only is NULL.
     */
    SCID_API scid_error
    scid_database_read_only_get(
        const scid_database* database,
        int*                 out_read_only);


    /**
     * @brief Retrieves a custom metadata property value by key name.
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[in]  key               Null-terminated metadata key name (e.g. `"description"`,
     * `"type"`). Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated property
     * value. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Property retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database, @p key, or @p out_text_size is NULL.
     * @retval SCID_ERROR_NOT_FOUND  If no metadata property exists with the given key.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_database_metadata_set()
     */
    SCID_API scid_error
    scid_database_metadata_get(
        const scid_database* database,
        const char*          key,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    /**
     * @brief Sets or updates a custom metadata property in the database.
     *
     * @param[in,out] database Pointer to the database. Must not be NULL.
     * @param[in]     key      Null-terminated metadata key name. Must not be NULL.
     * @param[in]     value    Null-terminated property value string. Must not be NULL.
     *
     * @retval SCID_OK           Property set successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database, @p key, or @p value is NULL.
     * @retval SCID_ERROR        If the database is read-only.
     *
     * @see scid_database_metadata_get()
     */
    SCID_API scid_error
    scid_database_metadata_set(
        scid_database* database,
        const char*    key,
        const char*    value);


    /**
     * @brief Retrieves the total number of custom metadata entries stored in the database.
     *
     * @param[in]  database  Pointer to the database. Must not be NULL.
     * @param[out] out_count Pointer receiving the metadata entry count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_count is NULL.
     *
     * @see scid_database_metadata_at_get()
     */
    SCID_API scid_error
    scid_database_metadata_count_get(
        const scid_database* database,
        size_t*              out_count);


    /**
     * @brief Retrieves a metadata key-value pair by zero-based index.
     *
     * @param[in]  database            Pointer to the database. Must not be NULL.
     * @param[in]  index               Zero-based index of the metadata entry (`0..metadata_count -
     * 1`).
     * @param[out] out_key             Caller-allocated buffer receiving the key string. May be NULL
     * if capacity is 0.
     * @param[in]  out_key_capacity    Capacity of @p out_key in bytes.
     * @param[out] out_key_size        Pointer receiving key bytes written (excluding null
     * terminator). Must not be NULL.
     * @param[out] out_value           Caller-allocated buffer receiving the value string. May be
     * NULL if capacity is 0.
     * @param[in]  out_value_capacity  Capacity of @p out_value in bytes.
     * @param[out] out_value_size      Pointer receiving value bytes written (excluding null
     * terminator). Must not be NULL.
     *
     * @retval SCID_OK               Metadata pair retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If any mandatory argument is NULL, or @p index is out of
     * bounds.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_key_capacity or @p out_value_capacity is
     * insufficient.
     */
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

    /** @} */


    /**
     * @name Aggregate Statistics
     * @brief Global database-wide summary statistics across all games.
     * @{
     */

    /**
     * @brief Retrieves the chronological date span (earliest and latest game dates) in the
     * database.
     *
     * Dates are formatted in standard PGN date notation (`"YYYY.MM.DD"`).
     *
     * @param[in]  database              Pointer to the database. Must not be NULL.
     * @param[out] out_min_date          Caller-allocated buffer receiving the earliest date string.
     *                                   May be NULL if @p out_min_date_capacity is 0.
     * @param[in]  out_min_date_capacity Capacity of @p out_min_date in bytes.
     * @param[out] out_min_date_size     Pointer receiving bytes written for min date. Must not be
     * NULL.
     * @param[out] out_max_date          Caller-allocated buffer receiving the latest date string.
     *                                   May be NULL if @p out_max_date_capacity is 0.
     * @param[in]  out_max_date_capacity Capacity of @p out_max_date in bytes.
     * @param[out] out_max_date_size     Pointer receiving bytes written for max date. Must not be
     * NULL.
     *
     * @retval SCID_OK               Date range retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If any mandatory pointer argument is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If either buffer capacity is insufficient.
     */
    SCID_API scid_error
    scid_database_stats_date_range_get(
        const scid_database* database,
        char*                out_min_date,
        size_t               out_min_date_capacity,
        size_t*              out_min_date_size,
        char*                out_max_date,
        size_t               out_max_date_capacity,
        size_t*              out_max_date_size);


    /**
     * @brief Retrieves the total number of games matching a specific outcome result string.
     *
     * @param[in]  database  Pointer to the database. Must not be NULL.
     * @param[in]  result    Result filter string (`"1-0"`, `"0-1"`, `"1/2-1/2"`, or `"*"`). Must
     * not be NULL.
     * @param[out] out_count Pointer receiving the matching game count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database, @p result, or @p out_count is NULL, or @p result
     * is invalid.
     */
    SCID_API scid_error
    scid_database_stats_result_count_get(
        const scid_database* database,
        const char*          result,
        size_t*              out_count);


    /**
     * @brief Retrieves the total number of games stored in the database.
     *
     * @param[in]  database  Pointer to the database. Must not be NULL.
     * @param[out] out_count Pointer receiving the total game count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_count is NULL.
     */
    SCID_API scid_error
    scid_database_game_count_get(
        const scid_database* database,
        size_t*              out_count);

    /** @} */


    /**
     * @name Game Storage & Modification
     * @brief Importing PGN streams, appending, replacing, and soft-deleting games.
     * @{
     */

    /**
     * @brief Parses and imports one or more PGN games from a text buffer into the database.
     *
     * @param[in,out] database                Pointer to the database to append games to. Must not
     * be NULL.
     * @param[in]     pgn                     Buffer containing raw PGN text. Must not be NULL.
     * @param[in]     pgn_size                Size of the @p pgn buffer in bytes.
     * @param[out]    out_diagnostic          Caller-allocated buffer receiving parser diagnostics
     * on error. May be NULL if @p out_diagnostic_capacity is 0.
     * @param[in]     out_diagnostic_capacity Capacity of @p out_diagnostic in bytes.
     * @param[out]    out_diagnostic_size     Pointer receiving diagnostic bytes written. May be
     * NULL if unneeded.
     * @param[out]    out_imported_count      Pointer receiving count of games successfully
     * imported. Must not be NULL.
     *
     * @retval SCID_OK           PGN stream imported successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database, @p pgn, or @p out_imported_count is NULL.
     * @retval SCID_ERROR        If the database is read-only or parsing fails.
     */
    SCID_API scid_error
    scid_database_import_pgn(
        scid_database* database,
        const char*    pgn,
        size_t         pgn_size,
        char*          out_diagnostic,
        size_t         out_diagnostic_capacity,
        size_t*        out_diagnostic_size,
        size_t*        out_imported_count);


    /**
     * @brief Appends a new game to the end of the database.
     *
     * @param[in,out] database Pointer to the database. Must not be NULL.
     * @param[in]     game     Pointer to the game entity to encode and append. Must not be NULL.
     * @param[in]     flags    Optional user flags string (e.g. `"W"`, `"B"`, `"D"`), or NULL/`""`
     * for none.
     *
     * @retval SCID_OK           Game appended successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p game is NULL.
     * @retval SCID_ERROR        If the database is read-only.
     *
     * @see scid_database_game_replace()
     */
    SCID_API scid_error
    scid_database_game_add(
        scid_database*   database,
        const scid_game* game,
        const char*      flags);


    /**
     * @brief Replaces an existing game at a specified zero-based index.
     *
     * @param[in,out] database Pointer to the database. Must not be NULL.
     * @param[in]     index    Zero-based index of the game to replace (`0..game_count - 1`).
     * @param[in]     game     Pointer to the new game entity. Must not be NULL.
     * @param[in]     flags    Optional user flags string, or NULL/`""` for none.
     *
     * @retval SCID_OK           Game replaced successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p game is NULL, or @p index is out of range.
     * @retval SCID_ERROR        If the database is read-only.
     *
     * @see scid_database_game_add()
     */
    SCID_API scid_error
    scid_database_game_replace(
        scid_database*   database,
        size_t           index,
        const scid_game* game,
        const char*      flags);


    /**
     * @brief Marks a game as deleted (soft deletion).
     *
     * The game remains in the database index but is flagged as deleted until compacted.
     *
     * @param[in,out] database Pointer to the database. Must not be NULL.
     * @param[in]     index    Zero-based index of the game to mark deleted (`0..game_count - 1`).
     *
     * @retval SCID_OK           Game marked deleted successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database is NULL, or @p index is out of range.
     * @retval SCID_ERROR        If the database is read-only.
     *
     * @see scid_database_game_undelete()
     * @see scid_database_game_deleted_get()
     */
    SCID_API scid_error
    scid_database_game_delete(
        scid_database* database,
        size_t         index);


    /**
     * @brief Clears the deleted flag for a game (restores a soft-deleted game).
     *
     * @param[in,out] database Pointer to the database. Must not be NULL.
     * @param[in]     index    Zero-based index of the game to restore (`0..game_count - 1`).
     *
     * @retval SCID_OK           Game undeleted successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database is NULL, or @p index is out of range.
     * @retval SCID_ERROR        If the database is read-only.
     *
     * @see scid_database_game_delete()
     * @see scid_database_game_deleted_get()
     */
    SCID_API scid_error
    scid_database_game_undelete(
        scid_database* database,
        size_t         index);

    /** @} */


    /**
     * @name Game Retrieval & Header Index Queries
     * @brief Loading full game entities, fast header index inspection, and PGN export.
     * @{
     */

    /**
     * @brief Decodes and retrieves a full @ref scid_game entity from the database by index.
     *
     * @param[in]  database            Pointer to the database. Must not be NULL.
     * @param[in]  index               Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_game            Pointer to a handle pointer receiving the newly allocated
     * game entity. Must not be NULL.
     * @param[out] out_flags           Optional caller-allocated buffer receiving user flags string
     * (may be NULL).
     * @param[in]  out_flags_capacity  Capacity of @p out_flags in bytes.
     * @param[out] out_flags_size      Pointer receiving user flags length (may be NULL if @p
     * out_flags is NULL).
     *
     * @retval SCID_OK               Game loaded successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database or @p out_game is NULL, or @p index is out of
     * range.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_flags_capacity is insufficient for user flags.
     *
     * @note The caller acquires ownership of @p out_game and must release it with @ref
     * scid_game_free().
     *
     * @see scid_database_game_export_pgn()
     */
    SCID_API scid_error
    scid_database_game_get(
        const scid_database* database,
        size_t               index,
        scid_game**          out_game,
        char*                out_flags,
        size_t               out_flags_capacity,
        size_t*              out_flags_size);


    /**
     * @brief Exports a game from the database directly into PGN text format.
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[in]  index             Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated PGN text.
     *                               May be NULL if @p out_text_capacity is 0 to query required
     * capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               PGN text exported successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database or @p out_text_size is NULL, or @p index is out
     * of range.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_database_game_get()
     */
    SCID_API scid_error
    scid_database_game_export_pgn(
        const scid_database* database,
        size_t               index,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    /**
     * @brief Queries a specific header tag value directly from the fast database index without
     * loading the game.
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[in]  index             Zero-based index of the game (`0..game_count - 1`).
     * @param[in]  name              Header tag name (e.g. `"White"`, `"Black"`, `"Event"`,
     * `"Site"`, `"Date"`). Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated tag
     * value. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Tag value retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database, @p name, or @p out_text_size is NULL, or @p
     * index is out of range.
     * @retval SCID_ERROR_NOT_FOUND  If the tag is not present in the game header.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_database_game_tag_get(
        const scid_database* database,
        size_t               index,
        const char*          name,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    /**
     * @brief Retrieves the halfmove (ply) count of a game directly from the database header index.
     *
     * @param[in]  database  Pointer to the database. Must not be NULL.
     * @param[in]  index     Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_count Pointer receiving the halfmove count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_count is NULL, or @p index is out of
     * range.
     */
    SCID_API scid_error
    scid_database_game_halfmove_count_get(
        const scid_database* database,
        size_t               index,
        size_t*              out_count);


    /**
     * @brief Retrieves the 1-based sequential game number (`index + 1`).
     *
     * @param[in]  database   Pointer to the database. Must not be NULL.
     * @param[in]  index      Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_number Pointer receiving the 1-based game number. Must not be NULL.
     *
     * @retval SCID_OK           Number retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_number is NULL, or @p index is out of
     * range.
     */
    SCID_API scid_error
    scid_database_game_number_get(
        const scid_database* database,
        size_t               index,
        size_t*              out_number);


    /**
     * @brief Checks if a game at a specified index is marked as deleted.
     *
     * @param[in]  database    Pointer to the database. Must not be NULL.
     * @param[in]  index       Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_deleted Pointer receiving non-zero (`1`) if deleted; zero (`0`) if active.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Query completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_deleted is NULL, or @p index is out of
     * range.
     *
     * @see scid_database_game_delete()
     * @see scid_database_game_undelete()
     */
    SCID_API scid_error
    scid_database_game_deleted_get(
        const scid_database* database,
        size_t               index,
        int*                 out_deleted);


    /**
     * @brief Retrieves the game outcome result string directly from the database header index.
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[in]  index             Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated result
     * string
     *                               (`"1-0"`, `"0-1"`, `"1/2-1/2"`, or `"*"`).
     *                               May be NULL if @p out_text_capacity is 0 to query required
     * capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Result string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database or @p out_text_size is NULL, or @p index is out
     * of range.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_database_game_result_get(
        const scid_database* database,
        size_t               index,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);


    /**
     * @brief Retrieves the classified ECO code of a game directly from the database header index.
     *
     * @param[in]  database Pointer to the database. Must not be NULL.
     * @param[in]  index    Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_code Pointer receiving the @ref scid_eco_code. Must not be NULL.
     *
     * @retval SCID_OK           ECO code retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p database or @p out_code is NULL, or @p index is out of
     * range.
     */
    SCID_API scid_error
    scid_database_game_eco_get(
        const scid_database* database,
        size_t               index,
        scid_eco_code*       out_code);


    /**
     * @brief Retrieves the played date of a game directly from the database header index.
     *
     * @param[in]  database          Pointer to the database. Must not be NULL.
     * @param[in]  index             Zero-based index of the game (`0..game_count - 1`).
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated date
     * string (`"YYYY.MM.DD"`). May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Date string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p database or @p out_text_size is NULL, or @p index is out
     * of range.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_database_game_date_get(
        const scid_database* database,
        size_t               index,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
