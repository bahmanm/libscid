#pragma once

#include "scid/core/date.h"
#include "scid/core/game_result.h"
#include "scid/core/rating.h"
#include "scid/database/common.h"
#include "scid/database/game_id.h"
#include "scid/database/matsig.h"
#include <array>
#include <cstdint>
#include <optional>

namespace scid::database {

/** Mask containing every database game flag bit. */
inline constexpr std::uint32_t GAME_FLAG_MASK_ALL = 0xffffffff;

/**
 * Flag positions used by database metadata.
 *
 * These flags are stored compactly in @ref IndexEntry and exposed as a raw
 * mask in @ref GameInfo.  Some flags are maintained by the storage layer
 * (for example start position and promotions), while the opening, theme, and
 * custom flags are user-visible classification marks.
 */
enum gameFlagT : std::uint32_t {
	/** Game has a non-standard start position. */
	GAME_FLAG_START = 0,
	/** Game contains at least one promotion. */
	GAME_FLAG_PROMO,
	/** Game contains at least one under-promotion. */
	GAME_FLAG_UPROMO,
	/** Game is marked as deleted. */
	GAME_FLAG_DELETE,
	/** User flag: notable White opening play. */
	GAME_FLAG_WHITE_OP,
	/** User flag: notable Black opening play. */
	GAME_FLAG_BLACK_OP,
	/** User flag: middlegame theme. */
	GAME_FLAG_MIDDLEGAME,
	/** User flag: endgame theme. */
	GAME_FLAG_ENDGAME,
	/** User flag: opening novelty. */
	GAME_FLAG_NOVELTY,
	/** User flag: pawn-structure theme. */
	GAME_FLAG_PAWN,
	/** User flag: tactical theme. */
	GAME_FLAG_TACTICS,
	/** User flag: kingside play. */
	GAME_FLAG_KSIDE,
	/** User flag: queenside play. */
	GAME_FLAG_QSIDE,
	/** User flag: brilliancy or good play. */
	GAME_FLAG_BRILLIANCY,
	/** User flag: blunder or bad play. */
	GAME_FLAG_BLUNDER,
	/** General user-defined flag. */
	GAME_FLAG_USER,
	/** First custom user flag. */
	GAME_FLAG_CUSTOM1,
	/** Second custom user flag. */
	GAME_FLAG_CUSTOM2,
	/** Third custom user flag. */
	GAME_FLAG_CUSTOM3,
	/** Fourth custom user flag. */
	GAME_FLAG_CUSTOM4,
	/** Fifth custom user flag. */
	GAME_FLAG_CUSTOM5,
	/** Sixth custom user flag. */
	GAME_FLAG_CUSTOM6,
	/** Number of defined flag positions. */
	GAME_FLAG_COUNT,
};

/**
 * Converts a Scid flag character to a flag mask.
 *
 * The mapping accepts the traditional Scid flag letters, digits 1-6 for the
 * custom flags, and @c ! / @c ? for brilliancy and blunder.  Unknown
 * characters return zero, which lets callers parse user-supplied flag strings
 * without treating an unsupported character as an assertion failure.
 */
std::uint32_t gameFlagMaskFromChar(char flag);

/**
 * Converts a user-visible Scid flag character to a flag index.
 *
 * This helper is intended for flag-count and custom-flag workflows.  It does
 * not treat the storage-maintained @c S, @c X, or @c Y flags as user-visible;
 * unsupported characters return zero.
 */
scid::core::uint gameFlagIndexFromChar(char flag);

/**
 * Builds a combined flag mask from a NUL-terminated string of Scid flag
 * characters.
 *
 * A null pointer or an empty string returns zero.  Unknown characters
 * contribute no bits.
 */
std::uint32_t gameFlagMaskFromString(const char* flags);

/**
 * Public value snapshot of the metadata stored for one database game.
 *
 * @ref GameInfo is produced from an @ref IndexEntry by @ref scidBaseT.  It is
 * deliberately a value type: callers can copy it freely without holding a
 * pointer into the database index.  Name fields such as @c white and @c event
 * are @c idNumberT handles into the database @ref NameBase, not string values.
 */
struct GameInfo {
	/** Offset of the encoded game record in the game data file. */
	std::uint64_t offset = 0;
	/** Length of the encoded game record in bytes. */
	std::uint32_t length = 0;
	/** NameBase player identifier for White. */
	idNumberT white = 0;
	/** NameBase player identifier for Black. */
	idNumberT black = 0;
	/** NameBase event identifier. */
	idNumberT event = 0;
	/** NameBase site identifier. */
	idNumberT site = 0;
	/** NameBase round identifier. */
	idNumberT round = 0;
	/** White rating value, or zero when unknown. */
	scid::core::ratingT whiteElo = 0;
	/** Black rating value, or zero when unknown. */
	scid::core::ratingT blackElo = 0;
	/** Rating system used for @c whiteElo. */
	scid::core::ratingTypeT whiteRatingType = 0;
	/** Rating system used for @c blackElo. */
	scid::core::ratingTypeT blackRatingType = 0;
	/** Date on which the game was played. */
	scid::core::dateT date = scid::core::ZERO_DATE;
	/** Date associated with the event as a whole. */
	scid::core::dateT eventDate = scid::core::ZERO_DATE;
	/** Game result. */
	scid::core::resultT result = scid::core::RESULT_None;
	/** Approximate number of recursive variations in the game. */
	scid::core::uint variationCount = 0;
	/** Approximate number of comments in the game. */
	scid::core::uint commentCount = 0;
	/** Approximate number of NAGs in the game. */
	scid::core::uint nagCount = 0;
	/** Number of half-moves in the main line, capped by the index format. */
	std::uint16_t halfMoveCount = 0;
	/** Material signature of the final mainline position. */
	matSigT finalMaterial = 0;
	/** Stored-line classification code used by opening/tree features. */
	scid::core::byte storedLineCode = 0;
	/** Packed ECO classification, or @c ECO_CODE_NONE. */
	EcoCode ecoCode = ECO_CODE_NONE;
	/** Raw bit mask of @c gameFlagT values. */
	std::uint32_t flags = 0;
	/** Home-pawn signature data; byte zero stores the number of valid entries. */
	std::array<scid::core::byte, 9> homePawnData = {};
	/** True for standard chess castling, false for Chess960. */
	bool chessStd = true;

