#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>

void test_database(void) {
    scid_database* database = NULL;
    size_t count = 99;
    int is_open = 0;

    assert(scid_database_create_memory("scratch", &database) == SCID_OK);
    assert(database != NULL);

    assert(scid_database_is_open(database, &is_open) == SCID_OK);
    assert(is_open == 1);

    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 0);

    assert(scid_database_create_memory(NULL, &database) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_memory("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(database, NULL) == SCID_ERROR_BAD_ARG);

    scid_database_free(database);
    scid_database_free(NULL);
}
