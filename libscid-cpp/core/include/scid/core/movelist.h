/** @file
 * Generated move records and fixed-capacity move lists.
 */
#pragma once

#include "scid/core/board.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

//////////////////////////////////////////////////////////////////////
//  MoveList:  Constants

namespace scid::core {

/** Maximum number of legal moves held by a MoveList. */
const uint  MAX_LEGAL_MOVES = 256;


///////////////////////////////////////////////////////////////////////////
//  MoveList:  Data Structures

/** Position-resolved, reversible move action.
 *
 * MoveAction is the form produced by Position move generation and by
 * Position::resolveMove().  Unlike MoveSpec, it records enough pre-move state
 * to apply and undo the move exactly, including castling rights, en-passant
 * target, halfmove clock, piece-list indices, and special capture squares.
 *
 * It semantically belongs with Position, but currently lives here because
 * MoveList stores generated actions and Position includes MoveList.
 */
struct MoveAction
{
    /** Origin square, or the same square as the destination for a null move. */
    squareT  from;
    /** Destination square, or rook square for resolved castling. */
    squareT  to;
    /** Promotion piece type, or EMPTY for a non-promotion. */
    pieceT   promote;
    /** Moving piece, including colour. */
    pieceT   movingPiece : 7;
    /** True when the action is castling. */
    pieceT   castling : 1;
    /** Pre-move index of the moving piece in Position's piece list. */
    byte     pieceNum;
    /** Pre-move index of the captured piece, when any. */
    byte     capturedNum;
    /** Captured piece, including colour, or EMPTY. */
    pieceT   capturedPiece;
    /** Captured square; differs from the destination only for en-passant captures. */
    squareT  capturedSquare;
    /** Castling rights before the action was applied. */
    byte     castleFlags;
    /** En-passant target before the action was applied. */
    squareT  epSquare;
    /** Halfmove clock before the action was applied. */
    ushort   oldHalfMoveClock;

	/** Returns true when the action is a null move. */
	bool isNullMove() const {
		return from == to && from != NULL_SQUARE &&
		       piece_Type(movingPiece) == KING;
	}

	/** Classifies castling direction.
	 *
	 * @returns +2 for kingside castling, -2 for queenside castling, or 0 for a
	 * non-castling action.
	 */
	int isCastle() const {
		if (castling)
			return to > from ? 2 : -2;

		return 0;
	}

	/** Writes the action as UCI coordinate notation.
	 *
	 * Null moves are written as @c 0000.  Promotions use lower-case promotion
	 * letters.  The returned iterator points one past the last character written.
	 */
	template <typename OutputIt> OutputIt toLongNotation(OutputIt dest) const {
		if (from == to) {
			// UCI standard for null move
			*dest++ = '0';
			*dest++ = '0';
			*dest++ = '0';
			*dest++ = '0';
		} else {
			*dest++ = square_FyleChar(from);
			*dest++ = square_RankChar(from);
			*dest++ = square_FyleChar(to);
			*dest++ = square_RankChar(to);
			if (promote != EMPTY) {
				constexpr const char promoChars[] = "  qrbn ";
				*dest++ = promoChars[piece_Type(promote)];
			}
		}
		return dest;
	}
};

/** MoveAction plus an ordering score used by search and move pickers. */
struct ScoredMove : public MoveAction {
	/** Higher scores are sorted before lower scores. */
	std::int32_t score;

	/** Orders moves by descending score. */
	bool operator<(const ScoredMove& b) const {
		// Highest score first
		return score > b.score;
	}
};

/** Fixed-capacity container for generated moves.
 *
 * MoveList deliberately avoids dynamic allocation in move generation and engine
 * search.  Its contents are invalidated by Clear(), resize(), emplace_back(),
 * and push_back() in the usual array-container sense.
 */
class MoveList {
	uint ListSize = 0;
	ScoredMove Moves[MAX_LEGAL_MOVES];

public:
	typedef ScoredMove* iterator;
	/** Returns an iterator to the first generated move. */
	iterator begin() { return Moves; };
	/** Returns an iterator one past the last generated move. */
	iterator end() { return Moves + ListSize; }
	/** Returns the number of moves currently stored. */
	uint Size() { return ListSize; }
	/** Removes all stored moves without clearing the underlying storage. */
	void Clear() { ListSize = 0; }
	/** Appends a value-initialised move and returns it for filling. */
	ScoredMove& emplace_back() {
		assert(ListSize < MAX_LEGAL_MOVES);
		ScoredMove& sm = Moves[ListSize++];
		sm = ScoredMove();
		return sm;
	}
	/** Changes the logical number of stored moves. */
	void resize(std::size_t count) {
		assert(count <= MAX_LEGAL_MOVES);
		ListSize = static_cast<uint>(count);
	}
	/** Appends an already constructed move. */
	void push_back(const ScoredMove& sm) {
		assert(ListSize < MAX_LEGAL_MOVES);
		Moves[ListSize++] = sm;
	}
	/** Returns a pointer to the move at @p index. */
	ScoredMove* Get(std::size_t index) {
		assert(index < ListSize);
		return &(Moves[index]);
	}
};


} // namespace scid::core
