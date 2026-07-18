#pragma once

/** @file
 * Notation helpers for positions and moves at movetext locations.
 */

#include "scid/core/move.h"
#include "scid/core/primitives.h"
#include <cstddef>
#include <optional>
#include <string>

namespace scid::core
{
    class Position;
} // namespace scid::core

namespace scid::core
{
    class Game;
    class MovetextLocation;

    namespace notation
    {

        /**
         * Builds a UCI "position" command for the position at a game location.
         *
         * Standard-start games are emitted as @c position @c startpos followed by the
         * moves from the current effective start. Non-standard starts and positions
         * after null moves are emitted from an explicit FEN.
         */
        std::string
        currentPositionUci(
            const Game&             game,
            const MovetextLocation& location);

        /**
         * Returns the previous move at a location in UCI long algebraic notation.
         *
         * @returns an empty string when there is no previous move.
         */
        std::string
        previousMoveUci(
            const Game&             game,
            const MovetextLocation& location);

        /**
         * Returns the next move at a location in UCI long algebraic notation.
         *
         * @returns an empty string when there is no next move.
         */
        std::string
        nextMoveUci(
            const Game&             game,
            const MovetextLocation& location);

        /**
         * Returns the SAN text of the previous move at a location.
         *
         * Stored SAN is preferred when available; otherwise it is generated from the
         * preceding position.
         *
         * @returns an empty string when there is no previous move or the stored move
         * cannot be replayed legally.
         */
        std::string
        previousSan(
            const Game&             game,
            const MovetextLocation& location);

        /**
         * Returns the SAN text of the next move at a location.
         *
         * Stored SAN is preferred when available; otherwise it is generated from the
         * current position.
         *
         * @returns an empty string when there is no next move or the stored move
         * cannot be replayed legally.
         */
        std::string
        nextSan(
            const Game&             game,
            const MovetextLocation& location);

        /**
         * Formats the first @p plyCount mainline half-moves as compact SAN text.
         */
        std::string
        partialMoveList(
            const Game& game,
            std::size_t plyCount);

    } // namespace notation
} // namespace scid::core

namespace scid::core
{

    /** Maximum storage size for legacy fixed-size SAN buffers. */
    inline constexpr std::size_t SAN_STRING_SIZE = 10;

    /** Legacy fixed-size SAN buffer type. */
    using sanStringT = char[SAN_STRING_SIZE];

    /** Controls whether generated SAN includes check and mate suffix testing. */
    using sanFlagT = byte;

    /** Generate SAN without checking for check or mate suffixes. */
    inline constexpr sanFlagT SAN_NO_CHECKTEST = 0;

    /** Generate SAN with check suffix testing. */
    inline constexpr sanFlagT SAN_CHECKTEST = 1;

    /** Generate SAN with mate suffix testing. */
    inline constexpr sanFlagT SAN_MATETEST = 2;

    /** Maximum storage size for legacy fixed-size UCI move buffers. */
    inline constexpr std::size_t UCI_MOVE_STRING_SIZE = 6;

} // namespace scid::core
