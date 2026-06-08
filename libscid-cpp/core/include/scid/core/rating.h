/** @file
 * Rating values and rating-system identifiers.
 */

#pragma once

#include "scid/core/primitives.h"
#include <cstddef>

namespace scid::core {

/** Numeric player rating value.
 *
 * A value of 0 is commonly used by callers to represent an unknown rating.
 * Database codecs may impose tighter storage limits than the full range of the
 * underlying type.
 */
using ratingT = ushort;

/** Identifier for the rating system stored with a player rating. */
using ratingTypeT = byte;

/** FIDE Elo rating. */
inline constexpr ratingTypeT RATING_Elo = 0;

/** Generic rating tag when the original rating system is not more specific. */
inline constexpr ratingTypeT RATING_Rating = 1;

/** Rapid rating. */
inline constexpr ratingTypeT RATING_Rapid = 2;

/** International Correspondence Chess Federation rating. */
inline constexpr ratingTypeT RATING_ICCF = 3;

/** United States Chess Federation rating. */
inline constexpr ratingTypeT RATING_USCF = 4;

/** Deutsche Wertungszahl rating. */
inline constexpr ratingTypeT RATING_DWZ = 5;

/** British/English Chess Federation rating.
 *
 * The legacy constant name is kept for compatibility; the public display name
 * in @c ratingTypeNames is "ECF".
 */
inline constexpr ratingTypeT RATING_BCF = 6;

/** Number of defined rating-system identifiers. */
inline constexpr std::size_t NUM_RATING_TYPES = 7;

/** Null-terminated table of PGN-facing rating-system names.
 *
 * The table is indexed by a @c ratingTypeT value and has a trailing nullptr
 * sentinel for legacy loops.
 */
inline constexpr const char* ratingTypeNames[NUM_RATING_TYPES + 1] = {
    "Elo", "Rating", "Rapid", "ICCF", "USCF", "DWZ", "ECF", nullptr};

} // namespace scid::core
