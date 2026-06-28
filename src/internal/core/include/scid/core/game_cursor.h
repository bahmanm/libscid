#pragma once

/** @file
 * Read-only traversal over a game's movetext tree.
 */

#include "scid/core/game.h"
#include "scid/core/movetext_location.h"
#include "scid/core/position.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace scid::core
{

    /**
     * A read-only cursor over a game's mainline and recursive variations.
     *
     * The cursor points between moves in the current line. At any location,
     * @ref previousMove() is the move immediately before the cursor and
     * @ref nextMove() is the move immediately after it. Variations are attached to
     * the next move, so entering a variation from a line start enters an alternate
     * line for that move.
     */
    class GameCursor
    {
        private:
            struct ParentFrame
            {
                    const MoveSequence* line = nullptr;
                    std::size_t nextIndex = 0;
                    std::size_t variationIndex = 0;
            };

        public:
            /**
             * Creates a cursor at the start of the game's mainline.
             */
            explicit GameCursor(const Game& game);

            /**
             * Returns the move immediately before the cursor.
             *
             * @returns nullptr when the cursor is at the start of the current line.
             */
            const Move*
            previousMove() const;

            /**
             * Returns the move immediately after the cursor.
             *
             * @returns nullptr when the cursor is at the end of the current line.
             */
            const Move*
            nextMove() const;

            /**
             * Returns the variation that owns the current line.
             *
             * @returns nullptr when the cursor is on the mainline.
             */
            const Variation*
            currentVariation() const;

            /**
             * Returns the moves needed to reach the current cursor position.
             *
             * For a variation, this list contains the parent-line moves before each
             * variation branch and then the moves inside the active variation path.
             */
            std::vector<const Move*>
            movesToCursor() const;

            /**
             * Replays the moves to the cursor and returns the current board position.
             *
             * @returns the resulting position, or std::nullopt if any stored move is
             * illegal from its preceding position.
             */
            std::optional<scid::core::Position>
            currentPosition() const;

            /**
             * Returns the ply count from the game start to the cursor.
             */
            std::size_t
            ply() const;

            /**
             * Returns the number of child variations attached to the next move.
             */
            std::size_t
            variationCount() const;

            /**
             * Returns the nesting depth of the current variation line.
             */
            std::size_t
            variationDepth() const;

            /**
             * Returns the zero-based sibling index of the current variation.
             *
             * The mainline reports zero.
             */
            std::size_t
            variationIndex() const;

            /**
             * Reports whether the cursor is before the first move of the current line.
             */
            bool
            isAtLineStart() const;

            /**
             * Reports whether the cursor is after the last move of the current line.
             */
            bool
            isAtLineEnd() const;

            /**
             * Reports whether the cursor is at the start of the current variation.
             */
            bool
            isAtVariationStart() const;

            /**
             * Reports whether the cursor is at the end of the current variation.
             */
            bool
            isAtVariationEnd() const;

            /**
             * Reports whether the cursor is at the start of the mainline.
             */
            bool
            isAtGameStart() const;

            /**
             * Reports whether the cursor is at the end of the mainline.
             */
            bool
            isAtGameEnd() const;

            /**
             * Reports whether the cursor is inside a variation with no moves.
             */
            bool
            isAtEmptyVariation() const;

            /**
             * Advances the cursor over the next move in the current line.
             *
             * @returns false when the cursor is already at line end.
             */
            bool
            next();

            /**
             * Moves the cursor back over the previous move in the current line.
             *
             * @returns false when the cursor is already at line start.
             */
            bool
            previous();

            /**
             * Enters a child variation of the next move.
             *
             * @param index the zero-based child variation index.
             * @returns false when there is no next move or the index is out of range.
             */
            bool
            enterVariation(std::size_t index);

            /**
             * Exits the current variation and returns to the parent line.
             *
             * The cursor is restored to the parent-line position from which the
             * variation was entered.
             *
             * @returns false when the cursor is already on the mainline.
             */
            bool
            exitVariation();

            /**
             * Moves the cursor to the start of the mainline.
             */
            void
            toStart();

            /**
             * Moves the cursor to the end of the mainline.
             */
            void
            toEnd();

            /**
             * Moves the cursor to a mainline ply.
             *
             * @returns false when @p ply is beyond the mainline length; the cursor is
             * left unchanged in that case.
             */
            bool
            toPly(std::size_t ply);

            /**
             * Captures the current cursor position as a restorable location.
             */
            MovetextLocation
            location() const;

            /**
             * Restores the cursor to a previously captured location in this game.
             *
             * @returns false when the location no longer exists in the current
             * movetext tree.
             */
            bool
            restore(MovetextLocation location);

        private:
            const MoveSequence&
            currentLine() const;

            const Game& game_;
            const MoveSequence* currentLine_ = nullptr;
            std::size_t nextIndex_ = 0;
            std::vector<ParentFrame> parents_;
    };

} // namespace scid::core
