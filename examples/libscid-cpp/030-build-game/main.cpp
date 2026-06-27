#include <scid/core/date.h>
#include <scid/core/error.h>
#include <scid/core/game.h>
#include <scid/core/game_result.h>
#include <scid/core/nags.h>
#include <scid/core/pgn/encode.h>
#include <scid/core/position.h>

#include <iostream>
#include <string>

int
main()
{
    constexpr const char* coordinate_moves[] = {
        "e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6",
    };

    scid::core::Game game;
    game.setEvent("Example Game");
    game.setSite("Berlin");
    game.setDate(scid::core::date_EncodeFromString("1892.??.??"));
    game.setRound("?");
    game.setWhiteName("Lasker, Emanuel");
    game.setBlackName("NN");
    game.setResult(scid::core::RESULT_White);

    scid::core::Position position;
    position.StdStart();

    for (const auto move_text : coordinate_moves)
    {
        scid::core::MoveSpec move;
        if (position.readCoordinateMoveSpec(move, move_text, false) != scid::core::OK)
        {
            std::cerr << "invalid move: " << move_text << '\n';
            return 1;
        }

        auto& game_move = game.appendMainlineMove(move);
        if (move_text == std::string("f1b5"))
        {
            game_move.metadata.nags.push_back(scid::core::Nag::GoodMove);
            game_move.metadata.comment = "The Ruy Lopez.";
        }

        if (position.applyMove(move) != scid::core::OK)
        {
            std::cerr << "illegal move: " << move_text << '\n';
            return 1;
        }
    }

    std::string pgn;
    scid::core::pgn::encode(game, pgn);

    std::cout << pgn;

    if (game.mainlineHalfMoveCount() != 6)
    {
        return 1;
    }
    if (pgn.find("[White \"Lasker, Emanuel\"]") == std::string::npos)
    {
        return 1;
    }
    if (pgn.find("Bb5 $1 {The Ruy Lopez.}") == std::string::npos)
    {
        return 1;
    }

    return 0;
}
