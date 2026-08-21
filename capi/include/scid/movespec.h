/**
 * @file movespec.h
 * @brief Chess move specification value type, UCI codecs, and SAN notation parser/formatter.
 */

#ifndef SCID_MOVESPEC_H
#define SCID_MOVESPEC_H

#include "scid/_platform.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup movespec Move Representation & Codecs
     * @brief Chess move specification value type, UCI codecs, and SAN notation parser/formatter.
     * @{
     */

    /**
     * @name Move Representation & Construction
     * @brief Value-type representation of chess moves and basic constructors.
     * @{
     */

    /**
     * @brief Forward declaration of position handle.
     * @see scid_position
     */
    typedef struct scid_position scid_position;

    /**
     * @brief Move specification record.
     *
     * A self-contained, register-passable value type representing a move from an
     * origin square to a destination square, along with the moving piece, captured piece,
     * optional pawn promotion piece, and castling flag. It is passed by value
     * across the C API.
     *
     * A null move (used in chess engine analysis and pass-turn variants) is represented
     * with `from == 0`, `to == 0`, `promotion == SCID_PIECE_NONE`, and `is_castling == 0`.
     *
     * @see scid_movespec_create()
     * @see scid_movespec_create_from_uci()
     * @see scid_movespec_create_from_san()
     */
    typedef struct scid_movespec
    {
            /**
             * @brief Origin square index (0..63, corresponding to a1..h8).
             *
             * Set to 0 for null moves.
             */
            scid_square from;

            /**
             * @brief Destination square index (0..63, corresponding to a1..h8).
             *
             * Set to 0 for null moves.
             */
            scid_square to;

            /**
             * @brief Target piece type for pawn promotion.
             *
             * Must be one of @ref SCID_PIECE_QUEEN, @ref SCID_PIECE_ROOK,
             * @ref SCID_PIECE_BISHOP, @ref SCID_PIECE_KNIGHT, or @ref SCID_PIECE_NONE
             * if the move is not a pawn promotion.
             */
            scid_piece promotion;

            /**
             * @brief Castling indicator flag.
             *
             * Set to non-zero (`1`) if the move is kingside or queenside castling;
             * `0` for regular non-castling moves.
             */
            int is_castling;
    } scid_movespec;


    /**
     * @brief Constructs a move specification value from raw primitives.
     *
     * @param[in]  from        Origin square index (0..63).
     * @param[in]  to          Destination square index (0..63).
     * @param[in]  promotion   Promotion piece type or @ref SCID_PIECE_NONE.
     * @param[in]  is_castling Non-zero if the move is a castling move; 0 otherwise.
     * @param[out] out_move    Pointer to the `scid_movespec` receiving the initialised struct.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Move specification initialised successfully.
     * @retval SCID_ERROR_BAD_ARG If @p out_move is NULL.
     *
     * @see scid_movespec_create_from_uci()
     * @see scid_movespec_create_from_san()
     */
    SCID_API scid_error
    scid_movespec_create(
        scid_square    from,
        scid_square    to,
        scid_piece     promotion,
        int            is_castling,
        scid_movespec* out_move);

    /** @} */


    /**
     * @name Universal Chess Interface (UCI) Codecs
     * @brief Coordinate-based notation parsing and serialisation (context-free).
     * @{
     */

    /**
     * @brief Parses a coordinate UCI string into a move specification.
     *
     * Parses standard coordinate notation such as `"e2e4"`, `"a7a8q"`, or the
     * null move `"0000"`. UCI moves are purely coordinate-based and do not require
     * a board position context for disambiguation.
     *
     * @param[in]  text     Null-terminated ASCII/UTF-8 UCI move string. Must not be NULL.
     * @param[out] out_move Pointer to the `scid_movespec` receiving the parsed move.
     *                      Must not be NULL.
     *
     * @retval SCID_OK           UCI string parsed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p text or @p out_move is NULL, or if the string
     *                            syntax is malformed (invalid coordinates, illegal length,
     *                            or unrecognised promotion character).
     *
     * @see scid_movespec_to_uci()
     */
    SCID_API scid_error
    scid_movespec_create_from_uci(
        const char*    text,
        scid_movespec* out_move);


    /**
     * @brief Formats a move specification as a coordinate UCI string.
     *
     * Emits coordinate notation such as `"e2e4"`, `"a7a8q"`, or `"0000"` (null move)
     * into the caller-provided buffer.
     *
     * @param[in]  move              The move specification to format.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated UCI
     * string. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes (at least 6 bytes
     * recommended).
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               UCI string formatted successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p out_text_size is NULL, or if @p move contains an invalid
     *                               square index or unsupported promotion piece.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_movespec_create_from_uci()
     */
    SCID_API scid_error
    scid_movespec_to_uci(
        scid_movespec move,
        char*         out_text,
        size_t        out_text_capacity,
        size_t*       out_text_size);

    /** @} */


    /**
     * @name Standard Algebraic Notation (SAN) Codecs
     * @brief Context-sensitive notation parsing and serialisation against a board position.
     * @{
     */

    /**
     * @brief Parses a Standard Algebraic Notation (SAN) string into a move specification.
     *
     * Resolves standard chess notation (e.g. `"Nf3"`, `"exd5"`, `"O-O"`, `"Rad1"`,
     * `"e8=Q#"`) against the provided board position to establish legal move disambiguation.
     *
     * @param[in]  position Pointer to the board position context. Must not be NULL.
     * @param[in]  text     Null-terminated ASCII/UTF-8 SAN move string. Must not be NULL.
     * @param[out] out_move Pointer to the `scid_movespec` receiving the parsed move.
     *                      Must not be NULL.
     *
     * @retval SCID_OK                 SAN move parsed and validated successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p position, @p text, or @p out_move is NULL.
     * @retval SCID_ERROR_INVALID_MOVE If the move text is unrecognised, ambiguous,
     *                                 or illegal in the given position.
     *
     * @see scid_movespec_to_san()
     */
    SCID_API scid_error
    scid_movespec_create_from_san(
        const scid_position* position,
        const char*          text,
        scid_movespec*       out_move);


    /**
     * @brief Formats a move specification as Standard Algebraic Notation (SAN).
     *
     * Generates standard chess notation (e.g. `"Nf3"`, `"exd5"`, `"O-O"`, `"Rad1"`,
     * `"e8=Q#"`) including necessary piece disambiguation, capture indicators (`x`),
     * checks (`+`), and checkmates (`#`) evaluated against the board position.
     *
     * @param[in]  position          Pointer to the board position context. Must not be NULL.
     * @param[in]  move              The move specification to format.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated SAN
     * string. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes (at least 10 bytes
     * recommended).
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK                 SAN string formatted successfully.
     * @retval SCID_ERROR_BAD_ARG      If @p position or @p out_text_size is NULL, or if @p move
     * contains invalid square or promotion values.
     * @retval SCID_ERROR_INVALID_MOVE If @p move is not a legal move in the given position.
     * @retval SCID_ERROR_BUFFER_FULL  If @p out_text_capacity is insufficient.
     *
     * @see scid_movespec_create_from_san()
     */
    SCID_API scid_error
    scid_movespec_to_san(
        const scid_position* position,
        scid_movespec        move,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
