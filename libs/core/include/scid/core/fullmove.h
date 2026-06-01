/** @file
 * Compact encoded moves with enough metadata for direct SAN formatting.
 */
#pragma once

#include "scid/core/board.h"

#include <cassert>
#include <cstdint>
#include <string>

namespace scid::core {

/** Compact value type for encoded moves.
 *
 * FullMove packs the move coordinates, special-move kind, moving piece, colour,
 * capture, promotion, disambiguation, and check marker into a 32-bit value.  It
 * is useful when moves need to be stored or sorted as small values while still
 * retaining enough information to format a simple SAN string without consulting
 * a Position.
 *
 * The lower 16 bits deliberately follow Stockfish's move layout.  The remaining
 * bits are libscid metadata; callers should use the accessors and mutators
 * rather than depending on those bits directly.
 */
class FullMove {
	// ** Lower 16 bits are compatible with Stockfish's Move **
	// bits  0- 5: destination square (from 0 to 63)
	// bits  6-11: origin square (from 0 to 63)
	// bits 12-13: promotion piece type -2 (from QUEEN-2 to KNIGHT-2)
	// bits 14-15: special move flag: promotion (1), en passant (2), castling (3)

	// ** Info for undoing the move **
	// bits 16-17: castling flags - TODO
	// bits 18-20: enpassant file - TODO
	// bits 21-23: captured pieceT

	// ** Info for direct SAN conversion **
	// bits 24-26: moving pieceT
	// bit     27: black to move
	// bit     28: ambiguous move, insert from fyle
	// bit     29: ambiguous move, insert from rank
	// bit     30: check

	// ** TODO: Use this flag to embed tags, variations, etc.. in a move stream
	// bit     31: special flag
	std::uint32_t m_;

public:
	/** Constructs an empty move or wraps an already encoded value. */
	constexpr FullMove(std::uint32_t m = 0) : m_(m){};

	/** Constructs a castling move.
	 *
	 * The destination stores the rook square rather than the king's final square
	 * so that Chess960 castling can be undone without losing the rook identity.
	 */
	FullMove(colorT c, squareT kingSq, squareT rookSq)
	    // Castle: encoding as king to rook allows the undoing of Chess960 moves
	    : FullMove(c, kingSq, rookSq, KING) {
		m_ |= (3 << 14);
	}

	/** Constructs a normal move by colour, origin, destination, and piece type. */
	FullMove(colorT c, squareT from, squareT to, pieceT pt) {
		m_ = to | (from << 6) | (pt << 24) | (c << 27);
	}

	/** Compares the complete encoded value. */
	bool operator==(FullMove const& f) const { return m_ == f.m_; }

	// Special moves:
	// NONE: encoded as 0 (from 0 to 0);
	// NULL: encoded as 65 (from 1 to 1);
	// PROMO: encoded setting the special move flag to 1
	// ENPASSANT: encoded setting the special move flag to 2
	// CASTLING: encoded setting the special move flag to 3, from is the square
	//           of the king and to is the square of the rook.
	//           If from < to it is castling king side.
	/** Returns false only for the empty encoded value. */
	explicit operator bool() const { return m_ != 0; }
	/** Returns true for the encoded null move. */
	bool    isNull()      const { return m_ == 0b01000001; }
	/** Returns true when the move carries a promotion piece. */
	bool    isPromo()     const { return (m_ & (3 << 14)) == (1 << 14); }
	/** Returns true when the move is an en-passant capture. */
	bool    isEnpassant() const { return (m_ & (3 << 14)) == (2 << 14); }
	/** Returns true when the move is castling. */
	bool    isCastle()    const { return (m_ & (3 << 14)) == (3 << 14); }

	/** Destination square, or rook square for castling. */
	squareT getTo()       const { return m_ & 0x3F; }
	/** Origin square. */
	squareT getFrom()     const { return (m_ >> 6) & 0x3F; }
	/** Moving piece type. */
	pieceT  getPiece()    const { return (m_ >> 24) & 0x07; }
	/** Moving side. */
	colorT  getColor()    const { return (m_ >> 27 & 1) ? BLACK : WHITE; }
	/** Promotion piece type.  Meaningful only when isPromo() is true. */
	pieceT  getPromo()    const { return ((m_ >> 12) & 0x03) +2; }
	/** Captured piece type, or EMPTY when the move is not a capture. */
	pieceT  getCaptured() const { return (m_ >> 21) & 0x07; }

	/** Formats the embedded move metadata as SAN.
	 *
	 * This uses only the information already stored in the value.  It cannot add
	 * mate markers or derive ambiguity by itself; callers must set capture,
	 * ambiguity, promotion, and check metadata before calling it.
	 */
	std::string getSAN() const {
		std::string res;
		const auto to = getTo();
		const auto from = getFrom();
		if (to == 0 && from == 0) return "--";
		if (isCastle()) {
			res = (to > from) ? "O-O" : "O-O-O";
			bool check = (m_ >> 30) & 1;
			if (check)
				res += "+";
			return res;
		}
		bool fromFyle = (m_ >> 28) & 1;
		bool fromRank = (m_ >> 29) & 1;
		bool check    = (m_ >> 30) & 1;
		bool capture  = (getCaptured() != 0);

		switch (getPiece()) {
		case BISHOP: res += "B"; break;
		case KNIGHT: res += "N"; break;
		case ROOK:   res += "R"; break;
		case QUEEN:  res += "Q"; break;
		case KING:   res += "K"; break;
		default: //PAWN
			if (capture) res += 'a' + (from % 8);
		}
		if (fromFyle) res += 'a' + (from % 8);
		if (fromRank) res += '1' + (from / 8);
		if (capture)  res += "x";
		res += 'a' + (to % 8);
		res += '1' + (to / 8);
		if (isPromo()) {
			switch (getPromo()) {
			case BISHOP: res += "=B"; break;
			case KNIGHT: res += "=N"; break;
			case ROOK:   res += "=R"; break;
			case QUEEN:  res += "=Q"; break;
			}
		}
		if (check) res += "+";
		return res;
	}

	/** Marks the move as a promotion to @p promo. */
	void setPromo(pieceT promo) {
		assert(promo == QUEEN || promo == ROOK || promo == BISHOP ||
		       promo == KNIGHT);
		m_ |= ((promo - 2) << 12) | (1 << 14);
	}

	/** Records the captured piece and optionally marks en passant. */
	void setCapture(pieceT piece, bool enPassant) {
		m_ |= ((piece & 0x07) << 21);
		if (enPassant) m_ |= (2 << 14);
	}

	/** Sets SAN disambiguation flags for origin file and rank. */
	void setAmbiguity(bool fyle, bool rank) {
		m_ &= ~(3 << 28);
		if (fyle)
			m_ |= (1 << 28);
		if (rank)
			m_ |= (1 << 29);
	}

	/** Appends a check marker when getSAN() is called. */
	void setCheck() { m_ |= (1 << 30); }
};


} // namespace scid::core
