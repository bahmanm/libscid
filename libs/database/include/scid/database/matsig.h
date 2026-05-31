//////////////////////////////////////////////////////////////////////
//
//  FILE:       matsig.h
//              Material signatures and home-pawn signatures.
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    1.9
//
//  Notice:     Copyright (c) 2000  Shane Hudson.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////


#ifndef SCID_MATSIG_H
#define SCID_MATSIG_H

#include "scid/core/board.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <utility>

namespace scid::database {

/**
 * Compact material signature used for fast database search prefilters.
 *
 * A material signature stores only piece counts, not squares.  It uses the
 * lower 24 bits of a 32-bit integer: pawns receive four bits per side, while
 * each non-pawn piece type receives two bits per side and is therefore capped
 * at three.  Kings are not represented.
 */
typedef std::uint32_t matSigT;

/**
 * @name Material Signature Layout
 *
 * From high to low bits, the layout is:
 * - bits 22-23: White queens
 * - bits 20-21: White rooks
 * - bits 18-19: White bishops
 * - bits 16-17: White knights
 * - bits 12-15: White pawns
 * - bits 10-11: Black queens
 * - bits 08-09: Black rooks
 * - bits 06-07: Black bishops
 * - bits 04-05: Black knights
 * - bits 00-03: Black pawns
 *
 * Pawns can represent ordinary counts from zero to eight.  Other piece counts
 * are saturated at three when signatures are made from a position.
 * @{
 */

#define SHIFT_BP 0
#define SHIFT_BN 4
#define SHIFT_BB 6
#define SHIFT_BR 8
#define SHIFT_BQ 10
#define SHIFT_WP 12
#define SHIFT_WN 16
#define SHIFT_WB 18
#define SHIFT_WR 20
#define SHIFT_WQ 22


/** Mask for black pawns. */
#define MASK_BP 0x0000000F
/** Mask for black knights. */
#define MASK_BN 0x00000030
/** Mask for black bishops. */
#define MASK_BB 0x000000C0
/** Mask for black rooks. */
#define MASK_BR 0x00000300
/** Mask for black queens. */
#define MASK_BQ 0x00000C00
/** Mask for white pawns. */
#define MASK_WP 0x0000F000
/** Mask for white knights. */
#define MASK_WN 0x00030000
/** Mask for white bishops. */
#define MASK_WB 0x000C0000
/** Mask for white rooks. */
#define MASK_WR 0x00300000
/** Mask for white queens. */
#define MASK_WQ 0x00C00000
/** @} */

/** Piece-code indexed masks for extracting or replacing a count. */
const matSigT
MASK_BY_PIECE [16] = {
    0,        //  0: Empty
    0,        //  1: scid::core::WK
    MASK_WQ,  //  2: scid::core::WQ
    MASK_WR,  //  3: scid::core::WR
    MASK_WB,  //  4: scid::core::WB
    MASK_WN,  //  5: scid::core::WN
    MASK_WP,  //  6: scid::core::WP
    0, 0,     //  7, 8: Invalid pieces
    0,        //  9: scid::core::BK
    MASK_BQ,  // 10: scid::core::BQ
    MASK_BR,  // 11: scid::core::BR
    MASK_BB,  // 12: scid::core::BB
    MASK_BN,  // 13: scid::core::BN
    MASK_BP,  // 14: scid::core::BP
    0         // 15: Invalid piece
};

/** Piece-code indexed bit shifts for extracting or replacing a count. */
const scid::core::uint
SHIFT_BY_PIECE[16] = {
    0, 0,      //  0: Empty,  1: scid::core::WK
    SHIFT_WQ,  //  2: scid::core::WQ
    SHIFT_WR,  //  3: scid::core::WR
    SHIFT_WB,  //  4: scid::core::WB
    SHIFT_WN,  //  5: scid::core::WN
    SHIFT_WP,  //  6: scid::core::WP
    0, 0, 0,   //  7, 8: Invalid pieces,  9: scid::core::BK
    SHIFT_BQ,  // 10: scid::core::BQ
    SHIFT_BR,  // 11: scid::core::BR
    SHIFT_BB,  // 12: scid::core::BB
    SHIFT_BN,  // 13: scid::core::BN
    SHIFT_BP,  // 14: scid::core::BP
    0          // 15: Invalid piece
};


/** Returns @p x with White and Black material fields exchanged. */
#define MATSIG_FlipColor(x) ((x) >> 12) | (((x) & 0x00000FFF) << 12)


/** Returns non-zero when @p x contains at least one White queen. */
#define MATSIG_Has_WQ(x)  ((x) & MASK_WQ)
/** Returns non-zero when @p x contains at least one Black queen. */
#define MATSIG_Has_BQ(x)  ((x) & MASK_BQ)
/** Returns non-zero when @p x contains at least one White rook. */
#define MATSIG_Has_WR(x)  ((x) & MASK_WR)
/** Returns non-zero when @p x contains at least one Black rook. */
#define MATSIG_Has_BR(x)  ((x) & MASK_BR)
/** Returns non-zero when @p x contains at least one White bishop. */
#define MATSIG_Has_WB(x)  ((x) & MASK_WB)
/** Returns non-zero when @p x contains at least one Black bishop. */
#define MATSIG_Has_BB(x)  ((x) & MASK_BB)
/** Returns non-zero when @p x contains at least one White knight. */
#define MATSIG_Has_WN(x)  ((x) & MASK_WN)
/** Returns non-zero when @p x contains at least one Black knight. */
#define MATSIG_Has_BN(x)  ((x) & MASK_BN)
/** Returns non-zero when @p x contains at least one White pawn. */
#define MATSIG_Has_WP(x)  ((x) & MASK_WP)
/** Returns non-zero when @p x contains at least one Black pawn. */
#define MATSIG_Has_BP(x)  ((x) & MASK_BP)

/** Returns non-zero when either side has a queen. */
#define MATSIG_HasQueens(x)   ((x) & (MASK_WQ | MASK_BQ))
/** Returns non-zero when either side has a rook. */
#define MATSIG_HasRooks(x)    ((x) & (MASK_WR | MASK_BR))
/** Returns non-zero when either side has a bishop. */
#define MATSIG_HasBishops(x)  ((x) & (MASK_WB | MASK_BB))
/** Returns non-zero when either side has a knight. */
#define MATSIG_HasKnights(x)  ((x) & (MASK_WN | MASK_BN))
/** Returns non-zero when either side has a pawn. */
#define MATSIG_HasPawns(x)    ((x) & (MASK_WP | MASK_BP))


/** Returns the White queen count from @p x. */
#define MATSIG_Count_WQ(x)  (((x) & MASK_WQ) >> SHIFT_WQ)
/** Returns the Black queen count from @p x. */
#define MATSIG_Count_BQ(x)  (((x) & MASK_BQ) >> SHIFT_BQ)
/** Returns the White rook count from @p x. */
#define MATSIG_Count_WR(x)  (((x) & MASK_WR) >> SHIFT_WR)
/** Returns the Black rook count from @p x. */
#define MATSIG_Count_BR(x)  (((x) & MASK_BR) >> SHIFT_BR)
/** Returns the White bishop count from @p x. */
#define MATSIG_Count_WB(x)  (((x) & MASK_WB) >> SHIFT_WB)
/** Returns the Black bishop count from @p x. */
#define MATSIG_Count_BB(x)  (((x) & MASK_BB) >> SHIFT_BB)
/** Returns the White knight count from @p x. */
#define MATSIG_Count_WN(x)  (((x) & MASK_WN) >> SHIFT_WN)
/** Returns the Black knight count from @p x. */
#define MATSIG_Count_BN(x)  (((x) & MASK_BN) >> SHIFT_BN)
/** Returns the White pawn count from @p x. */
#define MATSIG_Count_WP(x)  (((x) & MASK_WP) >> SHIFT_WP)
/** Returns the Black pawn count from @p x. */
#define MATSIG_Count_BP(x)  (((x) & MASK_BP) >> SHIFT_BP)

/**
 * Returns the count of piece @p p stored in material signature @p m.
 *
 * Kings and invalid piece codes have no material-signature field and return
 * zero.
 */
inline scid::core::uint
matsig_getCount (matSigT m, scid::core::pieceT p)
{
    return (m & MASK_BY_PIECE[p]) >> SHIFT_BY_PIECE[p];
}

/**
 * Returns @p m with the count for piece @p p replaced by @p count.
 *
 * Non-pawn counts are saturated at three to fit the two-bit fields.
 */
inline matSigT
matsig_setCount (matSigT m, scid::core::pieceT p, scid::core::uint count)
{
    // First we clear the old mask for this piece:
    m &= ~(MASK_BY_PIECE[p]);

	 // Avoid overflow.
	 if (p != scid::core::PAWN && count > 3)
		 count = 3;

    // Now we OR to add the new value in:
    m |= ((scid::core::uint) count) << SHIFT_BY_PIECE[p];
    return m;
}


/** Material signature for an empty board. */
const matSigT MATSIG_Empty = 0;

/** Material signature for the standard chess starting position. */
const matSigT MATSIG_StdStart =
   ((1 << SHIFT_WQ) | (1 << SHIFT_BQ) | (2 << SHIFT_WR) | (2 << SHIFT_BR) |
    (2 << SHIFT_WB) | (2 << SHIFT_BB) | (2 << SHIFT_WN) | (2 << SHIFT_BN) |
    (8 << SHIFT_WP) | (8 << SHIFT_BP));

/**
 * Returns a compact textual representation of @p matsig.
 *
 * The format is @c "white:black", using piece letters for non-pawns and a
 * digit for each side's pawn count.
 */
std::string
matsig_makeString (matSigT matsig);

/**
 * Returns true when @p mStart could legally decay into @p mTarget.
 *
 * This is a fast prefilter for material and position searches.  A game whose
 * final material cannot contain the searched material can be skipped without
 * decoding its moves.
 *
 * @param mStart material required by the searched position.
 * @param mTarget final material stored for a candidate game.
 * @param promos true when the candidate game contains promotions.
 * @param upromo true when the candidate game contains under-promotions.
 */
bool
matsig_isReachable (matSigT mStart, matSigT mTarget, bool promos, bool upromo);

/**
 * Returns true when @p mStart could decay into @p mTarget considering pawns
 * only.
 */
inline bool
matsig_isReachablePawns (matSigT mStart, matSigT mTarget)
{
    if (MATSIG_Count_WP(mStart) < MATSIG_Count_WP(mTarget)) { return false; }
    if (MATSIG_Count_BP(mStart) < MATSIG_Count_BP(mTarget)) { return false; }
    return true;
}

/**
 * Builds a material signature from a @ref scid::core::Position material array.
 *
 * The array must be indexed by Scid piece code, as returned by
 * @ref scid::core::Position::GetMaterial().  Non-pawn piece counts are
 * saturated at three.
 */
inline matSigT matsig_Make(const scid::core::byte* materialCounts) {
    matSigT m = 0;
    m |= std::min<matSigT>(3, materialCounts[scid::core::WQ]) << SHIFT_WQ;
    m |= std::min<matSigT>(3, materialCounts[scid::core::WR]) << SHIFT_WR;
    m |= std::min<matSigT>(3, materialCounts[scid::core::WB]) << SHIFT_WB;
    m |= std::min<matSigT>(3, materialCounts[scid::core::WN]) << SHIFT_WN;
    m |= matSigT(materialCounts[scid::core::WP]) << SHIFT_WP;
    m |= std::min<matSigT>(3, materialCounts[scid::core::BQ]) << SHIFT_BQ;
    m |= std::min<matSigT>(3, materialCounts[scid::core::BR]) << SHIFT_BR;
    m |= std::min<matSigT>(3, materialCounts[scid::core::BB]) << SHIFT_BB;
    m |= std::min<matSigT>(3, materialCounts[scid::core::BN]) << SHIFT_BN;
    m |= matSigT(materialCounts[scid::core::BP]) << SHIFT_BP;
    return m;
}


/**
 * Home-pawn signature with no pawns still on their original home squares.
 */
const scid::core::uint
HPSIG_Empty = 0x0;

/**
 * Home-pawn signature for the starting position.
 *
 * All sixteen pawns are still on their original second- or seventh-rank
 * squares.
 */
const scid::core::uint
HPSIG_StdStart = 0xFFFF;

/**
 * Returns true when a game's home-pawn change list could reach @p hpSig.
 *
 * @p changeList is the packed home-pawn data stored in @c IndexEntry; byte
 * zero is the number of half-byte entries, and subsequent half-bytes identify
 * pawns that left their home squares.  The function is used as a cheap exact
 * position-search prefilter.
 */
bool
hpSig_PossibleMatch (scid::core::uint hpSig, const scid::core::byte * changeList);

/**
 * Returns true when either home-pawn change list is a prefix of the other.
 *
 * This is used to cheaply detect games that may be truncated versions of one
 * another.
 */
bool
hpSig_Prefix (const scid::core::byte * changeListA, const scid::core::byte * changeListB);

/**
 * Returns the final home-pawn signature reached after applying @p changeList.
 */
scid::core::uint
hpSig_Final (const scid::core::byte * changeList);

/** Bit masks for home-pawn files, ordered a2-h2 then a7-h7. */
static const scid::core::uint hpSig_bitMask [16] = {
    // a2 to h2:
    0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
    // a7 to h7:
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

/**
 * Returns @p hpSig with the home-pawn bit for @p color and @p fyle set.
 */
inline scid::core::uint
hpSig_AddPawn (scid::core::uint hpSig, scid::core::colorT color, scid::core::fyleT fyle)
{
    assert(color == scid::core::WHITE || color == scid::core::BLACK);
    assert(fyle <= scid::core::H_FYLE);

    scid::core::uint val = (scid::core::uint) fyle;
    if (color == scid::core::BLACK) val += 8;
    return hpSig | hpSig_bitMask [val];
}

/**
 * Returns @p hpSig with the home-pawn bit for @p color and @p fyle cleared.
 */
inline scid::core::uint
hpSig_ClearPawn (scid::core::uint hpSig, scid::core::colorT color, scid::core::fyleT fyle)
{
    assert(color == scid::core::WHITE || color == scid::core::BLACK);
    assert(fyle <= scid::core::H_FYLE);

    scid::core::uint val = (scid::core::uint) fyle;
    if (color == scid::core::BLACK) val += 8;
    return hpSig & ~(hpSig_bitMask [val]);
}

/**
 * Creates a 16-bit bitmap of the pawns missing from their home ranks.
 *
 * Used to speed up the searches of positions with the same pawn structure.
 * @returns a std::pair containing the bitmap and the number of moved pawns.
 */
inline std::pair<std::uint16_t, std::uint16_t> hpSig_make(const scid::core::pieceT* board) {
	int hpSig = 0;
	int nMoved = 0;
	const scid::core::pieceT* b = board + scid::core::A2;
	// clang-format off
	if (*b != scid::core::WP) { hpSig |= 0x8000; ++nMoved; }  b++;  /* a2 */
	if (*b != scid::core::WP) { hpSig |= 0x4000; ++nMoved; }  b++;  /* b2 */
	if (*b != scid::core::WP) { hpSig |= 0x2000; ++nMoved; }  b++;  /* c2 */
	if (*b != scid::core::WP) { hpSig |= 0x1000; ++nMoved; }  b++;  /* d2 */
	if (*b != scid::core::WP) { hpSig |= 0x0800; ++nMoved; }  b++;  /* e2 */
	if (*b != scid::core::WP) { hpSig |= 0x0400; ++nMoved; }  b++;  /* f2 */
	if (*b != scid::core::WP) { hpSig |= 0x0200; ++nMoved; }  b++;  /* g2 */
	if (*b != scid::core::WP) { hpSig |= 0x0100; ++nMoved; }        /* h2 */
	b = board + scid::core::A7;
	if (*b != scid::core::BP) { hpSig |= 0x0080; ++nMoved; }  b++;  /* a7 */
	if (*b != scid::core::BP) { hpSig |= 0x0040; ++nMoved; }  b++;  /* b7 */
	if (*b != scid::core::BP) { hpSig |= 0x0020; ++nMoved; }  b++;  /* c7 */
	if (*b != scid::core::BP) { hpSig |= 0x0010; ++nMoved; }  b++;  /* d7 */
	if (*b != scid::core::BP) { hpSig |= 0x0008; ++nMoved; }  b++;  /* e7 */
	if (*b != scid::core::BP) { hpSig |= 0x0004; ++nMoved; }  b++;  /* f7 */
	if (*b != scid::core::BP) { hpSig |= 0x0002; ++nMoved; }  b++;  /* g7 */
	if (*b != scid::core::BP) { hpSig |= 0x0001; ++nMoved; }        /* h7 */
	// clang-format on

	return {static_cast<std::uint16_t>(hpSig), static_cast<std::uint16_t>(nMoved)};
}

/**
 * Returns true when @p changeList can explain the requested missing-pawn
 * bitmap and moved-pawn count.
 */
inline bool hpSig_match(int hpSig, int nMoved, const scid::core::byte* changeList) {
	// The first scid::core::byte of a changeList is the length (in halfbytes) of the
	// list, which can be any value from 0 to 16 inclusive.
	if (*changeList == 16 && nMoved == 16)
		return true;
	if (*changeList++ < nMoved)
		return false;

	int sig = 0;
	for (int i = 0, n = nMoved / 2; i < n; ++i) {
		sig |= 1 << (*changeList >> 4);
		sig |= 1 << (*changeList++ & 0x0F);
	}
	if (nMoved & 1)
		sig |= 1 << (*changeList >> 4);

	return sig == hpSig;
}


} // namespace scid::database
#endif

//////////////////////////////////////////////////////////////////////
//  EOF: matsig.h
//////////////////////////////////////////////////////////////////////
