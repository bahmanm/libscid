/*
* Copyright (c) 1999-2002  Shane Hudson
* Copyright (c) 2006-2009  Pascal Georges
* Copyright (C) 2014-2016  Fulvio Benini

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

#ifndef SCID_INDEX_H
#define SCID_INDEX_H

#include "scid/database/common.h"
#include "scid/database/game_id.h"
#include "scid/database/indexentry.h"
#include <memory>

//////////////////////////////////////////////////////////////////////
//  Index:  Class Definition

namespace scid::database
{

    /**
     * In-memory table of database index entries.
     *
     * A Scid database keeps its index resident in memory so list views, searches,
     * and random game access do not need to scan the encoded game data file.  The
     * entries are stored in chunked storage by the implementation, so pointers
     * returned by @ref GetEntry() remain stable while more entries are appended.
     */
    class Index
    {
        private:
            struct Impl;
            std::unique_ptr<Impl> impl_;

        public:
            /** Creates an empty index. */
            Index();
            /** Releases all index storage. */
            ~Index();
            Index(const Index&) = delete;
            Index&
            operator=(const Index&) = delete;

            /** Clears every entry and resets validation counters. */
            void
            Close();

            /**
             * Returns the index entry for game number @p g.
             *
             * @p g must be less than @ref GetNumGames().  The returned pointer remains
             * valid until the entry is replaced, the index is closed, or the index is
             * destroyed.
             */
            const IndexEntry*
            GetEntry(gamenumT g) const;

            /**
             * Returns the number of invalid name handles found while opening the index.
             *
             * To save space, an @ref IndexEntry stores @c idNumberT handles to names
             * kept in @ref NameBase.  If the index and namebase files are out of sync
             * or corrupted, some handles may not resolve to strings.  The codec counts
             * those repairs or invalid references here so callers can report database
             * health without walking the index again.
             */
            int
            GetBadNameIdCount() const;

            /** Replaces the number of invalid name handles reported for this index. */
            void
            setBadNameIdCount(int count);

            /** Returns the number of games represented by this index. */
            gamenumT
            GetNumGames() const;

            /** Appends @p ie as the next game's index entry. */
            void
            addEntry(const IndexEntry& ie);

            /**
             * Replaces the entry for game number @p replaced.
             *
             * @p replaced must already exist in the index.
             */
            void
            replaceEntry(
                const IndexEntry& ie,
                gamenumT          replaced);

        private:
            void
            Init();
    };

} // namespace scid::database
#endif // #ifdef SCID_INDEX_H

//////////////////////////////////////////////////////////////////////
//  EOF: index.h
//////////////////////////////////////////////////////////////////////
