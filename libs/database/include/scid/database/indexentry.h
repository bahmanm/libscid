/*
* Copyright (c) 1999-2002  Shane Hudson
* Copyright (c) 2006-2009  Pascal Georges
* Copyright (C) 2014-2017  Fulvio Benini

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

#ifndef SCID_INDEXENTRY_H
#define SCID_INDEXENTRY_H

#include "scid/core/game_result.h"
#include "scid/database/common.h"
#include "scid/core/date.h"
#include "scid/database/game_id.h"
#include "scid/database/matsig.h"
#include <cstring> //memcmp

namespace scid::database {

/**
 * Size of the home-pawn signature stored in an @ref IndexEntry.
 *
 * The first byte stores the number of valid entries.  The remaining eight
 * bytes contain the packed home-pawn data used by material and pawn-structure
 * searches.
 */
const scid::core::uint HPSIG_SIZE = 9;

/** Highest rating value that fits in the packed 12-bit rating fields. */
const scid::core::ratingT MAX_ELO = 4000;

/** Bit masks for the six custom flag positions within the custom flag byte. */
const scid::core::byte CUSTOM_FLAG_MASK[] = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5 };


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
 * Compact index record for one database game.
 *
 * An index entry is the database layer's fast path for list views, sorting,
 * filtering, and opening a game.  It stores the offset and length of the
 * encoded game body together with the searchable metadata that should not
 * require decoding movetext: name identifiers, dates, result, ratings, ECO,
 * material signatures, flags, and approximate annotation counts.
 *
 * Name fields are @c idNumberT handles into @ref NameBase; they are not
 * owned strings.  Count fields are stored in four-bit buckets, so values above
 * ten are approximate.  The setters assert when a value cannot be represented
 * by the packed field.  Prefer value-initialising new records as
 * @c IndexEntry{} unless every field will be assigned before use.
 */
class IndexEntry {
    uint64_t offset_         : 46; // Start of gamefile record for this game.
    uint64_t gameDataSize_   : 18; // Length of gamefile record for this game.

    uint32_t nComments_      :  4;
    uint32_t whiteID_        : 28;

    uint32_t nVariations_    :  4;
    uint32_t blackID_        : 28;

    uint32_t nNags_          :  4;
    uint32_t eventID_        : 28;

    uint32_t siteID_;

    uint32_t variant_        :  1;
    uint32_t roundID_        : 31;

    uint32_t whiteElo_       : 12;
    uint32_t date_           : 20;

    uint32_t blackElo_       : 12;
    uint32_t eventDate_      : 20;

    uint32_t numHalfMoves_   : 10;
    uint32_t flags_          : 22;

    uint32_t result_         :  2;
    uint32_t whiteEloType_   :  3;
    uint32_t blackEloType_   :  3;
    uint32_t finalMatSig_    : 24; // material of the final position in the game

    uint16_t ECOcode_;

    uint8_t  storedLineCode_;

