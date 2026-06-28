#pragma once

#include "scid/core/primitives.h"
#include <cstdint>

namespace scid::database
{

    /**
     * Zero-based game number within an opened database.
     *
     * A game number is a transient handle into the current database index.  It is
     * valid only while the database and its current ordering remain the same.
     */
    using gamenumT = scid::core::uint;

    /**
     * Identifier for a stored database name.
     *
     * Name identifiers are indexes into a @ref NameBase bucket such as players,
     * events, sites, or rounds.  The same numeric value may therefore refer to
     * different strings depending on the name type it is used with.
     */
    using idNumberT = std::uint32_t; // Should be idNameT

    /** Sentinel game number used when no database game is selected. */
    inline constexpr gamenumT INVALID_GAMEID = 0xffffffff;

} // namespace scid::database
