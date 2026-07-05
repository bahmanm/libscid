/*
# Copyright (C) 2016-2018 Fulvio Benini

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

#ifndef SCID_HFILTER_H
#define SCID_HFILTER_H

#include "scid/database/common.h"
#include "scid/database/game_id.h"
#include <algorithm>
#include <iterator>
#include <memory>

/**
 * @file
 * Database game filters and filtered game-number iteration.
 */
namespace scid::database
{

    /**
     * Dense per-game filter storage.
     *
     * A filter stores one byte for each game in a database.  A value of zero means
     * the game is excluded.  A non-zero value means the game is included and may
     * also carry a position hint: value 1 means the start position, value 2 means
     * the position after White's first move, and so on.  Position searches use that
     * hint to reopen matching games at the relevant ply.
     *
     * The all-included state is represented lazily: when every game has value 1,
     * @c data() is null and no byte array is allocated.  Calling @c Set() or
     * @c Fill() with any other value materialises storage.
     */
    class Filter
    {
            std::unique_ptr<scid::core::byte[]> data_;     // The actual filter data.
            gamenumT                            size_;     // Number of values in filter.
            gamenumT                            nonzero_;  // Number of nonzero values in filter.
            size_t                              capacity_; // Number of values allocated for data_.

        public:
            /** Creates a filter of @p size games, all included at value 1. */
            explicit Filter(gamenumT size)
                : size_(size),
                  nonzero_(size),
                  capacity_(0)
            {}

            /** Resets the filter to @p size games, all included at value 1. */
            void
            Init(gamenumT size)
            {
                data_ = nullptr;
                nonzero_ = size_ = size;
            }

            /**
             * Returns the allocated byte array, or null for the lazy all-included state.
             */
            scid::core::byte*
            data()
            {
                return data_.get();
            }

            /** Returns the number of included games. */
            gamenumT
            Count() const
            {
                return nonzero_;
            }

            /** Returns the number of games represented by the filter. */
            gamenumT
            Size() const
            {
                return size_;
            }

            /**
             * Changes the number of games represented by the filter.
             *
             * When storage is lazy, resizing preserves the all-included state.  When
             * storage is materialised, shrinking drops trailing values and growing
             * appends excluded games unless the existing filter already included every
             * game, in which case the new games are also included.
             */
            void
            Resize(gamenumT size)
            {
                if (!data_)
                {
                    nonzero_ = size;
                }
                else if (size < size_)
                {
                    auto* data = data_.get();
                    nonzero_ = size - static_cast<gamenumT>(std::count(data, data + size, 0));
                }
                else if (size > size_)
                {
                    if (size > capacity_)
                    {
                        auto tmp(std::move(data_));
                        allocate(size);
                        std::copy_n(tmp.get(), size_, data_.get());
                    }
                    scid::core::byte val = 0;
                    if (Count() == Size())
                    {
                        val = 1;
                        nonzero_ = size;
                    }
                    auto* data = data_.get();
                    std::fill(data + size_, data + size, val);
                }
                size_ = size;
            }

            /**
             * Returns the raw filter value for game @p index.
             *
             * A null backing array is interpreted as value 1 for every game.
             */
            scid::core::byte
            Get(gamenumT index) const
            {
                ASSERT(index < Size());
                return data_ ? data_[index] : 1;
            }

            /**
             * Sets the raw filter value for game @p index.
             *
             * Setting a value different from 1 materialises the backing array when the
             * filter is currently in the lazy all-included state.
             */
            void
            Set(gamenumT         index,
                scid::core::byte value)
            {
                ASSERT(index < Size());
                if (data_)
                {
                    if (value == 0)
                    {
                        if (data_[index] != 0)
                            --nonzero_;
                    }
                    else if (data_[index] == 0)
                    {
                        ++nonzero_;
                    }
                    data_[index] = value;
                }
                else if (value != 1)
                {
                    allocate(size_);
                    auto* data = data_.get();
                    std::fill(data, data + size_, 1);
                    data[index] = value;
                    if (value == 0)
                        --nonzero_;
                }
            }