    scid::core::byte     HomePawnData [HPSIG_SIZE];  // homePawnSig data.

public:
    /** Returns the byte offset of the encoded game record. */
    uint64_t  GetOffset() const { return offset_; }
    /** Returns the byte length of the encoded game record. */
    uint32_t  GetLength() const { return gameDataSize_; }
    /** Returns the NameBase identifier for the White player. */
    idNumberT GetWhite() const { return whiteID_; }
    /** Returns White's rating value, or zero when unknown. */
    scid::core::ratingT      GetWhiteElo() const { return whiteElo_; }
    /** Returns the rating system used for White's rating. */
    scid::core::ratingTypeT GetWhiteRatingType() const { return whiteEloType_; }
    /** Returns the NameBase identifier for the Black player. */
    idNumberT GetBlack() const { return blackID_; }
    /** Returns Black's rating value, or zero when unknown. */
    scid::core::ratingT      GetBlackElo() const { return blackElo_; }
    /** Returns the rating system used for Black's rating. */
    scid::core::ratingTypeT GetBlackRatingType() const { return blackEloType_; }
    /** Returns the NameBase identifier for the event. */
    idNumberT GetEvent() const { return eventID_; }
    /** Returns the NameBase identifier for the site. */
    idNumberT GetSite() const { return siteID_; }
    /** Returns the NameBase identifier for the round. */
    idNumberT GetRound() const { return roundID_; }
    /** Returns the date on which the game was played. */
    scid::core::dateT     GetDate() const { return date_; }
    /** Returns the date associated with the event as a whole. */
    scid::core::dateT     GetEventDate() const { return eventDate_; }
    /** Returns the game result. */
    scid::core::resultT   GetResult() const { return result_; }
    /** Returns the approximate number of variations stored for the game. */
    scid::core::uint      GetVariationCount() const { return DecodeCount(nVariations_); }
    /** Returns the approximate number of comments stored for the game. */
    scid::core::uint      GetCommentCount() const { return DecodeCount(nComments_); }
    /** Returns the approximate number of NAGs stored for the game. */
    scid::core::uint      GetNagCount() const { return DecodeCount(nNags_); }
    /** Returns the number of half-moves in the main line. */
    uint16_t  GetNumHalfMoves() const { return numHalfMoves_; }
    /** Returns the material signature of the final mainline position. */
    matSigT   GetFinalMatSig() const { return finalMatSig_; }
    /** Returns the stored-line classification code used by tree/opening features. */
    scid::core::byte      GetStoredLineCode() const { return storedLineCode_; }
    /** Returns the packed ECO classification, or @c ECO_CODE_NONE. */
    EcoCode GetEcoCode() const { return ECOcode_; }
    /** Returns true when every bit in @p mask is set. */
    bool      GetFlag(uint32_t mask) const { return (flags_ & mask) == mask; }
    /** Returns the raw flag bit mask. */
    uint32_t  GetRawFlags() const { return flags_; }
    /** Returns the packed four-bit variation, comment, and NAG count codes. */
    uint16_t  GetRaw4bitsCounts() const {
        uint16_t res = nVariations_ & 0x0F;
        res |= static_cast<uint16_t>(nComments_ & 0x0F) << 4;
        res |= static_cast<uint16_t>(nNags_ & 0x0F) << 8;
        return res;
    }

    /** Marks the game as using standard chess castling semantics. */
    void setChessStd() { variant_ = 0; }
    /** Marks the game as using Chess960 castling semantics. */
    void setChess960() { variant_ = 1; }
    /** Returns true when the game uses standard chess castling semantics. */
    bool isChessStd() const { return variant_ == 0; }

    /** Returns the packed home-pawn signature data. */
    const scid::core::byte* GetHomePawnData() const { return HomePawnData; }
    /**
     * Replaces the home-pawn signature data.
     *
     * @param hpCount number of valid packed entries.
     * @param hpVal eight bytes of packed home-pawn values.
     */
    void SetHomePawnData(scid::core::byte hpCount, const scid::core::byte hpVal[8]) {
        HomePawnData[0] = hpCount; // First scid::core::byte stores the count
        std::copy_n(hpVal, 8, HomePawnData + 1);
    }

