#include "scid/core/game.h"

#include <gtest/gtest.h>

namespace
{

    TEST(
        CoreGameTest,
        DefaultsToEmptyMetadataAndStandardStart)
    {
        scid::core::Game game;

        EXPECT_TRUE(game.event().empty());
        EXPECT_TRUE(game.site().empty());
        EXPECT_TRUE(game.round().empty());
        EXPECT_TRUE(game.white().name.empty());
        EXPECT_TRUE(game.black().name.empty());
        EXPECT_EQ(scid::core::ZERO_DATE, game.date());
        EXPECT_EQ(scid::core::ZERO_DATE, game.eventDate());
        EXPECT_EQ(scid::core::RESULT_None, game.result());
        EXPECT_TRUE(game.eco().empty());
        EXPECT_TRUE(game.extraTags().empty());
        EXPECT_FALSE(game.hasNonStandardStart());
        EXPECT_EQ(nullptr, game.startPosition());
        EXPECT_EQ(0, game.initialPlyCounter());
        EXPECT_TRUE(game.initialComment().empty());
    }

    TEST(
        CoreGameTest,
        StandardTagsMapToMetadataAndExtraTagsStaySeparate)
    {
        scid::core::Game game;

        game.addTag("Event", "Candidates");
        game.addTag("Site", "Toronto");
        game.addTag("White", "Player A");
        game.addTag("Black", "Player B");
        game.addTag("Round", "7");
        game.addTag("Annotator", "Example");

        EXPECT_EQ("Candidates", game.event());
        EXPECT_EQ("Toronto", game.site());
        EXPECT_EQ("Player A", game.white().name);
        EXPECT_EQ("Player B", game.black().name);
        EXPECT_EQ("7", game.round());

        ASSERT_EQ(1U, game.extraTags().size());
        EXPECT_EQ("Annotator", game.extraTags()[0].first);
        EXPECT_EQ("Example", game.extraTags()[0].second);
        ASSERT_NE(nullptr, game.findExtraTag("Annotator"));
        EXPECT_EQ("Example", *game.findExtraTag("Annotator"));

        game.removeExtraTag("Annotator");
        EXPECT_EQ(nullptr, game.findExtraTag("Annotator"));
        EXPECT_TRUE(game.extraTags().empty());
    }

    TEST(
        CoreGameTest,
        FindOrCreateTagReusesExistingValue)
    {
        scid::core::Game game;
        game.findOrCreateTag("White") = "Player A";
        game.findOrCreateTag("Annotator") = "Example";
        game.findOrCreateTag("Annotator").append(" 2");

        EXPECT_EQ("Player A", game.white().name);
        ASSERT_EQ(1U, game.extraTags().size());
        EXPECT_EQ("Annotator", game.extraTags()[0].first);
        EXPECT_EQ("Example 2", game.extraTags()[0].second);
    }

    TEST(
        CoreGameTest,
        StoresRatingsDatesAndResult)
    {
        scid::core::Game   game;
        scid::core::Player white;
        white.name = "Player A";
        white.rating.value = 2800;
        white.rating.type = scid::core::RATING_Rapid;
        scid::core::Player black;
        black.name = "Player B";
        black.rating.value = 2650;

        game.setWhite(white);
        game.setBlack(black);
        game.setDate(scid::core::date_parsePGNTag("2018.06.11", 10));
        game.setEventDate(scid::core::date_parsePGNTag("2018.06.01", 10));
        game.setResult(scid::core::RESULT_White);
        game.setEco("A01");

        EXPECT_EQ("Player A", game.white().name);
        EXPECT_EQ(2800, game.white().rating.value);
        EXPECT_EQ(scid::core::RATING_Rapid, game.white().rating.type);
        EXPECT_EQ("Player B", game.black().name);
        EXPECT_EQ(2650, game.black().rating.value);
        EXPECT_EQ(scid::core::date_parsePGNTag("2018.06.11", 10), game.date());
        EXPECT_EQ(scid::core::date_parsePGNTag("2018.06.01", 10), game.eventDate());
        EXPECT_EQ(scid::core::RESULT_White, game.result());
        EXPECT_EQ("1-0", game.resultString());
        EXPECT_EQ("A01", game.eco());
        EXPECT_EQ(2725, game.averageRating());
        EXPECT_EQ(game.date(), game.header().event.date);
        EXPECT_EQ(game.white().rating.value, game.header().white.rating.value);
    }

