#ifndef SCID_GAME_H
#define SCID_GAME_H

#include "scid/_platform.h"
#include "scid/movespec.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_game scid_game;

    typedef struct scid_game_cursor scid_game_cursor;

    typedef int scid_game_merge_moves_mode;

    enum
    {
        SCID_GAME_MERGE_MOVES_APPEND = 0,
        SCID_GAME_MERGE_MOVES_INSERT_VARIATION = 1,
        SCID_GAME_MERGE_MOVES_REPLACE = 2
    };


    SCID_API scid_error
    scid_game_create_blank(
        const scid_position* position,
        scid_game**          out_game);


    SCID_API scid_error
    scid_game_create(
        const scid_position* position,
        const char*          pgn,
        size_t               pgn_size,
        scid_game**          out_game,
        char*                out_diagnostic,
        size_t               out_diagnostic_capacity,
        size_t*              out_diagnostic_size);


    SCID_API void
    scid_game_free(scid_game* game);


    SCID_API scid_error
    scid_game_mainline_halfmove_count_get(
        const scid_game* game,
        size_t*          out_count);


    SCID_API scid_error
    scid_game_initial_comment_get(
        const scid_game* game,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);


    SCID_API scid_error
    scid_game_tag_get(
        const scid_game* game,
        const char*      name,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);


    SCID_API scid_error
    scid_game_tag_set(
        scid_game*  game,
        const char* name,
        const char* value);


    SCID_API scid_error
    scid_game_tag_count_get(
        const scid_game* game,
        size_t*          out_count);


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


    SCID_API scid_error
    scid_game_tag_remove(
        scid_game*  game,
        const char* name,
        int*        out_removed);


    SCID_API scid_error
    scid_game_start_position_get(
        const scid_game* game,
        scid_position*   out_position);


    SCID_API scid_error
    scid_game_final_position_get(
        const scid_game* game,
        scid_position*   out_position);


    SCID_API scid_error
    scid_game_merge_moves(
        scid_game*                 target_game,
        const scid_game_cursor*    target_cursor,
        const scid_game*           source_game,
        scid_game_merge_moves_mode mode,
        scid_game_cursor**         out_cursor);

#ifdef __cplusplus
}
#endif

#endif
