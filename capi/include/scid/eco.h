/**
 * @file eco.h
 * @brief Encyclopaedia of Chess Openings (ECO) classification and book management.
 */

#ifndef SCID_ECO_H
#define SCID_ECO_H

#include "scid/_platform.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup eco ECO Classification
     * @brief Opening classification, ECO code codecs, and opening book lookup.
     * @{
     */

    /**
     * @name ECO Code Operations
     * @brief Types, format specifiers, and codecs for 16-bit packed ECO opening codes.
     * @{
     */

    /**
     * @brief Compact numeric representation of an ECO opening code.
     *
     * Encodes standard alphanumeric opening classifications (such as "B20" or "C50a1")
     * into a compact 16-bit integer value suitable for storage, sorting, and indexing.
     *
     * @see scid_eco_code_from_string()
     * @see scid_eco_code_to_string()
     */
    typedef unsigned short scid_eco_code;

    /**
     * @brief ECO code constants and capacity limits.
     */
    enum scid_eco_constants
    {
        /** Sentinel value indicating no ECO classification or an unclassified position. */
        SCID_ECO_NONE = 0,

        /** Maximum buffer size required to store any formatted ECO code string including null
         * terminator. */
        SCID_ECO_STRING_CAPACITY = 6
    };

    /**
     * @brief Output formatting mode for ECO code string serialisation.
     */
    typedef int scid_eco_format;

    /**
     * @brief ECO string format specifiers.
     */
    enum scid_eco_formats
    {
        /** Basic 3-character format without sub-variations (e.g. "B20", "C84"). */
        SCID_ECO_FORMAT_BASIC = 0,

        /** Extended format including optional sub-variation suffixes (e.g. "B91a4", "C50a"). */
        SCID_ECO_FORMAT_EXTENDED = 1
    };


    /**
     * @brief Parses an alphanumeric ECO string into a compact numeric ECO code.
     *
     * Converts standard notation such as "B20" or "C50a1" into a `scid_eco_code`.
     * If the input string is not a valid ECO designator, @ref SCID_ECO_NONE is written
     * to @p out_code and @ref SCID_OK is returned.
     *
     * @param[in]  text     Null-terminated ASCII/UTF-8 ECO string. Must not be NULL.
     * @param[out] out_code Pointer to the `scid_eco_code` receiving the parsed code value. Must not
     * be NULL.
     *
     * @retval SCID_OK           String parsed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p text or @p out_code is NULL.
     *
     * @see scid_eco_code_to_string()
     */
    SCID_API scid_error
    scid_eco_code_from_string(
        const char*    text,
        scid_eco_code* out_code);


    /**
     * @brief Formats a numeric ECO code into an alphanumeric string.
     *
     * Writes the formatted code into the caller-provided buffer according to the
     * requested formatting style (@p format).
     *
     * @param[in]  code              The numeric ECO code to format.
     * @param[in]  format            Formatting mode (@ref SCID_ECO_FORMAT_BASIC or @ref
     * SCID_ECO_FORMAT_EXTENDED).
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated string.
     *                               May be NULL if @p out_text_capacity is 0 to query required
     * capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes (at least @ref
     * SCID_ECO_STRING_CAPACITY recommended).
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               String formatted successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p format is invalid or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @note If @p code is @ref SCID_ECO_NONE, an empty string `""` with size 0 is emitted.
     *
     * @see scid_eco_code_from_string()
     */
    SCID_API scid_error
    scid_eco_code_to_string(
        scid_eco_code   code,
        scid_eco_format format,
        char*           out_text,
        size_t          out_text_capacity,
        size_t*         out_text_size);

    /** @} */


    /**
     * @name ECO Book Management
     * @brief Opening book database lifecycle and board position classification queries.
     * @{
     */

    /**
     * @brief Opaque handle representing a loaded ECO opening classification book.
     *
     * An ECO book contains position-to-code and position-to-name mapping tables
     * loaded from a Scid `.eco` database file on disk.
     *
     * Instances are loaded using @ref scid_eco_book_load() and must be released
     * when no longer needed using @ref scid_eco_book_free().
     *
     * @note Instances are thread-safe for concurrent read-only queries (@ref
     * scid_eco_book_code_find() and @ref scid_eco_book_name_find()).
     *
     * @see scid_eco_book_load()
     * @see scid_eco_book_free()
     */
    typedef struct scid_eco_book scid_eco_book;


    /**
     * @brief Loads an ECO classification book from a file on disk.
     *
     * Reads and parses a Scid `.eco` book file, indexing all defined position
     * entries for fast lookup.
     *
     * @param[in]  path     Path to the `.eco` book file on disk. Must not be NULL.
     * @param[out] out_book Pointer to a handle pointer receiving the newly allocated
     *                      `scid_eco_book` instance on success. Must not be NULL.
     *
     * @retval SCID_OK              Book successfully loaded.
     * @retval SCID_ERROR_BAD_ARG   If @p path or @p out_book is NULL.
     * @retval SCID_ERROR_FILE_OPEN If the file does not exist or cannot be opened.
     * @retval SCID_ERROR_CORRUPT   If file parsing fails due to syntax corruption.
     *
     * @note Ownership of the loaded book is transferred to the caller. The caller
     *       must release it using @ref scid_eco_book_free().
     *
     * @see scid_eco_book_free()
     */
    SCID_API scid_error
    scid_eco_book_load(
        const char*     path,
        scid_eco_book** out_book);


    /**
     * @brief Releases an ECO classification book handle and all associated memory.
     *
     * @param[in,out] book Pointer to the ECO book handle to release. If NULL, this
     *                     function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_eco_book_free(scid_eco_book* book);


    /**
     * @brief Looks up the ECO code corresponding to an exact board position.
     *
     * Searches the loaded opening book for an exact match of the given board state.
     * If the position is not present in the book, @ref SCID_ECO_NONE is written to @p out_code.
     *
     * @param[in]  book     Pointer to the loaded ECO book. Must not be NULL.
     * @param[in]  position Pointer to the board position to classify. Must not be NULL.
     * @param[out] out_code Pointer receiving the matching numeric ECO code, or @ref SCID_ECO_NONE
     * if unclassified. Must not be NULL.
     *
     * @retval SCID_OK           Lookup completed successfully.
     * @retval SCID_ERROR_BAD_ARG If @p book, @p position, or @p out_code is NULL.
     *
     * @see scid_eco_book_name_find()
     */
    SCID_API scid_error
    scid_eco_book_code_find(
        const scid_eco_book* book,
        const scid_position* position,
        scid_eco_code*       out_code);


    /**
     * @brief Looks up the opening name corresponding to an exact board position.
     *
     * Searches the loaded opening book for an exact match of the given board state
     * and writes the opening name string (e.g. "Sicilian Defence", "Italian Game")
     * into @p out_text. If the position is not classified, an empty string `""` is written.
     *
     * @param[in]  book              Pointer to the loaded ECO book. Must not be NULL.
     * @param[in]  position          Pointer to the board position to classify. Must not be NULL.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated opening
     * name. May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null
     * terminator), or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               Lookup completed successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p book, @p position, or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient.
     *
     * @see scid_eco_book_code_find()
     */
    SCID_API scid_error
    scid_eco_book_name_find(
        const scid_eco_book* book,
        const scid_position* position,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