    TEST(
        CoreGameTest,
        SetsPlayerNamesAndRatingsDirectly)
    {
        scid::core::Game game;

        game.setWhiteName("Player A");
        game.setBlackName("Player B");
        game.setWhiteRating({2800, scid::core::RATING_Rapid});
        game.setBlackRating({0, scid::core::RATING_Elo});

        EXPECT_EQ("Player A", game.white().name);
        EXPECT_EQ("Player B", game.black().name);
        EXPECT_EQ(2800, game.white().rating.value);
        EXPECT_EQ(scid::core::RATING_Rapid, game.white().rating.type);
        EXPECT_EQ(0, game.averageRating());

        game.setWhiteRating({2500, static_cast<scid::core::ratingTypeT>(8)});
        EXPECT_EQ(2500, game.white().rating.value);
        EXPECT_EQ(scid::core::RATING_Elo, game.white().rating.type);
    }

    TEST(
        CoreGameTest,
        SetStartFenStoresNonStandardStartPosition)
    {
        const char*      fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
        scid::core::Game game;

        ASSERT_EQ(scid::core::OK, game.setStartFen(fen));

        EXPECT_TRUE(game.hasNonStandardStart());
        ASSERT_NE(nullptr, game.startPosition());
        EXPECT_EQ(48, game.initialPlyCounter());

        char printed[256];
        ASSERT_TRUE(game.hasNonStandardStart(printed, sizeof(printed)));
        EXPECT_STREQ(fen, printed);
    }

    TEST(
        CoreGameTest,
        InvalidStartFenLeavesExistingStartPositionUnchanged)
    {
        const char*      fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
        scid::core::Game game;
        ASSERT_EQ(scid::core::OK, game.setStartFen(fen));

        EXPECT_NE(scid::core::OK, game.setStartFen("invalid"));

        char printed[256];
        ASSERT_TRUE(game.hasNonStandardStart(printed, sizeof(printed)));
        EXPECT_STREQ(fen, printed);
    }

