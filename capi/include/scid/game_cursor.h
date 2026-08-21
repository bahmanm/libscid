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

    typedef struct scid_game scid_game;

    typedef struct scid_game_cursor scid_game_cursor;


    SCID_API scid_error
    scid_game_cursor_create(
        scid_game*         game,
        scid_game_cursor** out_cursor);


    SCID_API scid_error
    scid_game_cursor_clone(
        scid_game*              game,
        const scid_game_cursor* source_cursor,
        scid_game_cursor**      out_cursor);


    SCID_API void
    scid_game_cursor_free(scid_game_cursor* cursor);


    SCID_API scid_error
    scid_game_cursor_position_get(
        const scid_game_cursor* cursor,
        scid_position*          out_position);


    SCID_API scid_error
    scid_game_cursor_ply_get(
        const scid_game_cursor* cursor,
        size_t*                 out_ply);


    SCID_API scid_error
    scid_game_cursor_variation_count_get(
        const scid_game_cursor* cursor,
        size_t*                 out_count);


    SCID_API scid_error
    scid_game_cursor_variation_depth_get(
        const scid_game_cursor* cursor,
        size_t*                 out_depth);


    SCID_API scid_error
    scid_game_cursor_variation_index_get(
        const scid_game_cursor* cursor,
        size_t*                 out_index);


    SCID_API scid_error
    scid_game_cursor_is_line_start(
        const scid_game_cursor* cursor,
        int*                    out_is_line_start);


    SCID_API scid_error
    scid_game_cursor_is_line_end(
        const scid_game_cursor* cursor,
        int*                    out_is_line_end);


    SCID_API scid_error
    scid_game_cursor_is_game_start(
        const scid_game_cursor* cursor,
        int*                    out_is_game_start);


    SCID_API scid_error
    scid_game_cursor_is_game_end(
        const scid_game_cursor* cursor,
        int*                    out_is_game_end);


    SCID_API scid_error
    scid_game_cursor_is_variation_start(
        const scid_game_cursor* cursor,
        int*                    out_is_variation_start);


    SCID_API scid_error
    scid_game_cursor_is_variation_end(
        const scid_game_cursor* cursor,
        int*                    out_is_variation_end);


    SCID_API scid_error
    scid_game_cursor_is_variation_empty(
        const scid_game_cursor* cursor,
        int*                    out_is_variation_empty);


    SCID_API scid_error
    scid_game_cursor_comment_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    SCID_API scid_error
    scid_game_cursor_comment_set(
        scid_game*              game,
        const scid_game_cursor* cursor,
        const char*             comment);


    SCID_API scid_error
    scid_game_cursor_previous_movespec_get(
        const scid_game_cursor* cursor,
        scid_movespec*          out_move);


    SCID_API scid_error
    scid_game_cursor_previous_move_san_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    SCID_API scid_error
    scid_game_cursor_previous_move_comment_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    SCID_API scid_error
    scid_game_cursor_previous_move_nag_count_get(
        const scid_game_cursor* cursor,
        size_t*                 out_count);


    SCID_API scid_error
    scid_game_cursor_previous_move_nag_at_get(
        const scid_game_cursor* cursor,
        size_t                  index,
        scid_nag*               out_nag);


    SCID_API scid_error
    scid_game_cursor_next_movespec_get(
        const scid_game_cursor* cursor,
        scid_movespec*          out_move);


    SCID_API scid_error
    scid_game_cursor_next_move_san_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    SCID_API scid_error
    scid_game_cursor_next_move_comment_get(
        const scid_game_cursor* cursor,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);


    SCID_API scid_error
    scid_game_cursor_next_move_nag_count_get(
        const scid_game_cursor* cursor,
        size_t*                 out_count);


    SCID_API scid_error
    scid_game_cursor_next_move_nag_at_get(
        const scid_game_cursor* cursor,
        size_t                  index,
        scid_nag*               out_nag);


    SCID_API scid_error
    scid_game_cursor_to_start(
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_start_cursor);


    SCID_API scid_error
    scid_game_cursor_to_end(
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_end_cursor);


    SCID_API scid_error
    scid_game_cursor_to_ply(
        const scid_game_cursor* cursor,
        size_t                  ply,
        int*                    out_moved,
        scid_game_cursor**      out_ply_cursor);


    SCID_API scid_error
    scid_game_cursor_next(
        const scid_game_cursor* cursor,
        int*                    out_moved,
        scid_game_cursor**      out_next_cursor);


    SCID_API scid_error
    scid_game_cursor_previous(
        const scid_game_cursor* cursor,
        int*                    out_moved,
        scid_game_cursor**      out_previous_cursor);


    SCID_API scid_error
    scid_game_cursor_variation_enter(
        const scid_game_cursor* cursor,
        size_t                  index,
        int*                    out_entered,
        scid_game_cursor**      out_variation_cursor);


    SCID_API scid_error
    scid_game_cursor_variation_exit(
        const scid_game_cursor* cursor,
        int*                    out_exited,
        scid_game_cursor**      out_parent_cursor);


    SCID_API scid_error
    scid_game_cursor_move_add(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_movespec           move,
        scid_game_cursor**      out_next_cursor);


    SCID_API scid_error
    scid_game_cursor_variation_add(
        scid_game*              game,
        const scid_game_cursor* cursor,
        const char*             initial_comment,
        int*                    out_added,
        scid_game_cursor**      out_variation_cursor);


    SCID_API scid_error
    scid_game_cursor_nag_add(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_nag                nag,
        int*                    out_added);


    SCID_API scid_error
    scid_game_cursor_nag_remove(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int                     is_move_nag,
        int*                    out_removed);


    SCID_API scid_error
    scid_game_cursor_nag_clear(
        scid_game*              game,
        const scid_game_cursor* cursor);


    SCID_API scid_error
    scid_game_cursor_variation_promote_to_first(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int*                    out_promoted,
        scid_game_cursor**      out_promoted_cursor);


    SCID_API scid_error
    scid_game_cursor_variation_promote_to_mainline(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int*                    out_promoted,
        scid_game_cursor**      out_mainline_cursor);


    SCID_API scid_error
    scid_game_cursor_variation_delete(
        scid_game*              game,
        const scid_game_cursor* cursor,
        int*                    out_deleted,
        scid_game_cursor**      out_parent_cursor);


    SCID_API scid_error
    scid_game_cursor_truncate(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_cursor);


    SCID_API scid_error
    scid_game_cursor_truncate_before_cursor(
        scid_game*              game,
        const scid_game_cursor* cursor,
        scid_game_cursor**      out_cursor);

#ifdef __cplusplus
}
#endif

#endif
