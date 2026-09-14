#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "H_ConsoleIO.h"
#include "H_DndExceptions.h"

namespace {

// Drives ConsoleIO from a scripted input string and captures its output.
struct Harness {
    std::istringstream in;
    std::ostringstream out;
    ConsoleIO io;

    explicit Harness(const std::string& script)
        : in(script), out(), io(in, out) {}

    std::string written() const { return out.str(); }
};

} // namespace

// ---------------------------------------------------------------------------
// Validators
// ---------------------------------------------------------------------------

TEST(ValidateTest, IsTextRequiresALetter) {
    EXPECT_TRUE(Validate::isText("Gimli"));
    EXPECT_TRUE(Validate::isText("Level 5 Fighter"));
    EXPECT_FALSE(Validate::isText(""));
    EXPECT_FALSE(Validate::isText("12345"));
    EXPECT_FALSE(Validate::isText("   "));
}

TEST(ValidateTest, IsNameAllowsDigitsAndSymbols) {
    EXPECT_TRUE(Validate::isName("Gimli II"));
    EXPECT_TRUE(Validate::isName("R2-D2"));
    EXPECT_TRUE(Validate::isName("42"));
    EXPECT_FALSE(Validate::isName(""));
    EXPECT_FALSE(Validate::isName("   "));
}

TEST(ValidateTest, IsHitDiceAcceptsOnlyDFollowedByDigits) {
    EXPECT_TRUE(Validate::isHitDice("d6"));
    EXPECT_TRUE(Validate::isHitDice("d12"));
    EXPECT_TRUE(Validate::isHitDice("d100"));
    EXPECT_FALSE(Validate::isHitDice("d"));
    EXPECT_FALSE(Validate::isHitDice("6d"));
    EXPECT_FALSE(Validate::isHitDice("D12"));
    EXPECT_FALSE(Validate::isHitDice("0"));   // the value that corrupted saves
    EXPECT_FALSE(Validate::isHitDice(""));
}

// ---------------------------------------------------------------------------
// readInt / readMenuChoice
// ---------------------------------------------------------------------------

TEST(ConsoleIOTest, ReadIntAcceptsValueInRange) {
    Harness h("5\n");
    EXPECT_EQ(h.io.readInt("Level: ", 1, 20), 5);
}

TEST(ConsoleIOTest, ReadIntAcceptsBoundaries) {
    Harness lo("1\n");
    EXPECT_EQ(lo.io.readInt("n: ", 1, 20), 1);
    Harness hi("20\n");
    EXPECT_EQ(hi.io.readInt("n: ", 1, 20), 20);
}

TEST(ConsoleIOTest, ReadIntRejectsOutOfRangeThenAccepts) {
    Harness h("99\n0\n7\n");
    EXPECT_EQ(h.io.readInt("Level: ", 1, 20), 7);
    EXPECT_NE(h.written().find("Invalid entry"), std::string::npos);
}

// The core bug this layer exists to kill: a non-numeric entry used to leave
// the value at 0 and poison every later read.
TEST(ConsoleIOTest, ReadIntRejectsNonNumericThenRecovers) {
    Harness h("abc\n12\n");
    EXPECT_EQ(h.io.readInt("Level: ", 1, 20), 12);
}

TEST(ConsoleIOTest, ReadIntRejectsTrailingJunk) {
    Harness h("12abc\n8\n");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 8);
}

TEST(ConsoleIOTest, ReadIntRejectsBlankLine) {
    Harness h("\n3\n");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 3);
}

TEST(ConsoleIOTest, ReadIntToleratesSurroundingWhitespace) {
    Harness h("   4   \n");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 4);
}

TEST(ConsoleIOTest, ReadIntHandlesNegativeRange) {
    Harness h("-3\n");
    EXPECT_EQ(h.io.readInt("Adjust: ", -10, 10), -3);
}

TEST(ConsoleIOTest, ReadIntRejectsIntegerOverflow) {
    Harness h("99999999999999999999\n5\n");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 5);
}

TEST(ConsoleIOTest, ReadMenuChoiceAllowsZeroForBack) {
    Harness h("0\n");
    EXPECT_EQ(h.io.readMenuChoice("Choice: ", 6), 0);
}

TEST(ConsoleIOTest, ReadMenuChoiceRejectsAboveMax) {
    Harness h("9\n2\n");
    EXPECT_EQ(h.io.readMenuChoice("Choice: ", 6), 2);
}

// ---------------------------------------------------------------------------
// Text readers
// ---------------------------------------------------------------------------

TEST(ConsoleIOTest, ReadTextRejectsBlankThenAccepts) {
    Harness h("\n   \nGimli\n");
    EXPECT_EQ(h.io.readText("Name: "), "Gimli");
}

TEST(ConsoleIOTest, ReadTextTrimsResult) {
    Harness h("  Gimli  \n");
    EXPECT_EQ(h.io.readText("Name: "), "Gimli");
}

TEST(ConsoleIOTest, ReadTextPreservesInternalSpaces) {
    Harness h("Chaotic Neutral\n");
    EXPECT_EQ(h.io.readText("Alignment: "), "Chaotic Neutral");
}

TEST(ConsoleIOTest, ReadNameAcceptsDigitsWhereTextWouldNot) {
    Harness h("R2-D2\n");
    EXPECT_EQ(h.io.readName("Name: "), "R2-D2");
}

TEST(ConsoleIOTest, ReadHitDiceRejectsBadFormatThenAccepts) {
    Harness h("0\nD12\nd10\n");
    EXPECT_EQ(h.io.readHitDice("Hit dice: "), "d10");
    EXPECT_NE(h.written().find("dN"), std::string::npos);
}

