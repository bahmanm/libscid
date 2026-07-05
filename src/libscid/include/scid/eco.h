#ifndef SCID_ECO_H
#define SCID_ECO_H

#include "scid/_platform.h"
#include "scid/position.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned short scid_eco_code;

    enum
    {
        SCID_ECO_NONE = 0,
        SCID_ECO_STRING_CAPACITY = 6
    };

    typedef int scid_eco_format;

    enum
    {
        SCID_ECO_FORMAT_BASIC = 0,
        SCID_ECO_FORMAT_EXTENDED = 1
    };

    typedef struct scid_eco_book scid_eco_book;


    SCID_API scid_error
    scid_eco_code_from_string(
        const char*    text,
        scid_eco_code* out_code);


    SCID_API scid_error
    scid_eco_code_to_string(
        scid_eco_code   code,
        scid_eco_format format,
        char*           out_text,
        size_t          out_text_capacity,
        size_t*         out_text_size);


    SCID_API scid_error
    scid_eco_book_load(
        const char*     path,
        scid_eco_book** out_book);


    SCID_API void
    scid_eco_book_free(scid_eco_book* book);


    SCID_API scid_error
    scid_eco_book_code_find(
        const scid_eco_book* book,
        const scid_position* position,
        scid_eco_code*       out_code);


    SCID_API scid_error
    scid_eco_book_name_find(
        const scid_eco_book* book,
        const scid_position* position,
        char*                out_text,
        size_t               out_text_capacity,
        size_t*              out_text_size);

#ifdef __cplusplus
}
#endif

#endif
