from __future__ import annotations

import ctypes

from ._types import ScidMoveSpec


def bind_functions(lib: ctypes.CDLL) -> None:
    c_size_t_p = ctypes.POINTER(ctypes.c_size_t)
    c_void_p_p = ctypes.POINTER(ctypes.c_void_p)
    c_int_p = ctypes.POINTER(ctypes.c_int)
    c_ubyte_p = ctypes.POINTER(ctypes.c_ubyte)

    def bind(
        name: str,
        argtypes: list[object],
        restype: object = ctypes.c_ushort,
    ) -> None:
        function = getattr(lib, name)
        function.argtypes = argtypes
        function.restype = restype

    bind("scid_position_create_from_fen", [ctypes.c_char_p, c_void_p_p])
    bind("scid_position_free", [ctypes.c_void_p], None)
    bind(
        "scid_position_to_fen",
        [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )

    bind("scid_nag_create_from_string", [ctypes.c_char_p, c_ubyte_p])
    bind(
        "scid_nag_to_string",
        [ctypes.c_ubyte, ctypes.c_int, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
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
        "scid_game_cursor_variation_enter",
        [ctypes.c_void_p, ctypes.c_size_t, c_int_p, c_void_p_p],
    )
    bind("scid_game_cursor_variation_exit", [ctypes.c_void_p, c_int_p, c_void_p_p])
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
    bind(
        "scid_movespec_to_uci",
        [ScidMoveSpec, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
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
        [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, c_size_t_p],
    )
