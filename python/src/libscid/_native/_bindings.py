from __future__ import annotations

import ctypes

from ._types import (
    NativeProgressReportCallback,
    NativeShouldCancelFn,
    ScidMoveSpec,
)


def bind_functions(lib: ctypes.CDLL) -> None:
    c_size_t_p = ctypes.POINTER(ctypes.c_size_t)
    c_void_p_p = ctypes.POINTER(ctypes.c_void_p)
    c_int_p = ctypes.POINTER(ctypes.c_int)
    c_uint_p = ctypes.POINTER(ctypes.c_uint)
    c_ubyte_p = ctypes.POINTER(ctypes.c_ubyte)
    scid_filter_id = ctypes.c_int
    scid_filter_id_p = ctypes.POINTER(scid_filter_id)

    def bind(
        name: str,
        argtypes: list[object],
        restype: object = ctypes.c_ushort,
    ) -> None:
        function = getattr(lib, name)
        function.argtypes = argtypes
        function.restype = restype

    bind("scid_square_from_string", [ctypes.c_char_p, c_uint_p])

    bind("scid_position_create_from_fen", [ctypes.c_char_p, c_void_p_p])
    bind("scid_position_free", [ctypes.c_void_p], None)
    bind(
        "scid_position_to_fen",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind("scid_position_side_to_move_get", [ctypes.c_void_p, c_int_p])
    bind("scid_position_fullmove_number_get", [ctypes.c_void_p, c_uint_p])
    bind("scid_position_halfmove_clock_get", [ctypes.c_void_p, c_uint_p])
    bind("scid_position_is_check", [ctypes.c_void_p, c_int_p])
    bind("scid_position_is_checkmate", [ctypes.c_void_p, c_int_p])
    bind("scid_position_piece_at_get", [ctypes.c_void_p, ctypes.c_uint, c_uint_p])
    bind("scid_position_apply_san", [ctypes.c_void_p, ctypes.c_char_p])
    bind("scid_position_apply_uci", [ctypes.c_void_p, ctypes.c_char_p])
    bind(
        "scid_position_legal_moves",
        [
            ctypes.c_void_p,
            ctypes.POINTER(ScidMoveSpec),
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )

    bind("scid_nag_create_from_string", [ctypes.c_char_p, c_ubyte_p])
    bind(
        "scid_nag_to_string",
        [ctypes.c_ubyte, ctypes.c_int, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind(
        "scid_movespec_create_from_san",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ScidMoveSpec)],
    )
    bind(
        "scid_movespec_to_uci",
        [ScidMoveSpec, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind(
        "scid_movespec_to_san",
        [
            ctypes.c_void_p,
            ScidMoveSpec,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )

    bind(
        "scid_game_create",
        [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_void_p_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )
    bind("scid_game_create_blank", [ctypes.c_void_p, c_void_p_p])
    bind("scid_game_free", [ctypes.c_void_p], None)

    bind("scid_database_free", [ctypes.c_void_p], None)
    bind("scid_database_close", [ctypes.c_void_p])
    bind(
        "scid_database_open_pgn_read_only",
        [
            ctypes.c_char_p,
            NativeProgressReportCallback,
            ctypes.c_void_p,
            NativeShouldCancelFn,
            ctypes.c_void_p,
            c_void_p_p,
        ],
    )
    bind(
        "scid_database_type_get",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind("scid_database_read_only_get", [ctypes.c_void_p, c_int_p])
    bind("scid_database_game_count_get", [ctypes.c_void_p, c_size_t_p])
    bind("scid_database_filter_create", [ctypes.c_void_p, scid_filter_id_p])
    bind("scid_database_filter_delete", [ctypes.c_void_p, scid_filter_id])
    bind(
        "scid_database_filter_game_count_get",
        [ctypes.c_void_p, scid_filter_id, c_size_t_p],
    )
    bind(
        "scid_database_filter_game_indices_get",
        [
            ctypes.c_void_p,
            scid_filter_id,
            ctypes.c_char_p,
            ctypes.c_size_t,
            ctypes.c_size_t,
            c_size_t_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )
    bind(
        "scid_database_filter_game_index_at_row_get",
        [ctypes.c_void_p, scid_filter_id, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind(
        "scid_database_filter_game_row_for_index_get",
        [ctypes.c_void_p, scid_filter_id, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind("scid_search_header_criteria_create", [c_void_p_p])
    bind("scid_search_header_criteria_free", [ctypes.c_void_p], None)
    bind("scid_search_header_criteria_player_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind("scid_search_header_criteria_white_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind("scid_search_header_criteria_black_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind("scid_search_header_criteria_event_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind("scid_search_header_criteria_site_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind(
        "scid_search_header_criteria_site_country_set",
        [ctypes.c_void_p, ctypes.c_char_p],
    )
    bind("scid_search_header_criteria_round_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind("scid_search_header_criteria_result_set", [ctypes.c_void_p, ctypes.c_char_p])
    bind(
        "scid_search_header_criteria_date_range_set",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p],
    )
    bind(
        "scid_search_header_criteria_event_date_range_set",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p],
    )
    bind(
        "scid_search_header_criteria_eco_range_set",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p],
    )
    bind(
        "scid_search_header_criteria_game_number_range_set",
        [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t],
    )
    bind(
        "scid_search_header_criteria_halfmove_count_range_set",
        [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t],
    )
    bind(
        "scid_search_header_criteria_white_elo_range_set",
        [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t],
    )
    bind(
        "scid_search_header_criteria_black_elo_range_set",
        [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t],
    )
    bind(
        "scid_search_header_criteria_elo_difference_range_set",
        [ctypes.c_void_p, ctypes.c_int, ctypes.c_int],
    )
    bind(
        "scid_search_header_criteria_has_variations_set",
        [ctypes.c_void_p, ctypes.c_int],
    )
    bind(
        "scid_search_header_criteria_has_comments_set", [ctypes.c_void_p, ctypes.c_int]
    )
    bind("scid_search_header_criteria_has_nags_set", [ctypes.c_void_p, ctypes.c_int])

    bind("scid_search_board_criteria_create", [c_void_p_p])
    bind("scid_search_board_criteria_free", [ctypes.c_void_p], None)
    bind("scid_search_board_criteria_position_set", [ctypes.c_void_p, ctypes.c_void_p])
    bind("scid_search_board_criteria_match_set", [ctypes.c_void_p, ctypes.c_int])
    bind(
        "scid_search_board_criteria_include_variations_set",
        [ctypes.c_void_p, ctypes.c_int],
    )
    bind(
        "scid_search_board_criteria_include_flipped_set",
        [ctypes.c_void_p, ctypes.c_int],
    )

    bind(
        "scid_database_search_headers",
        [
            ctypes.c_void_p,
            scid_filter_id,
            scid_filter_id,
            ctypes.c_void_p,
            NativeProgressReportCallback,
            ctypes.c_void_p,
            NativeShouldCancelFn,
            ctypes.c_void_p,
        ],
    )
    bind(
        "scid_database_search_position",
        [
            ctypes.c_void_p,
            scid_filter_id,
            scid_filter_id,
            ctypes.c_void_p,
            NativeProgressReportCallback,
            ctypes.c_void_p,
            NativeShouldCancelFn,
            ctypes.c_void_p,
        ],
    )
    bind(
        "scid_database_search_board",
        [
            ctypes.c_void_p,
            scid_filter_id,
            scid_filter_id,
            ctypes.c_void_p,
            NativeProgressReportCallback,
            ctypes.c_void_p,
            NativeShouldCancelFn,
            ctypes.c_void_p,
        ],
    )
    bind(
        "scid_database_game_tag_get",
        [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )
    bind(
        "scid_database_game_get",
        [
            ctypes.c_void_p,
            ctypes.c_size_t,
            c_void_p_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )

    bind("scid_game_pgn_options_create", [c_void_p_p])
    bind("scid_game_pgn_options_free", [ctypes.c_void_p], None)
    bind("scid_game_pgn_options_symbolic_nags_set", [ctypes.c_void_p, ctypes.c_int])
    bind(
        "scid_game_pgn_options_supplemental_tags_set",
        [ctypes.c_void_p, ctypes.c_int],
    )
    bind("scid_game_pgn_options_comments_set", [ctypes.c_void_p, ctypes.c_int])
    bind("scid_game_pgn_options_variations_set", [ctypes.c_void_p, ctypes.c_int])
    bind("scid_game_pgn_options_line_width_set", [ctypes.c_void_p, ctypes.c_uint])

    bind("scid_game_mainline_halfmove_count_get", [ctypes.c_void_p, c_size_t_p])
    bind("scid_game_start_position_get", [ctypes.c_void_p, ctypes.c_void_p])
    bind("scid_game_final_position_get", [ctypes.c_void_p, ctypes.c_void_p])
    bind(
        "scid_game_merge_moves",
        [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int, c_void_p_p],
    )

    bind("scid_game_cursor_create", [ctypes.c_void_p, c_void_p_p])
    bind(
        "scid_game_cursor_clone",
        [ctypes.c_void_p, ctypes.c_void_p, c_void_p_p],
    )
    bind("scid_game_cursor_free", [ctypes.c_void_p], None)
    bind("scid_game_cursor_ply_get", [ctypes.c_void_p, c_size_t_p])
    bind("scid_game_cursor_variation_count_get", [ctypes.c_void_p, c_size_t_p])
    bind("scid_game_cursor_variation_depth_get", [ctypes.c_void_p, c_size_t_p])
    bind("scid_game_cursor_variation_index_get", [ctypes.c_void_p, c_size_t_p])
    bind("scid_game_cursor_position_get", [ctypes.c_void_p, ctypes.c_void_p])
    bind("scid_game_cursor_is_line_start", [ctypes.c_void_p, c_int_p])
    bind("scid_game_cursor_is_line_end", [ctypes.c_void_p, c_int_p])
    bind("scid_game_cursor_next", [ctypes.c_void_p, c_int_p, c_void_p_p])
    bind("scid_game_cursor_previous", [ctypes.c_void_p, c_int_p, c_void_p_p])
    bind("scid_game_cursor_to_start", [ctypes.c_void_p, c_void_p_p])
    bind("scid_game_cursor_to_end", [ctypes.c_void_p, c_void_p_p])
    bind(
        "scid_game_cursor_to_ply",
        [ctypes.c_void_p, ctypes.c_size_t, c_int_p, c_void_p_p],
    )
    bind(
        "scid_game_cursor_variation_enter",
        [ctypes.c_void_p, ctypes.c_size_t, c_int_p, c_void_p_p],
    )
    bind("scid_game_cursor_variation_exit", [ctypes.c_void_p, c_int_p, c_void_p_p])
    bind(
        "scid_game_cursor_move_add",
        [ctypes.c_void_p, ctypes.c_void_p, ScidMoveSpec, c_void_p_p],
    )
    bind(
        "scid_game_cursor_variation_add",
        [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p, c_int_p, c_void_p_p],
    )
    bind(
        "scid_game_cursor_variation_delete",
        [ctypes.c_void_p, ctypes.c_void_p, c_int_p, c_void_p_p],
    )
    bind(
        "scid_game_cursor_variation_promote_to_first",
        [ctypes.c_void_p, ctypes.c_void_p, c_int_p, c_void_p_p],
    )
    bind(
        "scid_game_cursor_variation_promote_to_mainline",
        [ctypes.c_void_p, ctypes.c_void_p, c_int_p, c_void_p_p],
    )
    bind("scid_game_cursor_truncate", [ctypes.c_void_p, ctypes.c_void_p, c_void_p_p])
    bind(
        "scid_game_cursor_truncate_before_cursor",
        [ctypes.c_void_p, ctypes.c_void_p, c_void_p_p],
    )
    bind(
        "scid_game_cursor_previous_move_san_get",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind(
        "scid_game_cursor_next_move_san_get",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind(
        "scid_game_cursor_previous_movespec_get",
        [ctypes.c_void_p, ctypes.POINTER(ScidMoveSpec)],
    )
    bind(
        "scid_game_cursor_next_movespec_get",
        [ctypes.c_void_p, ctypes.POINTER(ScidMoveSpec)],
    )
    bind("scid_game_cursor_previous_move_nag_count_get", [ctypes.c_void_p, c_size_t_p])
    bind(
        "scid_game_cursor_previous_move_nag_at_get",
        [ctypes.c_void_p, ctypes.c_size_t, c_ubyte_p],
    )
    bind("scid_game_cursor_next_move_nag_count_get", [ctypes.c_void_p, c_size_t_p])
    bind(
        "scid_game_cursor_next_move_nag_at_get",
        [ctypes.c_void_p, ctypes.c_size_t, c_ubyte_p],
    )
    bind(
        "scid_game_cursor_comment_get",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
    bind(
        "scid_game_cursor_comment_set",
        [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p],
    )
    bind(
        "scid_game_cursor_nag_add",
        [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ubyte, c_int_p],
    )
    bind(
        "scid_game_cursor_nag_remove",
        [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int, c_int_p],
    )
    bind("scid_game_cursor_nag_clear", [ctypes.c_void_p, ctypes.c_void_p])

    bind(
        "scid_game_tag_get",
        [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )
    bind("scid_game_tag_set", [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p])
    bind("scid_game_tag_count_get", [ctypes.c_void_p, c_size_t_p])
    bind(
        "scid_game_tag_at_get",
        [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )
    bind("scid_game_tag_remove", [ctypes.c_void_p, ctypes.c_char_p, c_int_p])
    bind(
        "scid_game_to_pgn",
        [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ],
    )
