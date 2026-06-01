/*
 * Copyright (C) 2000-2004  Shane Hudson..
 *
 * This file is part of Scid (Shane's Chess Information Database).
 *
 * Scid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * Scid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scid.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 * Fundamental scalar types, piece codes, colours, and square coordinates.
 *
 * Core board code uses compact byte-sized values for pieces, colours, squares,
 * ranks, files, and directions.  Valid board squares are numbered from A1 = 0
 * through H8 = 63, with COLOR_SQUARE and NULL_SQUARE as sentinel entries used
 * by legacy board arrays.
 */

#pragma once

#include <cstdint>

namespace scid::core {

/** 8-bit unsigned scalar used by legacy file formats and board tables. */
typedef unsigned char byte;      //  8 bit unsigned
/** 16-bit unsigned scalar used by counters and compact encodings. */
typedef std::uint16_t ushort;    // 16 bit unsigned
/** 32-bit unsigned scalar used by hashes, sizes, and database fields. */
typedef std::uint32_t uint;      // 32 bit unsigned
/** 32-bit signed scalar used by scores and signed counters. */
typedef std::int32_t  sint;      // 32 bit signed

/** Piece code.  Values may be piece types, coloured pieces, EMPTY, or sentinels. */
typedef byte pieceT;        // e.g ROOK or WK
/** Colour code: WHITE, BLACK, or NOCOLOR. */
typedef byte colorT;        // WHITE or BLACK
/** Square code: A1..H8, COLOR_SQUARE, or NULL_SQUARE. */
typedef byte squareT;       // e.g. A3
/** Rank code: RANK_1..RANK_8 or NO_RANK. */
typedef byte rankT;         // Chess board rank
/** File code.  Named "fyle" to avoid confusion with disk files. */
typedef byte fyleT;         // Chess board file
/** Direction code used as an index into movement tables. */
typedef byte directionT;    // e.g. UP_LEFT
/** A1-H8 diagonal index. */
typedef byte leftDiagT;     // Up-left diagonals
/** H1-A8 diagonal index. */
typedef byte rightDiagT;    // Up-right diagonals
/** Castling side code: QSIDE or KSIDE. */
typedef byte castleDirT;    // LEFT or RIGHT

/** Queenside castling direction. */
const castleDirT QSIDE = 0;
/** Kingside castling direction. */
const castleDirT KSIDE = 1;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PIECES COLORS
/** Number of real chess colours. */
const unsigned NUM_COLOR_TYPES = 2;
/** White side. */
const colorT WHITE = 0;
/** Black side. */
const colorT BLACK = 1;
/** Sentinel colour for empty or non-board values. */
const colorT NOCOLOR = 2;
/** Printable colour letters used by legacy board strings. */
const char COLOR_CHAR[3] = {'W', 'B', '_'};

/** Returns the opposite real colour.  @p c must be WHITE or BLACK. */
inline colorT color_Flip(colorT c) { return 1 - c; }

/** Returns the legacy printable colour character for @p c. */
inline char color_Char(colorT c) { return COLOR_CHAR[c]; }

// PIECE TYPES (without color; same value as a white piece)
/** Invalid or absent piece type. */
const pieceT INVALID_PIECE = 0;
/** Uncoloured king type. */
const pieceT KING = 1;
/** Uncoloured queen type. */
const pieceT QUEEN = 2;
/** Uncoloured rook type. */
const pieceT ROOK = 3;
/** Uncoloured bishop type. */
const pieceT BISHOP = 4;
/** Uncoloured knight type. */
const pieceT KNIGHT = 5;
/** Uncoloured pawn type. */
const pieceT PAWN = 6;

// PIECES:
//   Note that color(x) == ((x & 0x8) >> 3)  and  type(x) == (x & 0x7)
//   EMPTY is deliberately nonzero, and END_OF_BOARD is zero, so that
//   a board can be used as a regular 0-terminated string, provided
//   that board[NULL_SQUARE] == END_OF_BOARD, as it always should be.
/** Empty board square marker. */
const pieceT EMPTY = 7;
/** Zero terminator for legacy board strings. */
const pieceT END_OF_BOARD = 0;
/** White king. */
const pieceT WK = 1;
/** White queen. */
const pieceT WQ = 2;
/** White rook. */
const pieceT WR = 3;
/** White bishop. */
const pieceT WB = 4;
/** White knight. */
const pieceT WN = 5;
/** White pawn. */
const pieceT WP = 6;
/** Black king. */
const pieceT BK = 9;
/** Black queen. */
const pieceT BQ = 10;
/** Black rook. */
const pieceT BR = 11;
/** Black bishop. */
const pieceT BB = 12;
/** Black knight. */
const pieceT BN = 13;
/** Black pawn. */
const pieceT BP = 14;

/** Returns the colour of a coloured piece, or NOCOLOR for EMPTY. */
inline colorT piece_Color(pieceT p) {
	return (p == EMPTY) ? NOCOLOR : ((p & 8) >> 3);
}
/** Faster colour extraction when @p p is known not to be EMPTY. */
inline colorT piece_Color_NotEmpty(pieceT p) { return (p & 8) >> 3; }

/** Returns the uncoloured piece type encoded in @p p. */
inline pieceT piece_Type(pieceT p) { return (p & 7); }

/** Builds a coloured piece code from a real colour and an uncoloured type. */
inline pieceT piece_Make(colorT c, pieceT p) { return ((c << 3) | (p & 7)); }

// PIECE_CHAR[]: array of piece characters, capitals for White pieces.
/** Piece-code to character table. */
const char PIECE_CHAR[] = "xKQRBNP.xkqrbnpxMm";

/** Returns the piece-type character for @p p. */
inline char piece_Char(pieceT p) { return PIECE_CHAR[piece_Type(p)]; }

/** Compile-time byte-to-piece lookup for FEN and legacy board strings. */
class PieceFromByte {
    pieceT pieceFromByte_[256] = {};

public:
    /** Builds a lookup table where unrecognised bytes map to EMPTY. */
    constexpr PieceFromByte() {
        for (auto& e : pieceFromByte_) {
            e = EMPTY;
        }
        pieceFromByte_[(int)'K'] = WK;
        pieceFromByte_[(int)'k'] = BK;
        pieceFromByte_[(int)'Q'] = WQ;
        pieceFromByte_[(int)'q'] = BQ;
        pieceFromByte_[(int)'R'] = WR;
        pieceFromByte_[(int)'r'] = BR;
        pieceFromByte_[(int)'B'] = WB;
        pieceFromByte_[(int)'b'] = BB;
        pieceFromByte_[(int)'N'] = WN;
        pieceFromByte_[(int)'n'] = BN;
        pieceFromByte_[(int)'P'] = WP;
        pieceFromByte_[(int)'p'] = BP;
    };

