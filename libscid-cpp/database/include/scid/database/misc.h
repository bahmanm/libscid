
//////////////////////////////////////////////////////////////////////
//
//  FILE:       misc.h
//              Miscellaneous routines (File I/O, etc)
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    3.5
//
//  Notice:     Copyright (c) 2001-2003  Shane Hudson.  All rights reserved.
//              Copyright (C) 2015  Fulvio Benini
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////

#ifndef SCID_MISC_H
#define SCID_MISC_H

#include "scid/core/game_result.h"
#include "scid/database/common.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctype.h> // For isspace(), etc
#include <stdio.h>
#include <string>
#include <vector>

namespace scid::database
{

    /**
     * Inclusive numeric range parsed from one or two whitespace-separated values.
     *
     * A single value means an exact range.  Two values define the minimum and
     * maximum; reversed bounds are normalised.  The parser intentionally follows
     * @c strtol rules, so malformed text yields the numeric prefix that could be
     * parsed, or zero when no prefix exists.  Search helpers derive from this type
     * when they need the same permissive range syntax accepted by ScidUp command
     * arguments.
     */
    class StrRange
    {
        protected:
            long min_;
            long max_;

        protected:
            StrRange() : min_(0), max_(0)
            {}

        public:
            /**
             * Parses @p range as either @c "min" or @c "min max".
             *
             * Extra words after the second value are ignored.  Missing second values
             * become an exact range.
             */
            explicit StrRange(
                const char* range)
            {
                char* next;
                min_ = std::strtol(range, &next, 10);
                char* end;
                max_ = std::strtol(next, &end, 10);
                if (next == end)
                    max_ = min_;
                if (min_ > max_)
                    std::swap(min_, max_);
            }

            /** Returns true when @p val is inside the inclusive range. */
            bool
            inRange(
                long val) const
            {
                if (val < min_ || val > max_)
                    return false;
                return true;
            }
    };

    /**
     * Optional progress and cancellation callback.
     *
     * Long-running database operations receive a @ref Progress object and call it
     * periodically with work completed, total work, and sometimes a status
     * message.  Returning false requests cooperative cancellation.  A default
     * constructed @ref Progress object always returns true.
     *
     * @ref Progress owns its implementation pointer and is non-copyable.  Pass it
     * by reference when wiring UI progress into database operations.  Callers
     * should treat cancellation as cooperative: an operation may finish the current
     * batch before observing the next false result.
     */
    class Progress
    {
        public:
            /** Callback implementation owned by @ref Progress. */
            struct Impl
            {
                    virtual ~Impl()
                    {}
                    /**
                     * Reports progress and returns whether work should continue.
                     *
                     * @p done and @p total are operation-defined counters, not necessarily
                     * byte counts or game counts.  @p msg may be null when the operation has
                     * no status text to display.
                     */
                    virtual bool
                    report(size_t done, size_t total, const char* msg) = 0;
            };

            /**
             * Takes ownership of @p f.
             */
            Progress(
                Impl* f = NULL)
                : f_(f)
            {}
            Progress(const Progress&) = delete;
            /** Deletes the owned callback implementation. */
            ~Progress()
            {
                delete f_;
            }

            /** Reports progress without a message. */
            bool
            report(
                size_t done,
                size_t total) const
            {
                return operator()(done, total);
            }
            /** Reports progress and returns whether the caller should continue. */
            bool
            operator()(
                size_t done,
                size_t total,
                const char* msg = NULL) const
            {
                if (f_)
                    return f_->report(done, total, msg);
                return true;
            }

        private:
            Impl* f_;
    };

    /**
     * Operation used when combining a new search result with an existing filter.
     */
    enum filterOpT
    {
        /** Search only games currently included, removing non-matches. */
        FILTEROP_AND,
        /** Search games currently excluded, adding matches. */
        FILTEROP_OR,
        /** Start from all games, then keep only matches. */
        FILTEROP_RESET
    };

    /**
     * Parses a filter-combination operation.
     *
     * @c "A", @c "a", and @c "0" mean AND; @c "O", @c "o", and @c "1" mean OR;
     * @c "R", @c "r", and @c "2" mean reset.  Unknown values default to reset.
     */
    inline filterOpT
    strGetFilterOp(
        const char* str)
    {
        switch (*str)
        {
            // AND:
            case 'A':
            case 'a':
            case '0':
                return FILTEROP_AND;
            // OR:
            case 'O':
            case 'o':
            case '1':
                return FILTEROP_OR;
            // RESET:
            case 'R':
            case 'r':
            case '2':
                return FILTEROP_RESET;
        }
        // Default is RESET.
        return FILTEROP_RESET;
    }

