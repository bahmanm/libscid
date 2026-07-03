#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>

void
test_piece(void)
{
    scid_piece piece = SCID_PIECE_NONE;

    assert(scid_piece_type_from_string("K", &piece) == SCID_OK);
    assert(piece == SCID_PIECE_KING);

    assert(scid_piece_type_from_string("q", &piece) == SCID_OK);
    assert(piece == SCID_PIECE_QUEEN);

    assert(scid_piece_type_from_string("N", &piece) == SCID_OK);
    assert(piece == SCID_PIECE_KNIGHT);

    assert(scid_piece_type_from_string("P", &piece) == SCID_OK);
    assert(piece == SCID_PIECE_PAWN);

    assert(scid_piece_type_from_string("", &piece) == SCID_ERROR_BAD_ARG);
    assert(scid_piece_type_from_string("QQ", &piece) == SCID_ERROR_BAD_ARG);
    assert(scid_piece_type_from_string("x", &piece) == SCID_ERROR_BAD_ARG);
    assert(scid_piece_type_from_string(NULL, &piece) == SCID_ERROR_BAD_ARG);
    assert(scid_piece_type_from_string("Q", NULL) == SCID_ERROR_BAD_ARG);
}
