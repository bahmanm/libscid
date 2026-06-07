/*
# Copyright (C) 2014-2019 Fulvio Benini

* This file is part of Scid (Shane's Chess Information Database).
*
* Scid is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation.
*
* Scid is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Scid. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SCIDBASE_H
#define SCIDBASE_H

#include "scid/core/game.h"
#include "scid/core/game_result.h"
#include "scid/core/fullmove.h"
#include "scid/core/board.h"
#include "scid/database/game_id.h"
#include "scid/database/game_info.h"
#include "scid/database/hfilter.h"
#include "scid/database/index.h"
#include "scid/database/namebase.h"
#include "scid/database/tree.h"
#include <array>
#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace scid::core {
class Position;
}

namespace scid::database {

class ByteBuffer;
class GameView;
class Progress;
class SortCache;

/**
 * Piece-placement predicate used by material searches.
 *
 * A pattern can describe a piece constrained by rank, file, or exact square,
 * depending on how the search parser fills the fields.  A zero @c flag means
 * the pattern is negative: the described piece placement must not occur.
 */
struct patternT {
	/** Piece that must match, including colour. */
	scid::core::pieceT pieceMatch;
	/** Required rank, or the parser's wildcard value. */
	scid::core::rankT rankMatch;
	/** Required file, or the parser's wildcard value. */
	scid::core::fyleT fyleMatch;
	/** Match polarity and parser flags; zero means the pattern must not occur. */
	scid::core::byte flag; // 0 means this pattern must not occur.
};

/**
 * Strictness level for board-position matching.
 *
 * Exact searches compare complete board occupancy.  The looser modes are used
 * by database search features that intentionally collapse positions into
 * broader families: pawn structure, piece files, or material balance.  Use
 * the narrowest mode that matches the user's question; looser modes can
 * produce many games that are strategically related but not the same position.
 */
enum gameExactMatchT : int {
	/** Match the exact board. */
	GAME_EXACT_MATCH_Exact = 0,
	/** Match pawn structure. */
	GAME_EXACT_MATCH_Pawns,
	/** Match piece files. */
	GAME_EXACT_MATCH_Fyles,
	/** Match material only. */
	GAME_EXACT_MATCH_Material
};

/**
 * Open database session and primary entry point for database operations.
 *
 * A @ref scidBaseT object binds together the storage codec, in-memory
 * @ref Index, @ref NameBase, working filters, sort caches, and derived
 * statistics for one opened database.  The index is the cheap metadata layer:
 * it answers list, sort, filter, tree, and many search-prefilter questions
 * without decoding movetext.  The codec owns the persistent representation and
 * is used when a caller needs a full @ref scid::core::Game or when a change
 * must be written back.
 *
 * Game numbers are zero-based indexes into the current database index.  They
 * are convenient handles for the current open session, not durable external
 * identifiers; callers should refresh them after compaction or after reopening
 * a database that may have been modified.  Name fields are likewise stored as
 * @c idNumberT handles into the current database's @ref NameBase; resolve
 * them with @ref tagRoster() or @ref getNameBase() before presenting them
 * outside the database layer.
 *
 * Modifying operations update the codec, index, filters, caches, and the
 * cache-invalidation token as needed.  They are not a general transactional
 * abstraction: when a codec or filesystem failure happens after partial work,
 * the documented operation-specific error semantics describe what remains
 * changed.
 */
struct scidBaseT {
	/** Summary of a player-rating update operation. */
	struct RatingUpdateStats {
		/** Number of individual rating fields changed. */
		scid::core::uint changedRatings = 0;
		/** Number of games that had at least one rating changed. */
		scid::core::uint changedGames = 0;
	};

	/**
	 * Whole-database statistics derived from the index.
	 *
	 * These values are not filter-scoped.  They are built lazily by
	 * @ref getStats() and cached until the database cache-invalidation token is
	 * advanced by a modifying operation.
	 */
	struct Stats {
		/** Number of games with each index flag set. */
		scid::core::uint flagCount[IndexEntry::IDX_NUM_FLAGS];
		/** Earliest known game date, or zero when the database is empty. */
		scid::core::dateT minDate;
		/** Latest known game date, or zero when the database is empty. */
		scid::core::dateT maxDate;
		/** Number of games with a non-zero year. */
		uint64_t nYears;
		/** Sum of non-zero game years, for average-year calculations. */
		uint64_t sumYears;
		/** Number of games by result code. */
		scid::core::uint nResults[scid::core::NUM_RESULT_TYPES];
		/** Number of non-zero individual player ratings. */
		scid::core::uint nRatings;
		/** Sum of non-zero individual player ratings. */
		uint64_t sumRatings;
		/** Lowest non-zero player rating, or zero when none are known. */
		scid::core::uint minRating;
		/** Highest non-zero player rating, or zero when none are known. */
		scid::core::uint maxRating;

		/** Builds statistics by scanning the database index. */
		Stats(const scidBaseT* dbase);

		/** ECO frequency and result summary. */
		struct Eco {
			/** Number of games in this ECO bucket. */
			scid::core::uint count;
			/** Number of games in this ECO bucket by result code. */
			scid::core::uint results[scid::core::NUM_RESULT_TYPES];

			/** Creates an empty ECO summary. */
			Eco();
		};
		/**
		 * Returns statistics for an ECO prefix.
		 *
		 * An empty string returns all games with a valid ECO code.  One-, two-,
		 * and three-character strings return progressively narrower ECO
		 * groups; four- and five-character strings return the exact reduced
		 * ECO bucket.  Unknown or invalid ECO strings return null.
		 */
		const Eco* getEcoStats(const char* ecoStr) const;

	private:
		Eco ecoEmpty_;
		Eco ecoValid_;
		Eco ecoStats_[(1 + (1 << 16) / 131) * 27];
		Eco ecoGroup1_[(1 + (1 << 16) / 131) / 100];
		Eco ecoGroup2_[(1 + (1 << 16) / 131) / 10];
		Eco ecoGroup3_[(1 + (1 << 16) / 131)];
	};