    /**
     * Legacy string and parser helpers used by database import, search, and
     * namebase compatibility code.
     *
     * These functions intentionally preserve Scid's older command and namebase
     * semantics.  Some helpers are permissive parsers for user-entered search
     * arguments; others implement byte-exact or legacy-compatible comparisons
     * that should not be replaced with locale-aware string APIs unless the call
     * site explicitly wants different behaviour.
     */

    /**
     * Returns a case-insensitive hash of the first four characters of @p str.
     *
     * Shorter strings are padded by their terminating NUL byte.  The hash is used
     * as a compact sortable prefix key for names.
     */
    inline uint32_t
    strStartHash(
        const char* str)
    {
        ASSERT(str != 0);
        const unsigned char* s = reinterpret_cast<const unsigned char*>(str);

        uint32_t tmp = static_cast<unsigned char>(tolower(*s));
        uint32_t result = tmp << 24;
        if (*s == '\0')
            return result;
        tmp = static_cast<unsigned char>(tolower(*++s));
        result += tmp << 16;
        if (*s == '\0')
            return result;
        tmp = static_cast<unsigned char>(tolower(*++s));
        result += tmp << 8;
        if (*s == '\0')
            return result;
        result += static_cast<unsigned char>(tolower(*++s));
        return result;
    }

    /** Allocates and returns a copy of @p str using @c new[]. */
    char*
    strDuplicate(const char* str);

    /** Appends @p extra to @p target and returns the new end pointer. */
    char*
    strAppend(char* target, const char* extra);
    /**
     * Copies @p orig into @p target using exactly @p length bytes when
     * @p length is non-negative, padding with @p pad as needed.
     */
    scid::core::uint
    strPad(char* target, const char* orig, int length, char pad);
    /** Returns the first occurrence of @p matchChar in @p target, or null. */
    const char*
    strFirstChar(const char* target, char matchChar);
    /** Returns the last occurrence of @p matchChar in @p target, or null. */
    const char*
    strLastChar(const char* target, char matchChar);
    /** Removes every occurrence of @p ch from @p str in place. */
    void
    strStrip(char* str, char ch);

    /** Returns the first character in @p target that is not in @p trimChars. */
    const char*
    strTrimLeft(const char* target, const char* trimChars);
    /** Returns @p target after skipping leading ASCII whitespace. */
    inline const char*
    strTrimLeft(
        const char* target)
    {
        return strTrimLeft(target, " \t\r\n");
    }
    /**
     * Truncates @p target at the last @p suffixChar.
     *
     * @returns the number of characters removed, or zero when the suffix
     *          character is not present.
     */
    scid::core::uint
    strTrimSuffix(char* target, char suffixChar);
    /** Removes unknown month/day suffixes from a @c yyyy.mm.dd date string. */
    void
    strTrimDate(char* str);
    /** Removes Scid board-mark and arrow codes from a comment string. */
    void
    strTrimMarkCodes(char* str);
    /** Removes simple HTML-like tags from @p str in place. */
    void
    strTrimMarkup(char* str);
    /** Returns the first non-whitespace character in @p str. */
    const char*
    strFirstWord(const char* str);
    /** Returns the start of the word after the current word in @p str. */
    const char*
    strNextWord(const char* str);

    /** Returns an empty string for one item, otherwise @c "s". */
    inline const char*
    strPlural(
        scid::core::uint x)
    {
        return (x == 1 ? "" : "s");
    }

    /** Returns true when @p str is empty, @c "?", or @c "-". */
    bool
    strIsUnknownName(const char* str);

    /** Returns true when @p name looks like a single surname token. */
    bool
    strIsSurnameOnly(const char* name);

    /**
     * Parses a permissive true/false value from @p str.
     *
     * Values are matched case-insensitively against @c true, @c yes, @c on,
     * @c 1, @c ja, @c si, @c oui, and their false counterparts @c false, @c no,
     * @c off, and @c 0.  Prefixes are accepted in either direction, but ambiguous
     * or unknown text returns false.
     */
    bool
    strGetBoolean(const char* str);

    /**
     * Extracts a signed base-10 integer from @p str.
     *
     * Non-numeric strings return zero, following @c strtol behaviour.
     */
    inline int
    strGetInteger(
        const char* str)
    {
        return std::strtol(str, NULL, 10);
    }

    /**
     * Extracts an unsigned base-10 integer from @p str.
     *
     * Non-numeric strings return zero, following @c strtoul behaviour.
     */
    inline uint32_t
    strGetUnsigned(
        const char* str)
    {
        ASSERT(str != NULL);
        return static_cast<uint32_t>(std::strtoul(str, NULL, 10));
    }

