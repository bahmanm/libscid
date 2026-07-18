#include "scid/primitives.h"

#include "scid/libscid/support.h"

#include "scid/core/nags.h"
#include "scid/core/primitives.h"

#include <cctype>
#include <cstring>
#include <string>
#include <string_view>

using namespace scid::libscid;

scid_error
scid_square_from_string(
    const char*  text,
    scid_square* out_square)
{
    return parse_square(text, out_square);
}


scid_error
scid_square_to_string(
    scid_square square,
    char*       out_text,
    size_t      out_text_capacity,
    size_t*     out_text_size)
{
    if (!square_is_valid(square))
    {
        return SCID_ERROR_BAD_ARG;
    }

    char text[] = {
        static_cast<char>('a' + scid::core::square_Fyle(square)),
        static_cast<char>('1' + scid::core::square_Rank(square)), '\0'};

    return write_text(text, out_text, out_text_capacity, out_text_size);
}


scid_error
scid_piece_type_from_string(
    const char* text,
    scid_piece* out_piece)
{
    if (text == nullptr || out_piece == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (text[0] == '\0' || text[1] != '\0')
    {
        return SCID_ERROR_BAD_ARG;
    }

    switch (text[0])
    {
        case 'K':
        case 'k':
            *out_piece = scid::core::KING;
            return SCID_OK;
        case 'Q':
        case 'q':
            *out_piece = scid::core::QUEEN;
            return SCID_OK;
        case 'R':
        case 'r':
            *out_piece = scid::core::ROOK;
            return SCID_OK;
        case 'B':
        case 'b':
            *out_piece = scid::core::BISHOP;
            return SCID_OK;
        case 'N':
        case 'n':
            *out_piece = scid::core::KNIGHT;
            return SCID_OK;
        case 'P':
        case 'p':
            *out_piece = scid::core::PAWN;
            return SCID_OK;
        default:
            return SCID_ERROR_BAD_ARG;
    }
}

scid_error
scid_nag_create_from_string(
    const char* text,
    scid_nag*   out_nag)
{
    if (text == nullptr || out_nag == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_nag = scid::core::nagCode(scid::core::nagFromString(text));
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_nag_to_string(
    scid_nag nag,
    int      as_symbol,
    char*    out_text,
    size_t   out_text_capacity,
    size_t*  out_text_size)
{
    try
    {
        const std::string text =
            scid::core::nagToString(scid::core::nagFromCode(nag), as_symbol != 0);

        return write_text(text, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}
