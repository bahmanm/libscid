#include "test_libscid.h"

#include "scid/scid.h"

void
test_piece(void)
{
    scid_piece piece = SCID_PIECE_NONE;

    TEST_ASSERT(scid_piece_type_from_string("K", &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_KING);

    TEST_ASSERT(scid_piece_type_from_string("q", &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_QUEEN);

    TEST_ASSERT(scid_piece_type_from_string("N", &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_KNIGHT);

    TEST_ASSERT(scid_piece_type_from_string("P", &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_PAWN);

    TEST_ASSERT(scid_piece_type_from_string("", &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_piece_type_from_string("QQ", &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_piece_type_from_string("x", &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_piece_type_from_string(NULL, &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_piece_type_from_string("Q", NULL) == SCID_ERROR_BAD_ARG);
}
