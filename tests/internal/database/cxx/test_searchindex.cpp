#include "scid/core/game.h"
#include "scid/database/misc.h"
#include "scid/database/scidbase.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace scid::database
{
    scid::core::errorT
    search_index(
        const scidBaseT* base,
        HFilter&         filter,
        int              argc,
        const char**     argv,
        const Progress&  progress);
}

namespace
{

    scid::core::Game
    makeGame(
        std::string               white,
        std::string               black,
        scid::core::ratingT       whiteElo,
        scid::core::ratingT       blackElo,
        scid::core::resultT const result)
    {
        scid::core::Game game;
        game.setWhiteName(white);
        game.setBlackName(black);
        game.setWhiteRating({whiteElo, scid::core::RATING_Elo});
        game.setBlackRating({blackElo, scid::core::RATING_Elo});
        game.setResult(result);
        return game;
    }

    std::vector<scid::database::gamenumT>
    includedGames(scid::database::HFilter const& filter)
    {
        std::vector<scid::database::gamenumT> result;
        for (auto gameNumber : *filter)
        {
            result.push_back(gameNumber);
        }
        return result;
    }

    void
    expectSearch(
        scid::database::scidBaseT&                   database,
        std::vector<const char*>                     args,
        std::vector<scid::database::gamenumT> const& expected)
    {
        auto filterId = database.newFilter();
        auto filter = database.getFilter(filterId);

        EXPECT_EQ(
            scid::core::OK, scid::database::search_index(
                                &database, filter, static_cast<int>(args.size()), args.data(),
                                scid::database::Progress()));
        EXPECT_EQ(expected, includedGames(filter));
    }

} // namespace

TEST(
    SearchIndexTest,
    FiltersGameNumberAndRatingRanges)
{
    scid::database::scidBaseT database;
    ASSERT_EQ(scid::core::OK, database.open("MEMORY", scid::database::FMODE_Create, "Memory"));

    ASSERT_EQ(
        scid::core::OK,
        database.addGame(makeGame("A", "B", 2500, 2400, scid::core::RESULT_White), ""));
    ASSERT_EQ(
        scid::core::OK,
        database.addGame(makeGame("C", "D", 2700, 2650, scid::core::RESULT_Draw), ""));
    ASSERT_EQ(
        scid::core::OK,
        database.addGame(makeGame("E", "F", 2200, 2600, scid::core::RESULT_Black), ""));
    ASSERT_EQ(
        scid::core::OK,
        database.addGame(makeGame("G", "H", 2800, 2300, scid::core::RESULT_White), ""));

    expectSearch(database, {"-gnum", "2 3"}, {1, 2});
    expectSearch(database, {"-gnum", "-2 -1"}, {2, 3});
    expectSearch(database, {"-elo", "2400 2700"}, {0, 1});
    expectSearch(database, {"-welo", "2600 2800"}, {1, 3});
    expectSearch(database, {"-belo", "2400 2600"}, {0, 2});
    expectSearch(database, {"-delo", "-100 100"}, {0, 1});
}
