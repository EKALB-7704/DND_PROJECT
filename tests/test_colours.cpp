#include <gtest/gtest.h>
#include "H_Colours.h"

// ── Macro definitions ─────────────────────────────────────────────────────────

TEST(ColoursTest, MacroValuesMatchColourCodes)
{
    EXPECT_EQ(BLACK,         0);
    EXPECT_EQ(BLUE,          1);
    EXPECT_EQ(GREEN,         2);
    EXPECT_EQ(AQUA,          3);
    EXPECT_EQ(RED,           4);
    EXPECT_EQ(PURPLE,        5);
    EXPECT_EQ(YELLOW,        6);
    EXPECT_EQ(WHITE,         7);
    EXPECT_EQ(GRAY,          8);
    EXPECT_EQ(LIGHT_BLUE,    9);
    EXPECT_EQ(LIGHT_GREEN,   10);
    EXPECT_EQ(LIGHT_AQUA,    11);
    EXPECT_EQ(LIGHT_RED,     12);
    EXPECT_EQ(LIGHT_PURPLE,  13);
    EXPECT_EQ(LIGHT_YELLOW,  14);
    EXPECT_EQ(BRIGHT_WHITE,  15);
}

// ── Construction ──────────────────────────────────────────────────────────────

TEST(ColourManagerTest, ConstructsWithoutCrashing)
{
    EXPECT_NO_FATAL_FAILURE(ColourManager cm);
}

// ── setColour: valid codes ─────────────────────────────────────────────────────

TEST(ColourManagerTest, SetColourAllValidCodesDoNotCrash)
{
    ColourManager cm;
    for (int code = 0; code <= 15; ++code)
    {
        EXPECT_NO_FATAL_FAILURE(cm.setColour(code))
            << "crashed on colour code " << code;
    }
    // Restore default white so subsequent test output is readable
    cm.setColour(WHITE);
}
