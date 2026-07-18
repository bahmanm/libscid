/*
 * Copyright (C) 2018  Fulvio Benini
 *
 * This file is part of SCID (Shane's Chess Information Database).
 *
 * SCID is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * SCID is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SCID. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/** @file
 * PGN parser entry points.
 *
 * The parser reads PGN text into the core Game model.  It accepts complete
 * games as well as movetext fragments used by editing commands, and can append
 * parsed moves at an existing MovetextLocation.
 */

#pragma once

#include "scid/core/game.h"
#include "scid/core/movetext_location.h"
#include <cstddef>
#include <string>

namespace scid::core::pgn
{

    /** Aggregated parse progress and diagnostics.
     *
     * ParseLog is cumulative: callers may reuse the same instance across multiple
     * parseGame() calls to collect total bytes, line counts, game counts, and
     * warning/error text.  Diagnostics are formatted for humans and include the
     * game and line number tracked by the parser.
     */
    struct ParseLog
    {
            /** Human-readable warnings and errors produced while parsing. */
            std::string log;
            /** Number of input bytes consumed across parsed games. */
            unsigned long long n_bytes = 0;
            /** Number of input lines processed across parsed games. */
            unsigned long long n_lines = 0;
            /** Number of parse attempts recorded as games. */
            unsigned long long n_games = 0;
    };

    /** Parses PGN text into @p game.
     *
     * The game is not cleared before parsing.  This lets callers append movetext to
     * an existing game, but callers that want replacement semantics should clear or
     * construct the Game themselves first.  Parsing may succeed while still adding
     * warnings to @p log.
     *
     * @param input memory containing the PGN text.
     * @param inputLen number of bytes available at @p input.
     * @param game destination game.
     * @param log cumulative parse diagnostics and progress counters.
     * @returns true when the input was parsed without discarding a trailing portion
     * after a fatal parse error.
     */
    bool
    parseGame(
        const char*       input,
        size_t            inputLen,
        scid::core::Game& game,
        ParseLog&         log);
    /** Parses PGN text into @p game starting at @p location.
     *
     * This overload is used by editors that insert moves, comments, NAGs, and
     * variations at the current cursor location.  On success, @p location is
     * updated to the parser's final movetext location.
     *
     * @param input memory containing the PGN text.
     * @param inputLen number of bytes available at @p input.
     * @param game destination game.
     * @param location movetext location where parsing starts and where the final
     * parser location is stored.
     * @param log cumulative parse diagnostics and progress counters.
     * @returns true when the input was parsed without discarding a trailing portion
     * after a fatal parse error.
     */
    bool
    parseGame(
        const char*                   input,
        size_t                        inputLen,
        scid::core::Game&             game,
        scid::core::MovetextLocation& location,
        ParseLog&                     log);

} // namespace scid::core::pgn