    /** Sets the encoded game-record offset and asserts if it is truncated. */
    void SetOffset(uint64_t offset) {
        offset_ = offset;
        ASSERT(GetOffset() == offset);
    }
    /** Sets the encoded game-record length and asserts if it is truncated. */
    void SetLength(size_t length) {
        gameDataSize_ = length;
        ASSERT(GetLength() == length);
    }
    /** Sets the White player NameBase identifier and asserts if it is truncated. */
    void SetWhite(idNumberT id) {
        whiteID_ = id;
        ASSERT(GetWhite() == id);
    }
    /** Sets White's rating value and asserts if it is truncated. */
    void SetWhiteElo(scid::core::ratingT elo) {
        whiteElo_ = elo;
        ASSERT(GetWhiteElo() == elo);
    }
    /** Sets White's rating-system code and asserts if it is truncated. */
    void SetWhiteRatingType(scid::core::ratingTypeT b) {
        whiteEloType_ = b;
        ASSERT(GetWhiteRatingType() == b);
    }
    /** Sets the Black player NameBase identifier and asserts if it is truncated. */
    void SetBlack(idNumberT id) {
        blackID_ = id;
        ASSERT(GetBlack() == id);
    }
    /** Sets Black's rating value and asserts if it is truncated. */
    void SetBlackElo(scid::core::ratingT elo) {
        blackElo_ = elo;
        ASSERT(GetBlackElo() == elo);
    }
    /** Sets Black's rating-system code and asserts if it is truncated. */
    void SetBlackRatingType(scid::core::ratingTypeT b) {
        blackEloType_ = b;
        ASSERT(GetBlackRatingType() == b);
    }
    /** Sets the event NameBase identifier and asserts if it is truncated. */
    void SetEvent(idNumberT id) {
        eventID_ = id;
        ASSERT(GetEvent() == id);
    }
    /** Sets the site NameBase identifier and asserts if it is truncated. */
    void SetSite(idNumberT id) {
        siteID_ = id;
        ASSERT(GetSite() == id);
    }
    /** Sets the round NameBase identifier and asserts if it is truncated. */
    void SetRound(idNumberT id) {
        roundID_ = id;
        ASSERT(GetRound() == id);
    }
    /** Sets the game date and asserts if it is truncated. */
    void SetDate(scid::core::dateT date) {
        date_ = date;
        ASSERT(GetDate() == date);
    }
    /** Sets the event date and asserts if it is truncated. */
    void SetEventDate(scid::core::dateT edate) {
        eventDate_ = edate;
        ASSERT(GetEventDate() == edate);
    }
    /** Sets the game result and asserts if it is truncated. */
    void SetResult(scid::core::resultT res) {
        result_ = res;
        ASSERT(GetResult() == res);
    }
    /** Stores the approximate variation count using Scid's four-bit buckets. */
    void SetVariationCount(unsigned x) { nVariations_ = EncodeCount(x); }
    /** Stores the approximate comment count using Scid's four-bit buckets. */
    void SetCommentCount(unsigned x) { nComments_ = EncodeCount(x); }
    /** Stores the approximate NAG count using Scid's four-bit buckets. */
    void SetNagCount(unsigned x) { nNags_ = EncodeCount(x); }
    /** Sets the raw four-bit variation count code and asserts if it is truncated. */
    void SetRawVariationCount(unsigned x) {
        nVariations_ = x;
        ASSERT(x == nVariations_);
    }
    /** Sets the raw four-bit comment count code and asserts if it is truncated. */
    void SetRawCommentCount(unsigned x) {
        nComments_ = x;
        ASSERT(x == nComments_);
    }
    /** Sets the raw four-bit NAG count code and asserts if it is truncated. */
    void SetRawNagCount(unsigned x) {
        nNags_ = x;
        ASSERT(x == nNags_);
    }
    /** Sets the mainline half-move count and asserts if it is truncated. */
    void SetNumHalfMoves(scid::core::ushort b) {
        numHalfMoves_ = b;
        ASSERT(GetNumHalfMoves() == b);
    }
    /** Sets the final-position material signature and asserts if it is truncated. */
    void SetFinalMatSig(matSigT ms) {
        finalMatSig_ = ms;
        ASSERT(GetFinalMatSig() == ms);
    }
    /** Sets the stored-line classification code. */
    void SetStoredLineCode(scid::core::byte b) {
        storedLineCode_ = b;
        ASSERT(GetStoredLineCode() == b);
    }
    /** Sets the packed ECO classification. */
    void SetEcoCode(EcoCode eco) {
        ECOcode_ = eco;
        ASSERT(GetEcoCode() == eco);
    }
    /**
     * Sets or clears every bit in @p flagMask.
     *
     * Passing @c true ORs the mask into the record; passing @c false clears
     * all bits contained in the mask.
     */
    void SetFlag(uint32_t flagMask, bool set) {
        if (set)
            flags_ |= flagMask;
        else
            flags_ &= ~flagMask;
    }

    /** Returns the year component of @ref GetDate(), or zero when unknown. */
    scid::core::uint  GetYear () const { return scid::core::date_GetYear (GetDate()); }
    /** Returns the month component of @ref GetDate(), or zero when unknown. */
    scid::core::uint  GetMonth() const { return scid::core::date_GetMonth (GetDate()); }
    /** Returns the day component of @ref GetDate(), or zero when unknown. */
    scid::core::uint  GetDay ()  const { return scid::core::date_GetDay (GetDate()); }

