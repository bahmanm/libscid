/** @file
 * Numeric annotation glyphs and PGN annotation conversion helpers.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace scid::core
{

    /** Numeric annotation glyph values used in PGN movetext.
     *
     * The enum names cover the annotation values recognised by libscid.  Values map
     * directly to PGN @c $n NAG codes and may also have a symbolic representation
     * such as @c !, @c ??, or @c D.
     */
    enum class Nag : std::uint8_t
    {
        None = 0,
        GoodMove = 1,
        PoorMove = 2,
        ExcellentMove = 3,
        Blunder = 4,
        InterestingMove = 5,
        DubiousMove = 6,
        OnlyMove = 8,
        Equal = 10,
        Unclear = 13,
        WhiteSlight = 14,
        BlackSlight = 15,
        WhiteClear = 16,
        BlackClear = 17,
        WhiteDecisive = 18,
        BlackDecisive = 19,
        WhiteCrushing = 20,
        BlackCrushing = 21,
        ZugZwang = 22,
        BlackZugZwang = 23,
        MoreRoom = 26,
        DevelopmentAdvantage = 35,
        WithInitiative = 36,
        WithAttack = 40,
        WithBlackAttack = 41,
        Compensation = 44,
        SlightCentre = 48,
        Centre = 50,
        SlightKingSide = 54,
        ModerateKingSide = 56,
        KingSide = 58,
        SlightQueenSide = 60,
        ModerateQueenSide = 62,
        QueenSide = 64,
        SlightCounterPlay = 130,
        BlackSlightCounterPlay = 131,
        CounterPlay = 132,
        BlackCounterPlay = 133,
        DecisiveCounterPlay = 134,
        BlackDecisiveCounterPlay = 135,
        TimeLimit = 136,
        WithIdea = 140,
        BetterIs = 142,
        VariousMoves = 144,
        Comment = 145,
        Novelty = 146,
        WeakPoint = 147,
        Ending = 148,
        File = 149,
        Diagonal = 150,
        BishopPair = 151,
        OppositeBishops = 153,
        SameBishops = 154,
        Etc = 190,
        DoublePawns = 191,
        SeparatedPawns = 192,
        UnitedPawns = 193,
        Diagram = 201,
        See = 210,
        Mate = 211,
        PassedPawn = 212,
        MorePawns = 213,
        With = 214,
        Without = 215
    };

    /** Returns the PGN numeric code for @p nag. */
    constexpr std::uint8_t
    nagCode(Nag nag)
    {
        return std::to_underlying(nag);
    }

    /** Builds a Nag value from a numeric PGN code. */
    constexpr Nag
    nagFromCode(std::uint8_t value)
    {
        return static_cast<Nag>(value);
    }

    /** Largest named Nag code currently recognised by libscid. */
    inline constexpr std::uint8_t maxNagCode = nagCode(Nag::Without);

    /** Formats @p nag as a symbolic annotation when possible, or @c $n.
     *
     * Nag::None formats as an empty string.  When @p asSymbol is true but no symbol
     * is known for the code, numeric @c $n form is used.
     */
    std::string
    nagToString(
        Nag  nag,
        bool asSymbol);
    /** Returns the symbolic form for @p nag, or an empty view if none is known. */
    std::string_view
    nagToSymbol(Nag nag);
    /** Parses symbolic, bare numeric, or @c $n NAG text.
     *
     * Unknown, malformed, or out-of-range text returns Nag::None.
     */
    Nag
    nagFromString(std::string_view text);

} // namespace scid::core
