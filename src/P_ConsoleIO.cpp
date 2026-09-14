#include "H_ConsoleIO.h"
#include "H_Validate.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {

// Parses a complete integer, rejecting trailing junk ("12abc") that
// operator>> would have silently accepted as 12.
bool parseInt(const std::string& text, int& out)
{
    const std::string t = Validate::trim(text);
    if (t.empty()) return false;

    try
    {
        size_t consumed = 0;
        const int value = std::stoi(t, &consumed);
        if (consumed != t.size()) return false; // trailing junk
        out = value;
        return true;
    }
    catch (const std::invalid_argument&) { return false; } // not a number
    catch (const std::out_of_range&)     { return false; } // exceeds int
}

// Same idea as parseInt, for decimal fields.
bool parseFloat(const std::string& text, float& out)
{
    const std::string t = Validate::trim(text);
    if (t.empty()) return false;

    try
    {
        size_t consumed = 0;
        const float value = std::stof(t, &consumed);
        if (consumed != t.size()) return false;
        out = value;
        return true;
    }
    catch (const std::invalid_argument&) { return false; }
    catch (const std::out_of_range&)     { return false; }
}

} // namespace

ConsoleIO::ConsoleIO(std::istream& input, std::ostream& output)
    : in(input), out(output) {}

std::ostream& ConsoleIO::os() { return out; }

bool ConsoleIO::readRawLine(std::string& line)
{
    if (!std::getline(in, line)) return false;

    // Tolerate CRLF save/console data on Linux.
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return true;
}

int ConsoleIO::readInt(const std::string& prompt, int min, int max)
{
    std::string line;
    while (true)
    {
        out << prompt;
        if (!readRawLine(line))
        {
            throw EndOfInput("input closed while reading: " + prompt);
        }

        int value = 0;
        if (parseInt(line, value) && value >= min && value <= max)
        {
            return value;
        }

        out << "Invalid entry. Enter a whole number between "
            << min << " and " << max << ".\n";
    }
}

int ConsoleIO::readIntFrom(const std::string& prompt, const std::vector<int>& allowed)
{
    std::string line;
    while (true)
    {
        out << prompt;
        if (!readRawLine(line))
        {
            throw EndOfInput("input closed while reading: " + prompt);
        }

        int value = 0;
        if (parseInt(line, value) &&
            std::find(allowed.begin(), allowed.end(), value) != allowed.end())
        {
            return value;
        }

        out << "Invalid entry. Choose one of:";
        for (size_t i = 0; i < allowed.size(); i++)
        {
            out << (i == 0 ? " " : ", ") << allowed[i];
        }
        out << "\n";
    }
}

float ConsoleIO::readFloat(const std::string& prompt, float min, float max)
{
    std::string line;
    while (true)
    {
        out << prompt;
        if (!readRawLine(line))
        {
            throw EndOfInput("input closed while reading: " + prompt);
        }

        float value = 0.0f;
        if (parseFloat(line, value) && value >= min && value <= max)
        {
            return value;
        }

        out << "Invalid entry. Enter a number between " << min << " and " << max << ".\n";
    }
}

int ConsoleIO::readMenuChoice(const std::string& prompt, int maxOption)
{
    return readInt(prompt, 0, maxOption);
}

std::string ConsoleIO::readMatching(const std::string& prompt,
                                    const std::function<bool(const std::string&)>& valid,
                                    const std::string& errorHint)
{
    std::string line;
    while (true)
    {
        out << prompt;
        if (!readRawLine(line))
        {
            throw EndOfInput("input closed while reading: " + prompt);
        }

        const std::string value = Validate::trim(line);
        if (valid(value)) return value;

        out << errorHint << "\n";
    }
}

std::string ConsoleIO::readText(const std::string& prompt)
{
    return readMatching(prompt, Validate::isText,
                        "Entry must contain at least one letter.");
}

std::string ConsoleIO::readName(const std::string& prompt)
{
    return readMatching(prompt, Validate::isName,
                        "Entry cannot be blank.");
}

std::string ConsoleIO::readHitDice(const std::string& prompt)
{
    return readMatching(prompt, Validate::isHitDice,
                        "Hit dice must be in the format dN (e.g. d8, d12).");
}

bool ConsoleIO::readYesNo(const std::string& prompt)
{
    std::string line;
    while (true)
    {
        out << prompt;
        if (!readRawLine(line))
        {
            throw EndOfInput("input closed while reading: " + prompt);
        }

        std::string value = Validate::trim(line);
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (value == "y" || value == "yes") return true;
        if (value == "n" || value == "no")  return false;

        out << "Please enter y or n.\n";
    }
}
