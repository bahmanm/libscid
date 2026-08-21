/**
 * @file game_cursor.h
 * @brief Hierarchical chess game movetext cursor, tree navigation, commentary, NAGs, and variation editing.
 */

#ifndef SCID_GAME_CURSOR_H
#define SCID_GAME_CURSOR_H

#include "scid/_platform.h"
#include "scid/movespec.h"
#include "scid/position.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup game_cursor Game Movetext Cursor
     * @brief Hierarchical movetext tree traversal, ply positioning, commentary, NAG annotations, and variation manipulation.
     * @{
     */

    /**
     * @name Cursor Lifecycle & Cloning
     * @brief Allocation, duplication, and deallocation of movetext cursors.
     * @{
     */

    typedef struct scid_game scid_game;

    /**
     * @brief Opaque handle representing a navigational position within a game's movetext tree.
     *
     * Points to a specific node (ply, mainline or sub-variation branch) inside a @ref scid_game.
     * Navigation operations produce newly allocated cursors pointing to target locations,
     * maintaining immutable value semantics for cursor traversal.
     *
     * Cursors are created using @ref scid_game_cursor_create() or navigation functions,
     * and must be released when no longer needed using @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_create()
     * @see scid_game_cursor_free()
     */
    typedef struct scid_game_cursor scid_game_cursor;


    /**
     * @brief Creates a new cursor initialised at the starting position of a game.
     *
     * @param[in]  game       Pointer to the game entity. Must not be NULL.
     * @param[out] out_cursor Pointer to a handle pointer receiving the newly allocated cursor.
     *                        Must not be NULL.
     *
     * @retval SCID_OK           Cursor created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game or @p out_cursor is NULL.
     *
     * @note Ownership of the created cursor is transferred to the caller. The caller
     *       must release it using @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_free()
     */
    SCID_API scid_error
    scid_game_cursor_create(
        scid_game*         game,
        scid_game_cursor** out_cursor);


    /**
     * @brief Clones an existing cursor to duplicate its exact location within the game tree.
     *
     * @param[in]  game          Pointer to the game entity bound to @p source_cursor. Must not be NULL.
     * @param[in]  source_cursor Pointer to the cursor to clone. Must not be NULL.
     * @param[out] out_cursor    Pointer to a handle pointer receiving the newly allocated duplicate cursor.
     *                           Must not be NULL.
     *
     * @retval SCID_OK           Cursor cloned successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p source_cursor, or @p out_cursor is NULL, or if
     *                            @p source_cursor belongs to a different game.
     *
     * @note The caller acquires ownership of @p out_cursor and must release it with @ref scid_game_cursor_free().
     */
    SCID_API scid_error
    scid_game_cursor_clone(
        scid_game*              game,
        const scid_game_cursor* source_cursor,
        scid_game_cursor**      out_cursor);


    /**
     * @brief Releases a cursor handle and all associated resources.
     *
     * @param[in,out] cursor Pointer to the cursor handle to release. If NULL, this
     *                       function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_game_cursor_free(scid_game_cursor* cursor);

    /** @} */


    /**
     * @name Position & Location Queries
     * @brief Board state inspection, ply indices, variation depth, and boundary predicates.
     * @{
     */

    /**
     * @brief Populates a board position handle with the board state at the cursor location.
     *
     * @param[in]  cursor       Pointer to the cursor. Must not be NULL.
     * @param[out] out_position Pointer to an existing `scid_position` handle to populate. Must not be NULL.
     *
     * @retval SCID_OK                 Position populated successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p cursor or @p out_position is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If the game contains an invalid move leading up to this node.
     */
    SCID_API scid_error
    scid_game_cursor_position_get(
        const scid_game_cursor* cursor,
        scid_position*          out_position);


    /**
     * @brief Retrieves the 0-based ply count from the start of the current line to this cursor node.
     *
     * @param[in]  cursor  Pointer to the cursor. Must not be NULL.
     * @param[out] out_ply Pointer receiving the ply index. Must not be NULL.
     *
     * @retval SCID_OK           Ply count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_ply is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_ply_get(
        const scid_game_cursor* cursor,
        size_t*                 out_ply);


    /**
     * @brief Retrieves the number of sub-variations branching from the upcoming move at this cursor node.
     *
     * @param[in]  cursor    Pointer to the cursor. Must not be NULL.
     * @param[out] out_count Pointer receiving the number of branching variations. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_count is NULL.
     *
     * @see scid_game_cursor_variation_enter()
     */
    SCID_API scid_error
    scid_game_cursor_variation_count_get(
        const scid_game_cursor* cursor,
        size_t*                 out_count);


    /**
     * @brief Retrieves the variation nesting depth at this cursor node.
     *
     * Returns `0` for the mainline, `1` for direct sub-variations off the mainline,
     * `2` for variations within variations, etc.
     *
     * @param[in]  cursor    Pointer to the cursor. Must not be NULL.
     * @param[out] out_depth Pointer receiving the nesting depth. Must not be NULL.
     *
     * @retval SCID_OK           Depth retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_depth is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_variation_depth_get(
        const scid_game_cursor* cursor,
        size_t*                 out_depth);


    /**
     * @brief Retrieves the index of the current variation among sibling variations at the parent fork.
     *
     * Returns `0` if the cursor is on the first variation (or mainline), `1` for the second variation, etc.
     *
     * @param[in]  cursor    Pointer to the cursor. Must not be NULL.
     * @param[out] out_index Pointer receiving the sibling variation index. Must not be NULL.
     *
     * @retval SCID_OK           Index retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_index is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_variation_index_get(
        const scid_game_cursor* cursor,
        size_t*                 out_index);


    /**
     * @brief Checks if the cursor is at the beginning of the current line or variation.
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_line_start Pointer receiving non-zero (`1`) if at line start; `0` otherwise.
     *                               Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_line_start is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_line_start(
        const scid_game_cursor* cursor,
        int*                    out_is_line_start);


    /**
     * @brief Checks if the cursor is at the terminal end of the current line or variation (no subsequent move).
     *
     * @param[in]  cursor          Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_line_end Pointer receiving non-zero (`1`) if at line end; `0` otherwise.
     *                             Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_line_end is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_line_end(
        const scid_game_cursor* cursor,
        int*                    out_is_line_end);


    /**
     * @brief Checks if the cursor is at the very beginning of the entire game (start of mainline).
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_game_start Pointer receiving non-zero (`1`) if at game start; `0` otherwise.
     *                               Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_game_start is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_game_start(
        const scid_game_cursor* cursor,
        int*                    out_is_game_start);


    /**
     * @brief Checks if the cursor is at the final position of the mainline.
     *
     * @param[in]  cursor          Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_game_end Pointer receiving non-zero (`1`) if at mainline end; `0` otherwise.
     *                             Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_game_end is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_game_end(
        const scid_game_cursor* cursor,
        int*                    out_is_game_end);


    /**
     * @brief Checks if the cursor is at the starting position of a sub-variation (depth > 0).
     *
     * @param[in]  cursor                 Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_variation_start Pointer receiving non-zero (`1`) if at variation start; `0` otherwise.
     *                                    Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_variation_start is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_variation_start(
        const scid_game_cursor* cursor,
        int*                    out_is_variation_start);


    /**
     * @brief Checks if the cursor is at the terminal end of a sub-variation (depth > 0).
     *
     * @param[in]  cursor               Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_variation_end Pointer receiving non-zero (`1`) if at variation end; `0` otherwise.
     *                                  Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_variation_end is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_variation_end(
        const scid_game_cursor* cursor,
        int*                    out_is_variation_end);


    /**
     * @brief Checks if the current variation contains zero moves (empty branch containing only comments).
     *
     * @param[in]  cursor                 Pointer to the cursor. Must not be NULL.
     * @param[out] out_is_variation_empty Pointer receiving non-zero (`1`) if variation has no moves; `0` otherwise.
     *                                    Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_is_variation_empty is NULL.
     */
    SCID_API scid_error
    scid_game_cursor_is_variation_empty(
        const scid_game_cursor* cursor,
        int*                    out_is_variation_empty);

    /** @} */


    /**
     * @name Adjacent Move Inspection & Commentary
     * @brief Inspection of previous/next moves, comments, and NAGs at the cursor node.
     * @{
     */

    /**
     * @brief Retrieves commentary text attached directly to the current cursor position.
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated comment string.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null terminator),
     *                               or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               Comment retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p cursor or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_game_cursor_comment_set()
     */
    SCID_API scid_error
    scid_game_cursor_comment_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    /**
     * @brief Sets or updates the commentary text at the current cursor node.
     *
     * @param[in,out] game    Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor  Pointer to the cursor indicating the node to update. Must not be NULL.
     * @param[in]     comment Null-terminated comment string (or `""` to clear). Must not be NULL.
     *
     * @retval SCID_OK           Comment updated successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p cursor, or @p comment is NULL.
     *
     * @see scid_game_cursor_comment_get()
     */
    SCID_API scid_error
    scid_game_cursor_comment_set(
        scid_game*              game,
        const scid_game_cursor* cursor,
        const char*             comment);


    /**
     * @brief Retrieves the incoming move (leading into this cursor node) as a @ref scid_movespec.
     *
     * @param[in]  cursor   Pointer to the cursor. Must not be NULL.
     * @param[out] out_move Pointer receiving the previous move specification. Must not be NULL.
     *
     * @retval SCID_OK           Move retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_move is NULL, or if cursor is at line start.
     *
     * @see scid_game_cursor_next_movespec_get()
     */
    SCID_API scid_error
    scid_game_cursor_previous_movespec_get(
        const scid_game_cursor* cursor,
        scid_movespec*          out_move);


    /**
     * @brief Retrieves the incoming move formatted in Standard Algebraic Notation (SAN).
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated SAN string.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               SAN string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p cursor or @p out_text_size is NULL, or if cursor is at line start.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_game_cursor_next_move_san_get()
     */
    SCID_API scid_error
    scid_game_cursor_previous_move_san_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    /**
     * @brief Retrieves commentary text attached to the incoming move.
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated comment string.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Comment retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p cursor or @p out_text_size is NULL, or if cursor is at line start.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_game_cursor_previous_move_comment_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    /**
     * @brief Retrieves the count of Numeric Annotation Glyphs (NAGs) attached to the incoming move.
     *
     * @param[in]  cursor    Pointer to the cursor. Must not be NULL.
     * @param[out] out_count Pointer receiving the NAG count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_count is NULL, or if cursor is at line start.
     *
     * @see scid_game_cursor_previous_move_nag_at_get()
     */
    SCID_API scid_error
    scid_game_cursor_previous_move_nag_count_get(
        const scid_game_cursor* cursor,
        size_t*                 out_count);


    /**
     * @brief Retrieves a NAG code from the incoming move by zero-based index.
     *
     * @param[in]  cursor  Pointer to the cursor. Must not be NULL.
     * @param[in]  index   Zero-based index of the NAG to retrieve.
     * @param[out] out_nag Pointer receiving the @ref scid_nag code. Must not be NULL.
     *
     * @retval SCID_OK           NAG retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_nag is NULL, or if @p index is out of bounds.
     *
     * @see scid_game_cursor_previous_move_nag_count_get()
     */
    SCID_API scid_error
    scid_game_cursor_previous_move_nag_at_get(
        const scid_game_cursor* cursor,
        size_t                  index,
        scid_nag*               out_nag);


    /**
     * @brief Retrieves the upcoming move (departing from this cursor node) as a @ref scid_movespec.
     *
     * @param[in]  cursor   Pointer to the cursor. Must not be NULL.
     * @param[out] out_move Pointer receiving the next move specification. Must not be NULL.
     *
     * @retval SCID_OK           Move retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_move is NULL, or if cursor is at line end.
     *
     * @see scid_game_cursor_previous_movespec_get()
     */
    SCID_API scid_error
    scid_game_cursor_next_movespec_get(
        const scid_game_cursor* cursor,
        scid_movespec*          out_move);


    /**
     * @brief Retrieves the upcoming move formatted in Standard Algebraic Notation (SAN).
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated SAN string.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               SAN string retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p cursor or @p out_text_size is NULL, or if cursor is at line end.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_game_cursor_previous_move_san_get()
     */
    SCID_API scid_error
    scid_game_cursor_next_move_san_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    /**
     * @brief Retrieves commentary text attached to the upcoming move.
     *
     * @param[in]  cursor            Pointer to the cursor. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated comment string.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or required capacity.
     *                               Must not be NULL.
     *
     * @retval SCID_OK               Comment retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p cursor or @p out_text_size is NULL, or if cursor is at line end.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     */
    SCID_API scid_error
    scid_game_cursor_next_move_comment_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    /**
     * @brief Retrieves the count of Numeric Annotation Glyphs (NAGs) attached to the upcoming move.
     *
     * @param[in]  cursor    Pointer to the cursor. Must not be NULL.
     * @param[out] out_count Pointer receiving the NAG count. Must not be NULL.
     *
     * @retval SCID_OK           Count retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_count is NULL, or if cursor is at line end.
     *
     * @see scid_game_cursor_next_move_nag_at_get()
     */
    SCID_API scid_error
    scid_game_cursor_next_move_nag_count_get(
        const scid_game_cursor* cursor,
        size_t*                 out_count);


    /**
     * @brief Retrieves a NAG code from the upcoming move by zero-based index.
     *
     * @param[in]  cursor  Pointer to the cursor. Must not be NULL.
     * @param[in]  index   Zero-based index of the NAG to retrieve.
     * @param[out] out_nag Pointer receiving the @ref scid_nag code. Must not be NULL.
     *
     * @retval SCID_OK           NAG retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_nag is NULL, or if @p index is out of bounds.
     *
     * @see scid_game_cursor_next_move_nag_count_get()
     */
    SCID_API scid_error
    scid_game_cursor_next_move_nag_at_get(
        const scid_game_cursor* cursor,
        size_t                  index,
        scid_nag*               out_nag);

    /** @} */


    /**
     * @name Navigation & Traversal
     * @brief Navigational step and jump operations returning new cursor handles.
     * @{
     */

    /**
     * @brief Creates a new cursor positioned at the beginning of the entire game.
     *
     * @param[in]  cursor           Pointer to the source cursor. Must not be NULL.
     * @param[out] out_start_cursor Pointer to a handle pointer receiving the newly allocated cursor.
     *                              Must not be NULL.
     *
     * @retval SCID_OK           Cursor created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_start_cursor is NULL.
     *
     * @note The caller acquires ownership of @p out_start_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_to_end()
     */
    SCID_API scid_error
    scid_game_cursor_to_start(
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_start_cursor);


    /**
     * @brief Creates a new cursor positioned at the end of the mainline.
     *
     * @param[in]  cursor         Pointer to the source cursor. Must not be NULL.
     * @param[out] out_end_cursor Pointer to a handle pointer receiving the newly allocated cursor.
     *                            Must not be NULL.
     *
     * @retval SCID_OK           Cursor created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p cursor or @p out_end_cursor is NULL.
     *
     * @note The caller acquires ownership of @p out_end_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_to_start()
     */
    SCID_API scid_error
    scid_game_cursor_to_end(
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_end_cursor);


    /**
     * @brief Creates a new cursor positioned at a specified ply within the current line.
     *
     * @param[in]  cursor         Pointer to the source cursor. Must not be NULL.
     * @param[in]  ply            Target 0-based ply index within the line.
     * @param[out] out_moved      Pointer receiving non-zero (`1`) if navigation succeeded; `0` if ply is out of range.
     *                            Must not be NULL.
     * @param[out] out_ply_cursor Pointer to a handle pointer receiving the newly allocated cursor on success.
     *                            Must not be NULL.
     *
     * @retval SCID_OK           Navigation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_ply_cursor and must release it with @ref scid_game_cursor_free().
     */
    SCID_API scid_error
    scid_game_cursor_to_ply(
        const scid_game_cursor* cursor,
        size_t                  ply,
        int*                    out_moved,
        scid_game_cursor**      out_ply_cursor);


    /**
     * @brief Advances one ply forward along the current line.
     *
     * @param[in]  cursor          Pointer to the source cursor. Must not be NULL.
     * @param[out] out_moved       Pointer receiving non-zero (`1`) if advanced forward; `0` if already at line end.
     *                             Must not be NULL.
     * @param[out] out_next_cursor Pointer to a handle pointer receiving the newly allocated cursor on success.
     *                             Must not be NULL.
     *
     * @retval SCID_OK           Step completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_next_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_previous()
     */
    SCID_API scid_error
    scid_game_cursor_next(
        const scid_game_cursor* cursor,
        int*                    out_moved,
        scid_game_cursor**      out_next_cursor);


    /**
     * @brief Steps one ply backward towards the start of the current line.
     *
     * @param[in]  cursor              Pointer to the source cursor. Must not be NULL.
     * @param[out] out_moved           Pointer receiving non-zero (`1`) if stepped backward; `0` if already at line start.
     *                                 Must not be NULL.
     * @param[out] out_previous_cursor Pointer to a handle pointer receiving the newly allocated cursor on success.
     *                                 Must not be NULL.
     *
     * @retval SCID_OK           Step completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_previous_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_next()
     */
    SCID_API scid_error
    scid_game_cursor_previous(
        const scid_game_cursor* cursor,
        int*                    out_moved,
        scid_game_cursor**      out_previous_cursor);


    /**
     * @brief Enters a sub-variation branching from the current position.
     *
     * @param[in]  cursor               Pointer to the source cursor at the branching node. Must not be NULL.
     * @param[in]  index                Zero-based index of the sub-variation to enter (`0..variation_count - 1`).
     * @param[out] out_entered          Pointer receiving non-zero (`1`) if variation entered; `0` if @p index is invalid.
     *                                  Must not be NULL.
     * @param[out] out_variation_cursor Pointer to a handle pointer receiving the newly allocated cursor at the
     *                                  start of the sub-variation. Must not be NULL.
     *
     * @retval SCID_OK           Operation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_variation_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_exit()
     * @see scid_game_cursor_variation_count_get()
     */
    SCID_API scid_error
    scid_game_cursor_variation_enter(
        const scid_game_cursor* cursor,
        size_t                  index,
        int*                    out_entered,
        scid_game_cursor**      out_variation_cursor);


    /**
     * @brief Exits the current sub-variation back to its parent line.
     *
     * Returns a cursor positioned at the fork node in the parent line.
     *
     * @param[in]  cursor            Pointer to the source cursor in the variation. Must not be NULL.
     * @param[out] out_exited        Pointer receiving non-zero (`1`) if exited; `0` if already on mainline.
     *                               Must not be NULL.
     * @param[out] out_parent_cursor Pointer to a handle pointer receiving the newly allocated parent cursor.
     *                               Must not be NULL.
     *
     * @retval SCID_OK           Operation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_parent_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_enter()
     */
    SCID_API scid_error
    scid_game_cursor_variation_exit(
        const scid_game_cursor* cursor,
        int*                    out_exited,
        scid_game_cursor**      out_parent_cursor);

    /** @} */


    /**
     * @name Movetext Mutation & Variation Editing
     * @brief Tree editing operations: appending moves, adding/promoting/deleting variations, and truncation.
     * @{
     */

    /**
     * @brief Appends a chess move to the current line at the cursor position.
     *
     * If the cursor is at the end of the line, the move extends the line. If the cursor is before
     * existing moves, an error is returned (use @ref scid_game_cursor_variation_add() to branch).
     *
     * @param[in,out] game            Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor          Pointer to the cursor indicating the insertion node. Must not be NULL.
     * @param[in]     move            Move specification to execute and record.
     * @param[out]    out_next_cursor Pointer to a handle pointer receiving the newly allocated cursor positioned
     *                                after the added move. Must not be NULL.
     *
     * @retval SCID_OK                 Move appended successfully.
     * @retval SCID_ERROR_BAD_ARG      If any argument is NULL, cursor is not at line end, or move is invalid.
     * @retval SCID_ERROR_INVALID_MOVE If @p move is illegal in the current board position.
     *
     * @note The caller acquires ownership of @p out_next_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_add()
     */
    SCID_API scid_error
    scid_game_cursor_move_add(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_movespec           move,
        scid_game_cursor**      out_next_cursor);


    /**
     * @brief Creates a new sub-variation branching from the current position.
     *
     * Requires the cursor to have an upcoming move to branch away from.
     *
     * @param[in,out] game                 Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor               Pointer to the cursor at the branch point. Must not be NULL.
     * @param[in]     initial_comment      Optional comment attached to the start of the variation (may be NULL).
     * @param[out]    out_added            Pointer receiving non-zero (`1`) if variation was created; `0` otherwise.
     *                                     Must not be NULL.
     * @param[out]    out_variation_cursor Pointer to a handle pointer receiving a newly allocated cursor at the
     *                                     start of the new variation on success. Must not be NULL.
     *
     * @retval SCID_OK           Variation created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p cursor, @p out_added, or @p out_variation_cursor is NULL,
     *                            or if cursor is at the end of the line.
     *
     * @note The caller acquires ownership of @p out_variation_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_delete()
     */
    SCID_API scid_error
    scid_game_cursor_variation_add(
        scid_game*              game,
        const scid_game_cursor* cursor,
        const char*             initial_comment,
        int*                    out_added,
        scid_game_cursor**      out_variation_cursor);


    /**
     * @brief Adds a Numeric Annotation Glyph (NAG) to the incoming move.
     *
     * @param[in,out] game      Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor    Pointer to the cursor situated immediately after the target move. Must not be NULL.
     * @param[in]     nag       NAG code to attach (1..255).
     * @param[out]    out_added Pointer receiving non-zero (`1`) if NAG added; `0` if already present. Must not be NULL.
     *
     * @retval SCID_OK           NAG added successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p cursor, or @p out_added is NULL, or cursor is at line start.
     *
     * @see scid_game_cursor_nag_remove()
     * @see scid_game_cursor_nag_clear()
     */
    SCID_API scid_error
    scid_game_cursor_nag_add(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_nag                nag,
        int*                    out_added);


    /**
     * @brief Removes a NAG annotation from the incoming move.
     *
     * @param[in,out] game        Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor      Pointer to the cursor situated immediately after the target move. Must not be NULL.
     * @param[in]     is_move_nag Non-zero (`1`) to remove move assessment NAGs (1..6); zero (`0`) for positional NAGs.
     * @param[out]    out_removed Pointer receiving non-zero (`1`) if a NAG was removed; `0` otherwise. Must not be NULL.
     *
     * @retval SCID_OK           Removal processed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game, @p cursor, or @p out_removed is NULL, or cursor is at line start.
     *
     * @see scid_game_cursor_nag_add()
     */
    SCID_API scid_error
    scid_game_cursor_nag_remove(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int                     is_move_nag,
        int*                    out_removed);


    /**
     * @brief Removes all NAG annotations from the incoming move.
     *
     * @param[in,out] game   Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor Pointer to the cursor situated immediately after the target move. Must not be NULL.
     *
     * @retval SCID_OK           NAGs cleared successfully.
     * @retval SCID_ERROR_BAD_ARG If @p game or @p cursor is NULL, or cursor is at line start.
     */
    SCID_API scid_error
    scid_game_cursor_nag_clear(
        scid_game*              game,
        const scid_game_cursor* cursor);


    /**
     * @brief Promotes the current sub-variation to become the first (primary) variation at its fork.
     *
     * @param[in,out] game                Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor              Pointer to the cursor in the variation to promote. Must not be NULL.
     * @param[out]    out_promoted        Pointer receiving non-zero (`1`) if promoted; `0` if already first or mainline.
     *                                    Must not be NULL.
     * @param[out]    out_promoted_cursor Pointer to a handle pointer receiving a newly allocated cursor at the
     *                                    equivalent position in the promoted variation on success. Must not be NULL.
     *
     * @retval SCID_OK           Promotion completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_promoted_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_promote_to_mainline()
     */
    SCID_API scid_error
    scid_game_cursor_variation_promote_to_first(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int*                    out_promoted,
        scid_game_cursor**      out_promoted_cursor);


    /**
     * @brief Promotes the current sub-variation all the way up to become the new mainline of the game.
     *
     * The former mainline is converted into a sub-variation branching from the same position.
     *
     * @param[in,out] game                Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor              Pointer to the cursor in the variation to promote. Must not be NULL.
     * @param[out]    out_promoted        Pointer receiving non-zero (`1`) if promoted; `0` if already on mainline.
     *                                    Must not be NULL.
     * @param[out]    out_mainline_cursor Pointer to a handle pointer receiving a newly allocated cursor at the
     *                                    equivalent position in the new mainline on success. Must not be NULL.
     *
     * @retval SCID_OK           Promotion completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_mainline_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_promote_to_first()
     */
    SCID_API scid_error
    scid_game_cursor_variation_promote_to_mainline(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int*                    out_promoted,
        scid_game_cursor**      out_mainline_cursor);


    /**
     * @brief Deletes the current sub-variation and all its descendant moves from the game tree.
     *
     * @param[in,out] game              Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor            Pointer to the cursor inside the variation to delete. Must not be NULL.
     * @param[out]    out_deleted       Pointer receiving non-zero (`1`) if deleted; `0` if on mainline. Must not be NULL.
     * @param[out]    out_parent_cursor Pointer to a handle pointer receiving a newly allocated cursor positioned
     *                                  at the parent fork node on success. Must not be NULL.
     *
     * @retval SCID_OK           Deletion completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_parent_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_variation_add()
     */
    SCID_API scid_error
    scid_game_cursor_variation_delete(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int*                    out_deleted,
        scid_game_cursor**      out_parent_cursor);


    /**
     * @brief Truncates the current line by removing all moves subsequent to the cursor position.
     *
     * @param[in,out] game       Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor     Pointer to the cursor indicating the truncation point. Must not be NULL.
     * @param[out]    out_cursor Pointer to a handle pointer receiving a newly allocated cursor at the
     *                           new terminal position. Must not be NULL.
     *
     * @retval SCID_OK           Truncation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_truncate_before_cursor()
     */
    SCID_API scid_error
    scid_game_cursor_truncate(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_cursor);


    /**
     * @brief Truncates the beginning of the current line by removing all moves preceding the cursor position.
     *
     * Sets the starting position of the line or game to the board state at the cursor.
     *
     * @param[in,out] game       Pointer to the game to mutate. Must not be NULL.
     * @param[in]     cursor     Pointer to the cursor indicating the new start node. Must not be NULL.
     * @param[out]    out_cursor Pointer to a handle pointer receiving a newly allocated cursor at the
     *                           new start position. Must not be NULL.
     *
     * @retval SCID_OK           Truncation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If any argument is NULL.
     *
     * @note The caller acquires ownership of @p out_cursor and must release it with @ref scid_game_cursor_free().
     *
     * @see scid_game_cursor_truncate()
     */
    SCID_API scid_error
    scid_game_cursor_truncate_before_cursor(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_cursor);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
