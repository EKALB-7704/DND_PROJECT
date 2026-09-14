#ifndef CONSOLE_IO_H
#define CONSOLE_IO_H

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "H_DndExceptions.h" // EndOfInput

// Reusable validators for free-form text input.
// Free functions so they can be tested and composed without a CharacterManager.
namespace Validate {
    // Printable, non-empty, and contains at least one letter.
    bool isText(const std::string& input);

    // Printable, non-empty, and not entirely whitespace.
    // Looser than isText: allows digits and symbols (e.g. "Gimli II").
    bool isName(const std::string& input);

    // Hit dice format: 'd' followed by at least one digit (e.g. d6, d12, d100).
    bool isHitDice(const std::string& input);
}

// Wraps all console reads behind one validated interface.
//
// Two design rules hold throughout:
//
//  1. Every read goes through std::getline. Nothing uses operator>>, which is
//     what left stray newlines in the buffer and made a failed extraction
//     poison every later read. Parsing happens on the extracted line instead.
//  2. Streams are injected rather than hardcoded to std::cin/std::cout, so the
//     menu layer can be driven from a std::istringstream under test.
class ConsoleIO {
private:
    std::istream& in;
    std::ostream& out;

public:
    explicit ConsoleIO(std::istream& input = std::cin, std::ostream& output = std::cout);

    // Reads one raw line. Returns false at EOF. This is the only function that
    // reports stream death by return value; every reader below turns it into an
    // EndOfInput exception, because they have no valid value to hand back.
    bool readRawLine(std::string& line);

    // Integer in [min, max] inclusive. Re-prompts until valid.
    int readInt(const std::string& prompt, int min, int max);

    // Integer restricted to an explicit set, for choices that are not a
    // contiguous range (e.g. die sides: 4, 6, 8, 10, 12, 20, 100).
    int readIntFrom(const std::string& prompt, const std::vector<int>& allowed);

    // Menu selection in [0, maxOption]. 0 is conventionally Back/Exit.
    // Replaces the bare `std::cin >> choice` in every menu loop, where a
    // non-numeric entry used to silently yield 0 and back out or quit.
    int readMenuChoice(const std::string& prompt, int maxOption);

    // A line satisfying `valid`; `errorHint` is shown after a rejected entry.
    std::string readMatching(const std::string& prompt,
                             const std::function<bool(const std::string&)>& valid,
                             const std::string& errorHint);

    // Common readMatching specialisations.
    std::string readText(const std::string& prompt);
    std::string readName(const std::string& prompt);
    std::string readHitDice(const std::string& prompt);

    // Accepts y/yes/n/no, case-insensitive.
    bool readYesNo(const std::string& prompt);

    // Escape hatch for code still writing its own output.
    std::ostream& os();
};

#endif
