#include <scid/core/date.h>
#include <scid/core/error.h>
#include <scid/core/game.h>
#include <scid/core/game_cursor.h>
#include <scid/core/game_result.h>
#include <scid/database/scidbase.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{

    constexpr int ctest_skip = 77;
    constexpr auto white_name = "Lasker, Emanuel";
    constexpr auto black_name = "Capablanca, Jose Raul";

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

    std::string
    date_to_string(
        scid::core::dateT date)
    {
        char text[16] = {};
        scid::core::date_DecodeToString(date, text);
        return text;
    }

    std::optional<scid::database::gamenumT>
    find_lasker_capablanca(
        const scid::database::scidBaseT& db)
    {
        for (scid::database::gamenumT game_id = 0; game_id < db.numGames(); ++game_id)
        {
            const auto tags = db.tagRoster(game_id);
            if (std::string_view(tags.white) == white_name &&
                std::string_view(tags.black) == black_name)
            {
                return game_id;
            }
        }

        return std::nullopt;
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
    const auto open_err =
        db.open("SCID5", scid::database::FMODE_ReadOnly, database.string().c_str());

    if (open_err != scid::core::OK)
    {
        std::cerr << "could not open database, error " << open_err << '\n';
        return 1;
    }

    const auto target_game = find_lasker_capablanca(db);
    if (!target_game)
    {
        std::cerr << "could not find " << white_name << " - " << black_name << '\n';
        return 1;
    }

    scid::core::Game game;
    std::array<char, 22> scid_flags = {};
    if (db.loadGame(*target_game, game, scid_flags.data(), scid_flags.size()) != scid::core::OK)
    {
        return 1;
    }

    scid::core::GameCursor cursor(game);
    if (!cursor.toPly(8))
    {
        return 1;
    }

    const auto position = cursor.currentPosition();
    if (!position)
    {
        return 1;
    }

    const auto filter_id = db.newFilter();
    auto filter = db.getFilter(filter_id);

    if (!db.setPositionSearchFilter(*position, filter, scid::database::Progress()))
    {
        db.deleteFilter(filter_id.c_str());
        return 1;
    }
    if (filter.size() == 0 || filter.get(*target_game) == 0)
    {
        db.deleteFilter(filter_id.c_str());
        return 1;
    }

    constexpr auto sort_by_date = "d+";
    if (!db.createSortCache(sort_by_date))
    {
        db.deleteFilter(filter_id.c_str());
        return 1;
    }

    const auto row_count = std::min<std::size_t>(filter.size(), 5);
    std::vector<scid::database::gamenumT> rows(row_count);
    const auto listed = db.listGames(sort_by_date, 0, rows.size(), filter, rows.data());

    db.releaseSortCache(sort_by_date);

    char fen[128] = {};
    position->PrintFEN(fen, sizeof(fen));

    std::cout << "Source: " << white_name << " - " << black_name << '\n'
              << "Ply: " << cursor.ply() << '\n'
              << "Position: " << fen << '\n'
              << "Matches: " << filter.size() << '\n';

    for (std::size_t row = 0; row < listed; ++row)
    {
        const auto game_id = rows[row];
        const auto info = db.gameInfo(game_id);
        const auto tags = db.tagRoster(game_id);

        std::cout << '#' << game_id + 1 << ' ' << date_to_string(info.date) << ' ' << tags.white
                  << " - " << tags.black << ' ' << scid::core::RESULT_LONGSTR[info.result] << '\n';
    }

    db.deleteFilter(filter_id.c_str());

    return listed == row_count ? 0 : 1;
}
