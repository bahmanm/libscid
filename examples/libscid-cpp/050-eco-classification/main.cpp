#include <scid/core/error.h>
#include <scid/core/position.h>
#include <scid/eco/book.h>
#include <scid/eco/code.h>

#include <iostream>
#include <string>
#include <string_view>

namespace
{

    bool
    play(
        scid::core::Position& position,
        std::string_view san)
    {
        scid::core::MoveSpec move;
        return position.parseMoveSpec(move, san) == scid::core::OK &&
               position.applyMove(move) == scid::core::OK;
    }

} // namespace

int
main()
{
    auto [err, book] = scid::eco::Book::load(LIBSCID_EXAMPLE_ECO_FILE);
    if (err != scid::eco::OK)
    {
        std::cerr << "could not load ECO file, error " << err << '\n';
        return 1;
    }

    scid::core::Position position;
    position.StdStart();

    for (const auto san : {"d4", "d5", "c4", "e6"})
    {
        if (!play(position, san))
        {
            std::cerr << "could not play " << san << '\n';
            return 1;
        }
    }

    const auto eco = book.findEcoString(position);
    const auto code = book.findEco(position);

    scid::eco::String code_text;
    scid::eco::toBasicString(code, code_text);

    char fen[128] = {};
    position.PrintFEN(fen, sizeof(fen));

    std::cout << "Position: " << fen << '\n'
              << "ECO: " << eco << '\n'
              << "Code: " << code_text << '\n';

    if (eco != "D30 [Queen's Gambit Declined]")
    {
        return 1;
    }
    if (std::string(code_text) != "D30")
    {
        return 1;
    }

    return 0;
}
