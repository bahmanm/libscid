#pragma once

/** @file
 * PGN-order traversal helpers for game cursors.
 */

#include "scid/core/game_cursor.h"

namespace scid::core::pgn
{

    /**
     * Advances a cursor to the next location in PGN traversal order.
     *
     * The traversal enters child variations before continuing along the parent
     * line, matching PGN Recursive Annotation Variation order.
     *
     * @returns false when the cursor is already at the final traversable location.
     */
    bool
    nextLocation(GameCursor& cursor);

    /**
     * Moves a cursor to a one-based PGN traversal location.
     *
     * Location one is the start of the mainline.
     *
     * @returns false when @p location is beyond the traversal range.
     */
    bool
    seekLocation(GameCursor& cursor, unsigned location);

    /**
     * Returns the one-based PGN traversal location of a cursor.
     */
    unsigned
    locationOf(const GameCursor& cursor);

    /**
     * Returns the PGN display offset for a cursor.
     *
     * When the cursor is at the start of a variation, the offset is reported as
     * the nearest parent location that has printable move text before the
     * variation.
     */
    unsigned
    offsetOf(const GameCursor& cursor);

} // namespace scid::core::pgn
