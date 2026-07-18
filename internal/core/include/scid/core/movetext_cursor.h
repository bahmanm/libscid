#pragma once

/** @file
 * Mutable traversal and editing over a game's movetext tree.
 */

#include "scid/core/game.h"
#include "scid/core/movetext_location.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace scid::core
{

    /**
     * A mutable cursor over a game's mainline and recursive variations.
     *
     * MovetextCursor uses the same between-moves location model as
     * @ref GameCursor, but it can edit the current line, the current variation,
     * and the move adjacent to the cursor. Editing operations update the cursor to
     * the most useful location for continued editing; callers can persist that
     * state with @ref location().
     */
    class MovetextCursor
    {
        private:
            struct ParentFrame
            {
                    MoveSequence* line = nullptr;
                    std::size_t   nextIndex = 0;
                    std::size_t   variationIndex = 0;
            };

        public:
            /**
             * Creates a cursor at the start of the game's mainline.
             */
            explicit MovetextCursor(Game& game);

            /**
             * Returns the mutable move immediately before the cursor.
             *
             * @returns nullptr when the cursor is at the start of the current line.
             */
            Move*
            previousMove();

            /**
             * Returns the move immediately before the cursor.
             *
             * @returns nullptr when the cursor is at the start of the current line.
             */
            const Move*
            previousMove() const;

            /**
             * Returns the mutable move immediately after the cursor.
             *
             * @returns nullptr when the cursor is at the end of the current line.
             */
            Move*
            nextMove();

            /**
             * Returns the move immediately after the cursor.
             *
             * @returns nullptr when the cursor is at the end of the current line.
             */
            const Move*
            nextMove() const;

            /**
             * Returns the mutable variation that owns the current line.
             *
             * @returns nullptr when the cursor is on the mainline.
             */
            Variation*
            currentVariation();

            /**
             * Returns the variation that owns the current line.
             *
             * @returns nullptr when the cursor is on the mainline.
             */
            const Variation*
            currentVariation() const;

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
            restore(const MovetextLocation& location);

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
             * Adds a move at the cursor and advances over it.
             *
             * If the cursor is not at the end of the current line, the existing
             * continuation from the cursor onwards is discarded before the move is
             * appended.
             *
             * @returns the newly inserted move.
             */
            Move&
            addMove(MoveSpec spec);

            /**
             * Adds a child variation to the next move and enters it.
             *
             * @param initialComment the comment placed before the variation's first
             * move.
             * @returns the newly inserted variation, or nullptr when there is no next
             * move to branch from.
             */
            Variation*
            addVariation(std::string_view initialComment = {});

            /**
             * Replaces comments and NAGs on the previous move.
             *
             * @returns false when there is no previous move.
             */
            bool
            setPreviousMoveMetadata(MoveMetadata metadata);

            /**
             * Sets cached SAN text on the previous move.
             *
             * @returns false when there is no previous move.
             */
            bool
            setPreviousMoveSan(std::string_view san);

            /**
             * Sets cached SAN text on the next move.
             *
             * @returns false when there is no next move.
             */
            bool
            setNextMoveSan(std::string_view san);

            /**
             * Sets the initial comment of the current variation.
             *
             * @returns false when the cursor is on the mainline.
             */
            bool
            setCurrentVariationInitialComment(std::string_view comment);

            /**
             * Sets the comment associated with the current cursor position.
             *
             * At a line start this updates the game's initial comment or the current
             * variation's initial comment. Elsewhere it updates the previous move's
             * comment.
             */
            bool
            setComment(std::string_view comment);

            /**
             * Adds or replaces a NAG on the previous move.
             *
             * Move-quality NAGs replace the existing move-quality NAG, and position
             * evaluation NAGs replace the existing position evaluation NAG. Other NAGs
             * are appended up to the per-move limit.
             *
             * @returns false when the move already has too many NAGs.
             */
            bool
            addPreviousMoveNag(Nag nag);

            /**
             * Removes the first move-quality or position-evaluation NAG from the
             * previous move.
             *
             * @param moveNag true removes a move-quality NAG; false removes a position
             * evaluation NAG.
             */
            bool
            removePreviousMoveNag(bool moveNag);

            /**
             * Removes all NAGs from the previous move.
             */
            void
            clearPreviousMoveNags();

            /**
             * Moves the current variation to the first sibling variation slot.
             *
             * @returns false when the cursor is on the mainline or the current
             * variation no longer exists.
             */
            bool
            promoteVariationToFirst();

            /**
             * Promotes the current variation into its parent line.
             *
             * The previous mainline continuation becomes the first child variation of
             * the promoted line's first move. The cursor exits the variation and points
             * into the promoted line at the corresponding ply.
             *
             * @returns false when the cursor is on the mainline or the current
             * variation no longer exists.
             */
            bool
            promoteVariationToMainline();

            /**
             * Deletes the current variation and exits to its parent line.
             *
             * @returns false when the cursor is on the mainline or the current
             * variation no longer exists.
             */
            bool
            deleteVariation();

            /**
             * Deletes all moves from the cursor to the end of the current line.
             */
            void
            truncate();

            /**
             * Replaces the game mainline with the suffix from the cursor onwards.
             *
             * The cursor is moved to the start of the new mainline. This is used when
             * callers turn the current board position into the new start position.
             */
            void
            truncateBeforeCursor();

        private:
            MoveSequence&
            currentLine();
            const MoveSequence&
            currentLine() const;

            Game&                    game_;
            MoveSequence*            currentLine_ = nullptr;
            std::size_t              nextIndex_ = 0;
            std::vector<ParentFrame> parents_;
    };

} // namespace scid::core