	scidBaseT();
	~scidBaseT();

	/**
	 * Opens a database using the requested codec and file mode.
	 *
	 * @p dbType is one of @c "MEMORY", @c "PGN", @c "SCID4", or @c "SCID5".
	 * On success, the index, namebase, default filter, and codec are ready for
	 * use.  A return value of @ref scid::core::ERROR_NameDataLoss is a warning:
	 * the database is open, but some corrupted names were repaired or lost.
	 * Other errors leave the object closed.
	 *
	 * @param dbType codec name.
	 * @param fMode requested file access mode.
	 * @param filename database filename or basename understood by the codec.
	 * @param progress optional progress/cancellation callback.
	 */
	scid::core::errorT open(std::string_view dbType, fileModeT fMode,
	                        const char* filename,
	                        const Progress& progress = {});

	/**
	 * Closes the current database and releases codec, index, namebase, filter,
	 * statistics, and sort-cache state.
	 */
	void Close();

	/**
	 * Returns a display filename for the opened database.
	 *
	 * Closed databases return @c "<empty>".  In-memory databases with no
	 * backing files return @c "<clipbase>".  File-backed codecs return the
	 * first filename reported by the codec.
	 */
	std::string getFileName() const;
	/** Returns true when a database is currently open. */
	bool isOpen() const { return inUse; }
	/** Returns true when the opened database was opened read-only. */
	bool isReadOnly() const { return fileMode_ == FMODE_ReadOnly; }
	/** Returns the number of games in the current index. */
	gamenumT numGames() const { return idx->GetNumGames(); }

	/**
	 * Returns codec-specific database metadata.
	 *
	 * Typical keys include @c "type", @c "description", @c "autoload", and
	 * custom flag names.  The supported keys and persistence semantics depend
	 * on the active codec.
	 */
	std::vector<std::pair<const char*, std::string>> getExtraInfo() const;

	/**
	 * Stores codec-specific database metadata and flushes it to storage.
	 *
	 * Read-only databases return @ref scid::core::ERROR_FileReadOnly.
	 * Unsupported keys return the codec's error code.
	 */
	scid::core::errorT setExtraInfo(const char* tagname,
	                                const char* new_value);

	/**
	 * Returns the index entry for game @p g.
	 *
	 * @p g must be less than @ref numGames().  The returned pointer is owned by
	 * the database and remains valid until the index entry is replaced or the
	 * database is closed.
	 */
	const IndexEntry* getIndexEntry(gamenumT g) const {
		assert(g < numGames());
		return idx->GetEntry(g);
	}
	/**
	 * Returns the index entry for @p g, or null when @p g is out of range.
	 */
	const IndexEntry* getIndexEntry_bounds(gamenumT g) const {
		static_assert(std::is_unsigned_v<gamenumT>);
		return g < numGames() ? getIndexEntry(g) : nullptr;
	}
	/**
	 * Returns a value snapshot of one game's index metadata.
	 *
	 * @p g must be less than @ref numGames().
	 */
	GameInfo gameInfo(gamenumT g) const;
	/**
	 * Returns @ref gameInfo() for @p g, or @c std::nullopt when out of range.
	 */
	std::optional<GameInfo> gameInfoBounds(gamenumT g) const {
		static_assert(std::is_unsigned_v<gamenumT>);
		return g < numGames() ? std::optional<GameInfo>{gameInfo(g)}
		                      : std::nullopt;
	}
	/**
	 * Applies a partial metadata update to one game.
	 *
	 * Only engaged fields in @p update are changed.  The implementation
	 * rewrites the game so index metadata and encoded standard tags stay in
	 * sync.
	 *
	 * @returns @ref scid::core::OK, @ref scid::core::ERROR_BadArg for an
	 *          invalid game number, or a storage/codec error.
	 */
	scid::core::errorT updateGameInfo(gamenumT g, const GameInfoUpdate& update);
	/**
	 * Resolves the Seven Tag Roster name fields for game @p gnum.
	 *
	 * The returned string pointers are owned by the database namebase.
	 */
	TagRoster tagRoster(gamenumT gnum) const {
		return tagRoster(*getIndexEntry(gnum));
	}
	/**
	 * Resolves the Seven Tag Roster name fields for @p ie.
	 *
	 * The index entry must belong to this database's namebase.
	 */
	TagRoster tagRoster(IndexEntry const& ie) const {
		return TagRoster::make(ie, *nb_);
	}

	/**
	 * Returns the database name table.
	 *
	 * The returned object is owned by the database and remains valid until the
	 * database is destroyed.
	 */
	const NameBase* getNameBase() const { return nb_; }

	/**
	 * Returns the highest known rating for a player in this database.
	 *
	 * @p playerID is a player-name identifier from this database's
	 * @ref NameBase.  The first call scans both White and Black fields across
	 * the current index and caches the maximum rating for every player ID.
	 * Unknown ratings contribute zero, and a player with no rated games returns
	 * zero.
	 */
	scid::core::ratingT peakElo(idNumberT playerID) const {
		if (peakEloCache_.empty()) {
			for (gamenumT gnum = 0, n = numGames(); gnum < n; gnum++) {
				IndexEntry const& ie = *getIndexEntry(gnum);
				auto updateMax = [&](auto id, auto elo) {
					auto& max_value = peakEloCache_[id];
					max_value = std::max(max_value, elo);
				};
				updateMax(ie.GetWhite(), ie.GetWhiteElo());
				updateMax(ie.GetBlack(), ie.GetBlackElo());
			}
		}
		return peakEloCache_[playerID];
	}

