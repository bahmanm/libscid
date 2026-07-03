/*
 * Copyright (C) 1999-2000  Shane Hudson
 * Copyright (C) 2017  Fulvio Benini

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
 *
 */

#ifndef SCID_ECO_BOOK_H
#define SCID_ECO_BOOK_H

#include "scid/core/error.h"
#include "scid/eco/code.h"
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace scid::core
{
    class Position;
}

namespace scid::eco
{

    /** Error code type used by the ECO loader. */
    using Error = scid::core::errorT;

    /** Position type classified by an ECO book. */
    using Position = scid::core::Position;

    inline constexpr Error OK = scid::core::OK;
    inline constexpr Error ERROR_FileOpen = scid::core::ERROR_FileOpen;
    inline constexpr Error ERROR_Corrupt = scid::core::ERROR_Corrupt;

    /**
     * Exact-position ECO classifier loaded from a Scid ECO file.
     *
     * A book stores the final position of each ECO line together with its ECO code,
     * mnemonic name, and move text.  Lookup is exact: a position must match a
     * stored compact board, not merely share a prefix of the opening line.  The
     * implementation first indexes by position hash and then verifies the compact
     * board, so hash collisions do not change the result.
     *
     * Strings returned by lookup methods are views into memory owned by the book.
     * They remain valid until the book is destroyed or moved from.
     */
    class Book
    {
        public:
            /** One ECO-file line split into display-friendly fields. */
            struct Line
            {
                    /** ECO code text, including any Scid subcode extension. */
                    std::string_view code;
                    /** Mnemonic opening name without the surrounding brackets. */
                    std::string_view name;
                    /** Move text that reaches the classified position. */
                    std::string_view moves;
            };

        private:
            struct BookData
            {
                    std::unique_ptr<char[]> compactStr;
                    std::unique_ptr<char[]> comment;

                    BookData(
                        char* compact,
                        char* comm)
                        : compactStr(compact),
                          comment(comm)
                    {}
            };

            std::unordered_multimap<unsigned, BookData> pos_;
            std::vector<const char*>                    comments_;
            unsigned                                    lineCount_ = 0;
            unsigned leastMaterial_ = 32; // The smallest amount of material in any
                                          // position in the book. In the range 0..32.

        public:
            /** Result type returned by ECO book loading. */
            using LoadResult = std::expected<Book, Error>;

            /**
             * Loads a Scid ECO file.
             *
             * The file is a text list of lines in the form:
             *
             * @code
             * C50a "Italian Game" 1.e4 e5 2.Nf3 Nc6 3.Bc4 *
             * @endcode
             *
             * Blank text and comment lines beginning with @c # are skipped while
             * looking for the next ECO code.  Each move sequence is parsed from the
             * standard chess starting position.  If the file cannot be opened, the
             * result contains @c ERROR_FileOpen.  If a line is malformed or contains
             * an illegal move, the result contains @c ERROR_Corrupt.
             *
             * @returns the loaded book, or the load error.
             */
            static LoadResult
            load(const std::filesystem::path& path);

            /**
             * Returns the ECO code and mnemonic name for @p position.
             *
             * The returned view has the form @c "C50a [Italian Game]".  It is empty
             * when the exact position is not present in the book.
             */
            std::string_view
            findEcoString(const Position& position) const;

            /**
             * Returns the compact ECO code for @p position.
             *
             * @returns the corresponding ECO code, or @c ECO_None when the exact
             *          position is not present in the book.
             */
            Code
            findEco(const Position& position) const;

            /**
             * Returns loaded ECO lines whose code starts with @p ecoPrefix.
             *
             * Matching is byte-exact and case-sensitive.  An empty prefix returns every
             * loaded line.  The returned views are owned by the book and are reported
             * in file order.
             */
            std::vector<Line>
            linesWithPrefix(std::string_view ecoPrefix) const;

            /**
             * Returns the loader's source-line counter.
             *
             * The value is useful for diagnostics, but it follows the loader's scanning
             * rules rather than promising an editor-style physical line count.
             */
            unsigned
            lineCount() const
            {
                return lineCount_;
            }
            /**
             * Returns the fewest pieces on the board among indexed ECO positions.
             *
             * Empty books report 32, the maximum material count used as the initial
             * value.
             */
            unsigned
            fewestPieces() const
            {
                return leastMaterial_;
            }
            /** Returns the number of indexed ECO positions. */
            size_t
            size() const
            {
                return pos_.size();
            }
    };

} // namespace scid::eco

#endif // SCID_ECO_BOOK_H