            /**
             * Sets every game to @p value.
             *
             * Filling with 1 releases storage and returns to the lazy all-included
             * representation.
             */
            void
            Fill(scid::core::byte value)
            {
                if (value == 1)
                {
                    data_ = nullptr;
                    nonzero_ = size_;
                }
                else
                {
                    if (!data_)
                    {
                        allocate(size_);
                    }
                    auto* data = data_.get();
                    std::fill(data, data + size_, value);
                    nonzero_ = (value == 0) ? 0 : size_;
                }
            }

        private:
            void
            allocate(size_t size)
            {
                auto capacity = (size | 63) + 1;
                data_ = std::make_unique<scid::core::byte[]>(capacity);
                capacity_ = capacity;
            }
    };

    /**
     * Handle-like view over a main filter, optionally intersected with a mask.
     *
     * @ref HFilter is the type most database APIs accept.  It deliberately feels
     * like a pointer to a sparse map of included games:
     *
     * @code
     * for (gamenumT gnum : *filter) {
     *     auto value = filter->get(gnum);
     * }
     * @endcode
     *
     * The visible set is the main filter when no mask is present, or the
     * intersection of main and mask when a mask is present.  Mutating operations
     * always update the main filter only.  With a mask, a game can be present in
     * the main filter but still invisible because the mask excludes it; for
     * included games, @c get() returns the mask value.
     *
     * The stored byte value is one greater than the ply hint used by callers:
     * zero means excluded, one means included at the start position, and N means
     * included with a hint of N - 1 half-moves.
     */
    class HFilter
    {
            Filter*       main_;
            const Filter* mask_;

        public:
            /**
             * Forward iterator over included or excluded game numbers.
             *
             * The iterator yields only game numbers.  Use @c HFilter::get() if the
             * associated filter value or ply hint is needed.
             */
            class const_iterator
            {
                    gamenumT       gnum_;
                    gamenumT       end_;
                    const HFilter* hfilter_;
                    bool           inFilter_;

                public:
                    typedef std::forward_iterator_tag iterator_category;
                    typedef std::ptrdiff_t            difference_type;
                    typedef gamenumT                  value_type;
                    typedef const gamenumT*           pointer;
                    typedef const gamenumT&           reference;

                    /** Creates an iterator positioned at the first matching game. */
                    const_iterator(
                        gamenumT       gnum,
                        gamenumT       end,
                        const HFilter* hfilter,
                        bool           inFilter = true)
                        : gnum_(gnum),
                          end_(end),
                          hfilter_(hfilter),
                          inFilter_(inFilter)
                    {
                        ASSERT(hfilter != 0);
                        if (gnum_ != end_)
                        {
                            bool included = (hfilter_->get(gnum_) != 0);
                            if (included != inFilter_)
                                operator++();
                        }
                    }

                    /** Returns the current game number. */
                    reference
                    operator*() const
                    {
                        return gnum_;
                    }

                    /** Advances to the next game with the requested inclusion state. */
                    const_iterator&
                    operator++()
                    {
                        while (++gnum_ != end_)
                        {
                            bool included = (hfilter_->get(gnum_) != 0);
                            if (included == inFilter_)
                                break;
                        }
                        return *this;
                    }

                    /** Returns true when the iterators point to different positions. */
                    bool
                    operator!=(const const_iterator& b) const
                    {
                        return gnum_ != b.gnum_ || hfilter_ != b.hfilter_;
                    }
                    /** Returns true when the iterators point to the same position. */
                    bool
                    operator==(const const_iterator& b) const
                    {
                        return !operator!=(b);
                    }
            };

            /** Returns an iterator over the first visible included game. */
            const_iterator
            begin() const
            {
                return const_iterator(0, main_->Size(), this);
            }
            /** Returns the end sentinel for included-game iteration. */
            const_iterator
            end() const
            {
                return const_iterator(main_->Size(), main_->Size(), this);
            }
            /** Returns an iterator over the first visible excluded game. */
            const_iterator
            beginInverted() const
            {
                return const_iterator(0, main_->Size(), this, false);
            }
            /** Returns the end sentinel for excluded-game iteration. */
            const_iterator
            endInverted() const
            {
                return const_iterator(main_->Size(), main_->Size(), this, false);
            }
            /** Returns the number of visible excluded games. */
            size_t
            sizeInverted() const
            {
                return main_->Size() - size();
            }

