#include "scid/core/date.h"
#include "scid/core/error.h"
#include "scid/database/misc.h"
#include "scid/database/namebase.h"
#include "scid/spelling/spelling.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace
{

    std::filesystem::path
    testFilePath(std::string_view stem)
    {
        auto path = std::filesystem::temp_directory_path();
        path /= std::string(stem) + "_" +
                std::to_string(::testing::UnitTest::GetInstance()->random_seed()) + ".ssp";
        return path;
    }


    void
    writeFile(
        const std::filesystem::path& path,
        std::string_view             contents)
    {
        std::ofstream out(path);
        ASSERT_TRUE(out.good());
        out << contents;
    }


    class SpellingTest : public ::testing::Test
    {
        protected:
            std::filesystem::path path_;

            void
            SetUp() override
            {
                path_ =
                    testFilePath(::testing::UnitTest::GetInstance()->current_test_info()->name());
            }


            void
            TearDown() override
            {
                std::error_code ec;
                std::filesystem::remove(path_, ec);
            }


            std::unique_ptr<scid::spelling::SpellChecker>
            load(std::string_view contents)
            {
                writeFile(path_, contents);

                const auto path = path_.string();
                auto [err, spelling] =
                    scid::spelling::SpellChecker::create(path.c_str(), scid::database::Progress());
                EXPECT_EQ(scid::core::OK, err);
                EXPECT_NE(nullptr, spelling);
                return std::move(spelling);
            }
    };

    constexpr std::string_view kSpellingFile = R"ssp(
@PLAYER ", .-"
Polgar, Judit #GM+W HUN [2735] 1976
= J. Polgar
%Bio Youngest player to enter the FIDE top 100
%Elo 2009:2693,2687,?,2701,2710
Kasparov, Garry #GM RUS [2851] 1963
@SITE
New York
= New-York
)ssp";

} // namespace

TEST_F(
    SpellingTest,
    LoadsCanonicalNamesAndAliases)
{
    auto spelling = load(kSpellingFile);
    ASSERT_NE(nullptr, spelling);

    EXPECT_EQ(2u, spelling->numCorrectNames(scid::database::NAME_PLAYER));
    EXPECT_EQ(1u, spelling->numCorrectNames(scid::database::NAME_SITE));

    auto alias = spelling->find(scid::database::NAME_PLAYER, "J. Polgar");
    ASSERT_EQ(1u, alias.size());
    EXPECT_STREQ("Polgar, Judit", alias[0]);

    auto surnameLast = spelling->find(scid::database::NAME_PLAYER, "Judit Polgar");
    ASSERT_EQ(1u, surnameLast.size());
    EXPECT_STREQ("Polgar, Judit", surnameLast[0]);

    auto site = spelling->find(scid::database::NAME_SITE, "New-York");
    ASSERT_EQ(1u, site.size());
    EXPECT_STREQ("New York", site[0]);
}


TEST_F(
    SpellingTest,
    ReturnsPlayerMetadataAndElo)
{
    auto spelling = load(kSpellingFile);
    ASSERT_NE(nullptr, spelling);

    std::vector<const char*> bio;
    const auto*              info = spelling->getPlayerInfo("Judit Polgar", &bio);
    ASSERT_NE(nullptr, info);

    EXPECT_STREQ("GM", info->getTitle());
    EXPECT_EQ("HUN", info->getLastCountry());
    EXPECT_EQ(2735, info->getPeakRating());
    EXPECT_EQ(scid::core::date_EncodeFromString("1976"), info->getBirthdate());
    EXPECT_EQ(scid::core::ZERO_DATE, info->getDeathdate());
    ASSERT_EQ(1u, bio.size());
    EXPECT_STREQ("Youngest player to enter the FIDE top 100", bio[0]);

    ASSERT_TRUE(spelling->hasEloData());
    const auto* elo = spelling->getPlayerElo("J. Polgar");
    ASSERT_NE(nullptr, elo);
    EXPECT_EQ(2701, elo->getElo(scid::core::date_EncodeFromString("2009.09.01")));
}