	/**
	 * Decodes a complete game into @p dest.
	 *
	 * This loads standard tags, extra tags, start position, movetext,
	 * comments, NAGs, and variations.  When @p scidFlags is non-null and
	 * @p scidFlagsLen is non-zero, it receives a NUL-terminated string of the
	 * user-visible Scid flags for the game.
	 */
	scid::core::errorT loadGame(const IndexEntry& ie, scid::core::Game& dest,
	                            char* scidFlags,
	                            std::size_t scidFlagsLen) const;
	/**
	 * Bounds-checked overload of @ref loadGame() by game number.
	 */
	scid::core::errorT loadGame(gamenumT gNum, scid::core::Game& dest,
	                            char* scidFlags,
	                            std::size_t scidFlagsLen) const;
	/**
	 * Loads only the start position and movetext into @p dest.
	 *
	 * Standard tags, extra tags, comments, and NAGs are skipped.  This is the
	 * cheaper path for callers that need playable moves but not full PGN
	 * metadata.
	 */
	scid::core::errorT loadGameMovesOnly(gamenumT gNum,
	                                     scid::core::Game& dest) const;
	/**
	 * Loads only the start position and movetext for @p ie.
	 */
	scid::core::errorT loadGameMovesOnly(const IndexEntry& ie,
	                                     scid::core::Game& dest) const;
	/**
	 * Decodes only the non-standard stored tag pairs for game @p gNum.
	 *
	 * The standard PGN tags represented by the index/namebase are not included.
	 * Tags are appended to @p dest.
	 */
	scid::core::errorT gameTags(
	    gamenumT gNum,
	    std::vector<std::pair<std::string, std::string>>& dest) const;
	/**
	 * Loads standard tags and Scid flag text without decoding movetext.
	 */
	scid::core::errorT loadStandardTags(gamenumT gNum,
	                                    scid::core::Game& dest,
	                                    char* scidFlags,
	                                    std::size_t scidFlagsLen) const;
	/**
	 * Decodes only the non-standard stored tag pairs for @p ie.
	 */
	scid::core::errorT gameTags(
	    const IndexEntry& ie,
	    std::vector<std::pair<std::string, std::string>>& dest) const;
	/**
	 * Returns up to @p maxPly mainline moves for game @p gNum.
	 *
	 * Out-of-range game numbers return an empty vector.
	 */
	std::vector<scid::core::FullMove> mainlineMoves(
	    gamenumT gNum, std::size_t maxPly) const;
	/**
	 * Returns up to @p maxPly mainline moves for @p ie.
	 */
	std::vector<scid::core::FullMove> mainlineMoves(
	    const IndexEntry* ie, std::size_t maxPly) const;
	/**
	 * Returns SAN for @p count moves after skipping @p plyToSkip half-moves.
	 *
	 * Out-of-range game numbers return an empty string.
	 */
	std::string moveSAN(gamenumT gNum, int plyToSkip, int count) const;
	/**
	 * Returns SAN for @p count moves from @p ie after skipping @p plyToSkip
	 * half-moves.
	 */
	std::string moveSAN(const IndexEntry* ie, int plyToSkip, int count) const;
	/**
	 * Replaces game dates in a filtered set of games.
	 *
	 * Only games included in @p filter whose index date equals @p oldDate are
	 * changed.  The update rewrites the affected @ref IndexEntry records; it
	 * does not decode or re-encode full game movetext.  Progress is reported
	 * while scanning the filter, and cancellation stops before the next batch
	 * of index entries is processed.
	 *
	 * @returns a pair of error code and number of games whose date changed.
	 */
	std::pair<scid::core::errorT, size_t>
	replaceGameDates(HFilter filter, const Progress& progress,
	                 scid::core::dateT oldDate, scid::core::dateT newDate);
	/**
	 * Replaces event dates in a filtered set of games.
	 *
	 * This is the event-date counterpart of @ref replaceGameDates(): it scans
	 * @p filter, changes only entries whose event date equals @p oldDate, and
	 * persists the modified index entries.
	 *
	 * @returns a pair of error code and number of games whose event date
	 *          changed.
	 */
	std::pair<scid::core::errorT, size_t>
	replaceGameEventDates(HFilter filter, const Progress& progress,
	                      scid::core::dateT oldDate,
	                      scid::core::dateT newDate);
	/**
	 * Sets one player's rating in every filtered game where they appear.
	 *
	 * @p player is a player-name ID from this database's @ref NameBase.  For
	 * each included game, the White and Black fields are checked independently;
	 * either or both sides may be updated.  Updated ratings are marked with
	 * @p ratingType.  The operation is an index-only rewrite.
	 *
	 * @returns a pair of error code and number of games where at least one
	 *          side's rating field changed.
	 */
	std::pair<scid::core::errorT, size_t>
	setPlayerRatings(HFilter filter, const Progress& progress, idNumberT player,
	                 scid::core::ratingT rating,
	                 scid::core::ratingTypeT ratingType);
	/**
	 * Updates player ratings in a filtered set using a caller-provided source.
	 *
	 * @p ratingFor is called as @c ratingFor(playerId, gameDate) for each side
	 * whose rating may be updated.  It should return a non-zero Elo value when
	 * a rating is available, or zero to leave that side unchanged.  If
	 * @p overwrite is false, existing non-zero ratings are not queried or
	 * changed.  Ratings written by this function use
	 * @ref scid::core::RATING_Elo.
	 *
	 * When @p saveRatings is false, the function performs the same filtered
	 * scan and returns counts, but does not write index entries or require a
	 * modifying transaction.  When it is true, matching entries are rewritten
	 * through the normal index-transform path.
	 *
	 * @returns an error code plus counts of changed rating fields and games
	 *          with at least one available rating.
	 */
	template <typename TRatingResolver>
	std::pair<scid::core::errorT, RatingUpdateStats> updatePlayerRatings(
	    HFilter filter, const Progress& progress, bool overwrite,
	    bool saveRatings, TRatingResolver ratingFor);
	/**
	 * Tests one game against a board-position search.
	 *
	 * This is the low-level matcher behind the full board-search workflow.
	 * Callers are expected to have already applied cheap index prefilters and
	 * pass those conclusions in @p possibleMatch and
	 * @p possibleFlippedMatch.  When either flag is false, the corresponding
	 * position pointer is ignored; when it is true, @p pos or @p posFlip must
	 * point at the normal or colour-flipped search position to test.
	 *
	 * Without @p useVariations the function decodes only the main line from
	 * storage and leaves @p game as scratch state supplied by the caller.  With
	 * @p useVariations it decodes the movetext into @p game so variations can
	 * be walked recursively.  The search type controls whether the comparison
	 * requires the exact board, only pawns, pawn files, or material.
	 *
	 * @p ply is set to zero when the game does not match, and to one when it
	 * does.  It is a filter inclusion value, not the precise ply at which the
	 * match was found.
	 *
	 * @returns @ref scid::core::OK when the game record was readable,
	 *          @ref scid::core::ERROR_FileRead when stored game data could not
	 *          be loaded.
	 */
	scid::core::errorT searchBoard(const IndexEntry& ie,
	                               scid::core::Game& game,
	                               scid::core::Position* pos,
	                               scid::core::Position* posFlip,
	                               bool useVariations,
	                               bool possibleMatch,
	                               bool possibleFlippedMatch,
	                               gameExactMatchT searchType,
	                               scid::core::uint& ply) const;
	/**
	 * Bounds-checked overload of @ref searchBoard() by game number.
	 *
	 * @returns @ref scid::core::ERROR_BadArg when @p gNum is outside the
	 *          database, otherwise the result of the index-entry overload.
	 */
	scid::core::errorT searchBoard(gamenumT gNum,
	                               scid::core::Game& game,
	                               scid::core::Position* pos,
	                               scid::core::Position* posFlip,
	                               bool useVariations,
	                               bool possibleMatch,
	                               bool possibleFlippedMatch,
	                               gameExactMatchT searchType,
	                               scid::core::uint& ply) const;
	/**
	 * Tests one game against material and piece-placement constraints.
	 *
	 * This function is the game-decoding part of a material search.  The
	 * caller supplies piece-count ranges in @p min and @p max, indexed by
	 * piece constants such as @ref scid::core::WQ and aggregate minor-piece
	 * entries such as @ref scid::core::WM.  Optional @p patterns add
	 * rank/file/square predicates on top of the material counts.
	 *
	 * @p minPly and @p maxPly are half-move bounds in the decoded main line.
	 * @p matchLength requires that many consecutive matching positions before
	 * the game is accepted.  @p oppBishops and @p sameBishops restrict
	 * single-bishop endgames by bishop-square colour.  @p minDiff and
	 * @p maxDiff bound White's material value minus Black's.
	 *
	 * As with @ref searchBoard(), @p possibleMatch and
	 * @p possibleFlippedMatch are index-prefilter results supplied by the
	 * caller.  The flipped arrays and patterns are used only when the flipped
	 * flag is true.  The function searches only the main line and reports a
	 * boolean match; it does not update any filter value or expose the matching
	 * ply.
	 *
	 * @returns true when either the normal or flipped material search matches;
	 *          false when it does not match, @p gNum is invalid in the overload
	 *          below, or the stored game data cannot be read.
	 */
	bool materialSearchMatch(const IndexEntry& ie, bool possibleMatch,
	                         bool possibleFlippedMatch,
	                         scid::core::byte* min, scid::core::byte* max,
	                         scid::core::byte* minFlipped,
	                         scid::core::byte* maxFlipped,
	                         patternT* patterns, std::size_t patternCount,
	                         patternT* flippedPatterns,
	                         std::size_t flippedPatternCount, int minPly,
	                         int maxPly, int matchLength, bool oppBishops,
	                         bool sameBishops, int minDiff,
	                         int maxDiff) const;
	/**
	 * Bounds-checked overload of @ref materialSearchMatch() by game number.
	 */
	bool materialSearchMatch(gamenumT gNum, bool possibleMatch,
	                         bool possibleFlippedMatch,
	                         scid::core::byte* min, scid::core::byte* max,
	                         scid::core::byte* minFlipped,
	                         scid::core::byte* maxFlipped,
	                         patternT* patterns, std::size_t patternCount,
	                         patternT* flippedPatterns,
	                         std::size_t flippedPatternCount, int minPly,
	                         int maxPly, int matchLength, bool oppBishops,
	                         bool sameBishops, int minDiff,
	                         int maxDiff) const;
	/**
	 * Replaces @p filter with games whose main line reaches @p pos.
	 *
	 * This is the high-level exact-position search used by tree and novelty
	 * workflows.  It does not search variations and it does not try a
	 * colour-flipped position.  Matching filter values store the ply hint used
	 * by tree and game-list views: value 1 means the starting position, value
	 * 2 means after the first half-move, and so on, with large plies clamped to
	 * 255.  Non-matching games receive value zero.
	 *
	 * The implementation uses stored-line, home-pawn, and material signatures
	 * from the index before decoding candidate games.  A standard starting
	 * position is handled as a special case: ordinary standard-start games are
	 * included directly, while games with explicit start positions are decoded
	 * and tested.
	 *
	 * @returns false when @p progress requests cancellation.
	 */
	bool setPositionSearchFilter(const scid::core::Position& pos,
	                             HFilter& filter,
	                             const Progress& progress) const;

	/**
	 * Copies the games included in @p filter from another open database.
	 *
	 * Games are imported in filter iteration order.  The encoded game data,
	 * index entry, and Seven Tag Roster names are copied through the active
	 * codecs without first materialising a @ref scid::core::Game.  The source
	 * and destination databases must be different objects.
	 *
	 * The destination is modified inside a database transaction.  Read-only
	 * databases, databases opened with a recoverable open warning, codec
	 * failures, and attempts to import from @p srcBase itself return an error.
	 * If @p progress requests cancellation, the import stops after the current
	 * game and already-copied games remain in the destination.
	 */
	scid::core::errorT importGames(const scidBaseT* srcBase, const HFilter& filter,
	                   const Progress& progress);
	/**
	 * Imports games from an external file.
	 *
	 * At present @p dbType must name the PGN codec.  Parsed games are encoded
	 * into the destination database one at a time, using @p progress for
	 * cancellation.  Parser diagnostics and non-fatal import notes are appended
	 * to @p errorMsg; Chess960 games that cannot be represented by the current
	 * codec are skipped and reported there.
	 *
	 * As with the database-to-database overload, successful games imported
	 * before an error or cancellation remain in the destination.
	 */
	scid::core::errorT importGames(std::string_view dbType, const char* filename,
	                               const Progress& progress,
	                               std::string& errorMsg);

	/**
	 * Adds or replaces a stored game.
	 *
	 * @p game is encoded into the active database format together with
	 * @p scidFlags, the application-visible Scid flag string.  When
	 * @p replacedGameId names an existing game, that record is replaced.  Any
	 * value outside the current game range appends a new game instead; the
	 * default @c INVALID_GAMEID therefore means "append".
	 *
	 * The function updates the codec, index, namebase, filters, sort caches,
	 * and database cache-invalidation token as needed.  It does not offer
	 * rollback semantics: if a codec reports an error after writing part of the
	 * change, callers should treat the database state as codec-defined.
	 *
	 * @returns @ref scid::core::OK, @ref scid::core::ERROR_FileReadOnly,
	 *          an open-warning error that prevents modification, or a codec
	 *          error.
	 */
	scid::core::errorT saveGame(scid::core::Game const& game, const char* scidFlags,
	                gamenumT replacedGameId = INVALID_GAMEID);
	/**
	 * Appends @p game to the database.
	 *
	 * Convenience wrapper for @ref saveGame() with @c INVALID_GAMEID.
	 */
	scid::core::errorT addGame(scid::core::Game const& game, const char* scidFlags) {
		return saveGame(game, scidFlags, INVALID_GAMEID);
	}

	/**
	 * Returns one raw index flag for game @p gNum.
	 *
	 * @p flag is a bit mask such as one returned by @c gameFlagMaskFromChar().
	 * @p gNum must be a valid zero-based game number.
	 */
	bool getFlag(scid::core::uint flag, scid::core::uint gNum) const {
		return idx->GetEntry(gNum)->GetFlag(flag);
	}
	/**
	 * Sets or clears one raw index flag for one game.
	 *
	 * This rewrites only the game's @ref IndexEntry; it does not re-encode the
	 * movetext or stored tag data.  Duplicate-detection state is preserved.
	 * @p gNum must be a valid zero-based game number.
	 */
	scid::core::errorT setFlag(bool value, scid::core::uint flag, scid::core::uint gNum);
	/**
	 * Sets or clears one raw index flag for every game included in @p filter.
	 *
	 * This is an index-only bulk update.  Progress is not exposed by this
	 * public wrapper, so callers that need cancellation should batch their own
	 * work at a higher level.
	 */
	scid::core::errorT setFlags(bool value, scid::core::uint flag, const HFilter& filter);
	/**
	 * Toggles one raw index flag for one game.
	 *
	 * Equivalent to reading @ref getFlag() and writing the opposite value with
	 * @ref setFlag().
	 */
	scid::core::errorT invertFlag(scid::core::uint flag, scid::core::uint gNum);
	/**
	 * Toggles one raw index flag for every game included in @p filter.
	 */
	scid::core::errorT invertFlags(scid::core::uint flag, const HFilter& filter);

	/**
	 * Creates a named filter covering the current database.
	 *
	 * New filters start with every game included at value 1.  The returned ID
	 * is later passed to @ref getFilter(), @ref composeFilter(), or
	 * @ref deleteFilter().
	 */
	std::string newFilter();
	/**
	 * Deletes a filter created with @ref newFilter().
	 *
	 * The default filter and composed filter IDs are not owned by this function.
	 */
	void deleteFilter(const char* filterId);
	/**
	 * Resolves @p filterId to a filter handle.
	 *
	 * Recognised IDs include @c "dbfilter", @c "all", IDs returned by
	 * @ref newFilter(), and composed IDs in the form @c "+main+mask".  The
	 * returned handle compares equal to null when the ID cannot be resolved.
	 */
	HFilter getFilter(std::string_view filterId) const;
	/** Returns the database's default working filter. */
	HFilter defaultFilter() const { return HFilter(dbFilter); }
	/** Returns the number of games included in the default filter. */
	gamenumT defaultFilterCount() const { return dbFilter->Count(); }
	/** Returns the raw default-filter value for @p g. */
	scid::core::byte defaultFilterGet(gamenumT g) const { return dbFilter->Get(g); }
	/** Sets the raw default-filter value for @p g. */
	void defaultFilterSet(gamenumT g, scid::core::byte value) { dbFilter->Set(g, value); }
	/** Sets every default-filter entry to @p value. */
	void defaultFilterFill(scid::core::byte value) { dbFilter->Fill(value); }
	/** Returns a token that changes when cached database views should refresh. */
	uint64_t cacheInvalidationToken() const { return cacheInvalidationToken_; }

	/**
	 * Returns an ID for the intersection of two filters.
	 *
	 * A composed filter includes only games present in both components.  The
	 * main filter is the component mutated by non-const operations; the mask
	 * filter is read-only.  Composed IDs are lightweight strings, should not be
	 * deleted, and become invalid when either component is deleted.
	 *
	 * @param mainFilter valid identifier of the mutable main filter.
	 * @param maskFilter valid identifier of the read-only mask filter.
	 * @returns a composed filter ID, or an empty string when either component
	 *          cannot be resolved.
	 */
	std::string composeFilter(std::string_view mainFilter,
	                          std::string_view maskFilter) const;

