/** @file
 * Chess board constants, piece helpers, square geometry, and directions.
 */

#pragma once

#include "scid/core/primitives.h"

#include <cassert>

namespace scid::core
{

    /** White minor-piece aggregate marker used by material-only searches. */
    const pieceT WM = 16;
    /** Black minor-piece aggregate marker used by material-only searches. */
    const pieceT BM = 17;

    /** Number of piece-like codes covered by board lookup tables. */
    const uint MAX_PIECE_TYPES = 18;

    // PIECE_FLIP[]: array of pieces, with colors reversed.
    /** Maps each piece code to the same piece with colour reversed. */
    const pieceT PIECE_FLIP[MAX_PIECE_TYPES] = {
        END_OF_BOARD, BK, BQ, BR, BB, BN, BP, EMPTY, EMPTY, WK, WQ, WR, WB, WN, WP, EMPTY, BM, WM};

    /** True for uncoloured piece types that slide along rays. */
    const bool PIECE_IS_SLIDER[8] = {
        false, false, true, true, true, false, false, false,
    };

    /** Returns true when @p p has king as its uncoloured type. */
    inline bool
    piece_IsKing(
        pieceT p)
    {
        return (piece_Type(p) == KING);
    }

    /** Returns true when @p p is a queen, rook, or bishop type. */
    inline bool
    piece_IsSlider(
        pieceT p)
    {
        return PIECE_IS_SLIDER[piece_Type(p)];
    }

    /** Converts a SAN piece designator to an uncoloured piece type.
     *
     * Pawns have no SAN designator here, so @c 'P' and unknown characters return
     * EMPTY.
     */
    inline pieceT
    piece_FromChar(
        int x)
    {
        switch (x)
        {
            case 'K':
                return KING;
            case 'Q':
                return QUEEN;
            case 'R':
                return ROOK;
            case 'N':
                return KNIGHT;
            case 'B':
                return BISHOP;
            default:
                return EMPTY;
        }
    }

    /** Returns the A1-H8 diagonal index for @p sq. */
    inline leftDiagT
    square_LeftDiag(
        squareT sq)
    {
        return square_Rank(sq) + square_Fyle(sq);
    }

    /** Returns the H1-A8 diagonal index for @p sq. */
    inline rightDiagT
    square_RightDiag(
        squareT sq)
    {
        return (7 + square_Rank(sq) - square_Fyle(sq));
    }

    /** Returns WHITE for a light square and BLACK for a dark square. */
    inline colorT
    square_Color(
        squareT sq)
    {
        return 1 - (square_LeftDiag(sq) & 1);
    }

    /** Returns @p sq with its file mirrored, for example A1 becomes H1. */
    inline squareT
    square_FlipFyle(
        squareT sq)
    {
        return square_Make(A_FYLE + H_FYLE - square_Fyle(sq), square_Rank(sq));
    }

    /** Returns @p sq with its rank mirrored, for example A1 becomes A8. */
    inline squareT
    square_FlipRank(
        squareT sq)
    {
        return square_Make(square_Fyle(sq), RANK_1 + RANK_8 - square_Rank(sq));
    }

    /** Returns @p sq reflected across the A1-H8 diagonal. */
    inline squareT
    square_FlipDiag(
        squareT sq)
    {
        return square_Make(square_Rank(sq), square_Fyle(sq));
    }

    /** Chebyshev distance lookup by rank or file. */
    const uint rankFyleDist[64] = {0, 1, 2, 3, 4, 5, 6, 7, 1, 0, 1, 2, 3, 4, 5, 6, 2, 1, 0, 1, 2, 3,
                                   4, 5, 3, 2, 1, 0, 1, 2, 3, 4, 4, 3, 2, 1, 0, 1, 2, 3, 5, 4, 3, 2,
                                   1, 0, 1, 2, 6, 5, 4, 3, 2, 1, 0, 1, 7, 6, 5, 4, 3, 2, 1, 0};

