#include "scid/eco.h"

#include "scid/libscid/handles/eco_book.h"
#include "scid/libscid/handles/position.h"
#include "scid/libscid/support.h"

#include "scid/eco/book.h"
#include "scid/eco/code.h"

#include <utility>

using namespace scid::libscid;

scid_error
scid_eco_code_from_string(
    const char*    text,
    scid_eco_code* out_code)
{
    if (text == nullptr || out_code == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_code = scid::eco::fromString(text);
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_eco_code_to_string(
    scid_eco_code   code,
    scid_eco_format format,
    char*           out_text,
    size_t          out_text_capacity,
    size_t*         out_text_size)
{
    if (format != SCID_ECO_FORMAT_BASIC && format != SCID_ECO_FORMAT_EXTENDED)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        scid::eco::String text = {};
        scid::eco::toString(code, text, format == SCID_ECO_FORMAT_EXTENDED);
        return write_text(text, out_text, out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_eco_book_load(
    const char*     path,
    scid_eco_book** out_book)
{
    if (path == nullptr || out_book == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        auto book = scid::eco::Book::load(path);
        if (!book)
        {
            *out_book = nullptr;
            return book.error();
        }

        *out_book = new scid_eco_book{std::move(*book)};
        return SCID_OK;
    }
    catch (...)
    {
        *out_book = nullptr;
        return SCID_ERROR;
    }
}


void
scid_eco_book_free(scid_eco_book* book)
{
    delete book;
}


scid_error
scid_eco_book_code_find(
    const scid_eco_book* book,
    const scid_position* position,
    scid_eco_code*       out_code)
{
    if (book == nullptr || position == nullptr || out_code == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        *out_code = book->value.findEco(position->value);
        return SCID_OK;
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}


scid_error
scid_eco_book_name_find(
    const scid_eco_book* book,
    const scid_position* position,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (book == nullptr || position == nullptr)
    {
        return SCID_ERROR_BAD_ARG;
    }

    try
    {
        return write_text(
            eco_name_from_line(book->value.findEcoString(position->value)), out_text,
            out_text_capacity, out_text_size);
    }
    catch (...)
    {
        return SCID_ERROR;
    }
}
