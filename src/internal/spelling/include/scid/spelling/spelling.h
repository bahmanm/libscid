/*
# Copyright (C) 2015 Fulvio Benini

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

#ifndef SCID_SPELLING_SPELLING_H
#define SCID_SPELLING_SPELLING_H

#include "scid/core/date.h"
#include "scid/database/namebase.h"
#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * @file
 * Public API for loading Scid spelling files and using them to canonicalise
 * database names.
 *
 * A spelling file is a curated name authority.  It stores canonical player,
 * event, site, and round names, optional aliases for those names, general
 * text-normalisation rules, and optional player metadata such as title,
 * country, peak rating, biography lines, and rating history.
 */

namespace scid::spelling
{

    /**
     * Applies general spelling-file substitutions to a name.
     *
     * A spelling file may define prefix, infix, and suffix rules for each database
     * name type.  These rules are independent of the alias index used by
     * @ref SpellChecker::find(); they are a lower-level normalisation pass that
     * rewrites common textual variants before a caller stores or compares names.
     *
     * Rules use the same quoted-pair syntax as Scid spelling files:
     *
     * @code
     * %Prefix "II " "2. "
     * %Infix "3rd " "3. "
     * %Suffix "(Italy)" "ITA"
     * @endcode
     *
     * With those rules, the text
     * @code
     * II champ 3rd II 3rd (Italy) (Italy)
     * @endcode
     * becomes
     * @code
     * 2. champ 3. II 3. (Italy) ITA
     * @endcode
     */
    class NameNormalizer
    {
            typedef std::vector<std::pair<std::string, std::string>> Cont;
            Cont                                                     prefix_;
            Cont                                                     infix_;
            Cont                                                     suffix_;

        public:
            /**
             * Rewrites @p name in place using the loaded prefix, infix, and suffix
             * corrections.
             *
             * Prefix and suffix corrections stop after the first matching rule in
             * their respective lists.  Infix corrections are applied repeatedly for
             * each matching rule.  Matching is byte-exact and case-sensitive.
             *
             * @param name The name to update.  Must not be null.
             * @return The number of substitutions applied.
             */
            size_t
            normalize(std::string* name) const;

            /**
             * Adds a prefix correction parsed from a spelling-file directive.
             *
             * @p s must contain two quoted strings, for example
             * @code
             * %Prefix "wrong prefix" "correct prefix"
             * @endcode
             *
             * @return @ref scid::core::OK on success, or
             *         @ref scid::core::ERROR_CorruptData if the directive cannot be
             *         parsed.
             */
            scid::core::errorT
            addPrefix(const char* s);
            /**
             * Adds an infix correction parsed from a spelling-file directive.
             *
             * Infix rules replace every occurrence of the source fragment during
             * @ref normalize().
             *
             * @return @ref scid::core::OK on success, or
             *         @ref scid::core::ERROR_CorruptData if the directive cannot be
             *         parsed.
             */
            scid::core::errorT
            addInfix(const char* s);
            /**
             * Adds a suffix correction parsed from a spelling-file directive.
             *
             * @return @ref scid::core::OK on success, or
             *         @ref scid::core::ERROR_CorruptData if the directive cannot be
             *         parsed.
             */
            scid::core::errorT
            addSuffix(const char* s);

        private:
            scid::core::errorT
            add(Cont&       v,
                const char* s);
    };

    /**
     * Rating history for one player loaded from a spelling file.
     *
     * Spelling files encode rating history as one or more yearly groups:
     *
     * @code
     * %Elo YEAR:RATING,RATING,... YEAR:RATING,RATING,...
     * @endcode
     *
     * The number of ratings per year follows the publication cadence used by the
     * source data for that period.  @ref getElo() maps a game date to the rating
     * slot selected by Scid's historical cadence rules, returning zero when the
     * file has no usable value for the requested date.
     */
    class PlayerElo
    {
            std::vector<std::pair<uint16_t, scid::core::ratingT>> elo_;

        public:
            /**
             * Appends rating data parsed from a spelling-file @c %Elo line.
             *
             * Unknown ratings are encoded as @c ? and stored as zero.  Malformed input
             * stops parsing at the first invalid field; existing data already appended
             * to the object is kept.
             */
            void
            addEloData(const char* str);

            /**
             * Returns the player's rating for @p date.
             *
             * A return value of zero means that no rating was available for that date,
             * either because the year is absent, the period is outside the loaded data,
             * or the spelling file explicitly recorded an unknown rating.
             */
            scid::core::ratingT
            getElo(scid::core::dateT date) const;

#ifdef SCID_SPELLING_VALIDATE
            /**
             * Validates that the loaded rating history is sorted and has the expected
             * number of rating slots for each year.
             *
             * @return An empty string when the data is valid; otherwise a short
             *         diagnostic suitable for a validation log.
             */
            std::string
            isValid() const;
#endif
    };

