#include "scid/eco.h"

#include "scid/libscid/handles/eco_book.h"
#include "scid/libscid/handles/position.h"
#include "scid/libscid/support.h"

#include "scid/eco/book.h"
#include "scid/eco/code.h"

#include <memory>
#include <utility>

using namespace scid::libscid;

scid_error
scid_eco_code_from_string(
    const char*    text,
    scid_eco_code* out_code)
{
    if (any_null(text, out_code))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        *out_code = scid::eco::fromString(text);
        return SCID_OK;
    });
}


scid_error
scid_eco_code_to_string(
    scid_eco_code   code,
    scid_eco_format format,
    char*           out_text,
    size_t          out_text_capacity,
    size_t*         out_text_size)
{
    if (any_null(out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    if (format != SCID_ECO_FORMAT_BASIC && format != SCID_ECO_FORMAT_EXTENDED)
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        scid::eco::String text = {};
        scid::eco::toString(code, text, format == SCID_ECO_FORMAT_EXTENDED);
        return write_text(text, out_text, out_text_capacity, out_text_size);
    });
}


scid_error
scid_eco_book_load(
    const char*     path,
    scid_eco_book** out_book)
{
    if (any_null(path, out_book))
    {
        return SCID_ERROR_BAD_ARG;
    }

    *out_book = nullptr;

    return abi_guard([&]() -> scid_error {
        auto book = scid::eco::Book::load(path);
        if (!book)
        {
            return book.error();
        }

        auto book_handle = std::make_unique<scid_eco_book>(std::move(*book));
        *out_book = book_handle.release();
        return SCID_OK;
    });
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
    if (any_null(book, position, out_code))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        *out_code = book->value.findEco(position->value);
        return SCID_OK;
    });
}


scid_error
scid_eco_book_name_find(
    const scid_eco_book* book,
    const scid_position* position,
    char*                out_text,
    size_t               out_text_capacity,
    size_t*              out_text_size)
{
    if (any_null(book, position, out_text_size))
    {
        return SCID_ERROR_BAD_ARG;
    }

    return abi_guard([&]() -> scid_error {
        return write_text(
            eco_name_from_line(book->value.findEcoString(position->value)), out_text,
            out_text_capacity, out_text_size);
    });
}
