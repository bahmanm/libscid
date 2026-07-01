#ifndef SCID_SCID_H
#define SCID_SCID_H

#include <stddef.h>

#ifndef SCID_API
#if defined(_WIN32) && defined(SCID_SHARED)
#if defined(SCID_BUILDING_LIBRARY)
#define SCID_API __declspec(dllexport)
#else
#define SCID_API __declspec(dllimport)
#endif
#else
#define SCID_API
#endif
#endif

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

    typedef enum scid_colour
    {
        SCID_WHITE = 0,
        SCID_BLACK = 1
    } scid_colour;

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

    typedef struct scid_movespec
    {
            scid_square from;
            scid_square to;
            scid_piece promotion;
            int is_castling;
    } scid_movespec;

    typedef unsigned short scid_eco_code;

    enum
    {
        SCID_ECO_NONE = 0,
        SCID_ECO_STRING_CAPACITY = 6
    };

    typedef enum scid_eco_format
    {
        SCID_ECO_FORMAT_BASIC = 0,
        SCID_ECO_FORMAT_EXTENDED = 1
    } scid_eco_format;

    typedef struct scid_position scid_position;

    typedef struct scid_game scid_game;

    typedef struct scid_game_cursor scid_game_cursor;

    typedef struct scid_eco_book scid_eco_book;

    typedef struct scid_database scid_database;

    typedef enum scid_game_merge_moves_mode
    {
        SCID_GAME_MERGE_MOVES_APPEND = 0,
        SCID_GAME_MERGE_MOVES_INSERT_VARIATION = 1,
        SCID_GAME_MERGE_MOVES_REPLACE = 2
    } scid_game_merge_moves_mode;

    SCID_API scid_error
    scid_square_from_string(const char* text, scid_square* out_square);

    SCID_API scid_error
    scid_square_to_string(
        scid_square square,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_piece_type_from_string(const char* text, scid_piece* out_piece);

    SCID_API scid_error
    scid_movespec_create(
        scid_square from,
        scid_square to,
        scid_piece promotion,
        int is_castling,
        scid_movespec* out_move);

    SCID_API scid_error
    scid_movespec_create_from_uci(const char* text, scid_movespec* out_move);

    SCID_API scid_error
    scid_movespec_create_from_san(
        const scid_position* position,
        const char* text,
        scid_movespec* out_move);

    SCID_API scid_error
    scid_movespec_to_uci(
        scid_movespec move,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_movespec_to_san(
        const scid_position* position,
        scid_movespec move,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_nag_create_from_string(const char* text, scid_nag* out_nag);

    SCID_API scid_error
    scid_nag_to_string(
        scid_nag nag,
        int as_symbol,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_position_create_standard(scid_position** out_position);

    SCID_API scid_error
    scid_position_create_empty(scid_position** out_position);

    SCID_API scid_error
    scid_position_create_from_fen(const char* fen, scid_position** out_position);

    SCID_API scid_error
    scid_position_create_with_san(
        const scid_position* position,
        const char* san,
        scid_position** out_position);

    SCID_API scid_error
    scid_position_create_with_uci(
        const scid_position* position,
        const char* uci,
        scid_position** out_position);

    SCID_API void
    scid_position_free(scid_position* position);

    SCID_API scid_error
    scid_position_to_fen(
        const scid_position* position,
        char* out_fen,
        size_t out_fen_capacity,
        size_t* out_fen_size);

    SCID_API scid_error
    scid_position_apply_san(scid_position* position, const char* san);

    SCID_API scid_error
    scid_position_apply_uci(scid_position* position, const char* uci);

    SCID_API scid_error
    scid_position_is_start(const scid_position* position, int* out_is_start);

    SCID_API scid_error
    scid_position_is_check(const scid_position* position, int* out_is_check);

    SCID_API scid_error
    scid_position_is_checkmate(const scid_position* position, int* out_is_checkmate);

    SCID_API scid_error
    scid_position_is_legal(const scid_position* position, int* out_is_legal);

    SCID_API scid_error
    scid_position_side_to_move_get(const scid_position* position, scid_colour* out_side_to_move);

    SCID_API scid_error
    scid_position_fullmove_number_get(const scid_position* position, unsigned* out_fullmove_number);

    SCID_API scid_error
    scid_position_halfmove_clock_get(const scid_position* position, unsigned* out_halfmove_clock);

    SCID_API scid_error
    scid_position_piece_at_get(
        const scid_position* position,
        scid_square square,
        scid_piece* out_piece);

    SCID_API scid_error
    scid_game_create_empty(scid_game** out_game);

    SCID_API scid_error
    scid_game_create_from_position(const scid_position* position, scid_game** out_game);

    SCID_API scid_error
    scid_game_create_from_moves(
        const scid_position* position,
        const scid_movespec* moves,
        size_t move_count,
        scid_game** out_game);

    SCID_API scid_error
    scid_game_create_from_pgn(
        const char* pgn,
        size_t pgn_size,
        scid_game** out_game,
        char* out_diagnostic,
        size_t out_diagnostic_capacity,
        size_t* out_diagnostic_size);

    SCID_API void
    scid_game_free(scid_game* game);

    SCID_API scid_error
    scid_game_to_pgn(
        const scid_game* game,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_mainline_halfmove_count_get(const scid_game* game, size_t* out_count);

    SCID_API scid_error
    scid_game_initial_comment_get(
        const scid_game* game,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_movetext_clear(scid_game* game);

    SCID_API scid_error
    scid_game_tag_get(
        const scid_game* game,
        const char* name,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_tag_set(scid_game* game, const char* name, const char* value);

    SCID_API scid_error
    scid_game_tag_count_get(const scid_game* game, size_t* out_count);

    SCID_API scid_error
    scid_game_tag_at_get(
        const scid_game* game,
        size_t index,
        char* out_name,
        size_t out_name_capacity,
        size_t* out_name_size,
        char* out_value,
        size_t out_value_capacity,
        size_t* out_value_size);

    SCID_API scid_error
    scid_game_tag_remove(scid_game* game, const char* name, int* out_removed);

    SCID_API scid_error
    scid_game_start_position_get(const scid_game* game, scid_position* out_position);

    SCID_API scid_error
    scid_game_final_position_get(const scid_game* game, scid_position* out_position);

    SCID_API scid_error
    scid_game_merge_moves(
        scid_game* target_game,
        const scid_game_cursor* target_cursor,
        const scid_game* source_game,
        scid_game_merge_moves_mode mode,
        scid_game_cursor** out_cursor);

    SCID_API scid_error
    scid_game_cursor_create(scid_game* game, scid_game_cursor** out_cursor);

    SCID_API scid_error
    scid_game_cursor_clone(
        scid_game* game,
        const scid_game_cursor* source_cursor,
        scid_game_cursor** out_cursor);

    SCID_API void
    scid_game_cursor_free(scid_game_cursor* cursor);

    SCID_API scid_error
    scid_game_cursor_position_get(
        const scid_game_cursor* cursor,
        scid_position* out_position);

    SCID_API scid_error
    scid_game_cursor_ply_get(const scid_game_cursor* cursor, size_t* out_ply);

    SCID_API scid_error
    scid_game_cursor_variation_count_get(const scid_game_cursor* cursor, size_t* out_count);

    SCID_API scid_error
    scid_game_cursor_variation_depth_get(const scid_game_cursor* cursor, size_t* out_depth);

    SCID_API scid_error
    scid_game_cursor_variation_index_get(const scid_game_cursor* cursor, size_t* out_index);

    SCID_API scid_error
    scid_game_cursor_is_line_start(const scid_game_cursor* cursor, int* out_is_line_start);

    SCID_API scid_error
    scid_game_cursor_is_line_end(const scid_game_cursor* cursor, int* out_is_line_end);

    SCID_API scid_error
    scid_game_cursor_is_game_start(const scid_game_cursor* cursor, int* out_is_game_start);

    SCID_API scid_error
    scid_game_cursor_is_game_end(const scid_game_cursor* cursor, int* out_is_game_end);

    SCID_API scid_error
    scid_game_cursor_is_variation_start(
        const scid_game_cursor* cursor,
        int* out_is_variation_start);

    SCID_API scid_error
    scid_game_cursor_is_variation_end(
        const scid_game_cursor* cursor,
        int* out_is_variation_end);

    SCID_API scid_error
    scid_game_cursor_is_variation_empty(
        const scid_game_cursor* cursor,
        int* out_is_variation_empty);

    SCID_API scid_error
    scid_game_cursor_comment_get(
        const scid_game_cursor* cursor,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_cursor_comment_set(
        scid_game* game,
        const scid_game_cursor* cursor,
        const char* comment);

    SCID_API scid_error
    scid_game_cursor_previous_movespec_get(
        const scid_game_cursor* cursor,
        scid_movespec* out_move);

    SCID_API scid_error
    scid_game_cursor_previous_move_san_get(
        const scid_game_cursor* cursor,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_cursor_previous_move_comment_get(
        const scid_game_cursor* cursor,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_cursor_previous_move_nag_count_get(
        const scid_game_cursor* cursor,
        size_t* out_count);

    SCID_API scid_error
    scid_game_cursor_previous_move_nag_at_get(
        const scid_game_cursor* cursor,
        size_t index,
        scid_nag* out_nag);

    SCID_API scid_error
    scid_game_cursor_next_movespec_get(
        const scid_game_cursor* cursor,
        scid_movespec* out_move);

    SCID_API scid_error
    scid_game_cursor_next_move_san_get(
        const scid_game_cursor* cursor,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_cursor_next_move_comment_get(
        const scid_game_cursor* cursor,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_game_cursor_next_move_nag_count_get(
        const scid_game_cursor* cursor,
        size_t* out_count);

    SCID_API scid_error
    scid_game_cursor_next_move_nag_at_get(
        const scid_game_cursor* cursor,
        size_t index,
        scid_nag* out_nag);

    SCID_API scid_error
    scid_game_cursor_to_start(
        const scid_game_cursor* cursor,
        scid_game_cursor** out_start_cursor);

    SCID_API scid_error
    scid_game_cursor_to_end(
        const scid_game_cursor* cursor,
        scid_game_cursor** out_end_cursor);

    SCID_API scid_error
    scid_game_cursor_to_ply(
        const scid_game_cursor* cursor,
        size_t ply,
        int* out_moved,
        scid_game_cursor** out_ply_cursor);

    SCID_API scid_error
    scid_game_cursor_next(
        const scid_game_cursor* cursor,
        int* out_moved,
        scid_game_cursor** out_next_cursor);

    SCID_API scid_error
    scid_game_cursor_previous(
        const scid_game_cursor* cursor,
        int* out_moved,
        scid_game_cursor** out_previous_cursor);

    SCID_API scid_error
    scid_game_cursor_variation_enter(
        const scid_game_cursor* cursor,
        size_t index,
        int* out_entered,
        scid_game_cursor** out_variation_cursor);

    SCID_API scid_error
    scid_game_cursor_variation_exit(
        const scid_game_cursor* cursor,
        int* out_exited,
        scid_game_cursor** out_parent_cursor);

    SCID_API scid_error
    scid_game_cursor_move_add(
        scid_game* game,
        const scid_game_cursor* cursor,
        scid_movespec move,
        scid_game_cursor** out_next_cursor);

    SCID_API scid_error
    scid_game_cursor_variation_add(
        scid_game* game,
        const scid_game_cursor* cursor,
        const char* initial_comment,
        int* out_added,
        scid_game_cursor** out_variation_cursor);

    SCID_API scid_error
    scid_game_cursor_nag_add(
        scid_game* game,
        const scid_game_cursor* cursor,
        scid_nag nag,
        int* out_added);

    SCID_API scid_error
    scid_game_cursor_nag_remove(
        scid_game* game,
        const scid_game_cursor* cursor,
        int is_move_nag,
        int* out_removed);

    SCID_API scid_error
    scid_game_cursor_nag_clear(scid_game* game, const scid_game_cursor* cursor);

    SCID_API scid_error
    scid_game_cursor_variation_promote_to_first(
        scid_game* game,
        const scid_game_cursor* cursor,
        int* out_promoted,
        scid_game_cursor** out_promoted_cursor);

    SCID_API scid_error
    scid_game_cursor_variation_promote_to_mainline(
        scid_game* game,
        const scid_game_cursor* cursor,
        int* out_promoted,
        scid_game_cursor** out_mainline_cursor);

    SCID_API scid_error
    scid_game_cursor_variation_delete(
        scid_game* game,
        const scid_game_cursor* cursor,
        int* out_deleted,
        scid_game_cursor** out_parent_cursor);

    SCID_API scid_error
    scid_game_cursor_truncate(
        scid_game* game,
        const scid_game_cursor* cursor,
        scid_game_cursor** out_cursor);

    SCID_API scid_error
    scid_game_cursor_truncate_before_cursor(
        scid_game* game,
        const scid_game_cursor* cursor,
        scid_game_cursor** out_cursor);

    SCID_API scid_error
    scid_eco_code_from_string(const char* text, scid_eco_code* out_code);

    SCID_API scid_error
    scid_eco_code_to_string(
        scid_eco_code code,
        scid_eco_format format,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_eco_book_load(const char* path, scid_eco_book** out_book);

    SCID_API void
    scid_eco_book_free(scid_eco_book* book);

    SCID_API scid_error
    scid_eco_book_code_find(
        const scid_eco_book* book,
        const scid_position* position,
        scid_eco_code* out_code);

    SCID_API scid_error
    scid_eco_book_name_find(
        const scid_eco_book* book,
        const scid_position* position,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_create_memory(const char* name, scid_database** out_database);

    SCID_API scid_error
    scid_database_create_scid5(const char* path, scid_database** out_database);

    SCID_API scid_error
    scid_database_open_scid5(const char* path, scid_database** out_database);

    SCID_API scid_error
    scid_database_open_scid5_read_only(const char* path, scid_database** out_database);

    SCID_API scid_error
    scid_database_close(scid_database* database);

    SCID_API void
    scid_database_free(scid_database* database);

    SCID_API scid_error
    scid_database_is_open(const scid_database* database, int* out_is_open);

    SCID_API scid_error
    scid_database_filename_get(
        const scid_database* database,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_type_get(
        const scid_database* database,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_read_only_get(const scid_database* database, int* out_read_only);

    SCID_API scid_error
    scid_database_filter_create(
        scid_database* database,
        char* out_filter_id,
        size_t out_filter_id_capacity,
        size_t* out_filter_id_size);

    SCID_API scid_error
    scid_database_filter_delete(scid_database* database, const char* filter_id);

    SCID_API scid_error
    scid_database_filter_fill(scid_database* database, const char* filter_id, unsigned value);

    SCID_API scid_error
    scid_database_filter_value_set(
        scid_database* database,
        const char* filter_id,
        size_t game_index,
        unsigned value);

    SCID_API scid_error
    scid_database_filter_value_get(
        const scid_database* database,
        const char* filter_id,
        size_t game_index,
        unsigned* out_value);

    SCID_API scid_error
    scid_database_filter_count_get(
        const scid_database* database,
        const char* filter_id,
        size_t* out_count);

    SCID_API scid_error
    scid_database_filter_game_at_get(
        const scid_database* database,
        const char* filter_id,
        size_t index,
        size_t* out_game_index);

    SCID_API scid_error
    scid_database_game_list_get(
        const scid_database* database,
        const char* filter_id,
        const char* sort_criteria,
        size_t start,
        size_t count,
        size_t* out_game_indexes,
        size_t out_game_indexes_capacity,
        size_t* out_game_indexes_count);

    SCID_API scid_error
    scid_database_game_sorted_position_get(
        const scid_database* database,
        const char* filter_id,
        const char* sort_criteria,
        size_t game_index,
        size_t* out_position);

    SCID_API scid_error
    scid_database_save(scid_database* database);

    SCID_API scid_error
    scid_database_metadata_get(
        const scid_database* database,
        const char* key,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_metadata_set(scid_database* database, const char* key, const char* value);

    SCID_API scid_error
    scid_database_metadata_count_get(const scid_database* database, size_t* out_count);

    SCID_API scid_error
    scid_database_metadata_at_get(
        const scid_database* database,
        size_t index,
        char* out_key,
        size_t out_key_capacity,
        size_t* out_key_size,
        char* out_value,
        size_t out_value_capacity,
        size_t* out_value_size);

    SCID_API scid_error
    scid_database_stats_date_range_get(
        const scid_database* database,
        char* out_min_date,
        size_t out_min_date_capacity,
        size_t* out_min_date_size,
        char* out_max_date,
        size_t out_max_date_capacity,
        size_t* out_max_date_size);

    SCID_API scid_error
    scid_database_stats_result_count_get(
        const scid_database* database,
        const char* result,
        size_t* out_count);

    SCID_API scid_error
    scid_database_game_count_get(const scid_database* database, size_t* out_count);

    SCID_API scid_error
    scid_database_import_pgn(
        scid_database* database,
        const char* pgn,
        size_t pgn_size,
        char* out_diagnostic,
        size_t out_diagnostic_capacity,
        size_t* out_diagnostic_size,
        size_t* out_imported_count);

    SCID_API scid_error
    scid_database_game_add(scid_database* database, const scid_game* game, const char* flags);

    SCID_API scid_error
    scid_database_game_replace(
        scid_database* database,
        size_t index,
        const scid_game* game,
        const char* flags);

    SCID_API scid_error
    scid_database_game_delete(scid_database* database, size_t index);

    SCID_API scid_error
    scid_database_game_undelete(scid_database* database, size_t index);

    SCID_API scid_error
    scid_database_game_get(
        const scid_database* database,
        size_t index,
        scid_game** out_game,
        char* out_flags,
        size_t out_flags_capacity,
        size_t* out_flags_size);

    SCID_API scid_error
    scid_database_game_export_pgn(
        const scid_database* database,
        size_t index,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_game_tag_get(
        const scid_database* database,
        size_t index,
        const char* name,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_game_halfmove_count_get(
        const scid_database* database,
        size_t index,
        size_t* out_count);

    SCID_API scid_error
    scid_database_game_number_get(const scid_database* database, size_t index, size_t* out_number);

    SCID_API scid_error
    scid_database_game_deleted_get(const scid_database* database, size_t index, int* out_deleted);

    SCID_API scid_error
    scid_database_game_result_get(
        const scid_database* database,
        size_t index,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

    SCID_API scid_error
    scid_database_game_eco_get(
        const scid_database* database,
        size_t index,
        scid_eco_code* out_code);

    SCID_API scid_error
    scid_database_game_date_get(
        const scid_database* database,
        size_t index,
        char* out_text,
        size_t out_text_capacity,
        size_t* out_text_size);

#ifdef __cplusplus
}
#endif

#endif
