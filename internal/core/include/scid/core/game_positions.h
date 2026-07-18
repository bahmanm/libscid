/** @file
 * Position snapshots extracted from a game tree.
 */

#pragma once

#include "scid/core/game_cursor.h"
#include "scid/core/notation.h"
#include "scid/core/pgn/traversal.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace scid::core::gamepos
{

    /** A serialisable snapshot of one reachable position in a game.
     *
     * Game positions are produced in PGN traversal order: the main line is
     * interleaved with recursive annotation variations (RAVs), and the pair
     * @c RAVdepth / @c RAVnum identifies when the stream enters, leaves, or
     * switches between sibling variations.
     *
     * The field names mirror the legacy ScidUp representation.  Treat them as a
     * transport shape: the FEN and SAN fields describe the board reached at this
     * point, while the RAV fields describe where that point sits in the game tree.
     */
    struct GamePos
    {
            /** Variation nesting depth of this position.
             *
             * The main line has depth 0. A direct variation has depth 1, and nested
             * variations increase the depth further.
             */
            uint32_t RAVdepth;

            /** Sibling variation identifier at @c RAVdepth.
             *
             * Consumers can compare this with the previous position to decide whether a
             * variation continues or a new sibling variation has begun.
             */
            uint32_t RAVnum;

            /** Forsyth-Edwards Notation for the position after @c lastMoveSAN. */
            std::string FEN;

            /** Numeric Annotation Glyph codes attached to the move that reached this position. */
            std::vector<int> NAGs;

            /** Text annotation attached to this position or to the surrounding variation. */
            std::string comment;

            /** SAN of the move that reached this position, or empty for the game start. */
            std::string lastMoveSAN;
    };

    /** Append all position snapshots of a game to a caller-supplied container.
     *
     * The traversal follows PGN RAV semantics: a variation starts from the position
     * before the move immediately preceding the RAV, and nested RAVs are emitted
     * recursively. The resulting stream is intended for clients that need a flat
     * representation of a game tree while still being able to reconstruct
     * variation boundaries.
     *
     * To follow one variation from a given snapshot, keep reading while the next
     * snapshots have a greater @c GamePos::RAVdepth. The variation ends at the
     * first snapshot with a lower depth, an equal depth with a different
     * @c GamePos::RAVnum, or the end of @p dest.
     *
     * @code
     * auto positions = scid::core::gamepos::collectPositions(game);
     * for (const auto& pos : positions) {
     *     if (pos.RAVdepth == 0) {
     *         showMainlinePosition(pos.FEN, pos.lastMoveSAN);
     *     }
     * }
     * @endcode
     *
     * @tparam TCont Container type supporting @c emplace_back() and @c back().
     * @param game Game whose main line and variations are traversed.
     * @param dest Container to which snapshots are appended. Existing contents are
     *             preserved.
     */
    template <typename TCont>
    inline void
    collectPositions(
        const scid::core::Game& game,
        TCont&                  dest)
    {
        scid::core::GameCursor cursor(game);
        do
        {
            if (cursor.isAtVariationStart() && !cursor.isAtGameStart())
                continue;

            dest.emplace_back();
            auto& gamepos = dest.back();
            char  strBuf[256];
            auto  position = cursor.currentPosition();
            assert(position);
            position->PrintFEN(strBuf, sizeof(strBuf));
            gamepos.FEN = strBuf;
            gamepos.RAVdepth = cursor.variationDepth();
            gamepos.RAVnum = cursor.variationIndex();
            if (auto move = cursor.previousMove())
            {
                for (auto nag : move->metadata.nags)
                    gamepos.NAGs.push_back(scid::core::nagCode(nag));
                gamepos.comment = move->metadata.comment;
            }
            else if (auto variation = cursor.currentVariation())
            {
                gamepos.comment = variation->initialComment;
            }
            else
            {
                gamepos.comment = game.movetext().initialComment;
            }
            gamepos.lastMoveSAN = scid::core::notation::previousSan(game, cursor.location());

        } while (scid::core::pgn::nextLocation(cursor));
    }

    /** Return all position snapshots of a game.
     *
     * @param game Game whose main line and variations are traversed.
     * @returns A vector containing the same snapshots that the templated overload
     *          would append to an empty container.
     */
    inline std::vector<GamePos>
    collectPositions(const scid::core::Game& game)
    {
        std::vector<GamePos> res;
        collectPositions(game, res);
        return res;
    }

} // namespace scid::core::gamepos
