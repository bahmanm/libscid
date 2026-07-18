#ifndef SCID_PRIMITIVES_H
#define SCID_PRIMITIVES_H

#include "scid/_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned short scid_error;

    enum
    {
        SCID_OK = 0,
        SCID_ERROR = 1,
        SCID_ERROR_BAD_ARG = 3,

        SCID_ERROR_FILE_OPEN = 101,
        SCID_ERROR_FILE_READ_ONLY = 111,
        SCID_ERROR_CORRUPT = 152,

        SCID_ERROR_INVALID_FEN = 301,
        SCID_ERROR_INVALID_MOVE = 302,

        SCID_ERROR_BUFFER_FULL = 601
    };

    typedef int scid_colour;

    enum
    {
        SCID_WHITE = 0,
        SCID_BLACK = 1
    };

    typedef unsigned scid_square;

    typedef unsigned scid_piece;

    enum
    {
        SCID_PIECE_NONE = 0,
        SCID_PIECE_KING = 1,
        SCID_PIECE_QUEEN = 2,
        SCID_PIECE_ROOK = 3,
        SCID_PIECE_BISHOP = 4,
        SCID_PIECE_KNIGHT = 5,
        SCID_PIECE_PAWN = 6,

        SCID_PIECE_WHITE_KING = 1,
        SCID_PIECE_WHITE_QUEEN = 2,
        SCID_PIECE_WHITE_ROOK = 3,
        SCID_PIECE_WHITE_BISHOP = 4,
        SCID_PIECE_WHITE_KNIGHT = 5,
        SCID_PIECE_WHITE_PAWN = 6,

        SCID_PIECE_BLACK_KING = 9,
        SCID_PIECE_BLACK_QUEEN = 10,
        SCID_PIECE_BLACK_ROOK = 11,
        SCID_PIECE_BLACK_BISHOP = 12,
        SCID_PIECE_BLACK_KNIGHT = 13,
        SCID_PIECE_BLACK_PAWN = 14
    };

    typedef unsigned char scid_nag;

    SCID_API scid_error
    scid_square_from_string(
        const char*  text,
        scid_square* out_square);


    SCID_API scid_error
    scid_square_to_string(
        scid_square square,
        char*       out_text,
        size_t      out_text_capacity,
        size_t*     out_text_size);


    SCID_API scid_error
    scid_piece_type_from_string(
        const char* text,
        scid_piece* out_piece);


    SCID_API scid_error
    scid_nag_create_from_string(
        const char* text,
        scid_nag*   out_nag);


    SCID_API scid_error
    scid_nag_to_string(
        scid_nag nag,
        int      as_symbol,
        char*    out_text,
        size_t   out_text_capacity,
        size_t*  out_text_size);

#ifdef __cplusplus
}
#endif

#endif
