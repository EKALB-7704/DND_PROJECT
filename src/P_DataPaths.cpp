#include "H_DataPaths.h"
#include <system_error>

// Each OS has its own way to ask for the running executable's path;
// argv[0] is no substitute, since it is just the name when run from the PATH.
#if defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <vector>
#endif

namespace fs = std::filesystem;

namespace DataPaths {

fs::path executableDir()
{
    fs::path exe;
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
        exe = fs::path(std::wstring(buffer, len));
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) == 0)
        exe = fs::path(buffer.data());
#else
    std::error_code ec;
    exe = fs::read_symlink("/proc/self/exe", ec);
    if (ec) exe.clear();
#endif
    return exe.empty() ? fs::path() : exe.parent_path();
}

bool looksLikeDataDir(const fs::path& dir)
{
    std::error_code ec;
    return fs::is_regular_file(dir / "SpellBook.txt", ec) ||
           fs::is_directory(dir / "characters", ec);
}

fs::path resolveDataDir(const fs::path& explicitDir, const std::string& envDir,
                        const fs::path& exeDir, const fs::path& cwd)
{
    if (!explicitDir.empty()) return explicitDir;
    if (!envDir.empty()) return fs::path(envDir);

    if (!exeDir.empty())
    {
        fs::path dir = exeDir;
        for (int level = 0; level < 3; ++level)
        {
            if (looksLikeDataDir(dir / "data")) return dir / "data";
            if (!dir.has_parent_path() || dir.parent_path() == dir) break;
            dir = dir.parent_path();
        }
    }

    return cwd / "data";
}

} // namespace DataPaths
