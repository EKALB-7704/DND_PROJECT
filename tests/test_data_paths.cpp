#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "H_DataPaths.h"

namespace fs = std::filesystem;

namespace {

// A throwaway project layout: <root>/data/SpellBook.txt and <root>/build/release/.
struct ProjectLayout {
    fs::path root;

    ProjectLayout() {
        root = fs::temp_directory_path() /
               ("dnd_data_paths_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name());
        fs::remove_all(root);
        fs::create_directories(root / "data");
        fs::create_directories(root / "build" / "release");
        std::ofstream(root / "data" / "SpellBook.txt") << "#DNDSPELLS 1\n0\n";
    }
    ~ProjectLayout() { fs::remove_all(root); }
};

} // namespace

TEST(DataPathsTest, ExplicitDirWinsOverEverything) {
    ProjectLayout p;
    EXPECT_EQ(DataPaths::resolveDataDir("/chosen", "/from_env", p.root / "build", p.root),
              fs::path("/chosen"));
}

TEST(DataPathsTest, EnvironmentDirWinsOverSearch) {
    ProjectLayout p;
    EXPECT_EQ(DataPaths::resolveDataDir("", "/from_env", p.root / "build", p.root),
              fs::path("/from_env"));
}

TEST(DataPathsTest, FindsDataAboveTheBuildFolder) {
    ProjectLayout p;
    const fs::path elsewhere = fs::temp_directory_path();
    EXPECT_EQ(DataPaths::resolveDataDir("", "", p.root / "build", elsewhere),
              p.root / "data");
    EXPECT_EQ(DataPaths::resolveDataDir("", "", p.root / "build" / "release", elsewhere),
              p.root / "data");
}

TEST(DataPathsTest, FindsDataBesideTheExecutable) {
    ProjectLayout p;
    EXPECT_EQ(DataPaths::resolveDataDir("", "", p.root, fs::temp_directory_path()),
              p.root / "data");
}

TEST(DataPathsTest, FallsBackToWorkingDirectory) {
    ProjectLayout p;
    fs::remove_all(p.root / "data");
    EXPECT_EQ(DataPaths::resolveDataDir("", "", p.root / "build", p.root),
              p.root / "data");
}

TEST(DataPathsTest, CharactersFolderAloneCountsAsData) {
    ProjectLayout p;
    fs::remove(p.root / "data" / "SpellBook.txt");
    EXPECT_FALSE(DataPaths::looksLikeDataDir(p.root / "data"));
    fs::create_directories(p.root / "data" / "characters");
    EXPECT_TRUE(DataPaths::looksLikeDataDir(p.root / "data"));
}

TEST(DataPathsTest, ExecutableDirIsKnown) {
    EXPECT_FALSE(DataPaths::executableDir().empty());
}
