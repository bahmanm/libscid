/** @file
 * Small fixed-size square collections used by move generation.
 */

#pragma once

#include "scid/core/board.h"

#include <cassert>

namespace scid::core
{

    /** Maximum SquareList capacity: 64 board squares plus NULL_SQUARE. */
    constexpr uint MAX_SQUARELIST = 65;

    /** Fixed-capacity list of squares.
     *
     * SquareList preserves insertion order except when Remove() is used, which
     * compacts by moving the last square into the removed slot.  It performs no
     * allocation and is intended for short-lived attack and move-generation lists.
     */
    class SquareList
    {
            uint    ListSize;
            squareT Squares[MAX_SQUARELIST];

        public:
            /** Constructs an empty list. */
            SquareList()
            {
                ListSize = 0;
            }

            /** Clears the list.  Kept for legacy call sites. */
            void
            Init()
            {
                ListSize = 0;
            }
            /** Clears the list. */
            void
            Clear()
            {
                ListSize = 0;
            }
            /** Appends @p sq.  The caller must not exceed MAX_SQUARELIST entries. */
            void
            Add(squareT sq)
            {
                Squares[ListSize] = sq;
                ListSize++;
            }
            /** Returns the number of stored squares. */
            uint
            Size()
            {
                return ListSize;
            }

            /** Returns the square at @p index. */
            squareT
            Get(uint index)
            {
                assert(index < ListSize);
                return Squares[index];
            }

            /** Returns true when @p sq is present in the list. */
            bool
            Contains(squareT sq)
            {
                for (uint i = 0; i < ListSize; i++)
                {
                    if (Squares[i] == sq)
                    {
                        return true;
                    }
                }
                return false;
            }

            /** Removes @p index by replacing it with the current last element. */
            void
            Remove(uint index)
            {
                assert(index < ListSize);
                ListSize--;
                if (index != ListSize)
                {
                    Squares[index] = Squares[ListSize];
                }
            }
    };

    /** 64-bit set of on-board squares.
     *
     * SquareSet is a compact membership filter for legal move generation.  Only
     * A1..H8 are valid inputs; sentinel squares are rejected by assertions.
     */
    class SquareSet
    {
            unsigned long long bits_ = 0;

        public:
            /** Adds @p sq to the set. */
            void
            Add(squareT sq)
            {
                assert(sq < 64);
                bits_ |= 1ull << sq;
            }

            /** Returns true when @p sq is present in the set. */
            bool
            Contains(squareT sq)
            {
                assert(sq < 64);
                return (bits_ & (1ull << sq)) != 0;
            }
    };

} // namespace scid::core
