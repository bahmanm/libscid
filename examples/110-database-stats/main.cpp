#include <scid/core/date.h>
#include <scid/core/error.h>
#include <scid/core/game_result.h>
#include <scid/database/scidbase.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace {

constexpr int ctest_skip = 77;

bool has_scid5_files( const std::filesystem::path& database )
{
    for( const auto extension : { ".si5", ".sg5", ".sn5" } ) {
        if( !std::filesystem::exists( database.string() + extension ) ) {
            return false;
        }
    }

    return true;
}

std::string date_to_string( scid::core::dateT date )
{
    char text[ 16 ] = {};
    scid::core::date_DecodeToString( date, text );
    return text;
}

} // namespace

int main( int argc, char** argv )
{
    const auto database = std::filesystem::path{
        argc > 1 ? argv[ 1 ] : LIBSCID_EXAMPLE_DATABASE,
    };

    if( !has_scid5_files( database ) ) {
        std::cerr << "missing SCID5 database: " << database
                  << ".{si5,sg5,sn5}\n";
        return ctest_skip;
    }

    scid::database::scidBaseT db;
    const auto err = db.open(
        "SCID5",
        scid::database::FMODE_ReadOnly,
        database.string().c_str() );

    if( err != scid::core::OK ) {
        std::cerr << "could not open database, error " << err << '\n';
        return 1;
    }
    if( db.numGames() == 0 ) {
        return 1;
    }

    const auto& stats = db.getStats();
    const auto* eco_stats = stats.getEcoStats( "" );

    const auto result_total =
        stats.nResults[ scid::core::RESULT_None ]
        + stats.nResults[ scid::core::RESULT_White ]
        + stats.nResults[ scid::core::RESULT_Black ]
        + stats.nResults[ scid::core::RESULT_Draw ];

    if( result_total != db.numGames() ) {
        return 1;
    }
    if( stats.nYears == 0 || stats.minDate > stats.maxDate ) {
        return 1;
    }
    if( eco_stats == nullptr ) {
        return 1;
    }
    if( stats.nRatings > 0 && stats.minRating > stats.maxRating ) {
        return 1;
    }

    const auto average_year = stats.nYears == 0
        ? 0
        : static_cast<unsigned>( stats.sumYears / stats.nYears );
    const auto average_rating = stats.nRatings == 0
        ? 0
        : static_cast<unsigned>( stats.sumRatings / stats.nRatings );

    std::cout << "Games: " << db.numGames() << '\n'
              << "Dates: " << date_to_string( stats.minDate )
              << " .. " << date_to_string( stats.maxDate ) << '\n'
              << "Average year: " << average_year << '\n'
              << "Results: "
              << scid::core::RESULT_LONGSTR[ scid::core::RESULT_White ]
              << '=' << stats.nResults[ scid::core::RESULT_White ] << ' '
              << scid::core::RESULT_LONGSTR[ scid::core::RESULT_Draw ]
              << '=' << stats.nResults[ scid::core::RESULT_Draw ] << ' '
              << scid::core::RESULT_LONGSTR[ scid::core::RESULT_Black ]
              << '=' << stats.nResults[ scid::core::RESULT_Black ] << ' '
              << "*=" << stats.nResults[ scid::core::RESULT_None ] << '\n'
              << "Rated players: " << stats.nRatings << '\n'
              << "Average rating: " << average_rating << '\n'
              << "ECO-tagged games: " << eco_stats->count << '\n';

    return 0;
}