	/** Returns true when every bit in @p mask is set. */
	bool hasFlag(std::uint32_t mask) const { return (flags & mask) == mask; }
	/** Returns true when the game has a non-standard start position. */
	bool hasStartFlag() const { return hasFlag(1u << GAME_FLAG_START); }
	/** Returns true when the game contains a promotion. */
	bool hasPromotionsFlag() const { return hasFlag(1u << GAME_FLAG_PROMO); }
	/** Returns true when the game contains an under-promotion. */
	bool hasUnderPromoFlag() const { return hasFlag(1u << GAME_FLAG_UPROMO); }
	/** Returns true when the game is marked as deleted. */
	bool hasDeleteFlag() const { return hasFlag(1u << GAME_FLAG_DELETE); }
	/** Returns true when the index reports at least one comment. */
	bool hasComments() const { return commentCount > 0; }
	/** Returns true when the index reports at least one variation. */
	bool hasVariations() const { return variationCount > 0; }
	/** Returns the year component of @c date, or zero when unknown. */
	scid::core::uint year() const { return scid::core::date_GetYear(date); }
	/** Returns the month component of @c date, or zero when unknown. */
	scid::core::uint month() const { return scid::core::date_GetMonth(date); }
	/** Returns the day component of @c date, or zero when unknown. */
	scid::core::uint day() const { return scid::core::date_GetDay(date); }
	/**
	 * Computes Scid's compact game-quality rating.
	 *
	 * The value is derived from both players' ratings and then adjusted for
	 * annotations and short draws.  It is a search/sort heuristic, not a chess
	 * engine evaluation.
	 */
	scid::core::byte rating() const;
	/**
	 * Writes the set flags from @p flags into @p dest.
	 *
	 * When @p flags is null, the traditional user-visible flag order is used.
	 * @p dest must have room for every requested flag character plus the
	 * terminating NUL byte.
	 *
	 * @returns the number of flag characters written, excluding the terminator.
	 */
	scid::core::uint flagString(char* dest, const char* flags) const;
};

/**
 * Partial metadata update for an existing database game.
 *
 * Only engaged optionals are applied.  This lets callers update inexpensive
 * header/index fields without loading and re-encoding the complete game body.
 */
struct GameInfoUpdate {
	/** Replacement game date. */
	std::optional<scid::core::dateT> date;
	/** Replacement event NameBase identifier. */
	std::optional<idNumberT> event;
	/** Replacement round NameBase identifier. */
	std::optional<idNumberT> round;
	/** Replacement White rating. */
	std::optional<scid::core::ratingT> whiteElo;
	/** Replacement Black rating. */
	std::optional<scid::core::ratingT> blackElo;
	/** Replacement ECO classification. */
	std::optional<EcoCode> ecoCode;

	/** Returns true when the update would leave every field unchanged. */
	bool empty() const {
		return !date && !event && !round && !whiteElo && !blackElo &&
		       !ecoCode;
	}
};

} // namespace scid::database
