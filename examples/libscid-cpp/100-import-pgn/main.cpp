#include <scid/core/error.h>
#include <scid/core/game.h>
#include <scid/core/game_result.h>
#include <scid/database/scidbase.h>

#include <array>
#include <iostream>
#include <string>

int main()
{
    scid::database::scidBaseT db;
    if( db.open( "MEMORY", scid::database::FMODE_Create, "Imported PGN" )
        != scid::core::OK ) {
        std::cerr << "could not create in-memory database\n";
        return 1;
    }

    std::string import_errors;
    const auto import_err = db.importGames(
        "PGN",
        LIBSCID_EXAMPLE_PGN_FILE,
        scid::database::Progress(),
        import_errors );

    if( import_err != scid::core::OK ) {
        std::cerr << "could not import PGN, error " << import_err << '\n'
                  << import_errors;
        return 1;
    }
    if( db.numGames() != 2 ) {
        return 1;
    }

    const auto tags = db.tagRoster( 0 );

    scid::core::Game game;
    std::array<char, 22> scid_flags = {};
    if( db.loadGame( 0, game, scid_flags.data(), scid_flags.size() )
        != scid::core::OK ) {
        return 1;
    }

    std::cout << "Imported: " << db.numGames() << '\n'
              << "First: " << tags.white << " - " << tags.black << '\n'
              << "Result: " << scid::core::RESULT_LONGSTR[ game.result() ] << '\n'
              << "Half moves: " << game.mainlineHalfMoveCount() << '\n';

    if( std::string( tags.white ) != "Lasker, Emanuel" ) {
        return 1;
    }
    if( std::string( tags.black ) != "Capablanca, Jose Raul" ) {
        return 1;
    }
    if( game.mainlineHalfMoveCount() != 83 ) {
        return 1;
    }
    if( game.result() != scid::core::RESULT_White ) {
        return 1;
    }

    return 0;
}