    /** Returns the piece represented by @p idx, or EMPTY if none is known. */
    pieceT operator[](unsigned char idx) const { return pieceFromByte_[idx]; }
};
/** Global byte-to-piece lookup table. */
constexpr inline auto pieceFromByte = PieceFromByte();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SQUARES
/** Board squares in rank-major order, followed by sentinel squares. */
const squareT A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
              A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14,
              H2 = 15, A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21,
              G3 = 22, H3 = 23, A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28,
              F4 = 29, G4 = 30, H4 = 31, A5 = 32, B5 = 33, C5 = 34, D5 = 35,
              E5 = 36, F5 = 37, G5 = 38, H5 = 39, A6 = 40, B6 = 41, C6 = 42,
              D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47, A7 = 48, B7 = 49,
              C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55, A8 = 56,
              B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
              COLOR_SQUARE = 64, NULL_SQUARE = 65,
              NS = 65; // NS is abbreviation for NULL_SQUARE.

/** Rank constants and invalid rank sentinel. */
const rankT RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3, RANK_5 = 4,
            RANK_6 = 5, RANK_7 = 6, RANK_8 = 7, NO_RANK = 64;

/** File constants and invalid file sentinel. */
const fyleT
    // we use "fyle" instead of "file" to avoid confusion with disk files.
    A_FYLE = 0,
    B_FYLE = 1, C_FYLE = 2, D_FYLE = 3, E_FYLE = 4, F_FYLE = 5, G_FYLE = 6,
    H_FYLE = 7, NO_FYLE = 64;

/** Converts @c '1'..@c '8' to a rank, or NO_RANK for invalid input. */
inline rankT rank_FromChar(char c) {
	if (c < '1' || c > '8') {
		return NO_RANK;
	} else
		return (c - '1');
}

/** Converts @c 'a'..@c 'h' to a file, or NO_FYLE for invalid input. */
inline fyleT fyle_FromChar(char c) {
	if (c < 'a' || c > 'h') {
		return NO_FYLE;
	} else
		return (c - 'a');
}

/** Builds a square from file and rank codes. */
constexpr squareT square_Make(fyleT f, rankT r) { return ((r << 3) | f); }

/** Returns the file of @p sq. */
constexpr fyleT square_Fyle(squareT sq) { return (sq & 0x7); }

/** Returns the rank of @p sq. */
constexpr rankT square_Rank(squareT sq) { return ((sq >> 3) & 0x7); }

/** Returns @p sq from White's perspective for WHITE, Black's for BLACK. */
constexpr squareT square_Relative(colorT c, squareT sq) {
	return static_cast<squareT>(sq ^ (c * 56));
}

/** Returns @p r from White's perspective for WHITE, Black's for BLACK. */
constexpr rankT rank_Relative(colorT c, rankT r) {
	return static_cast<rankT>(r ^ (c * 7));
}

} // namespace scid::core
