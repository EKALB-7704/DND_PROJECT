#include "H_Validate.h"

#include <cctype>

namespace Validate {

// Trims leading and trailing whitespace.
std::string trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n\f\v");
    return s.substr(first, last - first + 1);
}

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
