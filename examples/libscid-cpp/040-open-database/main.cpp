#include <scid/core/error.h>
#include <scid/database/scidbase.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace
{

    constexpr int ctest_skip = 77;

    bool
    has_scid5_files(
        const std::filesystem::path& database)
    {
        for (const auto extension : {".si5", ".sg5", ".sn5"})
        {
            if (!std::filesystem::exists(database.string() + extension))
            {
                return false;
            }
        }

        return true;
    }

} // namespace

int
main(
    int argc,
    char** argv)
{
    const auto database = std::filesystem::path{
        argc > 1 ? argv[1] : LIBSCID_EXAMPLE_DATABASE,
    };

    if (!has_scid5_files(database))
    {
        std::cerr << "missing SCID5 database: " << database << ".{si5,sg5,sn5}\n";
        return ctest_skip;
    }

    scid::database::scidBaseT db;
    const auto err = db.open("SCID5", scid::database::FMODE_ReadOnly, database.string().c_str());

    if (err != scid::core::OK)
    {
        std::cerr << "could not open database, error " << err << '\n';
        return 1;
    }
    if (!db.isReadOnly() || db.numGames() == 0)
    {
        return 1;
    }

    const auto first_game = static_cast<scid::database::gamenumT>(0);
    const auto tags = db.tagRoster(first_game);
    const auto first_moves = db.moveSAN(first_game, 0, 8);

    std::cout << "Games: " << db.numGames() << '\n'
              << "First: " << tags.white << " - " << tags.black << '\n'
              << "Moves: " << first_moves << '\n';

    if (std::string(tags.white).empty() || std::string(tags.black).empty())
    {
        return 1;
    }
    if (first_moves.empty())
    {
        return 1;
    }

    return 0;
}