    /** Returns the distance in king moves between two on-board squares. */
    inline uint
    square_Distance(
        squareT from,
        squareT to)
    {
        assert(from <= H8 && to <= H8);
        uint rankd = rankFyleDist[(square_Rank(from) << 3) | square_Rank(to)];
        uint fyled = rankFyleDist[(square_Fyle(from) << 3) | square_Fyle(to)];
        return (rankd > fyled) ? rankd : fyled;
    }

    /** Returns the nearest corner square, choosing the lower-file/lower-rank corner on ties. */
    inline squareT
    square_NearestCorner(
        squareT sq)
    {
        if (square_Rank(sq) <= RANK_4)
        {
            return (square_Fyle(sq) <= D_FYLE) ? A1 : H1;
        }
        else
        {
            return (square_Fyle(sq) <= D_FYLE) ? A8 : H8;
        }
    }

    /** Returns true when @p sq is A1, H1, A8, or H8. */
    inline bool
    square_IsCornerSquare(
        squareT sq)
    {
        return (sq == A1 || sq == H1 || sq == A8 || sq == H8);
    }

    /** Returns true when @p sq is on the board edge. */
    inline bool
    square_IsEdgeSquare(
        squareT sq)
    {
        rankT rank = square_Rank(sq);
        if (rank == RANK_1 || rank == RANK_8)
        {
            return true;
        }
        fyleT fyle = square_Fyle(sq);
        if (fyle == A_FYLE || fyle == H_FYLE)
        {
            return true;
        }
        return false;
    }

