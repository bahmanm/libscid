/**
 * @file game.h
 * @brief Chess game aggregate entity, PGN tag management, boundary positions, and game merging.
 */

#ifndef SCID_GAME_H
#define SCID_GAME_H

#include "scid/_platform.h"
#include "scid/movespec.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup game Game Management
     * @brief Core chess game aggregate, PGN header tag roster, comments, boundary positions, and
     * move merging.
     * @{
     */

    /**
     * @name Game Lifecycle & Factories
     * @brief Opaque game handle creation, PGN parsing, and destruction.
     * @{
     */

    /**
     * @brief Opaque handle representing a chess game entity.
     *
     * Encapsulates the Seven Tag Roster (STR) metadata, supplemental PGN tags,
     * initial board starting position, and the hierarchical movetext tree
     * (mainline, variations, comments, and NAGs).
     *
     * Instances are created using @ref scid_game_create_blank() or @ref scid_game_create(),
     * and must be released when no longer needed using @ref scid_game_free().
     *
     * @see scid_game_create_blank()
     * @see scid_game_create()
     * @see scid_game_free()
     */
    typedef struct scid_game scid_game;


    /**
     * @brief Creates a new blank game starting from a specified board position.
     *
     * Initialises standard default PGN tags (`[Event "?"]`, `[Site "?"]`, `[Date "????.??.??"]`,
     * `[Round "?"]`, `[White "?"]`, `[Black "?"]`, `[Result "*"]`). If @p position is a
     * non-standard setup, a `[FEN "..."]` tag is automatically attached.
     *
     * @param[in]  position Pointer to the starting board position. Must not be NULL.
     * @param[out] out_game Pointer to a handle pointer receiving the newly allocated
     *                      `scid_game` instance. Must not be NULL.
     *
     * @retval SCID_OK           Game created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_game is NULL.
     *
     * @note Ownership of the created game is transferred to the caller. The caller
     *       must release it using @ref scid_game_free().
     *
     * @see scid_game_create()
     * @see scid_game_free()
     */
    SCID_API scid_error
    scid_game_create_blank(
        const scid_position* position,
        scid_game**          out_game);


    /**
     * @brief Creates a new game by parsing PGN text.
     *
     * Parses PGN tags, move notations, nested variations, comments, and NAGs from
     * the provided string buffer.
     *
     * @param[in]  position                Pointer to the starting position context. Must not be
     * NULL.
     * @param[in]  pgn                     Pointer to the PGN text buffer. Must not be NULL.
     * @param[in]  pgn_size                Length of @p pgn in bytes.
     * @param[out] out_game                Pointer to a handle pointer receiving the newly allocated
     * game. Must not be NULL.
     * @param[out] out_diagnostic          Optional caller-allocated buffer receiving parser
     * diagnostic messages. May be NULL if not requested.
     * @param[in]  out_diagnostic_capacity Capacity of @p out_diagnostic in bytes.
     * @param[out] out_diagnostic_size     Optional pointer receiving bytes written to @p
     * out_diagnostic. May be NULL if not requested.
     *
     * @retval SCID_OK                Game parsed and created successfully.
     * @retval SCID_ERROR_BAD_ARG     If @p position, @p pgn, or @p out_game is NULL.
     * @retval SCID_ERROR_CORRUPT     If PGN syntax is invalid or illegal moves are encountered.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_diagnostic_capacity is insufficient.
     *
     * @see scid_game_free()
     */
    SCID_API scid_error
    scid_game_create(
        const scid_position* position,
        const char*          pgn,
        size_t               pgn_size,
        scid_game**          out_game,
        char*                out_diagnostic,
        size_t               out_diagnostic_capacity,
        size_t*              out_diagnostic_size);


    /**
     * @brief Releases a game handle and all associated resources.
     *
     * @param[in,out] game Pointer to the game handle to release. If NULL, this
     *                     function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_game_free(scid_game* game);

    /** @} */


    /**
     * @name PGN Tag Management
     * @brief Inspection, modification, and iteration of PGN header tags.
     * @{
     */

    /**
     * @brief Retrieves the value of a PGN header tag by name.
     *
     * Queries standard Seven Tag Roster (STR) tags (e.g. `"Event"`, `"Site"`, `"Date"`,
     * `"Round"`, `"White"`, `"Black"`, `"Result"`), well-known supplemental tags (e.g. `"ECO"`,
     * `"EventDate"`, `"FEN"`), and arbitrary extra tags.
     *
     * If the specified tag is not present in the game, an empty string `""` is written to @p
     * out_text with `*out_text_size = 0`.
     *
     * @param[in]  game              Pointer to the game. Must not be NULL.
     * @param[in]  name              Null-terminated tag name (e.g. `"Event"`, `"ECO"`). Must not be
     * NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated tag
     * value. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               Tag value retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p game, @p name, or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_game_tag_set()
     */
    SCID_API scid_error
    scid_game_tag_get(
        const scid_game* game,
        const char*      name,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);


    /**
     * @brief Sets or updates the value of a PGN header tag.
     *
     * Updates an existing tag or adds a new supplemental tag. When updating the `"Result"` tag,
     * the value is validated against standard PGN result strings (`"1-0"`, `"0-1"`, `"1/2-1/2"`,
     * `"*"`).
     *
     * @param[in,out] game  Pointer to the game to mutate. Must not be NULL.
     * @param[in]     name  Null-terminated tag name. Must not be NULL.
     * @param[in]     value Null-terminated tag value. Must not be NULL.
     *
     * @retval SCID_OK           Tag updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p name, or @p value is NULL, or if @p value is
     * invalid for restricted tags (e.g. malformed `"Result"`).
     *
     * @see scid_game_tag_get()
     * @see scid_game_tag_remove()
     */
    SCID_API scid_error
    scid_game_tag_set(
        scid_game*  game,
        const char* name,
        const char* value);


    /**
     * @brief Retrieves the total number of PGN header tags present in the game.
     *
     * @param[in]  game      Pointer to the game. Must not be NULL.
     * @param[out] out_count Pointer receiving the total tag count. Must not be NULL.
     *
     * @retval SCID_OK           Tag count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game or @p out_count is NULL.
     *
     * @see scid_game_tag_at_get()
     */
    SCID_API scid_error
    scid_game_tag_count_get(
        const scid_game* game,
        size_t*          out_count);


    /**
     * @brief Retrieves a PGN tag key-value pair by zero-based index.
     *
     * Enables enumeration of all tags present in the game from index `0` to `count - 1`.
     *
     * @param[in]  game               Pointer to the game. Must not be NULL.
     * @param[in]  index              Zero-based index of the tag to retrieve.
     * @param[out] out_name           Caller-allocated buffer receiving the tag name.
     *                                May be NULL if @p out_name_capacity is 0 to query required
     * capacity.
     * @param[in]  out_name_capacity  Capacity of @p out_name in bytes.
     * @param[out] out_name_size      Pointer receiving bytes written to @p out_name (excluding null
     * terminator), or required capacity. Must not be NULL.
     * @param[out] out_value          Caller-allocated buffer receiving the tag value.
     *                                May be NULL if @p out_value_capacity is 0 to query required
     * capacity.
     * @param[in]  out_value_capacity Capacity of @p out_value in bytes.
     * @param[out] out_value_size     Pointer receiving bytes written to @p out_value (excluding
     * null terminator), or required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Tag name and value retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p game, @p out_name_size, or @p out_value_size is NULL, or
     * if
     *                               @p index is out of bounds.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_name_capacity or @p out_value_capacity is
     * insufficient.
     *
     * @see scid_game_tag_count_get()
     */
    SCID_API scid_error
    scid_game_tag_at_get(
        const scid_game* game,
        size_t           index,
        char*            out_name,
        size_t           out_name_capacity,
        size_t*          out_name_size,
        char*            out_value,
        size_t           out_value_capacity,
        size_t*          out_value_size);


    /**
     * @brief Removes a PGN header tag by name.
     *
     * Standard mandatory Seven Tag Roster (STR) tags and `"FEN"` cannot be removed (writes `0` to
     * @p out_removed).
     *
     * @param[in,out] game        Pointer to the game to mutate. Must not be NULL.
     * @param[in]     name        Null-terminated tag name to remove. Must not be NULL.
     * @param[out]    out_removed Pointer receiving non-zero (`1`) if the tag was present and
     * removed; `0` if the tag was not found or is non-removable. Must not be NULL.
     *
     * @retval SCID_OK           Tag removal processed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p name, or @p out_removed is NULL.
     *
     * @see scid_game_tag_set()
     */
    SCID_API scid_error
    scid_game_tag_remove(
        scid_game*  game,
        const char* name,
        int*        out_removed);

    /** @} */


    /**
     * @name Global Game Properties & Boundary Positions
     * @brief Mainline statistics, initial commentary, and start/final board positions.
     * @{
     */

    /**
     * @brief Retrieves the total number of halfmoves (ply) in the mainline of the game.
     *
     * @param[in]  game      Pointer to the game. Must not be NULL.
     * @param[out] out_count Pointer receiving the mainline ply count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game or @p out_count is NULL.
     */
    SCID_API scid_error
    scid_game_mainline_halfmove_count_get(
        const scid_game* game,
        size_t*          out_count);


    /**
     * @brief Retrieves the initial comment text appearing before the first move of the game.
     *
     * @param[in]  game              Pointer to the game. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated comment
     * text. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               Comment retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p game or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_game_initial_comment_get(
        const scid_game* game,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);


    /**
     * @brief Populates a board position handle with the initial starting position of the game.
     *
     * @param[in]  game         Pointer to the game. Must not be NULL.
     * @param[out] out_position Pointer to an existing `scid_position` handle to populate. Must not
     * be NULL.
     *
     * @retval SCID_OK           Position populated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game or @p out_position is NULL.
     *
     * @see scid_game_final_position_get()
     */
    SCID_API scid_error
    scid_game_start_position_get(
        const scid_game* game,
        scid_position*   out_position);


    /**
     * @brief Populates a board position handle with the final position reached at the end of the
     * mainline.
     *
     * @param[in]  game         Pointer to the game. Must not be NULL.
     * @param[out] out_position Pointer to an existing `scid_position` handle to populate. Must not
     * be NULL.
     *
     * @retval SCID_OK                 Position populated successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p game or @p out_position is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If a corrupt move sequence prevents reaching the end
     * position.
     *
     * @see scid_game_start_position_get()
     */
    SCID_API scid_error
    scid_game_final_position_get(
        const scid_game* game,
        scid_position*   out_position);

    /** @} */


    /**
     * @name Game Move Merging
     * @brief Merging moves and variations from a source game into a target game.
     * @{
     */

    typedef struct scid_game_cursor scid_game_cursor;

    /**
     * @brief Mode specifier controlling how moves from a source game are merged into a target game.
     */
    typedef int scid_game_merge_moves_mode;

    /**
     * @brief Merge mode enumerators.
     */
    enum scid_game_merge_moves_modes
    {
        /** Append source moves to the end of the current line (requires cursor at line end). */
        SCID_GAME_MERGE_MOVES_APPEND = 0,

        /** Insert source moves as a new sub-variation branching off the current position. */
        SCID_GAME_MERGE_MOVES_INSERT_VARIATION = 1,

        /** Replace subsequent moves in the current line by truncating and appending source moves.
         */
        SCID_GAME_MERGE_MOVES_REPLACE = 2
    };


    /**
     * @brief Merges the mainline move sequence from a source game into a target game at the cursor
     * position.
     *
     * Applies moves from @p source_game according to the chosen @p mode. The starting position
     * of @p source_game must match the board position at @p target_cursor.
     *
     * @param[in,out] target_game   Pointer to the target game being modified. Must not be NULL.
     * @param[in]     target_cursor Pointer to the cursor indicating the insertion position in @p
     * target_game. Must not be NULL.
     * @param[in]     source_game   Pointer to the source game providing moves. Must not be NULL.
     * @param[in]     mode          Merge strategy (@ref SCID_GAME_MERGE_MOVES_APPEND,
     *                              @ref SCID_GAME_MERGE_MOVES_INSERT_VARIATION, or @ref
     * SCID_GAME_MERGE_MOVES_REPLACE).
     * @param[out]    out_cursor    Pointer to a handle pointer receiving a newly allocated cursor
     * positioned at the end of the merged moves. Must not be NULL.
     *
     * @retval SCID_OK                 Moves merged successfully.
     * @retval SCID_ERROR_BAD_ARG      If any argument is NULL, @p mode is invalid, or cursor
     * position does not satisfy preconditions for @p mode.
     * @retval SCID_ERROR_INVALID_MOVE If board positions do not match or illegal moves are
     * encountered.
     *
     * @note The caller acquires ownership of @p out_cursor and must release it with @ref
     * scid_game_cursor_free().
     */
    SCID_API scid_error
    scid_game_merge_moves(
        scid_game*                 target_game,
        const scid_game_cursor*    target_cursor,
        const scid_game*           source_game,
        scid_game_merge_moves_mode mode,
        scid_game_cursor**         out_cursor);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
