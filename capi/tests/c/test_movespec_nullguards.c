#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>

static void
test_movespec_create_nullguards(void)
{
    TEST_ASSERT(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_movespec_create_from_uci_nullguards(void)
{
    scid_movespec move;

    TEST_ASSERT(scid_movespec_create_from_uci(NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci("e2e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci(NULL, NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_movespec_to_uci_nullguards(void)
{
    scid_movespec move;
    char          buffer[16];
    size_t        text_size = 0;

    TEST_ASSERT(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, &move) == SCID_OK);

    TEST_ASSERT(scid_movespec_to_uci(move, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_to_uci(move, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_movespec_to_uci(move, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 4);
}

static void
test_movespec_create_from_san_nullguards(void)
{
    scid_position* position = NULL;
    scid_movespec  move;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_movespec_create_from_san(NULL, "e4", &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(position, NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(position, "e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(NULL, NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(NULL, "e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(position, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_movespec_to_san_nullguards(void)
{
    scid_position* position = NULL;
    scid_movespec  move;
    char           buffer[16];
    size_t         text_size = 0;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);
    TEST_ASSERT(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, &move) == SCID_OK);

    TEST_ASSERT(scid_movespec_to_san(NULL, move, buffer, sizeof(buffer), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_to_san(position, move, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_to_san(NULL, move, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_to_san(NULL, move, NULL, 0, &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_to_san(position, move, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_movespec_to_san(position, move, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);

    scid_position_free(position);
}

void
test_movespec_nullguards(void)
{
    test_movespec_create_nullguards();
    test_movespec_create_from_uci_nullguards();
    test_movespec_to_uci_nullguards();
    test_movespec_create_from_san_nullguards();
    test_movespec_to_san_nullguards();
}
