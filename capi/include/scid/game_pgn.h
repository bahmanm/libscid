/**
 * @file game_pgn.h
 * @brief Portable Game Notation (PGN) serialisation, export formatting, and encoding options.
 */

#ifndef SCID_GAME_PGN_H
#define SCID_GAME_PGN_H

#include "scid/_platform.h"
#include "scid/game.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup game_pgn PGN Serialisation & Formatting
     * @brief Portable Game Notation (PGN) text generation, formatting flags, and export configuration.
     * @{
     */

    /**
     * @name PGN Formatting Options
     * @brief Configuration handle and setter functions controlling PGN encoding behavior.
     * @{
     */

    /**
     * @brief Opaque configuration handle for customizing PGN text generation.
     *
     * Holds encoding preferences including symbolic NAG rendering, inclusion of
     * supplemental tags, commentary emission, variation expansion, and line wrapping.
     *
     * Options handles are created using @ref scid_game_pgn_options_create() with default
     * settings, modified via setter functions, and destroyed via @ref scid_game_pgn_options_free().
     *
     * @see scid_game_pgn_options_create()
     * @see scid_game_pgn_options_free()
     */
    typedef struct scid_game_pgn_options scid_game_pgn_options;


    /**
     * @brief Allocates and initialises a new PGN export options handle with default settings.
     *
     * Default settings:
     * - Symbolic NAGs: enabled (`1`)
     * - Supplemental tags: enabled (`1`)
     * - Comments: enabled (`1`)
     * - Variations: enabled (`1`)
     * - Line width: 80 characters
     *
     * @param[out] out_options Pointer to a handle pointer receiving the newly allocated options.
     *                         Must not be NULL.
     *
     * @retval SCID_OK           Options handle created successfully.
     * @retval SCID_ERROR_BAD_ARG If @p out_options is NULL.
     *
     * @note The caller acquires ownership of @p out_options and must release it with @ref scid_game_pgn_options_free().
     *
     * @see scid_game_pgn_options_free()
     */
    SCID_API scid_error
    scid_game_pgn_options_create(scid_game_pgn_options** out_options);


    /**
     * @brief Releases a PGN options handle and all associated resources.
     *
     * @param[in,out] options Pointer to the options handle to release. If NULL, this
     *                        function performs no action.
     *
     * @note Passing NULL is guaranteed to be a safe no-op.
     */
    SCID_API void
    scid_game_pgn_options_free(scid_game_pgn_options* options);


    /**
     * @brief Configures whether Numeric Annotation Glyphs (NAGs) are rendered symbolically or numerically.
     *
     * When enabled (`1`), standard glyphs (NAG 1..6) are output as symbols (`!`, `?`, `!!`, `??`, `!?`, `?!`).
     * When disabled (`0`), all NAGs are output in standard numeric format (`$1`, `$2`, etc.).
     *
     * @param[in,out] options Pointer to the options handle. Must not be NULL.
     * @param[in]     enabled Non-zero (`1`) for symbolic glyphs; zero (`0`) for numeric `$N` codes.
     *
     * @retval SCID_OK           Option set successfully.
     * @retval SCID_ERROR_BAD_ARG If @p options is NULL.
     */
    SCID_API scid_error
    scid_game_pgn_options_symbolic_nags_set(
        scid_game_pgn_options* options,
        int                    enabled);


    /**
     * @brief Configures whether non-standard supplemental header tags are emitted in the PGN tag roster.
     *
     * The mandatory Seven Tag Roster (STR) and `FEN` (for non-standard setups) are always emitted.
     * Non-standard supplemental tags (e.g. `WhiteElo`, `ECO`, `Annotator`, `Time`) are included when enabled.
     *
     * @param[in,out] options Pointer to the options handle. Must not be NULL.
     * @param[in]     enabled Non-zero (`1`) to include supplemental tags; zero (`0`) to omit them.
     *
     * @retval SCID_OK           Option set successfully.
     * @retval SCID_ERROR_BAD_ARG If @p options is NULL.
     */
    SCID_API scid_error
    scid_game_pgn_options_supplemental_tags_set(
        scid_game_pgn_options* options,
        int                    enabled);


    /**
     * @brief Configures whether move and position comments are included in the exported movetext.
     *
     * @param[in,out] options Pointer to the options handle. Must not be NULL.
     * @param[in]     enabled Non-zero (`1`) to include `{ comment }` blocks; zero (`0`) to strip commentary.
     *
     * @retval SCID_OK           Option set successfully.
     * @retval SCID_ERROR_BAD_ARG If @p options is NULL.
     */
    SCID_API scid_error
    scid_game_pgn_options_comments_set(
        scid_game_pgn_options* options,
        int                    enabled);


    /**
     * @brief Configures whether sub-variations are recursively included in the exported movetext.
     *
     * @param[in,out] options Pointer to the options handle. Must not be NULL.
     * @param[in]     enabled Non-zero (`1`) to include `( variation )` branches; zero (`0`) for mainline only.
     *
     * @retval SCID_OK           Option set successfully.
     * @retval SCID_ERROR_BAD_ARG If @p options is NULL.
     */
    SCID_API scid_error
    scid_game_pgn_options_variations_set(
        scid_game_pgn_options* options,
        int                    enabled);


    /**
     * @brief Sets the maximum line width in characters for movetext line wrapping.
     *
     * Standard PGN format recommends a line limit of 80 characters.
     * Setting @p line_width to `0` disables line wrapping (unbounded single lines for movetext).
     *
     * @param[in,out] options    Pointer to the options handle. Must not be NULL.
     * @param[in]     line_width Maximum line width in columns, or `0` for unbounded line length.
     *
     * @retval SCID_OK           Option set successfully.
     * @retval SCID_ERROR_BAD_ARG If @p options is NULL.
     */
    SCID_API scid_error
    scid_game_pgn_options_line_width_set(
        scid_game_pgn_options* options,
        unsigned               line_width);

    /** @} */


    /**
     * @name PGN Generation
     * @brief Formatting and exporting game entities into standard PGN text.
     * @{
     */

    /**
     * @brief Encodes a game into Portable Game Notation (PGN) text.
     *
     * Exports the game's tag roster, commentary, NAG annotations, and movetext according
     * to the formatting preferences defined in @p options (or default settings if @p options is NULL).
     *
     * @param[in]  game              Pointer to the game entity to encode. Must not be NULL.
     * @param[in]  options           Pointer to custom export options, or NULL to use default settings.
     * @param[out] out_text          Caller-allocated buffer receiving the null-terminated PGN text.
     *                               May be NULL if @p out_text_capacity is 0 to query required capacity.
     * @param[in]  out_text_capacity Capacity of @p out_text in bytes.
     * @param[out] out_text_size     Pointer receiving the number of bytes written (excluding null terminator),
     *                               or required capacity if the buffer is too small. Must not be NULL.
     *
     * @retval SCID_OK               PGN text encoded successfully.
     * @retval SCID_ERROR_BAD_ARG    If @p game or @p out_text_size is NULL.
     * @retval SCID_ERROR_BUFFER_FULL If @p out_text_capacity is insufficient to hold the full PGN output.
     *
     * @see scid_game_create()
     * @see scid_game_pgn_options_create()
     */
    SCID_API scid_error
    scid_game_to_pgn(
        const scid_game*             game,
        const scid_game_pgn_options* options,
        char*                        out_text,
        size_t                       out_text_capacity,
        size_t*                      out_text_size);

    /** @} */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif
