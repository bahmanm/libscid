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

	/// Return the highest elo of the player (in the database's games)
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
	std::pair<scid::core::errorT, size_t>
	replaceGameDates(HFilter filter, const Progress& progress,
	                 scid::core::dateT oldDate, scid::core::dateT newDate);
	std::pair<scid::core::errorT, size_t>
	replaceGameEventDates(HFilter filter, const Progress& progress,
	                      scid::core::dateT oldDate,
	                      scid::core::dateT newDate);
	std::pair<scid::core::errorT, size_t>
	setPlayerRatings(HFilter filter, const Progress& progress, idNumberT player,
	                 scid::core::ratingT rating,
	                 scid::core::ratingTypeT ratingType);
	template <typename TRatingResolver>
	std::pair<scid::core::errorT, RatingUpdateStats> updatePlayerRatings(
	    HFilter filter, const Progress& progress, bool overwrite,
	    bool saveRatings, TRatingResolver ratingFor);
	scid::core::errorT searchBoard(const IndexEntry& ie,
	                               scid::core::Game& game,
	                               scid::core::Position* pos,
	                               scid::core::Position* posFlip,
	                               bool useVariations,
	                               bool possibleMatch,
	                               bool possibleFlippedMatch,
	                               gameExactMatchT searchType,
	                               scid::core::uint& ply) const;
	scid::core::errorT searchBoard(gamenumT gNum,
	                               scid::core::Game& game,
	                               scid::core::Position* pos,
	                               scid::core::Position* posFlip,
	                               bool useVariations,
	                               bool possibleMatch,
	                               bool possibleFlippedMatch,
	                               gameExactMatchT searchType,
	                               scid::core::uint& ply) const;
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
	 * Replaces @p filter with the games whose main line reaches @p pos.
	 *
	 * Matching values store the ply hint used by tree and game-list views:
	 * value 1 means the starting position, value 2 means after the first
	 * half-move, and so on.  The search first uses index-level prefilters
	 * where possible, then decodes candidate games as needed.
	 *
	 * @returns false when @p progress requests cancellation.
	 */
	bool setPositionSearchFilter(const scid::core::Position& pos,
	                             HFilter& filter,
	                             const Progress& progress) const;

	/**
	 * Imports all games included in @p filter from @p srcBase.
	 */
	scid::core::errorT importGames(const scidBaseT* srcBase, const HFilter& filter,
	                   const Progress& progress);
		scid::core::errorT importGames(std::string_view dbType, const char* filename,
		                   const Progress& progress, std::string& errorMsg);

	/**
	 * Add or replace a game into the database.
	 * @param game: core game data to store.
	 * @param scidFlags: database/application Scid flags for the game.
	 * @param replacedGameId: id of the game to replace.
	 *                        If >= numGames(), a new game will be added.
	 * @returns scid::core::OK if successful or an error code.
	 */
	scid::core::errorT saveGame(scid::core::Game const& game, const char* scidFlags,
	                gamenumT replacedGameId = INVALID_GAMEID);
	scid::core::errorT addGame(scid::core::Game const& game, const char* scidFlags) {
		return saveGame(game, scidFlags, INVALID_GAMEID);
	}

	bool getFlag(scid::core::uint flag, scid::core::uint gNum) const {
		return idx->GetEntry(gNum)->GetFlag(flag);
	}
	scid::core::errorT setFlag(bool value, scid::core::uint flag, scid::core::uint gNum);
	scid::core::errorT setFlags(bool value, scid::core::uint flag, const HFilter& filter);
	scid::core::errorT invertFlag(scid::core::uint flag, scid::core::uint gNum);
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
	scid::core::uint getNameFreq(nameT nt, idNumberT id) {
		if (nameFreq_[nt].size() == 0)
			nameFreq_ = getNameBase()->calcNameFreq(*idx);
		return nameFreq_[nt][id];
	}

	scid::core::errorT getCompactStat(unsigned long long* n_deleted,
	                      unsigned long long* n_unused,
	                      unsigned long long* n_sparse,
	                      unsigned long long* n_badNameId);
	scid::core::errorT compact(const Progress& progress);

	/**
	 * Increment the reference count of a SortCache object matching @e criteria.
	 * @param criteria: the list of fields by which games will be ordered.
	 *                  Each field should be followed by '+' to indicate an
	 *                  ascending order or by '-' for a descending order.
	 * @returns true on success
	 */
	bool createSortCache(const char* criteria);

	/**
	 * Decrement the reference count of the SortCache object matching @e
	 * criteria. Cached objects with refCount <= 0 are destroyed independently
	 * from the value of @e criteria.
	 * @param criteria: the list of fields by which games will be ordered.
	 *                  Each field should be followed by '+' to indicate an
	 *                  ascending order or by '-' for a descending order.
	 */
	void releaseSortCache(const char* criteria);

	/**
	 * Writes a page of filtered game numbers sorted by @p criteria.
	 *
	 * This function is faster when a matching sort cache has already been
	 * created with @ref createSortCache().
	 *
	 * @param criteria sort fields; each field is followed by @c + for
	 *                 ascending order or @c - for descending order.
	 * @param start zero-based row offset.
	 * @param count maximum number of rows to write.
	 * @param filter filter defining the visible game set.
	 * @param[out] destCont array receiving up to @p count game numbers.
	 * @returns the number of game numbers written.
	 */
	size_t listGames(const char* criteria, size_t start, size_t count,
	                 const HFilter& filter, gamenumT* destCont);

	/**
	 * Returns the sorted row of @p gameId within @p filter.
	 *
	 * Games outside the filter, unknown game IDs, and missing sort caches
	 * return @c INVALID_GAMEID.
	 */
	size_t sortedPosition(const char* criteria, const HFilter& filter,
	                      gamenumT gameId);

	/**
	 * Transform the names of the games included in @e hfilter.
	 * The function @e getID maps all the old idNumberT to the new idNumberT.
	 * It's invoked for each game and must accept as parameters a idNumberT and
	 * a const GameInfo&; must return the (eventually different) idNumberT.
	 * @param nt:       type of the names to be modified.
	 * @param hfilter:  HFilter containing the games to be transformed.
	 * @param progress: a Progress object used for GUI communications.
	 * @param newNames: optional vector of names to be added to the database.
	 * @param fnInit:   function that is invoked before beginning the
	 *                  transformation; must accept a vector that contains the
	 *                  idNumberTs of the names in @e newNames.
	 * @param getID:    function that maps the old idNumberTs to the new ones.
	 * @returns a std::pair containing scid::core::OK (or an error code) and the number of
	 * games modified.
	 */
	template <typename TInitFunc, typename TMapFunc>
	std::pair<scid::core::errorT, size_t>
	transformNames(nameT nt, HFilter hfilter, const Progress& progress,
	               const std::vector<std::string>& newNames, TInitFunc fnInit,
	               TMapFunc getID);

	/**
	 * Strip the games included in @e hfilter.
	 * @param hfilter: HFilter containing the games to be transformed.
	 * @param progress: a Progress object used for GUI communications.
	 * @param removeTags: extra PGN tags to remove from each matching game.
	 * @returns a std::pair containing scid::core::OK (or an error code) and the number of
	 * games modified.
	 */
	std::pair<scid::core::errorT, size_t>
	stripGames(HFilter hfilter, const Progress& progress,
	           std::vector<std::string_view> const& removeTags);

	std::unique_ptr<gamenumT[]> extractDuplicates() {
		return std::move(duplicates_);
	}
	void setDuplicates(std::unique_ptr<gamenumT[]> duplicates) {
		duplicates_ = std::move(duplicates);
	}
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

	/// This function must be called before modifying the games of the database.
	/// Currently this function do not guarantees that the database is not
	/// altered in case of errors.
	scid::core::errorT beginTransaction();

	/// Update caches and flush the database's files.
	/// This function must be called after changing one or more games.
	/// @param gameId: id of the modified game
	///                INVALID_GAMEID to update all games.
	/// @returns scid::core::OK if successful or an error code.
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
	 * Apply a transform operator to games' IndexEntry included in @e hfilter.
	 * The @p entry_op should accept a IndexEntry& parameter and return true when
	 * the IndexEntry was modified.
	 * @param hfilter:  HFilter containing the games to be transformed.
	 * @param progress: a Progress object used for GUI communications.
	 * @param entry_op: operator that will be applied to games' IndexEntry.
	 * @returns a std::pair containing scid::core::OK (or an error code) and the number of
	 * games modified.
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