        public: // Pointer interface
            /** Compares the main filter pointer with @p b. */
            bool
            operator==(const Filter* b) const
            {
                return main_ == b;
            }
            /** Compares the main filter pointer with @p b. */
            bool
            operator!=(const Filter* b) const
            {
                return main_ != b;
            }
            /** Returns this handle, supporting pointer-like call sites. */
            HFilter*
            operator->()
            {
                return this;
            }
            /** Returns this handle, supporting pointer-like call sites. */
            const HFilter*
            operator->() const
            {
                return this;
            }
            /** Returns this handle, supporting range-for over @c *filter. */
            HFilter&
            operator*()
            {
                return *this;
            }
            /** Returns this handle, supporting range-for over @c *filter. */
            const HFilter&
            operator*() const
            {
                return *this;
            }

        public:
            /**
             * Creates a filter handle.
             *
             * @p main must remain alive for the lifetime of the handle.  @p mask, when
             * supplied, must remain alive too and is treated as read-only.
             */
            explicit HFilter(
                Filter*       main,
                const Filter* mask = 0)
                : main_(main),
                  mask_(mask)
            {}

            /** Returns the mutable main filter. */
            Filter*
            mainFilter() const
            {
                return main_;
            }
            /** Returns the optional read-only mask filter. */
            const Filter*
            maskFilter() const
            {
                return mask_;
            }

            /** Excludes every game from the main filter. */
            void
            clear()
            {
                return main_->Fill(0);
            }
            /** Excludes @p gnum from the main filter. */
            void
            erase(gamenumT gnum)
            {
                return main_->Set(gnum, 0);
            }
            /**
             * Includes @p gnum in the main filter with a zero-based ply hint.
             *
             * The stored raw value is @p ply + 1.
             */
            void
            insert_or_assign(
                gamenumT gnum,
                uint8_t  ply)
            {
                return main_->Set(gnum, ply + 1);
            }
            /** Returns the number of visible included games. */
            gamenumT
            size() const
            {
                if (!mask_ || mask_->Count() == mask_->Size())
                    return main_->Count();
                if (main_->Count() == main_->Size())
                    return mask_->Count();
                return static_cast<gamenumT>(std::distance(begin(), end()));
            }

            /** Returns the number of games included in the main filter, ignoring any mask. */
            size_t
            mainSize() const
            {
                return main_->Count();
            }

            /**
             * Includes every game in the main filter at the start-position value.
             */
            void
            includeAll()
            {
                return main_->Fill(1);
            }

            /**
             * Returns the visible raw filter value for @p gnum.
             *
             * Zero means excluded.  A non-zero value means included and encodes
             * @c ply + 1.  When a mask exists and the main filter includes the game,
             * the mask's value is returned.
             */
            scid::core::byte
            get(gamenumT gnum) const
            {
                scid::core::byte res = main_->Get(gnum);
                if (res != 0 && mask_ != 0)
                    res = mask_->Get(gnum);

                return res;
            }

            /**
             * Sets the raw value for @p gnum in the main filter.
             *
             * Prefer @c insert_or_assign() when working with a zero-based ply hint.
             */
            void
            set(gamenumT         gnum,
                scid::core::byte value)
            {
                return main_->Set(gnum, value);
            }
    };

    /**
     * Range helper for iterating over games excluded by a filter.
     *
     * This is used by OR-style searches, where the search only needs to inspect
     * games that are not already included.
     */
    class HFilterInverted
    {
            const HFilter& hfilter_;

        public:
            /** Creates an inverted range over @p hfilter. */
            explicit HFilterInverted(const HFilter& hfilter) : hfilter_(hfilter)
            {
                ASSERT(hfilter != 0);
            }
            /** Returns an iterator over the first excluded game. */
            HFilter::const_iterator
            begin() const
            {
                return hfilter_.beginInverted();
            }
            /** Returns the end sentinel for excluded-game iteration. */
            HFilter::const_iterator
            end() const
            {
                return hfilter_.endInverted();
            }
            /** Returns the number of excluded games. */
            size_t
            size() const
            {
                return hfilter_.sizeInverted();
            }
    };

} // namespace scid::database
#endif
