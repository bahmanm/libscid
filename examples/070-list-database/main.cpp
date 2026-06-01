#include <scid/core/date.h>
#include <scid/core/error.h>
#include <scid/core/game_result.h>
#include <scid/database/scidbase.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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

    constexpr auto sort_by_date = "d+";
    if( !db.createSortCache( sort_by_date ) ) {
        return 1;
    }

    const auto row_count = std::min<std::size_t>( db.numGames(), 5 );
    std::vector<scid::database::gamenumT> rows( row_count );
    const auto listed = db.listGames(
        sort_by_date,
        0,
        rows.size(),
        db.defaultFilter(),
        rows.data() );

    db.releaseSortCache( sort_by_date );

    if( listed != row_count ) {
        return 1;
    }

    scid::core::dateT previous_date = scid::core::ZERO_DATE;
    std::cout << "Games: " << db.numGames() << '\n';

    for( std::size_t row = 0; row < listed; ++row ) {
        const auto game_id = rows[ row ];
        const auto info = db.gameInfo( game_id );
        const auto tags = db.tagRoster( game_id );

        if( row > 0 && info.date < previous_date ) {
            return 1;
        }
        previous_date = info.date;

        std::cout << '#' << game_id + 1 << ' '
                  << date_to_string( info.date ) << ' '
                  << tags.white << " - " << tags.black << ' '
                  << scid::core::RESULT_LONGSTR[ info.result ] << '\n';
    }

    return 0;
}
