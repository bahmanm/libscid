#include "scid/eco/code.h"

#include <gtest/gtest.h>
#include <string>

namespace {

std::string toExtended(scid::eco::Code code) {
	scid::eco::String str;
	scid::eco::toExtendedString(code, str);
	return str;
}

std::string toBasic(scid::eco::Code code) {
	scid::eco::String str;
	scid::eco::toBasicString(code, str);
	return str;
}

} // namespace

TEST(EcoCodeTest, ParsesCanonicalAndExtendedCodes) {
	EXPECT_EQ(scid::eco::ECO_None, scid::eco::fromString(""));
	EXPECT_EQ(scid::eco::ECO_None, scid::eco::fromString("Z99"));
	EXPECT_EQ(scid::eco::fromString("A00"), scid::eco::fromString("a00"));
	EXPECT_EQ("A00", toExtended(scid::eco::fromString("A00")));
	EXPECT_EQ("A00a", toExtended(scid::eco::fromString("A00a")));
	EXPECT_EQ("A00a4", toExtended(scid::eco::fromString("A00a4")));
	EXPECT_EQ("E99z4", toExtended(scid::eco::fromString("E99z4")));
}

TEST(EcoCodeTest, ConvertsBasicAndExtendedStrings) {
	auto code = scid::eco::fromString("B91a4");

	EXPECT_EQ("B91", toBasic(code));
	EXPECT_EQ("B91a4", toExtended(code));
}

TEST(EcoCodeTest, ComputesBasicAndLastSubCodes) {
	auto code = scid::eco::fromString("B91a");

	EXPECT_EQ("B91", toExtended(scid::eco::basicCode(code)));
	EXPECT_EQ("B91a4", toExtended(scid::eco::lastSubCode(code)));
	EXPECT_EQ("B91z4", toExtended(scid::eco::lastSubCode(scid::eco::fromString("B91"))));
	EXPECT_EQ(scid::eco::ECO_None, scid::eco::lastSubCode(scid::eco::ECO_None));
}

TEST(EcoCodeTest, ReducesScidExtendedSubcodes) {
	EXPECT_EQ(0, scid::eco::reduce(scid::eco::fromString("A00")));
	EXPECT_EQ(1, scid::eco::reduce(scid::eco::fromString("A00a")));
	EXPECT_EQ(1, scid::eco::reduce(scid::eco::fromString("A00a4")));
	EXPECT_EQ(2, scid::eco::reduce(scid::eco::fromString("A00b")));
	EXPECT_EQ(2700, scid::eco::reduce(scid::eco::fromString("B00")));
}
