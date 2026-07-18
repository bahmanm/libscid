#pragma once

/** @file
 * Stable locations inside a game's movetext tree.
 */

#include <cstddef>
#include <vector>

namespace scid::core
{

    class GameCursor;
    class MovetextCursor;

    /**
     * A restorable cursor position inside a game's movetext tree.
     *
     * A location is expressed as a path through nested variations plus the
     * zero-based index of the next move in the current line. It identifies the
     * same boundary between moves used by GameCursor and MovetextCursor:
     * the previous move is immediately before @ref nextIndex(), and the next move
     * is immediately at @ref nextIndex().
     */
    class MovetextLocation
    {
        public:
            /**
             * One descent from a parent line into a child variation.
             */
            struct Step
            {
                    bool
                    operator==(const Step&) const = default;

                    /** The move in the parent line that owns the child variation. */
                    std::size_t nextIndex = 0;
                    /** The zero-based child variation index on that parent move. */
                    std::size_t variationIndex = 0;
            };

            /**
             * Creates the location at the start of the mainline.
             */
            MovetextLocation() = default;

            bool
            operator==(const MovetextLocation&) const = default;

            /**
             * Returns the variation path from the mainline to the current line.
             */
            const std::vector<Step>&
            path() const
            {
                return path_;
            }

            /**
             * Returns the index of the next move in the current line.
             */
            std::size_t
            nextIndex() const
            {
                return nextIndex_;
            }


        private:
            MovetextLocation(
                std::vector<Step> path,
                std::size_t       nextIndex);

            std::vector<Step> path_;
            std::size_t       nextIndex_ = 0;

            friend class GameCursor;
            friend class MovetextCursor;
    };

} // namespace scid::core