    TEST(
        CoreGameTest,
        AppendsMainlineMovesWithMetadataAndVariations)
    {
        scid::core::Game game;

        auto& move = game.appendMainlineMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});
        move.san = "e4";
        move.metadata.comment = "Best by test";
        move.metadata.nags.push_back(scid::core::Nag::GoodMove);
        auto& childVariation = move.addVariation("Alternative line");
        auto& childMove =
            childVariation.line.appendMove({scid::core::D2, scid::core::D4, scid::core::EMPTY});
        childMove.san = "d4";

        ASSERT_EQ(1U, game.movetext().mainline.moves.size());
        EXPECT_EQ(1U, game.mainlineHalfMoveCount());
        auto const& savedMove = game.movetext().mainline.moves[0];
        EXPECT_EQ(scid::core::E2, savedMove.spec.from);
        EXPECT_EQ(scid::core::E4, savedMove.spec.to);
        EXPECT_EQ(scid::core::EMPTY, savedMove.spec.promotion);
        EXPECT_EQ("e4", savedMove.san);
        EXPECT_EQ("Best by test", savedMove.metadata.comment);
        ASSERT_EQ(1U, savedMove.metadata.nags.size());
        EXPECT_EQ(scid::core::Nag::GoodMove, savedMove.metadata.nags[0]);
        ASSERT_EQ(1U, savedMove.childVariations.size());
        EXPECT_EQ("Alternative line", savedMove.childVariations[0].initialComment);
        ASSERT_EQ(1U, savedMove.childVariations[0].line.moves.size());
        EXPECT_EQ(scid::core::D4, savedMove.childVariations[0].line.moves[0].spec.to);
    }

    TEST(
        CoreGameTest,
        MoveActionFormatsLongNotation)
    {
        EXPECT_EQ(
            "e2e4", (scid::core::MoveSpec{scid::core::E2, scid::core::E4, scid::core::EMPTY})
                        .longNotation());
        EXPECT_EQ(
            "a7a8q", (scid::core::MoveSpec{scid::core::A7, scid::core::A8, scid::core::QUEEN})
                         .longNotation());

        scid::core::MoveSpec nullMove{scid::core::E1, scid::core::E1, scid::core::EMPTY};
        EXPECT_TRUE(nullMove.isNull());
        EXPECT_EQ("0000", nullMove.longNotation());
    }

    TEST(
        CoreGameTest,
        ClearMovetextLeavesHeaderAndStartPositionIntact)
    {
        scid::core::Game game;
        game.setEvent("Candidates");
        ASSERT_EQ(scid::core::OK, game.setStartFen("8/K7/8/8/7k/8/8/8 w - - 45 25"));
        game.setInitialComment("Before the first move");
        game.appendMainlineMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});

        game.clearMovetext();

        EXPECT_EQ("Candidates", game.event());
        EXPECT_TRUE(game.hasNonStandardStart());
        EXPECT_TRUE(game.initialComment().empty());
        EXPECT_TRUE(game.movetext().mainline.moves.empty());
        EXPECT_EQ(0U, game.mainlineHalfMoveCount());
    }

    TEST(
        CoreGameTest,
        StripMovetextRemovesCommentsAndNagsButKeepsMoves)
    {
        scid::core::Game game;
        game.setInitialComment("Before the first move");
        auto& move = game.appendMainlineMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});
        move.san = "e4";
        move.metadata.comment = "Best by test";
        move.metadata.nags.push_back(scid::core::Nag::GoodMove);
        auto& variation = move.addVariation("Alternative line");
        auto& childMove =
            variation.line.appendMove({scid::core::D2, scid::core::D4, scid::core::EMPTY});
        childMove.metadata.comment = "Queen pawn";
        childMove.metadata.nags.push_back(scid::core::Nag::InterestingMove);

        game.stripMovetext(false, true, true);

        EXPECT_TRUE(game.initialComment().empty());
        auto const& savedMove = game.movetext().mainline.moves[0];
        EXPECT_EQ("e4", savedMove.san);
        EXPECT_TRUE(savedMove.metadata.comment.empty());
        EXPECT_TRUE(savedMove.metadata.nags.empty());
        ASSERT_EQ(1U, savedMove.childVariations.size());
        EXPECT_TRUE(savedMove.childVariations[0].initialComment.empty());
        auto const& savedChild = savedMove.childVariations[0].line.moves[0];
        EXPECT_TRUE(savedChild.metadata.comment.empty());
        EXPECT_TRUE(savedChild.metadata.nags.empty());
    }

    TEST(
        CoreGameTest,
        StripMovetextRemovesVariationsButKeepsMainlineMetadata)
    {
        scid::core::Game game;
        auto& move = game.appendMainlineMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});
        move.metadata.comment = "Best by test";
        move.metadata.nags.push_back(scid::core::Nag::GoodMove);
        move.addVariation("Alternative line")
            .line.appendMove({scid::core::D2, scid::core::D4, scid::core::EMPTY});

        game.stripMovetext(true, false, false);

        auto const& savedMove = game.movetext().mainline.moves[0];
        EXPECT_EQ("Best by test", savedMove.metadata.comment);
        ASSERT_EQ(1U, savedMove.metadata.nags.size());
        EXPECT_EQ(scid::core::Nag::GoodMove, savedMove.metadata.nags[0]);
        EXPECT_TRUE(savedMove.childVariations.empty());
    }

    TEST(
        CoreGameTest,
        GameHeaderClearResetsAllFields)
    {
        scid::core::GameHeader header;
        header.event.name = "Candidates";
        header.event.site = "Toronto";
        header.event.round = "5";
        header.event.date = scid::core::date_parsePGNTag("2024.06.14", 10);
        header.event.eventDate = scid::core::date_parsePGNTag("2024.06.01", 10);
        header.white.name = "Kasparov";
        header.white.rating = {2800, scid::core::RATING_Elo};
        header.black.name = "Karpov";
        header.black.rating = {2750, scid::core::RATING_Elo};
        header.result = scid::core::RESULT_White;
        header.eco = "B12";
        header.tags.emplace_back("Annotator", "Deep Blue");

        header.clear();

        EXPECT_TRUE(header.event.name.empty());
        EXPECT_TRUE(header.event.site.empty());
        EXPECT_TRUE(header.event.round.empty());
        EXPECT_EQ(scid::core::ZERO_DATE, header.event.date);
        EXPECT_EQ(scid::core::ZERO_DATE, header.event.eventDate);
        EXPECT_TRUE(header.white.name.empty());
        EXPECT_EQ(0, header.white.rating.value);
        EXPECT_TRUE(header.black.name.empty());
        EXPECT_EQ(0, header.black.rating.value);
        EXPECT_EQ(scid::core::RESULT_None, header.result);
        EXPECT_TRUE(header.eco.empty());
        EXPECT_TRUE(header.tags.empty());
    }

    TEST(
        CoreGameTest,
        GameHeaderSwapExchangesState)
    {
        scid::core::GameHeader h1;
        h1.event.name = "Event 1";
        h1.event.site = "Site 1";
        h1.white.name = "Player 1";
        h1.white.rating = {2600, scid::core::RATING_Elo};
        h1.result = scid::core::RESULT_White;
        h1.eco = "A01";
        h1.tags.emplace_back("Key1", "Val1");

        scid::core::GameHeader h2;
        h2.event.name = "Event 2";
        h2.event.site = "Site 2";
        h2.white.name = "Player 2";
        h2.white.rating = {2700, scid::core::RATING_Rapid};
        h2.result = scid::core::RESULT_Black;
        h2.eco = "B02";
        h2.tags.emplace_back("Key2", "Val2");

        h1.swap(h2);

        EXPECT_EQ("Event 2", h1.event.name);
        EXPECT_EQ("Site 2", h1.event.site);
        EXPECT_EQ("Player 2", h1.white.name);
        EXPECT_EQ(2700, h1.white.rating.value);
        EXPECT_EQ(scid::core::RATING_Rapid, h1.white.rating.type);
        EXPECT_EQ(scid::core::RESULT_Black, h1.result);
        EXPECT_EQ("B02", h1.eco);
        ASSERT_EQ(1U, h1.tags.size());
        EXPECT_EQ("Key2", h1.tags[0].first);

        EXPECT_EQ("Event 1", h2.event.name);
        EXPECT_EQ("Site 1", h2.event.site);
        EXPECT_EQ("Player 1", h2.white.name);
        EXPECT_EQ(2600, h2.white.rating.value);
        EXPECT_EQ(scid::core::RATING_Elo, h2.white.rating.type);
        EXPECT_EQ(scid::core::RESULT_White, h2.result);
        EXPECT_EQ("A01", h2.eco);
        ASSERT_EQ(1U, h2.tags.size());
        EXPECT_EQ("Key1", h2.tags[0].first);

        using std::swap;
        swap(h1, h2);

        EXPECT_EQ("Event 1", h1.event.name);
        EXPECT_EQ("Event 2", h2.event.name);
    }

    TEST(
        CoreGameTest,
        MovetextClearAndSwap)
    {
        scid::core::Movetext m1;
        m1.initialComment = "Intro comment";
        m1.mainline.appendMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});

        EXPECT_FALSE(m1.initialComment.empty());
        EXPECT_EQ(1U, m1.mainline.moves.size());

        m1.clear();

        EXPECT_TRUE(m1.initialComment.empty());
        EXPECT_TRUE(m1.mainline.moves.empty());

        m1.initialComment = "Line A";
        m1.mainline.appendMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});

        scid::core::Movetext m2;
        m2.initialComment = "Line B";
        m2.mainline.appendMove({scid::core::D2, scid::core::D4, scid::core::EMPTY});

        m1.swap(m2);

        EXPECT_EQ("Line B", m1.initialComment);
        ASSERT_EQ(1U, m1.mainline.moves.size());
        EXPECT_EQ(scid::core::D2, m1.mainline.moves[0].spec.from);

        EXPECT_EQ("Line A", m2.initialComment);
        ASSERT_EQ(1U, m2.mainline.moves.size());
        EXPECT_EQ(scid::core::E2, m2.mainline.moves[0].spec.from);

        using std::swap;
        swap(m1, m2);

        EXPECT_EQ("Line A", m1.initialComment);
        EXPECT_EQ("Line B", m2.initialComment);
    }

    TEST(
        CoreGameTest,
        GameClearResetsAllSubObjects)
    {
        scid::core::Game game;
        game.setEvent("World Championship");
        game.setWhiteName("Capablanca");
        ASSERT_EQ(scid::core::OK, game.setStartFen("8/K7/8/8/7k/8/8/8 w - - 45 25"));
        game.setInitialComment("Game comment");
        game.appendMainlineMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});

        EXPECT_FALSE(game.event().empty());
        EXPECT_TRUE(game.hasNonStandardStart());
        EXPECT_NE(nullptr, game.startPosition());
        EXPECT_EQ(1U, game.mainlineHalfMoveCount());

        game.clear();

        EXPECT_TRUE(game.event().empty());
        EXPECT_TRUE(game.white().name.empty());
        EXPECT_FALSE(game.hasNonStandardStart());
        EXPECT_EQ(nullptr, game.startPosition());
        EXPECT_TRUE(game.initialComment().empty());
        EXPECT_EQ(0U, game.mainlineHalfMoveCount());
    }

    TEST(
        CoreGameTest,
        GameSwapExchangesFullState)
    {
        scid::core::Game game1;
        game1.setEvent("London 1851");
        game1.setWhiteName("Anderssen");
        game1.appendMainlineMove({scid::core::E2, scid::core::E4, scid::core::EMPTY});

        scid::core::Game game2;
        game2.setEvent("Reykjavik 1972");
        game2.setWhiteName("Fischer");
        ASSERT_EQ(scid::core::OK, game2.setStartFen("8/K7/8/8/7k/8/8/8 w - - 45 25"));
        game2.appendMainlineMove({scid::core::D2, scid::core::D4, scid::core::EMPTY});

        game1.swap(game2);

        EXPECT_EQ("Reykjavik 1972", game1.event());
        EXPECT_EQ("Fischer", game1.white().name);
        EXPECT_TRUE(game1.hasNonStandardStart());
        ASSERT_NE(nullptr, game1.startPosition());
        ASSERT_EQ(1U, game1.mainlineHalfMoveCount());
        EXPECT_EQ(scid::core::D2, game1.movetext().mainline.moves[0].spec.from);

        EXPECT_EQ("London 1851", game2.event());
        EXPECT_EQ("Anderssen", game2.white().name);
        EXPECT_FALSE(game2.hasNonStandardStart());
        EXPECT_EQ(nullptr, game2.startPosition());
        ASSERT_EQ(1U, game2.mainlineHalfMoveCount());
        EXPECT_EQ(scid::core::E2, game2.movetext().mainline.moves[0].spec.from);

        using std::swap;
        swap(game1, game2);

        EXPECT_EQ("London 1851", game1.event());
        EXPECT_EQ("Reykjavik 1972", game2.event());
    }

} // namespace
