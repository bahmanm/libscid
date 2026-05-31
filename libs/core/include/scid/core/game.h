#pragma once

/** @file
 * Mutable chess game model used by the core API.
 */

#include "scid/core/date.h"
#include "scid/core/error.h"
#include "scid/core/game_result.h"
#include "scid/core/move.h"
#include "scid/core/nags.h"
#include "scid/core/notation.h"
#include "scid/core/position.h"
#include "scid/core/rating.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace scid::core {

class MovetextCursor;

/**
 * A PGN tag pair that is not represented by one of the typed header fields.
 */
using TagPair = std::pair<std::string, std::string>;

/**
 * A player's rating and the rating system it belongs to.
 *
 * A value of zero represents an unknown or unavailable rating.
 */
struct Rating {
	/** The numeric rating value. */
	scid::core::ratingT value = 0;
	/** The rating system used to interpret @c value. */
	scid::core::ratingTypeT type = scid::core::RATING_Elo;
};

/**
 * A chess player as recorded in a game header.
 */
struct Player {
	/** The player's display name, usually the PGN White or Black tag value. */
	std::string name;
	/** The player's rating metadata. */
	Rating rating;
};

/**
 * Event-level metadata for a game.
 *
 * The game date and event date are separate because tournaments commonly span
 * multiple days: @c date is the date of this game, while @c eventDate is
 * the start date or identifying date of the event.
 */
struct EventInfo {
	/** The event name, corresponding to the PGN Event tag. */
	std::string name;
	/** The site or venue, corresponding to the PGN Site tag. */
	std::string site;
	/** The round identifier, corresponding to the PGN Round tag. */
	std::string round;
	/** The date on which the game was played. */
	scid::core::dateT date = scid::core::ZERO_DATE;
	/** The date associated with the event as a whole. */
	scid::core::dateT eventDate = scid::core::ZERO_DATE;
};

/**
 * The structured header of a chess game.
 *
 * Standard game attributes are stored as typed fields. Supplemental PGN tags
 * that do not have a typed representation are kept in @c tags.
 */
struct GameHeader {
	/** Event, site, round, and date metadata. */
	EventInfo event;
	/** The player with the white pieces. */
	Player white;
	/** The player with the black pieces. */
	Player black;
	/** The game result, using Scid's compact result code. */
	scid::core::resultT result = scid::core::RESULT_None;
	/** The ECO code string, if known. */
	std::string eco;
	/** Supplemental PGN tags outside the typed header fields. */
	std::vector<TagPair> tags;
};

/**
 * Annotation metadata attached to a move.
 */
struct MoveMetadata {
	/** Numeric Annotation Glyphs associated with the move. */
	std::vector<Nag> nags;
	/** The free-form comment associated with the move. */
	std::string comment;
};

struct Variation;

/**
 * A played move in a mainline or variation.
 *
 * The move stores the portable move intent in @c spec. @c san may cache
 * the SAN text from imported PGN; when it is empty, notation can be generated
 * from the move and the preceding position.
 */
struct Move {
	/**
	 * Adds a child variation that branches before the following move.
	 *
	 * @param initialComment the comment that appears at the start of the
	 * variation, before its first move.
	 * @returns the newly appended variation.
	 */
	Variation& addVariation(std::string_view initialComment = {});

	/** The source, destination, and promotion intent of the move. */
	MoveSpec spec;
	/** SAN text for the move, when it is already known. */
	std::string san;
	/** Comments and NAGs attached to this move. */
	MoveMetadata metadata;
	/** Variations that branch from this move. */
	std::vector<Variation> childVariations;
};

/**
 * An ordered sequence of moves.
 *
 * A move sequence is used both for the game's mainline and for each recursive
 * variation line.
 */
struct MoveSequence {
	/**
	 * Appends a move to the end of the sequence.
	 *
	 * @param spec the portable move intent to store.
	 * @returns the newly appended move.
	 */
	Move& appendMove(MoveSpec spec);

