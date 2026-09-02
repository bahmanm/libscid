/**
 * @file primitives.h
 * @brief Foundational scalar types, error return codes, chess colours, squares, pieces, and NAG
 * annotations.
 */

#ifndef SCID_PRIMITIVES_H
#define SCID_PRIMITIVES_H

#include "scid/_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup primitives Primitive Types & Error Codes
     * @brief Foundational scalar types, error return codes, chess colours, squares, pieces, and NAG
     * annotations.
     * @{
     */

    /**
     * @name Status & Error Codes
     * @brief Standard error return codes reported by libscid operations.
     * @{
     */

    /**
     * @brief Numeric status code returned by public API functions.
     *
     * A value of @ref SCID_OK indicates complete success. Warning codes (such as @ref
     * SCID_WARNING_NAME_DATA_LOSS) indicate that an operation succeeded in a degraded but usable
     * state. Error codes represent fatal failures.
     */
    typedef unsigned short scid_error;

    /**
     * @brief Status and error code enumerators.
     */
    enum scid_error_code
    {
        /** Operation completed successfully. */
        SCID_OK = 0,

        /** Generic unspecified error occurred. */
        SCID_ERROR = 1,

        /** Operation was cancelled by user request or progress callback. */
        SCID_ERROR_USER_CANCEL = 2,

        /** Invalid argument or NULL pointer supplied to function. */
        SCID_ERROR_BAD_ARG = 3,

        /** Failed to open specified file on disk. */
        SCID_ERROR_FILE_OPEN = 101,

        /** Attempted write operation on a read-only database or file. */
        SCID_ERROR_FILE_READ_ONLY = 111,

        /** Corrupted database index, header, or data record encountered. */
        SCID_ERROR_CORRUPT = 152,

        /**
         * @brief Non-fatal warning: database was opened in degraded mode due to unresolvable name
         * records.
         *
         * The database handle is valid and populated for read-only inspection. Direct mutations
         * are rejected until the database is compacted or repaired.
         */
        SCID_WARNING_NAME_DATA_LOSS = 206,

        /** Malformed or syntactically invalid Forsyth–Edwards Notation (FEN) string. */
        SCID_ERROR_INVALID_FEN = 301,

        /** Move is illegal in current position or not recognized. */
        SCID_ERROR_INVALID_MOVE = 302,

        /** Destination buffer capacity is insufficient to receive output text or items. */
        SCID_ERROR_BUFFER_FULL = 601
    };

    /**
     * @brief Evaluates whether a status code represents a non-fatal, recoverable warning.
     *
     * @param[in] status Status code to inspect.
     *
     * @return Non-zero (`1`) if @p status is a warning; zero (`0`) otherwise.
     */
    SCID_API int
    scid_is_warning(scid_error status);


    /**
     * @brief Evaluates whether a status code represents a fatal failure.
     *
     * @param[in] status Status code to inspect.
     *
     * @return Non-zero (`1`) if @p status is a fatal error; zero (`0`) otherwise.
     */
    SCID_API int
    scid_is_error(scid_error status);

    /** @} */


    /**
     * @name Board Primitives (Colour, Square, Piece)
     * @brief Scalar types and codecs for players, board coordinates, and piece types.
     * @{
     */

    /**
     * @brief Player colour indicator.
     */
    typedef int scid_colour;

    /**
     * @brief Player colour enumerators.
     */
    enum scid_colour_type
    {
        /** White player. */
        SCID_WHITE = 0,

        /** Black player. */
        SCID_BLACK = 1
    };

    /**
     * @brief Board square coordinate index (0..63, corresponding to a1..h8).
     *
     * Squares are ordered rank by rank from a1 (0) to h8 (63):
     * - `a1 = 0, b1 = 1, ..., h1 = 7`
     * - `a2 = 8, b2 = 9, ..., h2 = 15`
     * - `...`
     * - `a8 = 56, b8 = 57, ..., h8 = 63`
     */
    typedef unsigned scid_square;


    /**
     * @brief Chess piece type indicator.
     *
     * Represents uncoloured piece kinds (1..6), white pieces (1..6), or black pieces (9..14).
     */
    typedef unsigned scid_piece;

    /**
     * @brief Chess piece enumerators.
     */
    enum scid_piece_type
    {
        /** Empty square or absence of a piece. */
        SCID_PIECE_NONE = 0,

        /** Uncoloured King. */
        SCID_PIECE_KING = 1,

        /** Uncoloured Queen. */
        SCID_PIECE_QUEEN = 2,

        /** Uncoloured Rook. */
        SCID_PIECE_ROOK = 3,

        /** Uncoloured Bishop. */
        SCID_PIECE_BISHOP = 4,

        /** Uncoloured Knight. */
        SCID_PIECE_KNIGHT = 5,

        /** Uncoloured Pawn. */
        SCID_PIECE_PAWN = 6,

        /** White King. */
        SCID_PIECE_WHITE_KING = 1,

        /** White Queen. */
        SCID_PIECE_WHITE_QUEEN = 2,

        /** White Rook. */
        SCID_PIECE_WHITE_ROOK = 3,

        /** White Bishop. */
        SCID_PIECE_WHITE_BISHOP = 4,

        /** White Knight. */
        SCID_PIECE_WHITE_KNIGHT = 5,

        /** White Pawn. */
        SCID_PIECE_WHITE_PAWN = 6,

        /** Black King. */
        SCID_PIECE_BLACK_KING = 9,

        /** Black Queen. */
        SCID_PIECE_BLACK_QUEEN = 10,

        /** Black Rook. */
        SCID_PIECE_BLACK_ROOK = 11,

        /** Black Bishop. */
        SCID_PIECE_BLACK_BISHOP = 12,

        /** Black Knight. */
        SCID_PIECE_BLACK_KNIGHT = 13,

        /** Black Pawn. */
        SCID_PIECE_BLACK_PAWN = 14
    };


    /**
     * @brief Parses standard two-character coordinate string into a square index.
     *
     * @param[in]  text       Null-terminated coordinate string (e.g. `"e4"`, `"a1"`, `"h8"`). Must
     * not be NULL.
     * @param[out] out_square Pointer receiving the parsed square index (0..63). Must not be NULL.
     *
     * @retval SCID_OK           Coordinate parsed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p text or @p out_square is NULL, or if syntax is not a valid
     * square name.
     *
     * @see scid_square_to_string()
     */
    SCID_API scid_error
    scid_square_from_string(
        const char*  text,
        scid_square* out_square);


    /**
     * @brief Formats a square index into a two-character coordinate string.
     *
     * @param[in]  square            Square index to format (0..63).
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated
     * coordinate string (e.g. `"e4"`). May be NULL if @p out_text_capacity is 0 to probe required
     * size.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes (at least 3 bytes
     * recommended).
     * @param[out] out_text_size     Pointer receiving bytes written (excluding null terminator), or
     * required capacity. Must not be NULL.
     *
     * @retval SCID_OK               Square formatted successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p square is out of range (> 63) or @p out_text_size is
     * NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_square_from_string()
     */
    SCID_API scid_error
    scid_square_to_string(
        scid_square square,
        char*       out_text,
        size_t      out_text_capacity,
        size_t*     out_text_size);


    /**
     * @brief Parses a single character piece symbol into an uncoloured piece kind.
     *
     * Recognises standard English piece letters (`'K'`, `'Q'`, `'R'`, `'B'`, `'N'`, `'P'`,
     * case-insensitive).
     *
     * @param[in]  text      Null-terminated single-character string (e.g. `"Q"`, `"n"`). Must not
     * be NULL.
     * @param[out] out_piece Pointer receiving the parsed uncoloured piece type. Must not be NULL.
     *
     * @retval SCID_OK           Piece parsed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p text or @p out_piece is NULL, or character is unrecognised.
     */
    SCID_API scid_error
    scid_piece_type_from_string(
        const char* text,
        scid_piece* out_piece);

    /** @} */


    /**
     * @name Numeric Annotation Glyphs (NAG)
     * @brief PGN move evaluation annotations and typographical glyph codecs.
     * @{
     */

    /**
     * @brief Numeric Annotation Glyph (NAG) code (1..255).
     *
     * Represents move evaluations (e.g. `!` = 1, `?` = 2, `!!` = 3, `??` = 4, `!?` = 5, `?!` = 6)
     * and positional assessments according to the PGN standard.
     */
    typedef unsigned char scid_nag;


    /**
     * @brief Parses a NAG string or glyph symbol into a numeric NAG code.
     *
     * Accepts numeric strings (e.g. `"$1"`, `"1"`), punctuation glyphs (e.g. `"!"`, `"?"`, `"!?"`),
     * or standard symbol annotations (e.g. `"+="`, `"-+"`, `"=\\"`, `"D"`).
     *
     * @param[in]  text    Null-terminated NAG string or symbol. Must not be NULL.
     * @param[out] out_nag Pointer receiving the parsed NAG code (1..255). Must not be NULL.
     *
     * @retval SCID_OK           NAG parsed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p text or @p out_nag is NULL, or if the string is
     * unrecognised.
     *
     * @see scid_nag_to_string()
     */
    SCID_API scid_error
    scid_nag_create_from_string(
        const char* text,
        scid_nag*   out_nag);


    /**
     * @brief Formats a NAG code as a dollar string or typographical symbol.
     *
     * @param[in]  nag               The NAG code to format (1..255).
     * @param[in]  as_symbol         Non-zero to format as typographical glyph (e.g. `"!"`, `"+="`),
     *                               or zero to emit dollar notation (e.g. `"$1"`, `"$14"`).
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated formatted
     * text. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               NAG formatted successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_nag_create_from_string()
     */
    SCID_API scid_error
    scid_nag_to_string(
        scid_nag nag,
        int      as_symbol,
        char*    out_text,
        size_t   out_text_capacity,
        size_t*  out_text_size);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
