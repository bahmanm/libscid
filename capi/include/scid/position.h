#ifndef SCID_POSITION_H
#define SCID_POSITION_H

#include "scid/_platform.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_position scid_position;


    SCID_API scid_error
    scid_position_create_from_fen(
        const char*     fen,
        scid_position** out_position);


    SCID_API scid_error
    scid_position_create_with_san(
        const scid_position* position,
        const char*          san,
        scid_position**      out_position);


    SCID_API scid_error
    scid_position_create_with_uci(
        const scid_position* position,
        const char*          uci,
        scid_position**      out_position);


    SCID_API void
    scid_position_free(scid_position* position);


    SCID_API scid_error
    scid_position_to_fen(
        const scid_position* position,
        char*                out_fen,
        size_t               out_fen_capacity,
        size_t*              out_fen_size);


    SCID_API scid_error
    scid_position_apply_san(
        scid_position* position,
        const char*    san);


    SCID_API scid_error
    scid_position_apply_uci(
        scid_position* position,
        const char*    uci);


    SCID_API scid_error
    scid_position_is_start(
        const scid_position* position,
        int*                 out_is_start);


    SCID_API scid_error
    scid_position_is_check(
        const scid_position* position,
        int*                 out_is_check);


    SCID_API scid_error
    scid_position_is_checkmate(
        const scid_position* position,
        int*                 out_is_checkmate);


    SCID_API scid_error
    scid_position_is_legal(
        const scid_position* position,
        int*                 out_is_legal);


    SCID_API scid_error
    scid_position_side_to_move_get(
        const scid_position* position,
        scid_colour*         out_side_to_move);


    SCID_API scid_error
    scid_position_fullmove_number_get(
        const scid_position* position,
        unsigned*            out_fullmove_number);


    SCID_API scid_error
    scid_position_halfmove_clock_get(
        const scid_position* position,
        unsigned*            out_halfmove_clock);


    SCID_API scid_error
    scid_position_piece_at_get(
        const scid_position* position,
        scid_square          square,
        scid_piece*          out_piece);

#ifdef __cplusplus
}
#endif

#endif