    /**
     * Metadata attached to a canonical player name.
     *
     * The primary player-info payload is the comment that follows a player entry
     * in the spelling file.  Scid convention stores title, gender marker, country,
     * peak rating, birth date, and optional death date in that comment:
     *
     * @code
     * Polgar, Judit           #GM+W HUN [2735] 1976
     * @endcode
     *
     * Additional free-form biography rows may follow the player entry as
     * @c %Bio lines.  Those rows are retrieved through @ref SpellChecker::getPlayerInfo().
     */
    class PlayerInfo
    {
            const char*              comment_;
            std::vector<const char*> bio_;

            friend class SpellingLoader;
            friend class SpellChecker;

        public:
            /**
             * Creates player metadata backed by @p s.
             *
             * Most callers receive @ref PlayerInfo from @ref SpellChecker rather than
             * constructing it directly.  The pointer is stored as-is; callers that do
             * construct @ref PlayerInfo directly must keep the string alive for the
             * lifetime of the object.
             */
            PlayerInfo(const char* s) : comment_(s)
            {}
            /**
             * Returns the first recognised chess title in the player comment.
             *
             * Examples include @c GM, @c IM, @c WGM, and @c WIM.  The empty string is
             * returned when the comment has no recognised title prefix.
             */
            const char*
            getTitle() const;
            /**
             * Returns the last three characters of the country field in the player
             * comment.
             *
             * This matches Scid's historical comment format, where the country follows
             * the title field.  The empty string is returned when no three-character
             * country code can be found.
             */
            std::string
            getLastCountry() const;
            /**
             * Returns the birth date encoded in the player comment.
             *
             * @return The parsed date, or @ref scid::core::ZERO_DATE when the comment
             *         does not contain a birth date.
             */
            scid::core::dateT
            getBirthdate() const;
            /**
             * Returns the death date encoded in the player comment.
             *
             * @return The parsed date, or @ref scid::core::ZERO_DATE when the comment
             *         does not contain a death date.
             */
            scid::core::dateT
            getDeathdate() const;
            /**
             * Returns the peak rating encoded in square brackets in the player comment.
             *
             * @return The rating value, or zero when no rating field is present.
             */
            scid::core::ratingT
            getPeakRating() const;
            /**
             * Returns the raw player comment.
             *
             * The returned pointer is never null.  It points into storage owned by the
             * containing @ref SpellChecker when the object came from a loaded spelling
             * file, and remains valid for that spell checker's lifetime.
             */
            const char*
            getComment() const;
    };

    /**
     * Loaded spelling-file index for canonical name lookup and player metadata.
     *
     * @ref SpellChecker owns the strings loaded from a Scid spelling file and
     * builds per-name-type indexes over their aliases.  It is designed for the
     * database spellcheck workflow: ask for corrections to a player, event, site,
     * or round name; apply optional general normalisation rules; and, for players,
     * retrieve title, country, biography, and rating information.
     *
     * Returned @c const @c char* values and @ref PlayerInfo / @ref PlayerElo
     * pointers are views into this object.  Keep the @ref SpellChecker alive while
     * using them.
     *
     * When @c SCID_SPELLING_VALIDATE is enabled at build time, loading also writes
     * a validation log beside the spelling file.
     */
    class SpellChecker
    {
            struct Idx
            {
                    std::string alias;
                    int32_t     idx = 0;

                    Idx();
                    Idx(const std::string& a,
                        int32_t            i);
                    bool
                    operator<(const Idx& b) const;
                    bool
                    operator<(const std::string& b) const;
            };
            typedef std::vector<Idx>::const_iterator IdxIt;

            struct ConstructionToken
            {
                private:
                    ConstructionToken() = default;
                    friend class SpellChecker;
            };

            NameNormalizer           general_[scid::database::NUM_NAME_TYPES];
            std::string              excludeChars_[scid::database::NUM_NAME_TYPES];
            std::vector<Idx>         idx_[scid::database::NUM_NAME_TYPES];
            std::vector<const char*> names_[scid::database::NUM_NAME_TYPES];
            std::vector<PlayerInfo>  pInfo_;
            std::vector<PlayerElo>   pElo_;
            std::deque<std::string>  strings_;

            friend class SpellingLoader;

        public:
            explicit SpellChecker(ConstructionToken)
            {}

