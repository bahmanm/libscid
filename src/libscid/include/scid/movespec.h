#ifndef SCID_MOVESPEC_H
#define SCID_MOVESPEC_H

#include "scid/_platform.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scid_movespec
    {
            scid_square from;
            scid_square to;
            scid_piece  promotion;
            int         is_castling;
    } scid_movespec;


    SCID_API scid_error
    scid_movespec_create(
        scid_square    from,
        scid_square    to,
        scid_piece     promotion,
        int            is_castling,
        scid_movespec* out_move);


    SCID_API scid_error
    scid_movespec_create_from_uci(
        const char*    text,
        scid_movespec* out_move);


    SCID_API scid_error
    scid_movespec_create_from_san(
        const scid_position* position,
        const char*          text,
        scid_movespec*       out_move);


    SCID_API scid_error
    scid_movespec_to_uci(
        scid_movespec move,
        char*         out_text,
        size_t        out_text_capacity,
        size_t*       out_text_size);


    SCID_API scid_error
    scid_movespec_to_san(
        const scid_position* position,
        scid_movespec        move,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);

#ifdef __cplusplus
}
#endif

#endif
