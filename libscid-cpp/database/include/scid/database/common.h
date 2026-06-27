//////////////////////////////////////////////////////////////////////
//
//  FILE:       common.h
//              Common macros, structures and constants.
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    3.6.6
//
//  Notice:     Copyright (c) 2000-2004  Shane Hudson.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//              Copyright (c) 2006-2007 Pascal Georges
//
//////////////////////////////////////////////////////////////////////

#ifndef SCID_COMMON_H
#define SCID_COMMON_H

#include "scid/core/board.h"
#include "scid/core/error.h"
#include "scid/core/rating.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>
#define ASSERT(f) assert(f)

namespace scid::database
{

    /**
     * Encoded Scid database format version.
     *
     * Versions are stored as @c major * 100 + minor, so @c 109 represents 1.9
     * and @c 110 represents 1.10.  The value describes the on-disk database
     * format, not the library release.
     */
    typedef unsigned short versionT;

    /** Current Scid 4 database format version. */
    const versionT SCID_VERSION = 400;

    /**
     * Compact numeric ECO code used by the database index.
     *
     * Use the ECO conversion helpers when crossing the public string boundary;
     * this type is the packed representation stored in index entries and used for
     * fast ECO searches.
     */
    using EcoCode = std::uint16_t;

    /** Sentinel ECO value used when a game has no known ECO classification. */
    inline constexpr EcoCode ECO_CODE_NONE = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // TYPE DEFINITIONS

    /**
     * File access mode requested when opening a database codec.
     */
    enum fileModeT
    {
        /** No file access has been requested. */
        FMODE_None = 0,
        /** Open an existing database for reads only. */
        FMODE_ReadOnly,
        /** Open an existing database for writes only. */
        FMODE_WriteOnly,
        /** Open an existing database for both reads and writes. */
        FMODE_Both,
        /** Create a new database, replacing any existing files as the codec allows. */
        FMODE_Create
    };

} // namespace scid::database
#endif // #ifdef SCID_COMMON_H

//////////////////////////////////////////////////////////////////////
//  EOF:  common.h
//////////////////////////////////////////////////////////////////////