// ---------------------------------------------------------------------------
// readYesNo
// ---------------------------------------------------------------------------

TEST(ConsoleIOTest, ReadYesNoAcceptsVariants) {
    Harness yes("YES\n");
    EXPECT_TRUE(yes.io.readYesNo("? "));
    Harness y("y\n");
    EXPECT_TRUE(y.io.readYesNo("? "));
    Harness no("No\n");
    EXPECT_FALSE(no.io.readYesNo("? "));
    Harness n("n\n");
    EXPECT_FALSE(n.io.readYesNo("? "));
}

TEST(ConsoleIOTest, ReadYesNoRepromptsOnGarbage) {
    Harness h("maybe\ny\n");
    EXPECT_TRUE(h.io.readYesNo("? "));
}

// ---------------------------------------------------------------------------
// EOF handling — a dead stream must terminate, not spin forever
// ---------------------------------------------------------------------------

TEST(ConsoleIOTest, ReadIntThrowsAtEndOfInput) {
    Harness h("");
    EXPECT_THROW(h.io.readInt("n: ", 1, 20), EndOfInput);
}

TEST(ConsoleIOTest, ReadTextThrowsAtEndOfInput) {
    Harness h("");
    EXPECT_THROW(h.io.readText("Name: "), EndOfInput);
}

TEST(ConsoleIOTest, ReadYesNoThrowsAtEndOfInput) {
    Harness h("");
    EXPECT_THROW(h.io.readYesNo("? "), EndOfInput);
}

// Exhausting a stream mid-retry must throw rather than loop on the dead stream.
TEST(ConsoleIOTest, ExhaustedRetriesThrowRatherThanSpin) {
    Harness h("abc\nxyz\n");
    EXPECT_THROW(h.io.readInt("n: ", 1, 20), EndOfInput);
}

TEST(ConsoleIOTest, EndOfInputIsADndException) {
    Harness h("");
    EXPECT_THROW(h.io.readInt("n: ", 1, 20), DndException);
}

// ---------------------------------------------------------------------------
// Sequencing — the leftover-newline bug class
// ---------------------------------------------------------------------------

// Mixing `cin >> n` with getline used to leave a newline behind, so the next
// line read came back empty. Every read is line-based now, so a number
// followed by text reads cleanly.
TEST(ConsoleIOTest, IntFollowedByTextReadsCleanly) {
    Harness h("3\nGimli\n");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 3);
    EXPECT_EQ(h.io.readText("Name: "), "Gimli");
}

TEST(ConsoleIOTest, TextFollowedByIntReadsCleanly) {
    Harness h("Gimli\n3\n");
    EXPECT_EQ(h.io.readText("Name: "), "Gimli");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 3);
}

TEST(ConsoleIOTest, ToleratesCrlfLineEndings) {
    Harness h("5\r\nGimli\r\n");
    EXPECT_EQ(h.io.readInt("n: ", 1, 20), 5);
    EXPECT_EQ(h.io.readText("Name: "), "Gimli");
}

TEST(ConsoleIOTest, PromptIsWrittenToInjectedStream) {
    Harness h("1\n");
    h.io.readInt("Level: ", 1, 20);
    EXPECT_NE(h.written().find("Level: "), std::string::npos);
}

// ---------------------------------------------------------------------------
// readIntFrom — non-contiguous choice sets
// ---------------------------------------------------------------------------

TEST(ConsoleIOTest, ReadIntFromAcceptsAllowedValue) {
    Harness h("20\n");
    EXPECT_EQ(h.io.readIntFrom("Die: ", {4, 6, 8, 10, 12, 20, 100}), 20);
}

TEST(ConsoleIOTest, ReadIntFromRejectsValueOutsideSet) {
    Harness h("7\n12\n");
    EXPECT_EQ(h.io.readIntFrom("Die: ", {4, 6, 8, 10, 12, 20, 100}), 12);
    EXPECT_NE(h.written().find("Choose one of"), std::string::npos);
}

TEST(ConsoleIOTest, ReadIntFromRejectsNonNumeric) {
    Harness h("d20\n20\n");
    EXPECT_EQ(h.io.readIntFrom("Die: ", {4, 20}), 20);
}

TEST(ConsoleIOTest, ReadIntFromThrowsAtEndOfInput) {
    Harness h("");
    EXPECT_THROW(h.io.readIntFrom("Die: ", {4, 6}), EndOfInput);
}

// ---------------------------------------------------------------------------
// readFloat
// ---------------------------------------------------------------------------

TEST(ConsoleIOTest, ReadFloatAcceptsDecimal) {
    Harness h("2.5\n");
    EXPECT_FLOAT_EQ(h.io.readFloat("Weight: ", 0.0f, 100.0f), 2.5f);
}

TEST(ConsoleIOTest, ReadFloatAcceptsWholeNumber) {
    Harness h("3\n");
    EXPECT_FLOAT_EQ(h.io.readFloat("Weight: ", 0.0f, 100.0f), 3.0f);
}

TEST(ConsoleIOTest, ReadFloatRejectsOutOfRangeThenAccepts) {
    Harness h("500\n1.5\n");
    EXPECT_FLOAT_EQ(h.io.readFloat("Weight: ", 0.0f, 100.0f), 1.5f);
}

TEST(ConsoleIOTest, ReadFloatRejectsNonNumeric) {
    Harness h("heavy\n0.5\n");
    EXPECT_FLOAT_EQ(h.io.readFloat("Weight: ", 0.0f, 100.0f), 0.5f);
}

TEST(ConsoleIOTest, ReadFloatThrowsAtEndOfInput) {
    Harness h("");
    EXPECT_THROW(h.io.readFloat("Weight: ", 0.0f, 1.0f), EndOfInput);
}
