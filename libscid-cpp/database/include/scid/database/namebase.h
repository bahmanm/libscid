/*
 * Copyright (C) 2014-2017 Fulvio Benini

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

#ifndef SCID_NAMEBASE_H
#define SCID_NAMEBASE_H

#include "scid/database/game_id.h"
#include "scid/database/index.h"
#include "scid/database/indexentry.h"
#include "scid/database/misc.h"
#include <algorithm>
#include <array>
#include <limits>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

namespace scid::database {

/**
 * Name category stored in a database namebase.
 *
 * Identifiers are only meaningful together with their category.  For example,
 * player id 7 and event id 7 are handles into different name buckets.
 */
using nameT = unsigned;
enum {
	/** Player names, shared by White and Black fields. */
	NAME_PLAYER,
	/** Event names. */
	NAME_EVENT,
	/** Site or venue names. */
	NAME_SITE,
	/** Round labels. */
	NAME_ROUND,
	/** Number of persisted name categories. */
	NUM_NAME_TYPES,
	/** Sentinel returned when text cannot be resolved to a name type. */
	NAME_INVALID = 99
};

/**
 * String table for the database's player, event, site, and round names.
 *
 * A database stores names once and refers to them from @ref IndexEntry by
 * @c idNumberT.  @ref NameBase owns those strings and maintains a
 * compatibility-sorted lookup index for each @c nameT bucket.  Returned
 * @c const @c char* values point into the namebase and remain valid until the
 * namebase is cleared or destroyed.
 *
 * IDs are assigned densely within each bucket.  They are stable for the
 * lifetime of the namebase, but a numeric ID has no meaning without its
 * corresponding @c nameT.
 */
class NameBase {
	std::vector<std::unique_ptr<const char[]>> names_[NUM_NAME_TYPES];
	struct idxCmp {
		bool operator()(const char* str1, const char* str2) const {
			// *** Compatibility ***
			// Older code used a custom StrTree class with a peculiar sorting:
			// - the first char was interpreted as an unsigned char;
			// - the remaining part was compared with the function
			// strComapare(),
			//   which converts the chars to ints, and is not consistent with
			//   the standard function strcmp().
			// The old StrTree class did also have unpredictable behaviors when
			// fed with names not sorted according to that criteria, for example
			// it could create Namebase objects with duplicate entries.
			// ***
			if (*str1 == *str2)
				return strCompare(str1, str2) < 0;

			return static_cast<uint32_t>(*str1) < static_cast<uint32_t>(*str2);
		}
	};
	std::map<const char*, idNumberT, idxCmp> idx_[NUM_NAME_TYPES];

public:
	/**
	 * Adds @p name to one name bucket and returns its new identifier.
	 *
	 * This function does not check for an existing equal name.  Call
	 * @ref namebase_find_or_add() when importing ordinary game metadata and
	 * uniqueness is required.
	 *
	 * @param nt name bucket receiving the new string.
	 * @param name string to copy into the namebase.
	 * @param hint optional insertion hint for callers already positioned in
	 *             the sorted lookup map.
	 * @returns the identifier assigned to @p name within @p nt.
	 */
	idNumberT namebase_add(
	    nameT nt, std::string_view name,
	    std::map<const char*, idNumberT, idxCmp>::iterator* hint = nullptr) {
		ASSERT(IsValidNameType(nt));
		ASSERT(names_[nt].size() <= std::numeric_limits<idNumberT>::max());

		char* alloc = new char[name.size() + 1];
		std::copy_n(name.data(), name.size(), alloc);
		alloc[name.size()] = '\0';
		idNumberT newID = static_cast<idNumberT>(names_[nt].size());
		names_[nt].emplace_back(alloc);
		if (hint) {
			idx_[nt].emplace_hint(*hint, alloc, newID);
		} else {
			idx_[nt].emplace(alloc, newID);
		}
		return newID;
	}

	/**
	 * Returns the identifier for @p name, adding it when needed.
	 *
	 * The lookup is exact under the namebase's legacy-compatible ordering.
	 * The returned ID is valid only for the bucket @p nt.
	 */
	idNumberT namebase_find_or_add(nameT nt, const char* name) {
		ASSERT(IsValidNameType(nt));

		auto& nb = idx_[nt];
		auto it = nb.lower_bound(name);
		if (it != nb.end() && !nb.key_comp()(name, it->first))
			return it->second;

		return namebase_add(nt, name, &it);
	}

	/**
	 * Returns the number of names stored in @p nt.
	 */
	size_t namebase_size(nameT nt) const {
		ASSERT(IsValidNameType(nt));

		return names_[nt].size();
	}

