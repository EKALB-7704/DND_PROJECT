#ifndef VALIDATE_H
#define VALIDATE_H

#include <string>

// Field validators, independent of where the value came from.
//
// These live outside H_ConsoleIO.h because they are not console-specific: the
// save loader uses the same rules to check data read from disk that the menus
// use to check data typed by a user. A value rejected at the prompt should not
// be silently accepted from a file.
namespace Validate {

// Trims leading and trailing whitespace.
std::string trim(const std::string& s);

// Printable, non-empty, and contains at least one letter.
bool isText(const std::string& input);

// Printable, non-empty, and not entirely whitespace.
// Looser than isText: allows digits and symbols (e.g. "Gimli II").
bool isName(const std::string& input);

// Hit dice format: 'd' followed by at least one digit (e.g. d6, d12, d100).
bool isHitDice(const std::string& input);

} // namespace Validate

#endif