    /** Sets the player NameBase identifier for @p col. */
    void SetPlayer(scid::core::colorT col, idNumberT id) {
        return (col == scid::core::BLACK) ? SetBlack(id) : SetWhite(id);
    }

    /**
     * Computes Scid's compact game-quality rating.
     *
     * The value is a sort/search heuristic derived from the players' ratings,
     * annotation density, and short-draw penalties.
     */
    scid::core::byte   GetRating() const;

    /** Returns true when the game has a non-standard start position. */
    bool GetStartFlag () const      { return GetFlag(1 << IDX_FLAG_START); }
    /** Returns true when the game contains a promotion. */
    bool GetPromotionsFlag () const { return GetFlag(1 << IDX_FLAG_PROMO); }
    /** Returns true when the game contains an under-promotion. */
    bool GetUnderPromoFlag() const  { return GetFlag(1 << IDX_FLAG_UPROMO); }
    /** Returns true when the comment count bucket is non-zero. */
    bool GetCommentsFlag () const   { return (GetCommentCount() > 0); }
    /** Returns true when the variation count bucket is non-zero. */
    bool GetVariationsFlag () const { return (GetVariationCount() > 0); }
    /** Returns true when the NAG count bucket is non-zero. */
    bool GetNagsFlag () const       { return (GetNagCount() > 0); }
    /** Returns true when the game is marked as deleted. */
    bool GetDeleteFlag () const     { return GetFlag(1 << IDX_FLAG_DELETE); }

    /**
     * Converts a traditional user-visible flag character to a flag index.
     *
     * Unsupported characters return zero for legacy compatibility.
     */
    static scid::core::uint CharToFlag (char ch);
    /**
     * Converts a Scid flag character to a mask suitable for @ref GetFlag() or
     * @ref SetFlag().
     *
     * This legacy helper asserts on unsupported characters.  Use
     * @c gameFlagMaskFromChar() when parsing untrusted input.
     */
    static uint32_t CharToFlagMask (char flag);
    /**
     * Builds a combined flag mask from a NUL-terminated string of Scid flag
     * characters.
     */
    static uint32_t StrToFlagMask (const char* flags);
    /**
     * Writes the set flags from @p flags into @p dest.
     *
     * When @p flags is null, the traditional user-visible flag order is used.
     * @p dest must have room for every requested flag character plus the
     * terminating NUL byte.
     *
     * @returns the number of flag characters written, excluding the terminator.
     */
    scid::core::uint GetFlagStr(char* dest, const char* flags) const;

    /** Sets or clears the non-standard-start flag. */
    void SetStartFlag (bool b)      { SetFlag(1 << IDX_FLAG_START, b); }
    /** Sets or clears the promotion flag. */
    void SetPromotionsFlag (bool b) { SetFlag(1 << IDX_FLAG_PROMO, b); }
    /** Sets or clears the under-promotion flag. */
    void SetUnderPromoFlag (bool b) { SetFlag(1 << IDX_FLAG_UPROMO, b); }
    /** Sets or clears the delete marker. */
    void SetDeleteFlag (bool b)     { SetFlag(1 << IDX_FLAG_DELETE, b); }
    /** Clears every stored flag bit. */
    void clearFlags() { return SetFlag(IDX_MASK_ALLFLAGS, false); }
    /**
     * Compares two entries while ignoring flag differences.
     *
     * This is used by storage backends that may persist flag-only changes
     * differently from full metadata rewrites.
     */
    bool equalExceptFlags(IndexEntry ie) const {
        ie.flags_ = flags_;
        static_assert(std::has_unique_object_representations_v<IndexEntry>);
        return memcmp(this, &ie, sizeof(IndexEntry)) == 0;
    }