	/**
	 * Splits @p filterId into its main and mask components.
	 *
	 * For a normal filter ID, the second component is empty.  For a composed
	 * filter, the first component is the main filter and the second is the
	 * mask.
	 */
	std::pair<std::string, std::string>
	getFilterComponents(std::string_view filterId) const;

	/**
	 * Returns cached whole-database statistics.
	 *
	 * The result is built from index metadata and is not affected by filters.
	 */
	const Stats& getStats() const;
	/**
	 * Returns move-tree statistics for the games visible in @p filter.
	 *
	 * For each included game, the filter value is interpreted as @c ply + 1.
	 * The move played at that ply is grouped into a @ref TreeNode, and the
	 * returned nodes are sorted by descending game count.
	 */
	std::vector<TreeNode> getTreeStat(const HFilter& filter) const;
	/**
	 * Returns how many index fields reference one stored name.
	 *
	 * Frequencies are computed lazily from the current index and cached until
	 * the database cache is invalidated by a modifying operation.  Player
	 * frequencies count both White and Black references; event, site, and
	 * round frequencies count their single corresponding field.
	 */
	scid::core::uint getNameFreq(nameT nt, idNumberT id) {
		if (nameFreq_[nt].size() == 0)
			nameFreq_ = getNameBase()->calcNameFreq(*idx);
		return nameFreq_[nt][id];
	}

	/**
	 * Computes the work that database compaction would be able to do.
	 *
	 * The four output counters are independent signals for a maintenance UI:
	 * @p n_deleted is the number of games carrying the delete flag,
	 * @p n_unused is the number of names that are no longer referenced by any
	 * non-deleted game, @p n_sparse counts live games whose stored offsets are
	 * out of physical order, and @p n_badNameId is the number of index entries
	 * that refer to invalid name IDs.
	 *
	 * This function only scans the index and namebase.  It does not rewrite
	 * files and it is safe to call before deciding whether a compaction is
	 * worth offering to the user.
	 */
	scid::core::errorT getCompactStat(unsigned long long* n_deleted,
	                      unsigned long long* n_unused,
	                      unsigned long long* n_sparse,
	                      unsigned long long* n_badNameId);
	/**
	 * Rewrites the database into a compact physical representation.
	 *
	 * Compaction copies every non-deleted game into a temporary database,
	 * carries across codec metadata, remaps the autoload game when present,
	 * drops unused names as a consequence of the rewrite, and replaces bad name
	 * references through the normal namebase repair path.  Deleted games are
	 * permanently removed.  Large non-PGN databases may also be reordered by
	 * stored-line, home-pawn, and material signatures so position searches have
	 * better locality; callers must not assume game numbers remain stable.
	 *
	 * On success the current database is closed, its files are replaced by the
	 * compacted files, and the database is reopened read/write.  Named filter
	 * handles and sort-cache registrations are recreated, but callers should
	 * treat filter contents and any cached game numbers as stale.  In-memory
	 * databases and codecs without filenames return
	 * @ref scid::core::ERROR_CodecUnsupFeat.
	 *
	 * If an error or cancellation happens while building the temporary
	 * database, the temporary files are removed and the original database is
	 * left in place.  Failures after file replacement can leave the database
	 * closed or partially replaced according to the filesystem operation that
	 * failed.
	 */
	scid::core::errorT compact(const Progress& progress);

	/**
	 * Retains a cached sort order for repeated filtered game-list queries.
	 *
	 * A sort criterion is a compact string of field/direction pairs.  Each
	 * field character is followed by @c + for ascending order or @c - for
	 * descending order; later pairs break ties from earlier pairs.  Supported
	 * fields include game number (@c N), date/year (@c d, @c y), event/site
	 * and country (@c e, @c s, @c c), round (@c n), players and ratings
	 * (@c w, @c b, @c W, @c B, @c R), result (@c r, @c 1, @c 5, @c 0),
	 * move count (@c m), ECO (@c o), annotation counts (@c C, @c V, @c A),
	 * deleted state (@c D), event date (@c E), and annotation rating (@c i).
	 *
	 * Retaining a cache is useful for views that repeatedly call
	 * @ref listGames() or @ref sortedPosition() with the same criteria.  The
	 * cache is reference-counted; each successful call must eventually be
	 * balanced by @ref releaseSortCache().
	 *
	 * @returns true when @p criteria is valid and the cache was retained.
	 */
	bool createSortCache(const char* criteria);

	/**
	 * Releases a retained sort cache.
	 *
	 * The cache matching @p criteria has its reference count decremented.
	 * During the same sweep, any cached sort whose reference count has reached
	 * zero is destroyed.  Releasing an unknown criterion is therefore harmless
	 * and still performs cache cleanup.
	 */
	void releaseSortCache(const char* criteria);

	/**
	 * Writes a page of filtered game numbers sorted by @p criteria.
	 *
	 * @p start and @p count describe a page in the sorted, filtered result
	 * set.  Only games included in @p filter are considered, and the zero-based
	 * game numbers written to @p destCont are suitable for @ref gameInfo(),
	 * @ref tagRoster(), and @ref loadGame().  The destination array must have
	 * room for at least @p count entries.
	 *
	 * A matching retained cache makes repeated calls cheap, but the function
	 * will also create an unretained cache on demand for one-off queries.
	 * Invalid criteria return zero rows.
	 *
	 * @returns the number of game numbers written.
	 */
	size_t listGames(const char* criteria, size_t start, size_t count,
	                 const HFilter& filter, gamenumT* destCont);

