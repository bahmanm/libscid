#include "bytebuf.h"
#include "game_search.h"
#include "game_storage.h"
#include "scid/core/board.h"
#include "scid/core/game.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

namespace
{

    bool
    materialMatches(
        const scid::core::Game&   game,
        scid::database::patternT* patterns,
        std::size_t               patternCount)
    {
        std::vector<scid::core::byte> encodedGame;
        scid::database::game_storage::encode(game, "", encodedGame);

        scid::core::byte min[scid::core::MAX_PIECE_TYPES] = {};
        scid::core::byte max[scid::core::MAX_PIECE_TYPES] = {};
        std::fill(std::begin(max), std::end(max), 16);

        auto buf = scid::database::ByteBuffer{encodedGame.data(), encodedGame.size()};
        return scid::database::game_search::materialMatch(
            false, buf, min, max, patterns, patternCount, 0, 0, 1, true, true, -64, 64);
    }

} // namespace

TEST(
    GameSearchTest,
    AggregateInitialisedPatternDefaultsToNegativePolarity)
{
    scid::core::Game game;

    scid::database::patternT noWhiteQueenOnDFile{
        scid::core::WQ, scid::core::NO_RANK, scid::core::D_FYLE};
    EXPECT_FALSE(materialMatches(game, &noWhiteQueenOnDFile, 1));

    noWhiteQueenOnDFile.flag = 1;
    EXPECT_TRUE(materialMatches(game, &noWhiteQueenOnDFile, 1));

    scid::database::patternT noWhiteQueenOnAFile{
        scid::core::WQ, scid::core::NO_RANK, scid::core::A_FYLE};
    EXPECT_TRUE(materialMatches(game, &noWhiteQueenOnAFile, 1));
}