	/**
	 * Inserts a name with a caller-specified identifier.
	 *
	 * This legacy loader path exists for older namebase files that already
	 * contain explicit IDs.  Prefer @ref namebase_add() or
	 * @ref namebase_find_or_add() for new code.
	 *
	 * If the name or ID already exists, the function returns false and the
	 * object should be discarded; sparse intermediate slots may also have been
	 * created while attempting the insert.
	 */
	bool insert(const char* name, size_t nameLen, nameT nt, idNumberT id) {
		if (id >= names_[nt].size())
			names_[nt].resize(id + size_t{1});

		if (names_[nt][id]) // A name with the same ID already exists
			return false;

		char* buf = new char[nameLen + 1];
		std::copy_n(name, nameLen, buf);
		buf[nameLen] = '\0';
		names_[nt][id].reset(buf);
		auto it = idx_[nt].emplace_hint(idx_[nt].end(), buf, id);
		return it->second == id; // Check that the name doesn't already exists
	}

	/**
	 * Clears all strings and lookup indexes.
	 */
	void Clear() { *this = NameBase(); }

	/**
	 * Returns the first identifiers whose names start with @p str.
	 *
	 * Matching follows the namebase sort order and is case-sensitive.  Results
	 * are returned in lookup order and stop once @p maxMatches have been found.
	 */
	std::vector<idNumberT> getFirstMatches(nameT nt, const char* str,
	                                       size_t maxMatches) const {
		ASSERT(IsValidNameType(nt) && str != NULL);

		std::vector<idNumberT> res;
		size_t len = strlen(str);
		for (auto it = idx_[nt].lower_bound(str);
		     it != idx_[nt].end() && res.size() < maxMatches; ++it) {
			const char* s = it->first;
			if (strlen(s) < len || !std::equal(str, str + len, s))
				break;
			res.emplace_back(it->second);
		}
		return res;
	}

	/**
	 * Returns the stored string for @p id in bucket @p nt.
	 *
	 * The returned pointer is owned by this namebase and remains valid until
	 * the namebase is cleared or destroyed.
	 */
	const char* GetName(nameT nt, idNumberT id) const {
		ASSERT(IsValidNameType(nt) && id < GetNumNames(nt));
		return names_[nt][id].get();
	}

	/**
	 * Returns the sorted lookup maps for every name bucket.
	 *
	 * Each map entry contains a borrowed string pointer and its identifier.
	 * The maps expose the legacy-compatible ordering used when serialising
	 * Scid 4 namebase files.
	 */
	const decltype(idx_)& getNames() const { return idx_; }

	/**
	 * Returns the first invalid identifier for @p nt.
	 *
	 * Because identifiers are dense, this is also the number of names in the
	 * bucket.
	 */
	idNumberT GetNumNames(nameT nt) const {
		ASSERT(IsValidNameType(nt));
		return static_cast<idNumberT>(names_[nt].size());
	}

	/**
	 * Finds an exact case-sensitive name.
	 *
	 * @param nt bucket to search.
	 * @param str full name text to find.
	 * @param[out] idPtr receives the identifier when the name exists.
	 * @returns @ref scid::core::OK, or
	 *          @ref scid::core::ERROR_NameNotFound when no exact match exists.
	 */
	scid::core::errorT FindExactName(nameT nt, const char* str, idNumberT* idPtr) const {
		ASSERT(IsValidNameType(nt) && str != NULL && idPtr != NULL);

		auto it = idx_[nt].find(str);
		if (it != idx_[nt].end()) {
			*idPtr = (*it).second;
			return scid::core::OK;
		}
		return scid::core::ERROR_NameNotFound;
	}

	/**
	 * Generates a four-character start hash for every name in @p nt.
	 *
	 * The resulting vector is indexed by @c idNumberT and is used by sort
	 * and lookup code that needs a compact, case-insensitive prefix key.
	 */
	std::vector<uint32_t> generateHashMap(nameT nt) const {
		std::vector<uint32_t> res(names_[nt].size());
		std::transform(
		    names_[nt].begin(), names_[nt].end(), res.begin(),
		    [](auto const& name) { return strStartHash(name.get()); });
		return res;
	}

	/**
	 * Counts how often each stored name is referenced by @p idx.
	 *
	 * The returned array is indexed first by @c nameT, then by
	 * @c idNumberT.  Player references count both White and Black fields.
	 */
	std::array<std::vector<int>, NUM_NAME_TYPES>
	calcNameFreq(Index const& idx) const {
		std::array<std::vector<int>, NUM_NAME_TYPES> resVec;
		for (nameT n = NAME_PLAYER; n < NUM_NAME_TYPES; n++) {
			resVec[n].resize(GetNumNames(n), 0);
		}
		for (gamenumT i = 0, n = idx.GetNumGames(); i < n; i++) {
			const IndexEntry* ie = idx.GetEntry(i);
			resVec[NAME_PLAYER][ie->GetWhite()] += 1;
			resVec[NAME_PLAYER][ie->GetBlack()] += 1;
			resVec[NAME_EVENT][ie->GetEvent()] += 1;
			resVec[NAME_SITE][ie->GetSite()] += 1;
			resVec[NAME_ROUND][ie->GetRound()] += 1;
		}
		return resVec;
	}

