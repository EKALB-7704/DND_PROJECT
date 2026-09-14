#include "H_SaveFormat.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include "H_DndExceptions.h"
#include "H_Validate.h"

namespace SaveFormat {

void writeHeader(std::ostream& file, const std::string& tag, int version)
{
    file << "#" << tag << " " << version << "\n";
}

bool readLine(std::istream& file, std::string& line)
{
    if (!std::getline(file, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return true;
}

bool parseWholeInt(const std::string& text, int& out)
{
    const std::string t = Validate::trim(text);
    if (t.empty()) return false;
    try
    {
        size_t consumed = 0;
        const int value = std::stoi(t, &consumed);
        if (consumed != t.size()) return false;  // trailing junk
        out = value;
        return true;
    }
    catch (const std::exception&) { return false; }  // not a number, or too big
}

bool parseInts(const std::string& line, int count, std::vector<int>& out)
{
    std::istringstream ss(line);
    out.clear();
    std::string tok;
    while (ss >> tok)
    {
        int v = 0;
        if (!parseWholeInt(tok, v)) return false;
        out.push_back(v);
    }
    return static_cast<int>(out.size()) == count;
}

int readHeader(std::istream& file, const std::string& tag,
               const std::string& what, int current)
{
    const std::streampos start = file.tellg();

    std::string line;
    if (!readLine(file, line))
    {
        // Empty file: treat as legacy and let the caller find nothing.
        file.clear();
        file.seekg(start);
        return 0;
    }

    // The trailing space keeps one tag from prefix-matching another.
    const std::string prefix = "#" + tag + " ";
    if (line.rfind(prefix, 0) != 0)
    {
        file.clear();
        file.seekg(start);
        return 0;  // headerless: a pre-versioning file
    }

    int version = 0;
    if (!parseWholeInt(line.substr(prefix.size()), version) || version < 0)
    {
        throw LoadError("unreadable version header in " + what);
    }
    if (version > current)
    {
        throw LoadError(what + " is version " + std::to_string(version) +
                        ", newer than this build supports (" +
                        std::to_string(current) + ")");
    }
    return version;
}

int readCount(std::istream& file, const std::string& what)
{
    std::string line;
    if (!readLine(file, line))
    {
        throw LoadError("missing record count in " + what);
    }

    int count = 0;
    if (!parseWholeInt(line, count))
    {
        throw LoadError("unreadable record count in " + what + ": \"" +
                        Validate::trim(line) + "\"");
    }
    if (count < 0 || count > kMaxRecords)
    {
        throw LoadError("implausible record count in " + what + " (" +
                        std::to_string(count) + ")");
    }
    return count;
}

int repairRange(int value, int lo, int hi, int fallback,
                const std::string& field, std::vector<std::string>* repairs)
{
    if (value >= lo && value <= hi) return value;
    if (repairs)
    {
        repairs->push_back(field + " was " + std::to_string(value) +
                           ", outside " + std::to_string(lo) + "-" + std::to_string(hi) +
                           "; set to " + std::to_string(fallback));
    }
    return fallback;
}

} // namespace SaveFormat
