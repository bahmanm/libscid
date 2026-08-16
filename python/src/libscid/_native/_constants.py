SCID_OK = 0
SCID_ERROR_BUFFER_FULL = 601

SCID_FILTER_ALL_GAMES = -1
SCID_FILTER_PRIMARY = -2

SCID_BOARD_SEARCH_MATCH_EXACT = 0
SCID_BOARD_SEARCH_MATCH_PAWNS = 1
SCID_BOARD_SEARCH_MATCH_FILES = 2

SCID_MAX_LEGAL_MOVES = 256

SCID_WHITE = 0
SCID_BLACK = 1

SCID_PIECE_SYMBOLS: dict[int, str | None] = {
    0: None,
    1: "K",
    2: "Q",
    3: "R",
    4: "B",
    5: "N",
    6: "P",
    9: "k",
    10: "q",
    11: "r",
    12: "b",
    13: "n",
    14: "p",
}


SCID_GAME_MERGE_MOVES_APPEND = 0

STANDARD_FEN = b"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
