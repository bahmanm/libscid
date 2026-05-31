/** @file
 * Position-independent move requests.
 */
#pragma once

#include "scid/core/board.h"

#include <string>

namespace scid::core {

/** Describes a requested move before it is resolved against a position.
 *
 * A MoveSpec is the small, portable move representation stored in games and
 * exchanged with notation helpers.  It names the origin and destination
 * squares, an optional promotion piece type, and whether the move must be
 * interpreted as castling.  It does not record captured pieces, castling
 * rights, en-passant state, or other undo data; use Position::resolveMove()
 * when a reversible, position-resolved MoveAction is needed.
 *
 * Castling is intentionally explicit.  In Chess960 a king move can share the
 * same coordinate shape as an ordinary king move, so callers should set
 * castling when the move is known to be castling rather than relying on square
 * geometry alone.
 */
struct MoveSpec {
	/** Origin square, or the same square as the destination for a null move. */
	scid::core::squareT from = scid::core::NULL_SQUARE;
	/** Destination square, or the same square as the origin for a null move. */
	scid::core::squareT to = scid::core::NULL_SQUARE;
	/** Promotion piece type, or EMPTY for a non-promotion. */
	scid::core::pieceT promotion = scid::core::EMPTY;
	/** True when the square pair should be resolved as castling. */
	bool castling = false;

	/** Returns true when this spec represents a null move. */
	bool isNull() const;

	/** Formats the spec as UCI coordinate notation.
	 *
	 * Null moves are written as @c 0000.  Promotions are appended as a lower-case
	 * piece letter, for example @c e7e8q.
	 */
	std::string longNotation() const;
};

} // namespace scid::core
