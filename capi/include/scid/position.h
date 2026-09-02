/**
 * @file position.h
 * @brief Chess board position state representation, FEN codecs, move generation, and evaluation
 * queries.
 */

#ifndef SCID_POSITION_H
#define SCID_POSITION_H

#include "scid/_platform.h"
#include "scid/movespec.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup position Board Position
     * @brief Chess board state management, FEN serialisation, move execution, and legality queries.
     * @{
     */

    /**
     * @name Lifecycle & Factories
     * @brief Opaque position handle allocation, cloning, and destruction.
     * @{
     */

#ifndef SCID_POSITION_TYPEDEF
#define SCID_POSITION_TYPEDEF
    /**
     * @brief Opaque handle representing a chess board position state.
     *
     * Encapsulates piece placement, active colour, castling availability rights,
     * en passant target square, halfmove clock (50-move rule), and fullmove counter.
     *
     * Instances are created using @ref scid_position_create_from_fen(),
     * @ref scid_position_create_with_san(), or @ref scid_position_create_with_uci(),
     * and must be released when no longer needed using @ref scid_position_free().
     *
     * @see scid_position_create_from_fen()
     * @see scid_position_free()
     */
    typedef struct scid_position scid_position;
#endif


    /**
     * @brief Creates a new board position initialised from a FEN string.
     *
     * @param[in]  fen          Null-terminated Forsyth–Edwards Notation (FEN) string. Must not be
     * NULL.
     * @param[out] out_position Pointer to a handle pointer receiving the newly allocated
     *                          `scid_position` instance on success. Must not be NULL.
     *
     * @retval SCID_OK                Position created successfully.
     * @retval SCID_ERROR_BAD_ARG     If @p fen or @p out_position is NULL.
     * @retval SCID_ERROR_INVALID_FEN If @p fen is malformed or invalid.
     *
     * @note Ownership of the created position is transferred to the caller. The caller
     *       must release it using @ref scid_position_free().
     *
     * @see scid_position_free()
     * @see scid_position_to_fen()
     */
    SCID_API scid_error
    scid_position_create_from_fen(
        const char*     fen,
        scid_position** out_position);


    /**
     * @brief Creates a new board position by cloning an existing position and applying a SAN move.
     *
     * Leaves the source @p position unmodified and returns a freshly allocated position reflecting
     * the move.
     *
     * @param[in]  position     Pointer to the source board position. Must not be NULL.
     * @param[in]  san          Null-terminated Standard Algebraic Notation move (e.g. `"Nf3"`,
     * `"O-O"`). Must not be NULL.
     * @param[out] out_position Pointer to a handle pointer receiving the newly allocated position.
     *                          Must not be NULL.
     *
     * @retval SCID_OK                 Position cloned and move applied successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p position, @p san, or @p out_position is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If @p san is illegal or ambiguous in @p position.
     *
     * @see scid_position_apply_san()
     */
    SCID_API scid_error
    scid_position_create_with_san(
        const scid_position* position,
        const char*          san,
        scid_position**      out_position);


    /**
     * @brief Creates a new board position by cloning an existing position and applying a UCI move.
     *
     * Leaves the source @p position unmodified and returns a freshly allocated position reflecting
     * the move.
     *
     * @param[in]  position     Pointer to the source board position. Must not be NULL.
     * @param[in]  uci          Null-terminated coordinate UCI move string (e.g. `"e2e4"`,
     * `"a7a8q"`). Must not be NULL.
     * @param[out] out_position Pointer to a handle pointer receiving the newly allocated position.
     *                          Must not be NULL.
     *
     * @retval SCID_OK                 Position cloned and move applied successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p position, @p uci, or @p out_position is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If @p uci is illegal in @p position.
     *
     * @see scid_position_apply_uci()
     */
    SCID_API scid_error
    scid_position_create_with_uci(
        const scid_position* position,
        const char*          uci,
        scid_position**      out_position);


    /**
     * @brief Releases a board position handle and all associated resources.
     *
     * @param[in,out] position Pointer to the position handle to release. If NULL, this
     *                         function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_position_free(scid_position* position);

    /** @} */


    /**
     * @name Serialisation & Export
     * @brief Conversion of board position state to standard string formats.
     * @{
     */

    /**
     * @brief Formats the board position as a standard Forsyth–Edwards Notation (FEN) string.
     *
     * Emits the complete 6-field FEN representation including piece placement, active colour,
     * castling availability, en passant square, halfmove clock, and fullmove number.
     *
     * @param[in]  position          Pointer to the board position. Must not be NULL.
     * @param[out] out_fen           Caller-allocated buffer receiving the null-terminated FEN
     * string. May be NULL if @p out_fen_capacity is 0 to query required capacity.
     * @param[in]  out_fen_capacity  Capacity of @p out_fen in bytes (at least 100 bytes
     * recommended).
     * @param[out] out_fen_size      Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               FEN string emitted successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p position or @p out_fen_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_fen_capacity is insufficient.
     *
     * @see scid_position_create_from_fen()
     */
    SCID_API scid_error
    scid_position_to_fen(
        const scid_position* position,
        char*                out_fen,
        size_t               out_fen_capacity,
        size_t*              out_fen_size);

    /** @} */


    /**
     * @name Move Execution & Mutation
     * @brief In-place state transitions applying chess moves.
     * @{
     */

    /**
     * @brief Applies a Standard Algebraic Notation (SAN) move in-place to the position.
     *
     * Updates piece positions, active colour, castling rights, en passant state,
     * halfmove clock, and fullmove number.
     *
     * @param[in,out] position Pointer to the board position to mutate. Must not be NULL.
     * @param[in]     san      Null-terminated SAN move string (e.g. `"e4"`, `"Nf3"`, `"O-O"`). Must
     * not be NULL.
     *
     * @retval SCID_OK                 Move applied successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p position or @p san is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If @p san is illegal or ambiguous in the current position.
     *
     * @see scid_position_apply_uci()
     */
    SCID_API scid_error
    scid_position_apply_san(
        scid_position* position,
        const char*    san);


    /**
     * @brief Applies a coordinate UCI move in-place to the position.
     *
     * Updates piece positions, active colour, castling rights, en passant state,
     * halfmove clock, and fullmove number.
     *
     * @param[in,out] position Pointer to the board position to mutate. Must not be NULL.
     * @param[in]     uci      Null-terminated coordinate UCI move string (e.g. `"e2e4"`,
     * `"a7a8q"`). Must not be NULL.
     *
     * @retval SCID_OK                 Move applied successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p position or @p uci is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If @p uci is illegal in the current position.
     *
     * @see scid_position_apply_san()
     */
    SCID_API scid_error
    scid_position_apply_uci(
        scid_position* position,
        const char*    uci);

    /** @} */


    /**
     * @name Move Generation & Validation
     * @brief Legal move list generation, check detection, and position legality validation.
     * @{
     */

    /**
     * @brief Maximum theoretical legal moves possible in any valid chess position.
     */
    enum scid_position_limits
    {
        SCID_MAX_LEGAL_MOVES = 256
    };


    /**
     * @brief Generates all strictly legal moves available in the current position.
     *
     * @param[in]  position           Pointer to the board position. Must not be NULL.
     * @param[out] out_moves          Caller-allocated array receiving the generated @ref
     * scid_movespec values. Must not be NULL.
     * @param[in]  out_moves_capacity Maximum number of move items @p out_moves can hold (recommend
     * @ref SCID_MAX_LEGAL_MOVES).
     * @param[out] out_moves_size     Pointer receiving the total count of legal moves written. Must
     * not be NULL.
     *
     * @retval SCID_OK               Legal moves generated successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p position, @p out_moves, or @p out_moves_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_moves_capacity is less than the number of legal
     * moves.
     */
    SCID_API scid_error
    scid_position_legal_moves(
        const scid_position* position,
        scid_movespec*       out_moves,
        size_t               out_moves_capacity,
        size_t*              out_moves_size);


    /**
     * @brief Checks if the position is identical to the standard chess starting position.
     *
     * @param[in]  position     Pointer to the board position. Must not be NULL.
     * @param[out] out_is_start Pointer receiving non-zero (`1`) if standard start position; `0`
     * otherwise. Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_is_start is NULL.
     */
    SCID_API scid_error
    scid_position_is_start(
        const scid_position* position,
        int*                 out_is_start);


    /**
     * @brief Checks if the king of the side to move is currently in check.
     *
     * @param[in]  position     Pointer to the board position. Must not be NULL.
     * @param[out] out_is_check Pointer receiving non-zero (`1`) if active player is in check; `0`
     * otherwise. Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_is_check is NULL.
     *
     * @see scid_position_is_checkmate()
     */
    SCID_API scid_error
    scid_position_is_check(
        const scid_position* position,
        int*                 out_is_check);


    /**
     * @brief Checks if the position is in checkmate (king in check with no legal moves).
     *
     * @param[in]  position         Pointer to the board position. Must not be NULL.
     * @param[out] out_is_checkmate Pointer receiving non-zero (`1`) if active player is checkmated;
     * `0` otherwise. Must not be NULL.
     *
     * @retval SCID_OK           Check completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_is_checkmate is NULL.
     *
     * @see scid_position_is_check()
     */
    SCID_API scid_error
    scid_position_is_checkmate(
        const scid_position* position,
        int*                 out_is_checkmate);


    /**
     * @brief Validates whether the board position satisfies all chess rules and invariants.
     *
     * Verifies that both kings exist, the non-active player is not in check, no pawns occupy the
     * 1st or 8th ranks, and piece counts do not violate chess constraints.
     *
     * @param[in]  position     Pointer to the board position. Must not be NULL.
     * @param[out] out_is_legal Pointer receiving non-zero (`1`) if position is strictly legal; `0`
     * otherwise. Must not be NULL.
     *
     * @retval SCID_OK           Validation completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_is_legal is NULL.
     */
    SCID_API scid_error
    scid_position_is_legal(
        const scid_position* position,
        int*                 out_is_legal);

    /** @} */


    /**
     * @name State Inspection & Square Queries
     * @brief Accessors for player to move, move clocks, and square piece content.
     * @{
     */

    /**
     * @brief Retrieves the colour of the player whose turn it is to move.
     *
     * @param[in]  position          Pointer to the board position. Must not be NULL.
     * @param[out] out_side_to_move  Pointer receiving @ref SCID_WHITE or @ref SCID_BLACK. Must not
     * be NULL.
     *
     * @retval SCID_OK           Side retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_side_to_move is NULL.
     */
    SCID_API scid_error
    scid_position_side_to_move_get(
        const scid_position* position,
        scid_colour*         out_side_to_move);


    /**
     * @brief Retrieves the 1-based fullmove number.
     *
     * Starts at 1 and increments after each move by Black.
     *
     * @param[in]  position            Pointer to the board position. Must not be NULL.
     * @param[out] out_fullmove_number Pointer receiving the fullmove number. Must not be NULL.
     *
     * @retval SCID_OK           Fullmove number retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_fullmove_number is NULL.
     */
    SCID_API scid_error
    scid_position_fullmove_number_get(
        const scid_position* position,
        unsigned*            out_fullmove_number);


    /**
     * @brief Retrieves the halfmove clock for the 50-move draw rule.
     *
     * Counts the number of halfmoves (ply) since the last pawn advance or piece capture.
     *
     * @param[in]  position           Pointer to the board position. Must not be NULL.
     * @param[out] out_halfmove_clock Pointer receiving the halfmove clock count. Must not be NULL.
     *
     * @retval SCID_OK           Halfmove clock retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_halfmove_clock is NULL.
     */
    SCID_API scid_error
    scid_position_halfmove_clock_get(
        const scid_position* position,
        unsigned*            out_halfmove_clock);


    /**
     * @brief Retrieves the piece residing on a specified board square.
     *
     * @param[in]  position  Pointer to the board position. Must not be NULL.
     * @param[in]  square    Square index to query (0..63).
     * @param[out] out_piece Pointer receiving the coloured piece identifier (@ref
     * SCID_PIECE_WHITE_PAWN,
     *                       @ref SCID_PIECE_BLACK_KING, etc.), or @ref SCID_PIECE_NONE if the
     * square is empty. Must not be NULL.
     *
     * @retval SCID_OK           Piece retrieved successfully.
     * @retval SCID_ERROR_BAD_ARG If @p position or @p out_piece is NULL, or @p square > 63.
     */
    SCID_API scid_error
    scid_position_piece_at_get(
        const scid_position* position,
        scid_square          square,
        scid_piece*          out_piece);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
