#include <scid/core/error.h>
#include <scid/core/movelist.h>
#include <scid/core/position.h>

#include <cstring>
#include <iostream>
#include <string>

int
main()
{
    constexpr auto moves = "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6";

    scid::core::Position position;
    position.StdStart();

    std::string san;
    if (position.applyCoordinateMoves(moves, std::strlen(moves), &san) != scid::core::OK)
    {
        std::cerr << "invalid move list\n";
        return 1;
    }

    char fen[128] = {};
    position.PrintFEN(fen, sizeof(fen));

    scid::core::MoveList legal_moves;
    position.GenerateMoves(&legal_moves);

    std::cout << san << '\n';
    std::cout << fen << '\n';
    std::cout << "Legal replies: " << legal_moves.Size() << '\n';

    if (san != "1.e4 e5 2.Nf3 Nc6 3.Bb5 a6")
    {
        return 1;
    }
    if (std::string(fen) != "r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4")
    {
        return 1;
    }
    if (legal_moves.Size() == 0)
    {
        return 1;
    }

    return 0;
}