    /** Flag positions in the packed index-entry flag mask. */
    enum {
        /** Game has a non-standard start position. */
        IDX_FLAG_START      =  0,
        /** Game contains at least one promotion. */
        IDX_FLAG_PROMO      =  1,
        /** Game contains at least one under-promotion. */
        IDX_FLAG_UPROMO     =  2,
        /** Game is marked as deleted. */
        IDX_FLAG_DELETE     =  3,
        /** User flag: notable White opening play. */
        IDX_FLAG_WHITE_OP   =  4,
        /** User flag: notable Black opening play. */
        IDX_FLAG_BLACK_OP   =  5,
        /** User flag: middlegame theme. */
        IDX_FLAG_MIDDLEGAME =  6,
        /** User flag: endgame theme. */
        IDX_FLAG_ENDGAME    =  7,
        /** User flag: opening novelty. */
        IDX_FLAG_NOVELTY    =  8,
        /** User flag: pawn-structure theme. */
        IDX_FLAG_PAWN       =  9,
        /** User flag: tactical theme. */
        IDX_FLAG_TACTICS    = 10,
        /** User flag: kingside play. */
        IDX_FLAG_KSIDE      = 11,
        /** User flag: queenside play. */
        IDX_FLAG_QSIDE      = 12,
        /** User flag: brilliancy or good play. */
        IDX_FLAG_BRILLIANCY = 13,
        /** User flag: blunder or bad play. */
        IDX_FLAG_BLUNDER    = 14,
        /** General user-defined flag. */
        IDX_FLAG_USER       = 15,
        /** First custom user flag. */
        IDX_FLAG_CUSTOM1    = 16,
        /** Second custom user flag. */
        IDX_FLAG_CUSTOM2    = 17,
        /** Third custom user flag. */
        IDX_FLAG_CUSTOM3    = 18,
        /** Fourth custom user flag. */
        IDX_FLAG_CUSTOM4    = 19,
        /** Fifth custom user flag. */
        IDX_FLAG_CUSTOM5    = 20,
        /** Sixth custom user flag. */
        IDX_FLAG_CUSTOM6    = 21,
        /** Number of defined flag positions. */
        IDX_NUM_FLAGS       = 22,
    };
    /** Mask used to address every flag bit in the entry. */
    static const uint32_t IDX_MASK_ALLFLAGS = 0xFFFFFFFF;

private:
    static scid::core::uint EncodeCount (scid::core::uint x) {
        if (x <= 10) { return x; }
        if (x <= 12) { return 10; }
        if (x <= 17) { return 11; }  // 11 indicates 15 (13-17)
        if (x <= 24) { return 12; }  // 12 indicates 20 (18-24)
        if (x <= 34) { return 13; }  // 13 indicates 30 (25-34)
        if (x <= 44) { return 14; }  // 14 indicates 40 (35-44)
        return 15;                   // 15 indicates 50 or more
    }
    static scid::core::uint DecodeCount (scid::core::uint x) {
        static scid::core::uint countCodes[16] = {0,1,2,3,4,5,6,7,8,9,10,15,20,30,40,50};
        return countCodes[x & 15];
    }
};


