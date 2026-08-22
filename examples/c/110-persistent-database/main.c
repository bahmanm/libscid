#include <scid/scid.h>

#include <stdio.h>
#include <string.h>

static int
check(
    scid_error  error,
    const char* call)
{
    if (error == SCID_OK)
    {
        return 1;
    }

    fprintf(stderr, "%s failed with scid_error %hu\n", call, error);
    return 0;
}


static int
text_equals(
    const char* text,
    size_t      text_size,
    const char* expected)
{
    return text_size == strlen(expected) && strncmp(text, expected, text_size) == 0;
}


static void
remove_scid5_database(const char* path)
{
    char filename[256];

    snprintf(filename, sizeof(filename), "%s.si5", path);
    remove(filename);

    snprintf(filename, sizeof(filename), "%s.sg5", path);
    remove(filename);

    snprintf(filename, sizeof(filename), "%s.sn5", path);
    remove(filename);
}


static int
parse_pgn(
    const char* pgn,
    scid_game** out_game)
{
    const char*    start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    scid_position* position = NULL;
    char           diagnostic[1024];
    size_t         diagnostic_size = 0;

    if (!check(
            scid_position_create_from_fen(start_fen, &position), "scid_position_create_from_fen") ||
        !check(
            scid_game_create(
                position, pgn, strlen(pgn), out_game, diagnostic, sizeof(diagnostic),
                &diagnostic_size),
            "scid_game_create"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_position_free(position);
        return 0;
    }

    scid_position_free(position);
    return 1;
}


int
main(void)
{
    const char*    path = "libscid-example-persistent-database";
    const char*    pgn = "[Event \"Persistent\"]\n"
                         "[Site \"Toronto\"]\n"
                         "[Date \"2026.06.14\"]\n"
                         "[White \"Alpha\"]\n"
                         "[Black \"Beta\"]\n"
                         "[Result \"1-0\"]\n"
                         "\n"
                         "1. e4 e5 2. Nf3 1-0\n";
    scid_database* created = NULL;
    scid_database* reopened = NULL;
    scid_game*     game = NULL;
    char           event[128];
    char           filename[256];
    char           description[128];
    char           metadata_key[32];
    char           metadata_value[128];
    char           min_date[32];
    char           max_date[32];
    char           result[16];
    char           type[16];
    char           flags[22];
    size_t         count = 0;
    size_t         description_size = 0;
    size_t         event_size = 0;
    size_t         filename_size = 0;
    size_t         result_size = 0;
    size_t         metadata_count = 0;
    size_t         metadata_key_size = 0;
    size_t         metadata_value_size = 0;
    size_t         min_date_size = 0;
    size_t         max_date_size = 0;
    size_t         result_count = 0;
    size_t         type_size = 0;
    size_t         flags_size = 0;
    int            is_open = 0;
    int            read_only = 0;

    remove_scid5_database(path);

    if (!parse_pgn(pgn, &game) ||
        !check(scid_database_create_scid5(path, &created), "scid_database_create_scid5") ||
        !check(scid_database_is_open(created, &is_open), "scid_database_is_open") || !is_open ||
        !check(scid_database_game_add(created, game, "M"), "scid_database_game_add") ||
        !check(scid_database_game_count_get(created, &count), "scid_database_game_count_get") ||
        count != 1 ||
        !check(
            scid_database_metadata_set(created, "description", "Example persistent database"),
            "scid_database_metadata_set") ||
        !check(scid_database_save(created), "scid_database_save") ||
        !check(scid_database_close(created), "scid_database_close") ||
        !check(scid_database_is_open(created, &is_open), "scid_database_is_open") || is_open)
    {
        scid_game_free(game);
        scid_database_free(created);
        remove_scid5_database(path);
        return 1;
    }

    scid_database_free(created);
    created = NULL;

    if (!check(
            scid_database_open_scid5_read_only(path, NULL, NULL, NULL, NULL, &reopened),
            "scid_database_open_scid5_read_only") ||
        !check(
            scid_database_type_get(reopened, type, sizeof(type), &type_size),
            "scid_database_type_get") ||
        !text_equals(type, type_size, "scid5") ||
        !check(
            scid_database_filename_get(reopened, filename, sizeof(filename), &filename_size),
            "scid_database_filename_get") ||
        !text_equals(filename, filename_size, "libscid-example-persistent-database.si5") ||
        !check(scid_database_read_only_get(reopened, &read_only), "scid_database_read_only_get") ||
        !read_only ||
        !check(
            scid_database_metadata_get(
                reopened, "description", description, sizeof(description), &description_size),
            "scid_database_metadata_get") ||
        !text_equals(description, description_size, "Example persistent database") ||
        !check(
            scid_database_metadata_count_get(reopened, &metadata_count),
            "scid_database_metadata_count_get") ||
        metadata_count != 9 ||
        !check(
            scid_database_metadata_at_get(
                reopened, 1, metadata_key, sizeof(metadata_key), &metadata_key_size, metadata_value,
                sizeof(metadata_value), &metadata_value_size),
            "scid_database_metadata_at_get") ||
        !text_equals(metadata_key, metadata_key_size, "description") ||
        !text_equals(metadata_value, metadata_value_size, "Example persistent database") ||
        !check(
            scid_database_stats_date_range_get(
                reopened, min_date, sizeof(min_date), &min_date_size, max_date, sizeof(max_date),
                &max_date_size),
            "scid_database_stats_date_range_get") ||
        !text_equals(min_date, min_date_size, "2026.06.14") ||
        !text_equals(max_date, max_date_size, "2026.06.14") ||
        !check(
            scid_database_stats_result_count_get(reopened, "1-0", &result_count),
            "scid_database_stats_result_count_get") ||
        result_count != 1 ||
        !check(scid_database_game_count_get(reopened, &count), "scid_database_game_count_get") ||
        count != 1 ||
        !check(
            scid_database_game_tag_get(reopened, 0, "Event", event, sizeof(event), &event_size),
            "scid_database_game_tag_get") ||
        !text_equals(event, event_size, "Persistent") ||
        !check(
            scid_database_game_result_get(reopened, 0, result, sizeof(result), &result_size),
            "scid_database_game_result_get") ||
        !text_equals(result, result_size, "1-0") ||
        !check(
            scid_database_game_get(reopened, 0, &game, flags, sizeof(flags), &flags_size),
            "scid_database_game_get") ||
        !text_equals(flags, flags_size, "M"))
    {
        scid_game_free(game);
        scid_database_free(reopened);
        remove_scid5_database(path);
        return 1;
    }

    printf(
        "reopened %.*s %.*s: %.*s %.*s flags=%.*s description=%.*s\n", (int)type_size, type,
        (int)filename_size, filename, (int)event_size, event, (int)result_size, result,
        (int)flags_size, flags, (int)description_size, description);

    scid_game_free(game);
    scid_database_close(reopened);
    scid_database_free(reopened);
    remove_scid5_database(path);
    return 0;
}
