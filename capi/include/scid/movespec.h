#ifndef SCID_MOVESPEC_H
#define SCID_MOVESPEC_H

#include "scid/_platform.h"
#include "scid/primitives.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SCID_POSITION_TYPEDEF
#define SCID_POSITION_TYPEDEF
    typedef struct scid_position scid_position;
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
