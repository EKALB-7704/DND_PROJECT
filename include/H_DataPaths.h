#ifndef DATA_PATHS_H
#define DATA_PATHS_H

#include <filesystem>
#include <string>

// Finds the data/ folder (SpellBook.txt and characters/) without depending on
// the directory the program was started from.
namespace DataPaths {

// Directory holding the running executable, or empty if the OS won't say.
std::filesystem::path executableDir();

// True if `dir` looks like a data folder: it holds SpellBook.txt or characters/.
bool looksLikeDataDir(const std::filesystem::path& dir);

// Picks the data folder, first match wins:
//   1. `explicitDir` (from --data-dir), used as given
//   2. `envDir` (from DND_DATA_DIR), used as given
//   3. a data/ folder in `exeDir` or up to two levels above it, so both
//      build/DND_PROJECT and build/release/DND_PROJECT find the project's data/
//   4. `cwd`/data, the original behaviour
// Empty arguments are skipped.
std::filesystem::path resolveDataDir(const std::filesystem::path& explicitDir,
                                     const std::string& envDir,
                                     const std::filesystem::path& exeDir,
                                     const std::filesystem::path& cwd);

} // namespace DataPaths

#endif
