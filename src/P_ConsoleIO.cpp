#include "H_ConsoleIO.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {

// Trims leading/trailing whitespace. Menu entries routinely arrive as " 3 "
// once the caller stops relying on operator>> to skip whitespace for them.
std::string trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n\f\v");
    return s.substr(first, last - first + 1);
}

// Parses a complete integer, rejecting trailing junk ("12abc") that
// operator>> would have silently accepted as 12.
bool parseInt(const std::string& text, int& out)
{
    const std::string t = trim(text);
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

} // namespace

namespace Validate {

bool isText(const std::string& input)
{
    if (input.empty()) return false;
    bool hasLetter = false;
    for (unsigned char c : input)
    {
        if (!std::isprint(c)) return false;
        if (std::isalpha(c)) hasLetter = true;
    }
    return hasLetter;
}

bool isName(const std::string& input)
{
    if (input.empty()) return false;
    for (unsigned char c : input)
    {
        if (!std::isprint(c)) return false;
    }
    // Reject entries that are only whitespace.
    return !trim(input).empty();
}

bool isHitDice(const std::string& input)
{
    if (input.size() < 2 || input[0] != 'd') return false;
    for (size_t i = 1; i < input.size(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) return false;
    }
    return true;
}

} // namespace Validate

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

        const std::string value = trim(line);
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

        std::string value = trim(line);
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (value == "y" || value == "yes") return true;
        if (value == "n" || value == "no")  return false;

        out << "Please enter y or n.\n";
    }
}