	/** The moves in this line, in playback order. */
	std::vector<Move> moves;
};

/**
 * A recursive annotation variation.
 *
 * In PGN terms, a variation is an alternate line that can be played by first
 * unplaying the move immediately before the variation.
 */
struct Variation {
	/** The comment that appears before the first move of the variation. */
	std::string initialComment;
	/** The moves that make up the variation line. */
	MoveSequence line;
};

/**
 * The playable move tree of a game.
 */
struct Movetext {
	/** The comment that appears before the first mainline move. */
	std::string initialComment;
	/** The primary line of play. */
	MoveSequence mainline;
};

/**
 * A mutable chess game made of header metadata, an optional start position, and
 * a recursive movetext tree.
 *
 * Game is the core API's editable representation of a single chess game. It is
 * the target of PGN parsing and database loading, and the source consumed by
 * PGN encoding, database storage, notation helpers, and cursor-based traversal.
 */
class Game {
public:
	/**
	 * Creates an empty game with a standard chess start position.
	 */
	Game();

	/**
	 * Resets the header, movetext, and start position to their empty defaults.
	 */
	void clear();

	/**
	 * Returns the structured game header.
	 */
	const GameHeader& header() const;

	/**
	 * Returns the complete movetext tree.
	 */
	const Movetext& movetext() const;

	/**
	 * Returns the comment before the first mainline move.
	 */
	std::string_view initialComment() const;

	/**
	 * Returns the number of half-moves in the mainline.
	 */
	std::size_t mainlineHalfMoveCount() const;

	/**
	 * Returns the event name.
	 */
	const std::string& event() const;

	/**
	 * Returns the event site or venue.
	 */
	const std::string& site() const;

	/**
	 * Returns the round identifier.
	 */
	const std::string& round() const;

	/**
	 * Returns the player with the white pieces.
	 */
	const Player& white() const;

	/**
	 * Returns the player with the black pieces.
	 */
	const Player& black() const;

	/**
	 * Returns the date on which the game was played.
	 */
	scid::core::dateT date() const;

	/**
	 * Returns the date associated with the event as a whole.
	 */
	scid::core::dateT eventDate() const;

	/**
	 * Returns the game result code.
	 */
	scid::core::resultT result() const;

	/**
	 * Returns the PGN result token for the current result.
	 */
	std::string_view resultString() const;

	/**
	 * Returns the ECO code string.
	 */
	const std::string& eco() const;

	/**
	 * Returns the mean of the players' ratings.
	 *
	 * If either rating is unknown, the average is reported as zero.
	 */
	scid::core::ratingT averageRating() const;

	/**
	 * Sets the event name.
	 */
	void setEvent(std::string_view value);

	/**
	 * Sets the event site or venue.
	 */
	void setSite(std::string_view value);

	/**
	 * Sets the round identifier.
	 */
	void setRound(std::string_view value);

	/**
	 * Sets the name of the player with the white pieces.
	 */
	void setWhiteName(std::string_view value);

	/**
	 * Sets the name of the player with the black pieces.
	 */
	void setBlackName(std::string_view value);

	/**
	 * Replaces the player metadata for White.
	 */
	void setWhite(Player value);

	/**
	 * Replaces the player metadata for Black.
	 */
	void setBlack(Player value);

	/**
	 * Sets White's rating, normalising an unknown rating type to Elo.
	 */
	void setWhiteRating(Rating value);

	/**
	 * Sets Black's rating, normalising an unknown rating type to Elo.
	 */
	void setBlackRating(Rating value);

	/**
	 * Sets the date on which the game was played.
	 */
	void setDate(scid::core::dateT value);

	/**
	 * Sets the date associated with the event as a whole.
	 */
	void setEventDate(scid::core::dateT value);

	/**
	 * Sets the game result code.
	 */
	void setResult(scid::core::resultT value);

	/**
	 * Sets the ECO code string.
	 */
	void setEco(std::string_view value);