inline scid::core::byte IndexEntry::GetRating() const {
    scid::core::ratingT welo = GetWhiteElo();
    scid::core::ratingT belo = GetBlackElo();
    auto rating = (welo != 0 && belo != 0) ? (welo + belo) / 140 : 0;
    static_assert(std::is_signed_v<decltype(rating)>);

    // Bonus for comments or Nags
    if (GetCommentCount() > 2 || GetNagCount() > 2) {
        if (rating < 21) { // Missing elo
            rating = 38;
        } else {
            rating += 6;
        }
    }

    // Early draw penalty
    if (GetResult() == scid::core::RESULT_Draw) {
        scid::core::uint moves = GetNumHalfMoves();
        if (moves < 80) {
            rating -= 3;
            if (moves < 60) {
                rating -= 2;
                if (moves < 40) rating -= 2;
            }
        }
    }

    if (rating < 0) return 0;
    else return static_cast<scid::core::byte> (rating);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IndexEntry::CharToFlag():
//    Returns the flag number corresponding to the given character.
inline scid::core::uint
IndexEntry::CharToFlag (char ch)
{
    scid::core::uint flag = 0;
    switch (toupper(ch)) {
        case 'D': flag = IDX_FLAG_DELETE;     break;
        case 'W': flag = IDX_FLAG_WHITE_OP;   break;
        case 'B': flag = IDX_FLAG_BLACK_OP;   break;
        case 'M': flag = IDX_FLAG_MIDDLEGAME; break;
        case 'E': flag = IDX_FLAG_ENDGAME;    break;
        case 'N': flag = IDX_FLAG_NOVELTY;    break;
        case 'P': flag = IDX_FLAG_PAWN;       break;
        case 'T': flag = IDX_FLAG_TACTICS;    break;
        case 'K': flag = IDX_FLAG_KSIDE;      break;
        case 'Q': flag = IDX_FLAG_QSIDE;      break;
        case '!': flag = IDX_FLAG_BRILLIANCY; break;
        case '?': flag = IDX_FLAG_BLUNDER;    break;
        case 'U': flag = IDX_FLAG_USER;       break;
        case '1': flag = IDX_FLAG_CUSTOM1;    break;
        case '2': flag = IDX_FLAG_CUSTOM2;    break;
        case '3': flag = IDX_FLAG_CUSTOM3;    break;
        case '4': flag = IDX_FLAG_CUSTOM4;    break;
        case '5': flag = IDX_FLAG_CUSTOM5;    break;
        case '6': flag = IDX_FLAG_CUSTOM6;    break;
    }
    return flag;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IndexEntry::CharToFlagMask():
//    Transform a char in a mask that can be used with GetFlag() and SetFlag()
inline uint32_t IndexEntry::CharToFlagMask(char flag)
{
    switch (toupper(flag)) {
        case 'S': return 1 << IDX_FLAG_START;
        case 'X': return 1 << IDX_FLAG_PROMO;
        case 'Y': return 1 << IDX_FLAG_UPROMO;
        case 'D': return 1 << IDX_FLAG_DELETE;
        case 'W': return 1 << IDX_FLAG_WHITE_OP;
        case 'B': return 1 << IDX_FLAG_BLACK_OP;
        case 'M': return 1 << IDX_FLAG_MIDDLEGAME;
        case 'E': return 1 << IDX_FLAG_ENDGAME;
        case 'N': return 1 << IDX_FLAG_NOVELTY;
        case 'P': return 1 << IDX_FLAG_PAWN;
        case 'T': return 1 << IDX_FLAG_TACTICS;
        case 'K': return 1 << IDX_FLAG_KSIDE;
        case 'Q': return 1 << IDX_FLAG_QSIDE;
        case '!': return 1 << IDX_FLAG_BRILLIANCY;
        case '?': return 1 << IDX_FLAG_BLUNDER;
        case 'U': return 1 << IDX_FLAG_USER;
        case '1': return 1 << IDX_FLAG_CUSTOM1;
        case '2': return 1 << IDX_FLAG_CUSTOM2;
        case '3': return 1 << IDX_FLAG_CUSTOM3;
        case '4': return 1 << IDX_FLAG_CUSTOM4;
        case '5': return 1 << IDX_FLAG_CUSTOM5;
        case '6': return 1 << IDX_FLAG_CUSTOM6;
    }

    ASSERT(0);
    return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IndexEntry::StrToFlagMask():
//    Transform a string in a mask that can be used with GetFlag() and SetFlag()
inline uint32_t IndexEntry::StrToFlagMask(const char* flags)
{
    if (flags == 0) return 0;

    uint32_t res = 0;
    while (*flags != 0) {
        res |= IndexEntry::CharToFlagMask(*(flags++));
    }
    return res;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IndexEntry::GetFlagStr():
//    Fills in the provided flag string with information on the
//    user-settable flags set for this game.
//    Returns the number of specified flags that are turned on.
inline scid::core::uint
IndexEntry::GetFlagStr(char* dest, const char* flags) const
{
    if (flags == NULL) { flags = "DWBMENPTKQ!?U123456"; }
    scid::core::uint count = 0;
    while (*flags != 0) {
        uint32_t mask = CharToFlagMask(*flags);
        ASSERT(mask != 0);
        if (GetFlag(mask)) {
            *dest++ = *flags;
            count++;
        }
        flags++;
    }
    *dest = 0;
    return count;
}


} // namespace scid::database
#endif