	/**
	 * Counts index references that do not resolve in this namebase.
	 *
	 * This is a consistency check for databases whose index and namebase files
	 * may have drifted apart or been repaired during load.
	 */
	size_t count_invalid_ids(Index const& idx) const {
		size_t n_invalid = 0;
		std::array<size_t, NUM_NAME_TYPES> maxID;
		for (auto n = nameT{}; n < NUM_NAME_TYPES; n++) {
			maxID[n] = GetNumNames(n);
		}
		for (gamenumT i = 0, n = idx.GetNumGames(); i < n; i++) {
			const IndexEntry* ie = idx.GetEntry(i);
			n_invalid += ie->GetWhite() < maxID[NAME_PLAYER] ? 0 : 1;
			n_invalid += ie->GetBlack() < maxID[NAME_PLAYER] ? 0 : 1;
			n_invalid += ie->GetEvent() < maxID[NAME_EVENT] ? 0 : 1;
			n_invalid += ie->GetSite() < maxID[NAME_SITE] ? 0 : 1;
			n_invalid += ie->GetRound() < maxID[NAME_ROUND] ? 0 : 1;
		}
		return n_invalid;
	}

	/**
	 * Returns true when @p nt is one of the persisted name buckets.
	 */
	static bool IsValidNameType(nameT nt) { return (nt < NUM_NAME_TYPES); }

	/**
	 * Resolves a textual name-type selector.
	 *
	 * The match is case-insensitive and ignores spaces.  Both abbreviations
	 * such as @c "pla" and longer command strings such as @c "player name" are
	 * accepted.
	 *
	 * @returns a valid @c nameT, or @c NAME_INVALID.
	 */
	static nameT NameTypeFromString(const char* str) {
		if (*str == '\0')
			return NAME_INVALID;
		if (strIsAlphaPrefix(str, "player"))
			return NAME_PLAYER;
		if (strIsAlphaPrefix(str, "event"))
			return NAME_EVENT;
		if (strIsAlphaPrefix(str, "site"))
			return NAME_SITE;
		if (strIsAlphaPrefix(str, "round"))
			return NAME_ROUND;
		if (strIsAlphaPrefix("player", str))
			return NAME_PLAYER;
		if (strIsAlphaPrefix("event", str))
			return NAME_EVENT;
		if (strIsAlphaPrefix("site", str))
			return NAME_SITE;
		if (strIsAlphaPrefix("round", str))
			return NAME_ROUND;
		return NAME_INVALID;
	}
};

/**
 * Borrowed string view of the PGN Seven Tag Roster names.
 *
 * @ref IndexEntry stores five of the seven standard PGN tags as identifiers
 * into @ref NameBase; event, site, round, white, and black.  @ref TagRoster
 * resolves those handles to string pointers when loading or exporting game
 * metadata, and maps strings back to identifiers when saving.
 */
struct TagRoster {
	/** PGN Event tag value. */
	const char* event;
	/** PGN Site tag value. */
	const char* site;
	/** PGN Round tag value. */
	const char* round;
	/** PGN White tag value. */
	const char* white;
	/** PGN Black tag value. */
	const char* black;

	/**
	 * Resolves an index entry's name identifiers through @p nb.
	 *
	 * The returned string pointers are owned by @p nb.
	 */
	template <typename TEntry>
	static TagRoster make(TEntry const& ie, NameBase const& nb) {
		TagRoster res;
		res.event = nb.GetName(NAME_EVENT, ie.GetEvent());
		res.site = nb.GetName(NAME_SITE, ie.GetSite());
		res.white = nb.GetName(NAME_PLAYER, ie.GetWhite());
		res.black = nb.GetName(NAME_PLAYER, ie.GetBlack());
		res.round = nb.GetName(NAME_ROUND, ie.GetRound());
		return res;
	}

	/**
	 * Writes this roster's strings into @p dest as name identifiers.
	 *
	 * @p getID is called as @c getID(nameT, const char*) and must return an
	 * @c {error, id} pair.  It may find existing names or add new ones,
	 * depending on the storage backend.  Mapping stops at the first error.
	 */
	template <typename TEntry, typename Fn>
	auto map(TEntry& dest, Fn getID) const {
		{
			auto [err, id] = getID(NAME_EVENT, event);
			if (err)
				return err;
			dest.SetEvent(id);
		}
		{
			auto [err, id] = getID(NAME_SITE, site);
			if (err)
				return err;
			dest.SetSite(id);
		}
		{
			auto [err, id] = getID(NAME_ROUND, round);
			if (err)
				return err;
			dest.SetRound(id);
		}
		{
			auto [err, id] = getID(NAME_PLAYER, white);
			if (err)
				return err;
			dest.SetWhite(id);
		}
		{
			auto [err, id] = getID(NAME_PLAYER, black);
			if (!err)
				dest.SetBlack(id);

			return err;
		}
	}
};


} // namespace scid::database
#endif // SCID_NAMEBASE_H