    /** Distance of each square from the closest edge, with -1 for sentinels. */
    const int edgeDist[66] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 2, 2, 2,  2,
                              1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2,  2,
                              2, 2, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1};

    /** Returns the distance from @p sq to the nearest board edge. */
    inline int
    square_EdgeDistance(
        squareT sq)
    {
        return edgeDist[sq];
    }

    /** Returns true when @p from and @p to form a knight move. */
    inline bool
    square_IsKnightHop(
        squareT from,
        squareT to)
    {
        assert(from <= H8 && to <= H8);
        uint rdist = rankFyleDist[(square_Rank(from) << 3) | square_Rank(to)];
        uint fdist = rankFyleDist[(square_Fyle(from) << 3) | square_Fyle(to)];
        // It is a knight hop only if one distance is two squares and the
        // other is one square -- that is, only if their product equals two.
        return ((rdist * fdist) == 2);
    }

    /** Returns the file character for @p sq. */
    inline char
    square_FyleChar(
        squareT sq)
    {
        return square_Fyle(sq) + 'a';
    }

    /** Returns the rank character for @p sq. */
    inline char
    square_RankChar(
        squareT sq)
    {
        return square_Rank(sq) + '1';
    }

    /** No direction or no aligned ray. */
    const directionT NULL_DIR = 0;
    /** One rank toward Black's home rank. */
    const directionT UP = 1;
    /** One rank toward White's home rank. */
    const directionT DOWN = 2;
    /** One file toward A-file. */
    const directionT LEFT = 4;
    /** One file toward H-file. */
    const directionT RIGHT = 8;
    /** Diagonal toward A8. */
    const directionT UP_LEFT = (UP | LEFT);
    /** Diagonal toward H8. */
    const directionT UP_RIGHT = (UP | RIGHT);
    /** Diagonal toward A1. */
    const directionT DOWN_LEFT = (DOWN | LEFT);
    /** Diagonal toward H1. */
    const directionT DOWN_RIGHT = (DOWN | RIGHT);

    /** Direction-opposite lookup table indexed by directionT values. */
    const directionT dirOpposite[11] = {
        NULL_DIR,
        DOWN, // opposite of UP (1)
        UP,   // opposite of DOWN (2)
        NULL_DIR,
        RIGHT,      // opposite of LEFT (4)
        DOWN_RIGHT, // opposite of UP_LEFT (5)
        UP_RIGHT,   // opposite of DOWN_LEFT (6)
        NULL_DIR,
        LEFT,      // opposite of RIGHT (8)
        DOWN_LEFT, // opposite of UP_RIGHT (9)
        UP_LEFT    // opposite of DOWN_RIGHT (10)
    };

    /** Returns the opposite direction for @p d. */
    inline directionT
    direction_Opposite(
        directionT d)
    {
        return dirOpposite[d];
    }

    /** Diagonal-direction lookup table indexed by directionT values. */
    const bool dirIsDiagonal[11] = {
        false, //  0 = NULL_DIR
        false, //  1 = UP
        false, //  2 = DOWN
        false, //  3 = Invalid
        false, //  4 = LEFT
        true,  //  5 = UP_LEFT
        true,  //  6 = DOWN_LEFT
        false, //  7 = Invalid
        false, //  8 = RIGHT
        true,  //  9 = UP_RIGHT
        true   // 10 = DOWN_RIGHT
    };

    /** Returns true when @p dir is one of the four diagonal directions. */
    inline bool
    direction_IsDiagonal(
        directionT dir)
    {
        return dirIsDiagonal[dir];
    }

    /** Board-array delta for a one-square step in each direction. */
    const int dirDelta[11] = {
        0,  // NULL_DIR
        8,  // UP
        -8, // DOWN
        0,  // Invalid
        -1, // LEFT
        7,  // UP_LEFT
        -9, // DOWN_LEFT
        0,  // Invalid
        1,  // RIGHT
        9,  // UP_RIGHT
        -7  // DOWN_RIGHT
    };

    /** Returns the board-array delta for @p dir. */
    inline int
    direction_Delta(
        directionT dir)
    {
        return dirDelta[dir];
    }

    /** Standard starting board, including COLOR_SQUARE and NULL_SQUARE sentinels. */
    const pieceT START_BOARD[66] = {
        WR,          WN,    WB,    WQ,    WK,    WB,    WN,    WR, // A1--H1
        WP,          WP,    WP,    WP,    WP,    WP,    WP,    WP, // A2--H2
        EMPTY,       EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY,       EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY,       EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, BP,    BP,    BP,    BP,
        BP,          BP,    BP,    BP,    BR,    BN,    BB,    BQ,    BK,    BB,    BN,    BR,
        EMPTY,       // COLOR_SQUARE
        END_OF_BOARD // NULL_SQUARE
    };

    /** Standard board square colours, with NOCOLOR for sentinels. */
    const colorT BOARD_SQUARECOLOR[66] = {
        BLACK,   WHITE,  BLACK, WHITE, BLACK, WHITE, BLACK, WHITE, // a1-h1
        WHITE,   BLACK,  WHITE, BLACK, WHITE, BLACK, WHITE, BLACK, // a2-h2
        BLACK,   WHITE,  BLACK, WHITE, BLACK, WHITE, BLACK, WHITE, // a3-h3
        WHITE,   BLACK,  WHITE, BLACK, WHITE, BLACK, WHITE, BLACK, // a4-h4
        BLACK,   WHITE,  BLACK, WHITE, BLACK, WHITE, BLACK, WHITE, // a5-h5
        WHITE,   BLACK,  WHITE, BLACK, WHITE, BLACK, WHITE, BLACK, // a6-h6
        BLACK,   WHITE,  BLACK, WHITE, BLACK, WHITE, BLACK, WHITE, // a7-h7
        WHITE,   BLACK,  WHITE, BLACK, WHITE, BLACK, WHITE, BLACK, // a8-h8
        NOCOLOR, NOCOLOR                                           // Color square and Null square
    };

    /** Returns true when two squares are king-adjacent, including equality. */
    inline bool
    square_Adjacent(
        squareT from,
        squareT to)
    {
        assert(from <= H8 && to <= H8);
        rankT fromRank = square_Rank(from);
        rankT toRank = square_Rank(to);
        int rdist = (int)fromRank - (int)toRank;
        if (rdist < -1 || rdist > 1)
        {
            return false;
        }
        fyleT fromFyle = square_Fyle(from);
        fyleT toFyle = square_Fyle(to);
        int fdist = (int)fromFyle - (int)toFyle;
        if (fdist < -1 || fdist > 1)
        {
            return false;
        }
        return true;
    }

} // namespace scid::core