    /** Case-insensitive string comparison using unsigned-character input. */
    inline int
    strCaseCompare(
        const char* str1,
        const char* str2)
    {
        ASSERT(str1 != NULL && str2 != NULL);
        const unsigned char* s1 = reinterpret_cast<const unsigned char*>(str1);
        const unsigned char* s2 = reinterpret_cast<const unsigned char*>(str2);
        int c1, c2;
        do
        {
            c1 = tolower(*s1++);
            c2 = tolower(*s2++);
            if (c1 == '\0')
                break;
        } while (c1 == c2);

        return c1 - c2;
    }

    /**
     * Compares round strings by leading unsigned number, then by text.
     *
     * This keeps round labels such as @c "2" before @c "10" while preserving a
     * deterministic order for suffixes like @c "2.1".
     */
    inline int
    strCompareRound(
        const char* str1,
        const char* str2)
    {
        ASSERT(str1 != NULL && str2 != NULL);
        uint32_t a = strGetUnsigned(str1);
        uint32_t b = strGetUnsigned(str2);
        if (a == b)
            return strCaseCompare(str1, str2);
        return (a < b) ? -1 : 1;
    }

    /** Returns true when two NUL-terminated strings are byte-equal. */
    inline bool
    strEqual(
        const char* str1,
        const char* str2)
    {
        ASSERT(str1 != NULL && str2 != NULL);
        return (std::strcmp(str1, str2) == 0);
    }

    /** Parses @p nResults signed integers from @p str into @p results. */
    void
    strGetIntegers(const char* str, int* results, scid::core::uint nResults);
    /** Parses @p nResults unsigned integers from @p str into @p results. */
    void
    strGetUnsigneds(const char* str, scid::core::uint* results, scid::core::uint nResults);
    /** Parses a PGN-style game result from @p str. */
    scid::core::resultT
    strGetResult(const char* str);

    /** Three-state yes/no flag used by legacy search parsers. */
    typedef scid::core::uint flagT;
    /** No yes/no flag was specified. */
    const flagT FLAG_EMPTY = 0;
    /** Positive yes/no flag. */
    const flagT FLAG_YES = 1;
    /** Negative yes/no flag. */
    const flagT FLAG_NO = 2;
    /** Both yes and no are accepted. */
    const flagT FLAG_BOTH = 3;
    /** Returns true when @p t includes @c FLAG_YES. */
    inline bool
    flag_Yes(
        flagT t)
    {
        return (t & FLAG_YES);
    }
    /** Returns true when @p t includes @c FLAG_NO. */
    inline bool
    flag_No(
        flagT t)
    {
        return (t & FLAG_NO);
    }
    /**
     * Parses a legacy yes/no flag from the first character of @p str.
     *
     * True-like values return @c FLAG_YES, false-like values return @c FLAG_NO,
     * @c B/@c b/@c 2 return @c FLAG_BOTH, and unknown values return
     * @c FLAG_EMPTY.
     */
    flagT
    strGetFlag(const char* str);

    /** Parses a coordinate square such as @c "a2", or returns @c NULL_SQUARE. */
    scid::core::squareT
    strGetSquare(const char* str);

    /** Trims @p target at the last dot, returning the number of removed chars. */
    inline scid::core::uint
    strTrimFileSuffix(
        char* target)
    {
        return strTrimSuffix(target, '.');
    }

    /** Returns the last dot in @p target, or null when there is no suffix. */
    inline const char*
    strFileSuffix(
        const char* target)
    {
        return strLastChar(target, '.');
    }

    /**
     * Finds a unique match in a null-terminated string table.
     *
     * Exact matches always win.  When @p exact is false, a unique abbreviation is
     * also accepted; ambiguous or missing matches return -1.
     */
    int
    strUniqueExactMatch(const char* keyStr, const char** strTable, bool exact);

    /** Finds a unique exact or abbreviated match in @p strTable. */
    inline int
    strUniqueMatch(
        const char* keyStr,
        const char** strTable)
    {
        return strUniqueExactMatch(keyStr, strTable, false);
    }
    /** Finds only a complete match in @p strTable. */
    inline int
    strExactMatch(
        const char* keyStr,
        const char** strTable)
    {
        return strUniqueExactMatch(keyStr, strTable, true);
    }

    /** Returns true when @p str contains @p ch. */
    inline bool
    strContainsChar(
        const char* str,
        char ch)
    {
        while (*str)
        {
            if (*str == ch)
            {
                return true;
            }
            str++;
        }
        return false;
    }

    /**
     * Legacy byte comparison used for Scid namebase compatibility.
     *
     * Avoid this for new ordering code.  It intentionally depends on signed
     * @c char conversion behaviour to match historical Scid name sorting.  Use
     * @c strcmp or @c strCaseCompare() when compatibility with old namebase
     * ordering is not required.
     */
    inline int
    strCompare(
        const char* s1,
        const char* s2)
    {
        ASSERT(s1 != NULL && s2 != NULL);
        while (1)
        {
            if (*s1 != *s2)
            {
                return ((int)*s1) - ((int)*s2);
            }
            if (*s1 == 0)
                break;
            s1++;
            s2++;
        }
        return 0;
    }

