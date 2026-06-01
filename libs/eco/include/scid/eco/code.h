#pragma once

#include <cstdint>

namespace scid::eco {

/**
 * Packed ECO classification code.
 *
 * ECO strings have a public textual form such as @c "B20" or @c "C50a1".
 * This type is the compact numeric representation used for storage, sorting,
 * and range calculations.  @c ECO_None is reserved for "no ECO code".
 */
using Code = std::uint16_t;

/**
 * Fixed-size buffer large enough for any NUL-terminated ECO string.
 *
 * The longest emitted form is five visible characters, for example
 * @c "E99z4", plus the terminating NUL byte.
 */
using String = char[6];

/** Sentinel value used when no ECO classification is known. */
inline constexpr Code ECO_None = 0;

/**
 * Writes @p ecoCode to @p ecoStr.
 *
 * When @p extensions is false, only the basic three-character ECO code is
 * emitted.  When true, Scid's optional lower-case subcode and numeric
 * extension are included.  @p ecoStr must have room for at least
 * @c String.  @c ECO_None writes an empty string.
 */
void toString(Code ecoCode, char* ecoStr, bool extensions = true);

/**
 * Writes only the basic three-character ECO code.
 *
 * This is a convenience wrapper around @c toString() for callers that want
 * @c "B20" rather than Scid's extended forms such as @c "B20a3".
 */
inline void toBasicString(Code ecoCode, char* ecoStr) {
	toString(ecoCode, ecoStr, false);
}

/**
 * Writes the full Scid ECO string, including optional subcodes.
 */
inline void toExtendedString(Code ecoCode, char* ecoStr) {
	toString(ecoCode, ecoStr, true);
}

/**
 * Parses an ECO string into a compact code.
 *
 * Canonical ECO text starts with @c A through @c E, case-insensitively,
 * followed by two digits, an optional lower-case subcode @c a through @c z,
 * and an optional numeric extension @c 1 through @c 4.  Prefixes such as
 * @c "B" and @c "B2" are accepted and map to the first code in that range.
 * Invalid leading text returns @c ECO_None.
 */
Code fromString(const char* ecoStr);

/**
 * Returns the last concrete code covered by @p ecoCode.
 *
 * Basic codes expand to their final extended subcode.  Letter-only extended
 * codes such as @c "B91a" expand through their @c 1..4 numeric extensions.
 * @c ECO_None returns @c ECO_None.
 */
Code lastSubCode(Code ecoCode);

/**
 * Removes Scid's extended subcode from @p ecoCode.
 *
 * @c ECO_None returns @c ECO_None.
 */
Code basicCode(Code ecoCode);

/**
 * Maps @p ecoCode to the dense ECO-statistics bucket used by database code.
 *
 * Each basic ECO code has one bucket for the basic line plus one bucket for
 * each lower-case subcode.  Numeric extensions @c 1..4 fold into the same
 * bucket as their parent subcode.  Use this for aggregate statistics, not for
 * display or round-tripping.  @p ecoCode must not be @c ECO_None.
 */
Code reduce(Code ecoCode);

} // namespace scid::eco
