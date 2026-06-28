/** @file
 * Shared status and error codes.
 */

#pragma once

namespace scid::core
{

    /** Status code returned by legacy Scid APIs.
     *
     * @c OK is zero and all error values are non-zero, so an errorT can be used
     * in simple boolean checks when only success/failure matters.
     */
    typedef unsigned short errorT;

    /** Shared status and error-code values.
     *
     * The numeric ranges are conventional: low values are general errors, 100s are
     * file I/O, 200s are database/name errors, 300s are position errors, 400s are
     * game/movetext errors, 600s are buffer errors, and 700s are codec feature
     * errors.
     */
    const errorT
        /** Success. */
        OK = 0,

        /** Unclassified failure. */
        ERROR = 1,
        /** Operation was cancelled by the user or progress callback. */
        ERROR_UserCancel = 2,
        /** Invalid argument or unsupported argument combination. */
        ERROR_BadArg = 3,

        /** File could not be opened. */
        ERROR_FileOpen = 101,
        /** File write failed. */
        ERROR_FileWrite = 102,
        /** File read failed. */
        ERROR_FileRead = 103,
        /** File seek failed. */
        ERROR_FileSeek = 104,
        /** File signature or magic bytes did not match the expected format. */
        ERROR_BadMagic = 105,
        /** Operation requires an open file but none is open. */
        ERROR_FileNotOpen = 106,
        /** File is already in use. */
        ERROR_FileInUse = 107,
        /** File was opened in a mode incompatible with the requested operation. */
        ERROR_FileMode = 108,
        /** File format version is not supported by this code. */
        ERROR_FileVersion = 109,
        /** File requires an older Scid compatibility path. */
        ERROR_OldScidVersion = 110,
        /** Operation would modify a read-only file. */
        ERROR_FileReadOnly = 111,
        /** Compaction failed while removing or replacing old database files. */
        ERROR_CompactRemove = 121,

        /** Memory allocation failed. */
        ERROR_MallocFailed = 151,
        /** Data is structurally corrupt or cannot be trusted. */
        ERROR_CorruptData = 152,
        /** Alias for @c ERROR_CorruptData. */
        ERROR_Corrupt = 152,

        /** Container or database has reached capacity. */
        ERROR_Full = 201,
        /** Requested name entry was not found. */
        ERROR_NameNotFound = 202,
        /** Alias for @c ERROR_NameNotFound. */
        ERROR_NotFound = 202,
        /** Requested name entry already exists. */
        ERROR_NameExists = 203,
        /** Alias for @c ERROR_NameExists. */
        ERROR_Exists = 203,
        /** Name table is empty. */
        ERROR_NameBaseEmpty = 204,
        /** Alias for @c ERROR_NameBaseEmpty. */
        ERROR_Empty = 204,
        /** Search or lookup completed without a match. */
        ERROR_NoMatchFound = 205,
        /** Some name data was lost or could not be recovered while loading. */
        ERROR_NameDataLoss = 206,
        /** Name exceeds the format limit. */
        ERROR_NameTooLong = 207,
        /** Name table has reached its entry limit. */
        ERROR_NameLimit = 208,
        /** Encoded offset exceeds the format limit. */
        ERROR_OffsetLimit = 209,
        /** Encoded game data exceeds the format limit. */
        ERROR_GameLengthLimit = 210,
        /** Database has reached its game-count limit. */
        ERROR_NumGamesLimit = 211,

        /** FEN text could not be parsed as a valid position. */
        ERROR_InvalidFEN = 301,
        /** Move text could not be parsed or applied legally. */
        ERROR_InvalidMove = 302,
        /** Position has an invalid piece count or material arrangement. */
        ERROR_PieceCount = 303,

        /** Generic game or movetext failure. */
        ERROR_Game = 400,
        /** Cursor or decoder reached the end of a move list. */
        ERROR_EndOfMoveList = 401,
        /** Cursor is already at the start of a move list. */
        ERROR_StartOfMoveList = 402,
        /** Requested variation does not exist. */
        ERROR_NoVariation = 403,
        /** Requested variation exists but contains no moves. */
        ERROR_EmptyVariation = 404,
        /** Variation nesting or count limit was exceeded. */
        ERROR_VariationLimit = 405,
        /** Encoded game or tag data could not be decoded. */
        ERROR_Decode = 406,
        /** Game movetext has reached its storage limit. */
        ERROR_GameFull = 407,

        /** Output buffer has no space for the requested write. */
        ERROR_BufferFull = 601,
        /** Input buffer did not contain enough data for the requested read. */
        ERROR_BufferRead = 602,

        /** Codec does not support the requested feature. */
        ERROR_CodecUnsupFeat = 701,
        /** Codec cannot represent a Chess960-specific feature. */
        ERROR_CodecChess960 = 702;

    static_assert(OK == false);

} // namespace scid::core