            /**
             * Loads a spelling file into a new spell checker.
             *
             * @param filename Path to the spelling file.  Must not be null.
             * @param progress Progress reporter called while reading large files.  If
             *        it reports cancellation, loading fails with
             *        @ref scid::core::ERROR_UserCancel.
             *
             * @return A pair containing @ref scid::core::OK and the new object on
             *         success.  On failure the error describes the load problem and the
             *         pointer is null.
             */
            static std::pair<
                scid::core::errorT,
                std::unique_ptr<SpellChecker>>
            create(
                const char*                     filename,
                const scid::database::Progress& progress);

            /**
             * Finds canonical names that match @p name.
             *
             * The query is first transformed by removing the characters excluded by the
             * current spelling-file section for @p nt.  If that transformed query is an
             * exact alias, the result contains only the corresponding canonical name.
             * Otherwise the result contains canonical names whose transformed aliases
             * start with the query, up to @p nMaxRes unique names.
             *
             * Matching is byte-exact and case-sensitive after excluded-character
             * removal.  Accents and letter case are significant unless the spelling
             * file provides explicit aliases for those variants.
             *
             * Player lookup has one additional convenience rule: if no match is found,
             * the text after the last space is moved to the front and lookup is tried
             * again, allowing names entered as "Judit Polgar" to match a canonical
             * surname-first spelling.
             *
             * @param nt Name category to search.
             * @param name User-provided or database-provided name.  Must not be null.
             * @param nMaxRes Maximum number of unique canonical names to return.
             * @return Canonical names owned by this spell checker, in index order.
             */
            std::vector<const char*>
            find(
                const scid::database::nameT& nt,
                const char*                  name,
                scid::core::uint             nMaxRes = 10) const;

            /**
             * Returns the general normalisation rules loaded for @p nt.
             *
             * These are the @c %Prefix, @c %Infix, and @c %Suffix rules from the
             * spelling-file section.  They are useful when a caller wants to apply the
             * same broad cleanup rules without performing alias lookup.  The returned
             * normalizer is always present; if the spelling file had no rules for
             * @p nt, calling @ref NameNormalizer::normalize() simply applies no
             * substitutions.
             */
            const NameNormalizer&
            getGeneralCorrections(const scid::database::nameT& nt) const;

            /**
             * Returns metadata for an unambiguous player name.
             *
             * Lookup uses the same player matching rules as @ref find().  If the query
             * resolves to exactly one canonical player, the returned object exposes the
             * comment fields for title, country, rating, and dates.  If @p bio is not
             * null, it is replaced with that player's free-form biography lines.
             *
             * Ambiguous prefix matches are treated as a failed lookup because there is
             * no single player record to return.
             *
             * @param name Player name or alias.  Must not be null.
             * @param bio Optional destination for biography lines owned by this spell
             *        checker.
             * @return Player metadata owned by this spell checker, or null when the
             *         name is absent or ambiguous.  On failure @p bio is left unchanged.
             */
            const PlayerInfo*
            getPlayerInfo(
                const char*               name,
                std::vector<const char*>* bio = 0) const;

            /**
             * Returns rating history for an unambiguous player name.
             *
             * @return A rating-history object owned by this spell checker, or null when
             *         no Elo data was loaded, the player is absent, or the name is
             *         ambiguous.
             */
            const PlayerElo*
            getPlayerElo(const char* name) const;

            /**
             * Reports whether the loaded spelling file contained any player Elo data.
             *
             * This is a coarse file-level check.  Individual players may still have no
             * ratings even when this returns true.
             */
            bool
            hasEloData() const;

            /**
             * Returns the number of canonical names loaded for @p nt.
             *
             * Aliases are not counted; this is the size of the canonical-name table for
             * the requested database name category.
             */
            size_t
            numCorrectNames(const scid::database::nameT& nt) const;

        private:
            SpellChecker(const SpellChecker&) = delete;
            SpellChecker&
            operator=(const SpellChecker&) = delete;

            scid::core::errorT
            read(
                const char*                     filename,
                const scid::database::Progress& progress);

            const char*
            storeString(const char* s);

            std::string
            normalizeAndTransform(
                const scid::database::nameT& nt,
                const char*                  s) const;

            std::pair<
                IdxIt,
                IdxIt>
            idxFind(
                const scid::database::nameT& nt,
                const char*                  prefix) const;

            std::pair<
                IdxIt,
                IdxIt>
            idxFindPlayer(const char* prefix) const;

            IdxIt
            idxFindPlayerUnambiguous(const char* name) const;

            class SpellingValidate;
    };

} // namespace scid::spelling

#endif