    /** Copies @p original into @p target, including the trailing NUL byte. */
    inline void
    strCopy(
        char* target,
        const char* original)
    {
        ASSERT(target != NULL && original != NULL);
        while (*original != 0)
        {
            *target = *original;
            target++;
            original++;
        }
        *target = 0;
    }

    /** Returns the length of the byte-exact common prefix of @p s1 and @p s2. */
    inline scid::core::uint
    strPrefix(
        const char* s1,
        const char* s2)
    {
        ASSERT(s1 != NULL && s2 != NULL);
        scid::core::uint count = 0;
        while (*s1 == *s2)
        {
            if (*s1 == 0)
            { // seen end of string, strings are identical
                return count;
            }
            count++;
            s1++;
            s2++;
        }
        return count;
    }

    /** Returns true when @p prefix is a byte-exact prefix of @p longStr. */
    inline bool
    strIsPrefix(
        const char* prefix,
        const char* longStr)
    {
        while (*prefix)
        {
            if (*longStr == 0)
            {
                return false;
            }
            if (*prefix != *longStr)
            {
                return false;
            }
            prefix++;
            longStr++;
        }
        return true;
    }

    /** Returns true when @p prefix is a case-insensitive prefix of @p longStr. */
    inline bool
    strIsCasePrefix(
        const char* prefix,
        const char* longStr)
    {
        typedef unsigned char U;
        while (*prefix)
        {
            if (*longStr == 0)
            {
                return false;
            }
            if (tolower(U(*prefix)) != tolower(U(*longStr)))
            {
                return false;
            }
            prefix++;
            longStr++;
        }
        return true;
    }

    /**
     * Returns true when @p prefix matches the start of @p longStr, ignoring case
     * and ASCII space characters.
     *
     * For example, @c strIsAlphaPrefix("smith,j", "Smith, John") is true.
     */
    inline bool
    strIsAlphaPrefix(
        const char* prefix,
        const char* longStr)
    {
        typedef unsigned char U;
        while (*prefix)
        {
            while (*prefix == ' ')
            {
                prefix++;
            }
            while (*longStr == ' ')
            {
                longStr++;
            }
            if (*longStr == 0)
            {
                return false;
            }
            if (tolower(U(*prefix)) != tolower(U(*longStr)))
            {
                return false;
            }
            prefix++;
            longStr++;
        }
        return true;
    }

    /** Returns true when @p longStr contains the byte-exact substring @p keyStr. */
    inline bool
    strContains(
        const char* longStr,
        const char* keyStr)
    {
        while (*longStr)
        {
            if (strIsPrefix(keyStr, longStr))
            {
                return true;
            }
            longStr++;
        }
        return false;
    }

    /**
     * Returns true when @p longStr contains @p keyStr, ignoring case and ASCII
     * space characters.
     *
     * For example, @c strAlphaContains("Smith, John", "th,j") is true.
     */
    inline bool
    strAlphaContains(
        const char* longStr,
        const char* keyStr)
    {
        while (*longStr)
        {
            if (strIsAlphaPrefix(keyStr, longStr))
            {
                return true;
            }
            longStr++;
        }
        return false;
    }

    /** Returns the length of @p str, excluding the trailing NUL byte. */
    inline scid::core::uint
    strLength(
        const char* str)
    {
        ASSERT(str != NULL);
        scid::core::uint len = 0;
        while (*str != 0)
        {
            len++;
            str++;
        }
        return len;
    }

    /**
     * Removes trailing characters from @p target while they occur in @p trimChars.
     *
     * @p nTrimCh is the number of characters in @p trimChars.  For example,
     * trimming @c "abcyzyz" with @c "yz" leaves @c "abc".
     */
    inline void
    strTrimRight(
        char* target,
        const char* trimChars,
        size_t nTrimCh)
    {
        const char* endTrim = trimChars + nTrimCh;
        size_t iCh = strlen(target);
        for (; iCh > 0; --iCh)
        {
            if (std::find(trimChars, endTrim, target[iCh - 1]) == endTrim)
                break;
        }
        target[iCh] = '\0';
    }
    /** Removes trailing ASCII whitespace from @p target. */
    inline void
    strTrimRight(
        char* target)
    {
        return strTrimRight(target, " \t\r\n", 4);
    }

} // namespace scid::database
#endif // #ifdef SCID_MISC_H

//////////////////////////////////////////////////////////////////////
//  EOF: misc.h
//////////////////////////////////////////////////////////////////////
