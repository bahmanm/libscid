/*
* Copyright (C) 1999 Shane Hudson
* Copyright (C) 2015-2020 Fulvio Benini

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

#pragma once

#include "scid/core/fullmove.h"
#include "scid/core/game_result.h"
#include "scid/database/common.h"
#include "scid/database/game_id.h"

namespace scid::database
{

    /**
     * Aggregate statistics for one candidate move in a database tree view.
     *
     * Tree nodes are produced by @c scidBaseT::getTreeStat() from a filter whose
     * values encode the ply reached in each game.  Each included game contributes
     * the move played at that ply, and games with the same move are folded into
     * one node.  The node therefore represents "from this filtered position, this
     * move was played N times", plus result, rating, and year summaries for those
     * games.
     */
    struct TreeNode
    {
            /** Sum of White ratings for games where both players have ratings. */
            unsigned long long eloWhiteSum = 0; // Sum of white Elos.
            /** Sum of Black ratings for games where both players have ratings. */
            unsigned long long eloBlackSum = 0; // Sum of bLack Elos.
            /** Sum of non-zero game years. */
            unsigned long long yearSum = 0; // Sum of years.
            /**
             * Result frequencies.
             *
             * Entry 0 stores the total number of games for this move.  Entries for
             * decisive/drawn results use the ordinary Scid result codes.
             */
            gamenumT freq[scid::core::NUM_RESULT_TYPES] = {}; // freq[0] is the total count.
            /** Number of games that contributed to @c eloWhiteSum and @c eloBlackSum. */
            gamenumT eloCount = 0; // Count of games with an Elo.
            /** Number of games that contributed to @c yearSum. */
            gamenumT yearCount = 0; // Count of games with year != 0.
            /** Move represented by this tree node. */
            scid::core::FullMove move;

        public:
            /** Creates an empty aggregate for @p m. */
            explicit TreeNode(
                scid::core::FullMove m)
                : move(m)
            {}

            /**
             * Adds one game's outcome metadata to this move aggregate.
             *
             * Rating sums are updated only when both players have positive ratings.
             * Year sums are updated only for non-zero years.
             */
            void
            add(
                scid::core::resultT result,
                int eloW,
                int eloB,
                unsigned year)
            {
                static_assert(scid::core::RESULT_None == 0);
                freq[0]++; // total count of games
                if (result != scid::core::RESULT_None)
                {
                    freq[result]++;
                }
                if (eloW > 0 && eloB > 0)
                {
                    ++eloCount;
                    eloWhiteSum += eloW;
                    eloBlackSum += eloB;
                }
                if (year > 0)
                {
                    yearSum += year;
                    ++yearCount;
                }
            }

            /**
             * Returns White's score for this move as tenths of a percent.
             *
             * The range is 0..1000, where 500 means 50.0%.  Games with no result are
             * excluded from the score denominator.  When no decisive/drawn results are
             * present, the neutral value 500 is returned.
             */
            int
            score() const
            {
                auto n = freq[scid::core::RESULT_White] + freq[scid::core::RESULT_Draw] +
                         freq[scid::core::RESULT_Black];
                auto res = 1000ull * freq[scid::core::RESULT_White] +
                           500ull * freq[scid::core::RESULT_Draw];
                return n ? static_cast<int>(res / n) : 500;
            }

            /**
             * Returns the move's rating performance from the mover's perspective.
             *
             * The calculation uses the FIDE fractional-score conversion table and the
             * average rating of the opposing side.  It returns zero when no game has
             * ratings for both players.
             */
            double
            eloPerformance() const
            {
                if (eloCount == 0)
                    return 0;

                int score = (this->score() + 5) / 10;
                auto eloOpp = eloBlackSum;
                if (move.getColor() != scid::core::WHITE)
                {
                    score = 100 - score;
                    eloOpp = eloWhiteSum;
                }
                return 1.0 * eloOpp / eloCount + FIDE_ratingTable[score];
            }

            /**
             * Returns the average rating of the side that played @c move.
             */
            double
            avgElo() const
            {
                if (eloCount == 0)
                    return 0;

                auto elo = (move.getColor() == scid::core::WHITE) ? eloWhiteSum : eloBlackSum;
                return 1.0 * elo / eloCount;
            }

            /** Returns the average non-zero game year, or zero when none are known. */
            double
            avgYear() const
            {
                return yearCount ? 1.0 * yearSum / yearCount : 0;
            }

            /** Returns the draw percentage among all games represented by this node. */
            double
            percDraws() const
            {
                return freq[0] ? 100.0 * freq[scid::core::RESULT_Draw] / freq[0] : 0;
            }

            /** Returns a comparator that orders more frequent moves first. */
            static auto
            cmp_ngames_desc()
            {
                return [](auto const& a, auto const& b) { return a.freq[0] > b.freq[0]; };
            }

        private:
            // FIDE table 8.1a of conversion from fractional score (from 0 to 1 with
            // 0.01 increments) into rating differences.
            static constexpr short FIDE_ratingTable[] = {
                -800, -677, -589, -538, -501, -470, -444, -422, -401, -383, -366, -351, -336,
                -322, -309, -296, -284, -273, -262, -251, -240, -230, -220, -211, -202, -193,
                -184, -175, -166, -158, -149, -141, -133, -125, -117, -110, -102, -95,  -87,
                -80,  -72,  -65,  -57,  -50,  -43,  -36,  -29,  -21,  -14,  -7,   0,    7,
                14,   21,   29,   36,   43,   50,   57,   65,   72,   80,   87,   95,   102,
                110,  117,  125,  133,  141,  149,  158,  166,  175,  184,  193,  202,  211,
                220,  230,  240,  251,  262,  273,  284,  296,  309,  322,  336,  351,  366,
                383,  401,  422,  444,  470,  501,  538,  589,  677,  800};
    };

} // namespace scid::database