	/**
	 * Returns the sorted row of @p gameId within @p filter.
	 *
	 * The returned value is a zero-based row in the same ordering used by
	 * @ref listGames().  Games outside @p filter, unknown game IDs, and
	 * invalid criteria return @c INVALID_GAMEID.  As with @ref listGames(),
	 * this function can use a retained cache or create an unretained cache on
	 * demand.
	 */
	size_t sortedPosition(const char* criteria, const HFilter& filter,
	                      gamenumT gameId);

	/**
	 * Remaps name IDs in filtered index entries.
	 *
	 * This is the shared machinery behind bulk player, event, site, and round
	 * edits.  Every string in @p newNames is first added to the active
	 * namebase; the resulting IDs are then passed to @p fnInit so the caller
	 * can build whatever lookup table @p getID needs.  The filter is scanned
	 * after that setup phase.
	 *
	 * @p getID is called as @c getID(oldId, gameInfo) and must return the ID
	 * that should be stored for that name occurrence.  For @c NAME_PLAYER it
	 * is called separately for the Black and White fields of each game; for
	 * the other name types it is called once for the corresponding field.
	 * Returning the original ID leaves that occurrence unchanged.
	 *
	 * The operation rewrites only @ref IndexEntry records.  Newly added names
	 * remain in the namebase even if no filtered game ultimately uses them, or
	 * if a later error or cancellation stops the scan.
	 *
	 * @returns a pair of error code and number of games whose index entry was
	 *          rewritten.
	 */
	template <typename TInitFunc, typename TMapFunc>
	std::pair<scid::core::errorT, size_t>
	transformNames(nameT nt, HFilter hfilter, const Progress& progress,
	               const std::vector<std::string>& newNames, TInitFunc fnInit,
	               TMapFunc getID);

	/**
	 * Removes stored extra PGN tags from games included in @p hfilter.
	 *
	 * Only tags whose names appear in @p removeTags are removed.  Standard
	 * tags represented by the index and namebase are not stripped through this
	 * API; the function rewrites the encoded tag section of each changed game
	 * and leaves movetext bytes unchanged.
	 *
	 * The operation runs inside a database transaction and reports progress as
	 * it scans the filter.  Games already rewritten before a later codec error
	 * or user cancellation remain changed.
	 *
	 * @returns a pair of error code and number of games whose stored tag block
	 *          was rewritten.
	 */
	std::pair<scid::core::errorT, size_t>
	stripGames(HFilter hfilter, const Progress& progress,
	           std::vector<std::string_view> const& removeTags);

	/**
	 * Transfers duplicate-game state out of the database object.
	 *
	 * Duplicate data is an optional side table indexed by game number.  Each
	 * entry stores the duplicate game number plus one, or zero when the game
	 * has no known duplicate.  Extracting the table clears the database's
	 * ownership; callers become responsible for keeping it aligned with the
	 * index they computed it from.
	 */
	std::unique_ptr<gamenumT[]> extractDuplicates() {
		return std::move(duplicates_);
	}
	/**
	 * Installs duplicate-game state owned by the database object.
	 *
	 * The array, when non-null, must contain one entry for each current game in
	 * the index and use the @c duplicate + 1 encoding described by
	 * @ref extractDuplicates().
	 */
	void setDuplicates(std::unique_ptr<gamenumT[]> duplicates) {
		duplicates_ = std::move(duplicates);
	}
	/**
	 * Returns the raw duplicate marker for @p gNum.
	 *
	 * A zero return value means no duplicate is recorded.  Non-zero values are
	 * one greater than the duplicate game number.
	 */
	gamenumT getDuplicates(gamenumT gNum) const {
		return duplicates_ ? duplicates_[gNum] : 0;
	}

private:
	struct Storage;

	bool inUse; // true if the database is open (in use).
	Filter* dbFilter;
	std::unique_ptr<Storage> storage_;
	Index* idx;
	NameBase* nb_;
	fileModeT fileMode_; // Read-only, write-only, or both.
	std::vector<std::pair<std::string, Filter*>> filters_;
	mutable Filter all_filter_{0};
	mutable Stats* stats_;
	std::array<std::vector<int>, NUM_NAME_TYPES> nameFreq_;
	// For each game: idx of duplicate game + 1 (0 if there is no duplicate).
	std::unique_ptr<gamenumT[]> duplicates_;
	std::vector<std::pair<std::string, SortCache*>> sortCaches_;
	mutable std::unordered_map<idNumberT, scid::core::ratingT> peakEloCache_;
	scid::core::errorT err_open_ = scid::core::OK;
	uint64_t cacheInvalidationToken_ = 0;

private:
	friend class SearchPos;

	static GameInfo makeGameInfo_(const IndexEntry& ie);
	ByteBuffer gameData(const IndexEntry& ie) const;
	GameView gameView(const IndexEntry* ie) const;
		scid::core::errorT openHelper(std::string_view dbType, fileModeT mode,
		                  const char* filename, const Progress& progress = {});

	void clear();

	/**
	 * Starts the storage update protocol used by modifying operations.
	 *
	 * This prepares the active codec for changes and rejects writes when the
	 * database is read-only or was opened with a warning that prevents safe
	 * modification.  It is an internal guard, not a rollback guarantee; callers
	 * still rely on each public operation's documented failure behaviour.
	 */
	scid::core::errorT beginTransaction();

	/**
	 * Finishes a storage update and refreshes derived database state.
	 *
	 * The function flushes the active codec, advances cache invalidation,
	 * updates filters and sort caches, and clears derived statistics affected
	 * by the change.  Pass @c INVALID_GAMEID when more than one game may have
	 * changed or when the affected game is not known precisely.
	 *
	 * @returns @ref scid::core::OK, or the first codec/cache update error.
	 */
	scid::core::errorT endTransaction(gamenumT gameId = INVALID_GAMEID);

