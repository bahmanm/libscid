/** @file
 * Game result codes and formatting tables.
 */

#pragma once

#include "scid/core/primitives.h"

namespace scid::core {

/** Number of result codes, including unknown/unfinished games. */
const uint NUM_RESULT_TYPES = 4;
/** Game result code used by Game and database statistics. */
typedef byte resultT;
/** Unknown, unfinished, or no-result game. */
const resultT RESULT_None = 0;
/** White win. */
const resultT RESULT_White = 1;
/** Black win. */
const resultT RESULT_Black = 2;
/** Draw. */
const resultT RESULT_Draw = 3;

/** Half-point scores from White's perspective: win=2, draw=1, loss=0. */
const uint RESULT_SCORE[4] = {1, 2, 0, 1};
/** Compact one-character result markers. */
const char RESULT_CHAR[4] = {'*', '1', '0', '='};
/** Short result strings used by legacy displays. */
const char RESULT_STR[4][4] = {"*", "1-0", "0-1", "=-="};
/** PGN result strings. */
const char RESULT_LONGSTR[4][8] = {"*", "1-0", "0-1", "1/2-1/2"};
/** Result from the opposite player's perspective. */
const resultT RESULT_OPPOSITE[4] = {
    RESULT_None, RESULT_Black, RESULT_White, RESULT_Draw};

} // namespace scid::core
