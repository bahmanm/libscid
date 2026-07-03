/** @file
 * Packed PGN/database date representation.
 *
 * Dates are stored in a 32-bit value as year, month, and day fields.  Unknown
 * fields are zero, so partial dates such as an unknown month or day can be
 * represented while preserving chronological ordering for known fields.
 */

//////////////////////////////////////////////////////////////////////
//
//  FILE:       date.h
//              Date format and inline date functions.
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    1.9
//
//  Notice:     Copyright (c) 1999  Shane Hudson.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////

#ifndef SCID_DATE_H
#define SCID_DATE_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <stdint.h>

// DATE STORAGE FORMAT:
// In memory, dates are stored in a 32-bit (4-byte) uint, of which only
// the lowest 3 bytes need be used, with the lowest 5 bits for the
// day, the next highest 4 bits for the month and the highest bits for
// the year. This makes date comparisons easy: a bigger date value is
// a more recent date.  If a field is unknown, its value is set to zero.
// On disk, the date is stored in 3 bytes.

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  CONSTANTS and MACROS:

namespace scid::core
{

    /** Packed date value used by games, indexes, and statistics. */
    typedef uint32_t dateT;

    /** Unknown date. */
    const dateT ZERO_DATE = 0;

    /** Bit shift of the year field. */
    const uint32_t YEAR_SHIFT = 9;
    /** Bit shift of the month field. */
    const uint32_t MONTH_SHIFT = 5;
    /** Bit shift of the day field. */
    const uint32_t DAY_SHIFT = 0;

    // DAY (31 days) 5 bits (32) , MONTH (12 months) 4 bits (16)

    /** Maximum representable year. */
    const uint32_t YEAR_MAX = 2047; // 2^11 - 1

/** Builds a packed date from numeric year, month, and day fields. */
#define DATE_MAKE(y, m, d)                                                                         \
    (((y) << scid::core::YEAR_SHIFT) | ((m) << scid::core::MONTH_SHIFT) | (d))

    /** Returns the year field, or 0 when unknown. */
    inline uint32_t
    date_GetYear(dateT date)
    {
        return (uint32_t)(date >> YEAR_SHIFT);
    }

    /** Returns the month field, or 0 when unknown. */
    inline uint32_t
    date_GetMonth(dateT date)
    {
        return (uint32_t)((date >> MONTH_SHIFT) & 15);
    }

    /** Returns the day field, or 0 when unknown. */
    inline uint32_t
    date_GetDay(dateT date)
    {
        return (uint32_t)(date & 31);
    }

    /** Returns true when the year, month, or day field is unknown. */
    inline bool
    date_isPartial(dateT date)
    {
        return date_GetYear(date) == 0 || date_GetMonth(date) == 0 || date_GetDay(date) == 0;
    }

    /** Writes a PGN date tag value into @p str.
     *
     * Unknown fields are written as question marks, for example
     * @c ????.??.?? or @c 2024.??.??.  The destination must have room for at
     * least eleven bytes including the terminator.
     */
    inline void
    date_DecodeToString(
        dateT date,
        char* str)
    {
        assert(str != NULL);
        uint32_t year, month, day;

        year = date_GetYear(date);
        month = date_GetMonth(date);
        day = date_GetDay(date);

        if (year == 0)
        {
            *str++ = '?';
            *str++ = '?';
            *str++ = '?';
            *str++ = '?';
        }
        else
        {
            *str++ = '0' + (year / 1000);
            *str++ = '0' + (year % 1000) / 100;
            *str++ = '0' + (year % 100) / 10;
            *str++ = '0' + (year % 10);
        }
        *str++ = '.';

        if (month == 0)
        {
            *str++ = '?';
            *str++ = '?';
        }
        else
        {
            *str++ = '0' + (month / 10);
            *str++ = '0' + (month % 10);
        }
        *str++ = '.';

        if (day == 0)
        {
            *str++ = '?';
            *str++ = '?';
        }
        else
        {
            *str++ = '0' + (day / 10);
            *str++ = '0' + (day % 10);
        }
        *str = 0;
    }

    /** Parses a dotted date string into dateT.
     *
     * This legacy parser accepts numeric fields in @c yyyy.mm.dd order.  Out of
     * range fields stop further precision from being added: an invalid month keeps
     * the parsed year, and an invalid day keeps the parsed year and month.
     */
    inline dateT
    date_EncodeFromString(const char* str)
    {
        // Do checks on str's validity as a date string:
        assert(str != NULL);

        dateT    date;
        uint32_t year, month, day;

        // convert year:
        year = std::strtoul(str, NULL, 10);
        if (year > YEAR_MAX)
        {
            year = 0;
        }
        date = year << YEAR_SHIFT;
        while (*str != 0 && *str != '.')
        {
            str++;
        }
        if (*str == '.')
        {
            str++;
        }

        // convert month:
        month = std::strtoul(str, NULL, 10);
        if (month > 12)
        {
            return date;
        }
        date |= (month << MONTH_SHIFT);
        while (*str != 0 && *str != '.')
        {
            str++;
        }
        if (*str == '.')
        {
            str++;
        }

        // convert day:
        day = std::strtoul(str, NULL, 10);
        if (day > 31)
        {
            return date;
        }
        date |= (day << DAY_SHIFT);

        return date;
    }

    /** Parses a PGN Date tag value.
     *
     * Full @c YYYY.MM.DD values are accepted, as are partial values with unknown
     * month/day fields.  The parser also accepts compact month/day forms such as
     * @c YYYY.M.D.  Invalid or out-of-range fields are treated as unknown.
     *
     * @param str memory containing the tag value.
     * @param len number of bytes in @p str.
     * @returns the parsed date, or ZERO_DATE when the year is missing or invalid.
     */
    inline dateT
    date_parsePGNTag(
        const char* str,
        size_t      len)
    {
        auto is_digit = [](auto v) { return v >= 0 && v <= 9; };

        if (len < 4 || len > 10)
            return {};

        int tmp[10];
        std::transform(str, str + len, tmp, [](unsigned char ch) {
            return ch - static_cast<unsigned char>('0');
        });
        std::fill(tmp + len, tmp + 10, -1);

        uint32_t year = tmp[0] * 1000 + tmp[1] * 100 + tmp[2] * 10 + tmp[3];
        if (!std::all_of(tmp, tmp + 4, is_digit) || year > YEAR_MAX)
            return {};

        uint32_t month = 0;
        if (!is_digit(tmp[4]) && is_digit(tmp[5]))
        {
            if (!is_digit(tmp[6]))
            {
                // Accept the format YYYY.M.DD or YYYY.M.D
                std::rotate(tmp + 5, tmp + 9, tmp + 10);
                tmp[5] = 0;
            }
            month = tmp[5] * 10 + tmp[6];
            if (month > 12)
                month = 0;
        }

        uint32_t day = 0;
        if (!is_digit(tmp[7]) && is_digit(tmp[8]))
        {
            day = is_digit(tmp[9]) ? tmp[8] * 10 + tmp[9] : tmp[8];
            if (day > 31)
                day = 0;
        }

        return (year << YEAR_SHIFT) | (month << MONTH_SHIFT) | (day << DAY_SHIFT);
    }

    /** Parses a PGN Date tag value described by a pointer pair. */
    inline dateT
    date_parsePGNTag(
        std::pair<
            const char*,
            const char*> str)
    {
        return date_parsePGNTag(str.first, std::distance(str.first, str.second));
    }

} // namespace scid::core
#endif // #ifndef SCID_DATE_H

//////////////////////////////////////////////////////////////////////
//  EOF: date.h
//////////////////////////////////////////////////////////////////////