		scid::core::errorT importGameHelper(const scidBaseT* sourceBase, scid::core::uint gNum);
		scid::core::errorT saveGameData(IndexEntry const& ie, TagRoster const& tags,
		                     ByteBuffer const& data, gamenumT replaced);
		scid::core::errorT saveIndexEntry(IndexEntry const& ie, gamenumT replaced);
		std::pair<scid::core::errorT, idNumberT> addName(nameT nt, const char* name);

		SortCache* getSortCache(const char* criteria);

	template <typename TOper>
	std::pair<scid::core::errorT, size_t>
	transformIndex(HFilter hfilter, const Progress& progress, TOper entry_op) {
		if (auto errModify = beginTransaction())
			return {errModify, 0};

		auto res = transformIndex_(hfilter, progress, entry_op);
		auto err = endTransaction();
		res.first = (res.first == scid::core::OK) ? err : res.first;
		return res;
	}

	/**
	 * Applies an index-entry transform to every game included in @p hfilter.
	 *
	 * @p entry_op receives a mutable copy of each @ref IndexEntry and returns
	 * true when that copy should be persisted.  The helper reports progress
	 * while scanning and stops cooperatively when @p progress requests
	 * cancellation.  It does not call @c beginTransaction() or
	 * @c endTransaction(); callers choose whether the scan is a dry run or a
	 * modifying operation.
	 *
	 * @returns an error code plus the number of entries rewritten.
	 */
	template <typename TOper>
	std::pair<scid::core::errorT, size_t>
	transformIndex_(HFilter hfilter, const Progress& progress, TOper entry_op) {
		size_t nCorrections = 0;
		size_t iProg = 0;
		size_t totProg = hfilter->size();
		for (auto& gnum : hfilter) {
			if ((++iProg % 8192 == 0) && !progress.report(iProg, totProg))
				return std::make_pair(scid::core::ERROR_UserCancel, nCorrections);

			IndexEntry newIE = *getIndexEntry(gnum);
			if (!entry_op(newIE))
				continue;

				auto err = saveIndexEntry(newIE, gnum);
			if (err != scid::core::OK)
				return std::make_pair(err, nCorrections);

			++nCorrections;
		}
		return std::make_pair(scid::core::OK, nCorrections);
	}
};

template <typename TInitFunc, typename TMapFunc>
std::pair<scid::core::errorT, size_t>
scidBaseT::transformNames(nameT nt, HFilter hfilter, const Progress& progress,
                          const std::vector<std::string>& newNames,
                          TInitFunc initFunc, TMapFunc getNewID) {
	if (auto errModify = beginTransaction())
		return {errModify, 0};

	std::vector<idNumberT> nameIDs(newNames.size());
	auto it = nameIDs.begin();
	for (auto& name : newNames) {
			auto id = addName(nt, name.c_str());
		if (id.first != scid::core::OK) {
			endTransaction();
			return std::make_pair(id.first, size_t(0));
		}
		*it++ = id.second;
	}

	initFunc(nameIDs);

	auto res = transformIndex_(hfilter, progress, [&](IndexEntry& ie) {
		const IndexEntry& ie_const = ie;
		idNumberT oldID;
		idNumberT oldBlackID = 0;
		idNumberT newBlackID = 0;
		switch (nt) {
		case NAME_PLAYER:
			oldID = ie_const.GetWhite();
			oldBlackID = ie_const.GetBlack();
			newBlackID = getNewID(oldBlackID, makeGameInfo_(ie_const));
			break;
		case NAME_EVENT:
			oldID = ie_const.GetEvent();
			break;
		case NAME_SITE:
			oldID = ie_const.GetSite();
			break;
		default:
			ASSERT(nt == NAME_ROUND);
			oldID = ie_const.GetRound();
		}
		const auto newID = getNewID(oldID, makeGameInfo_(ie_const));
		if (oldID == newID && oldBlackID == newBlackID)
			return false;

		switch (nt) {
		case NAME_PLAYER:
			ie.SetWhite(newID);
			ie.SetBlack(newBlackID);
			break;
		case NAME_EVENT:
			ie.SetEvent(newID);
			break;
		case NAME_SITE:
			ie.SetSite(newID);
			break;
		default:
			ASSERT(nt == NAME_ROUND);
			ie.SetRound(newID);
		}
		return true;
	});

	auto err = endTransaction();
	res.first = (res.first == scid::core::OK) ? err : res.first;
	return res;
}

template <typename TRatingResolver>
std::pair<scid::core::errorT, scidBaseT::RatingUpdateStats>
scidBaseT::updatePlayerRatings(HFilter filter, const Progress& progress,
                               bool overwrite, bool saveRatings,
                               TRatingResolver ratingFor) {
	RatingUpdateStats stats;
	auto entry_op = [&](IndexEntry& ie) {
		const auto date = ie.GetDate();
		const auto whiteElo = (!overwrite && ie.GetWhiteElo() != 0)
		                          ? 0
		                          : ratingFor(ie.GetWhite(), date);
		const auto blackElo = (!overwrite && ie.GetBlackElo() != 0)
		                          ? 0
		                          : ratingFor(ie.GetBlack(), date);
		const auto changes = (whiteElo != 0 ? 1 : 0) + (blackElo != 0 ? 1 : 0);
		if (changes == 0)
			return false;

		stats.changedRatings += changes;
		stats.changedGames++;
		if (!saveRatings)
			return false;

		if (whiteElo != 0) {
			ie.SetWhiteElo(whiteElo);
			ie.SetWhiteRatingType(scid::core::RATING_Elo);
		}
		if (blackElo != 0) {
			ie.SetBlackElo(blackElo);
			ie.SetBlackRatingType(scid::core::RATING_Elo);
		}
		return true;
	};

	if (!saveRatings) {
		auto res = transformIndex_(filter, progress, entry_op);
		return {res.first, stats};
	}

	auto res = transformIndex(filter, progress, entry_op);
	return {res.first, stats};
}

} // namespace scid::database
#endif
