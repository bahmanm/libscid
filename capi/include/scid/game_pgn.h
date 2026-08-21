#ifndef SCID_GAME_PGN_H
#define SCID_GAME_PGN_H

#include "scid/_platform.h"
#include "scid/game.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_game_pgn_options scid_game_pgn_options;


    SCID_API scid_error
    scid_game_pgn_options_create(scid_game_pgn_options** out_options);


    SCID_API void
    scid_game_pgn_options_free(scid_game_pgn_options* options);


    SCID_API scid_error
    scid_game_pgn_options_symbolic_nags_set(
        scid_game_pgn_options* options,
        int                    enabled);


    SCID_API scid_error
    scid_game_pgn_options_supplemental_tags_set(
        scid_game_pgn_options* options,
        int                    enabled);


    SCID_API scid_error
    scid_game_pgn_options_comments_set(
        scid_game_pgn_options* options,
        int                    enabled);


    SCID_API scid_error
    scid_game_pgn_options_variations_set(
        scid_game_pgn_options* options,
        int                    enabled);


    SCID_API scid_error
    scid_game_pgn_options_line_width_set(
        scid_game_pgn_options* options,
        unsigned               line_width);


    SCID_API scid_error
    scid_game_to_pgn(
        const scid_game*             game,
        const scid_game_pgn_options* options,
        char*                        out_text,
        size_t                       out_text_capacity,
        size_t*                      out_text_size);

#ifdef __cplusplus
}
#endif

#endif