	/**
	 * Adds a PGN tag value to the game.
	 *
	 * Standard tags represented by typed fields, such as Event, Site, Round,
	 * White, and Black, update those fields instead of creating an extra tag.
	 *
	 * @param tag the PGN tag name.
	 * @param value the PGN tag value.
	 * @returns the stored value so callers can inspect or modify it in place.
	 */
	std::string& addTag(std::string_view tag, std::string_view value);

	/**
	 * Finds an existing tag value or creates an empty one.
	 *
	 * Standard tags represented by typed fields return the corresponding typed
	 * field. Supplemental tags are searched in insertion order and appended if
	 * no matching tag exists.
	 *
	 * @param tag the PGN tag name.
	 * @returns the stored value so callers can modify it in place.
	 */
	std::string& findOrCreateTag(std::string_view tag);

	/**
	 * Returns the supplemental PGN tags.
	 *
	 * These are the tags that are not represented by the typed header fields.
	 */
	const std::vector<TagPair>& extraTags() const;

	/**
	 * Finds a supplemental PGN tag by name.
	 *
	 * @param tag the PGN tag name.
	 * @returns a pointer to the tag value, or nullptr when no supplemental tag
	 * with that name exists.
	 */
	const std::string* findExtraTag(std::string_view tag) const;

	/**
	 * Removes all supplemental PGN tags.
	 */
	void clearExtraTags();

	/**
	 * Removes every supplemental PGN tag with the given name.
	 */
	void removeExtraTag(std::string_view tag);

	/**
	 * Reports whether the game starts from an explicit position rather than the
	 * standard chess starting position.
	 */
	bool hasNonStandardStart() const;

	/**
	 * Reports whether the game has a non-standard start and optionally writes
	 * that position as FEN.
	 *
	 * @param outFen destination buffer for the FEN text; may be nullptr.
	 * @param outFenLen length of @p outFen in bytes.
	 * @returns true when a non-standard start position exists.
	 */
	bool hasNonStandardStart(char* outFen, std::size_t outFenLen) const;

	/**
	 * Returns the mutable non-standard start position, if one exists.
	 */
	scid::core::Position* startPosition();

	/**
	 * Returns the non-standard start position, if one exists.
	 */
	const scid::core::Position* startPosition() const;

	/**
	 * Parses and stores a non-standard start position from FEN.
	 *
	 * If parsing fails, the existing start position is left unchanged.
	 *
	 * @param fen the FEN text to parse.
	 * @returns @c OK on success, otherwise the parse error.
	 */
	scid::core::errorT setStartFen(const char* fen);

	/**
	 * Sets the non-standard start position from an already validated position.
	 */
	void setStartPosition(const scid::core::Position& position);

	/**
	 * Clears the non-standard start position and returns the game to the
	 * standard chess starting position.
	 */
	void clearStartPosition();

	/**
	 * Returns the ply counter of the start position.
	 *
	 * Games with the standard start position begin at ply zero.
	 */
	long long initialPlyCounter() const;

	/**
	 * Appends a move to the game mainline.
	 *
	 * @param spec the portable move intent to store.
	 * @returns the newly appended move.
	 */
	Move& appendMainlineMove(MoveSpec spec);

	/**
	 * Sets the comment before the first mainline move.
	 */
	void setInitialComment(std::string_view value);

	/**
	 * Clears the movetext while preserving the header and start position.
	 */
	void clearMovetext();

	/**
	 * Removes selected annotation layers from the movetext.
	 *
	 * @param variations when true, removes all child variations.
	 * @param comments when true, clears the initial comment, move comments, and
	 * variation initial comments.
	 * @param nags when true, clears all move NAGs.
	 */
	void stripMovetext(bool variations, bool comments, bool nags);

private:
	friend class MovetextCursor;

	std::string* findStandardTag(std::string_view tag);

	GameHeader header_;
	Movetext movetext_;
	std::optional<scid::core::Position> startPosition_;
};

} // namespace scid::core
