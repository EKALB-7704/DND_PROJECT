#ifndef SAVE_FORMAT_H
#define SAVE_FORMAT_H

#include <iosfwd>
#include <string>
#include <vector>

// Shared machinery for the on-disk save files.
//
// Every save file carries a version header as its first line, "#<TAG> <n>".
// Files written before versioning existed have no header and are read as
// version 0; the reader rewinds and parses the original layout.
//
// This lives in one place because all six save files had the same three
// weaknesses: no version marker, so adding a field mid-project silently
// shifted every later value; no validation, so a value that would be refused
// at a prompt was accepted from disk without comment; and unbounded counts,
// so a corrupt length field drove the reader off the end of the data.
namespace SaveFormat {

// Version tags, one per file. Kept together so they cannot collide.
constexpr const char* kCharacterTag  = "DNDCHAR";
constexpr const char* kFeaturesTag   = "DNDFEATS";
constexpr const char* kInventoryTag  = "DNDINV";
constexpr const char* kWalletTag     = "DNDWALLET";
constexpr const char* kSpellSlotsTag = "DNDSLOTS";
constexpr const char* kSpellBookTag  = "DNDSPELLS";

// Current version of each file. Bump one when its layout changes, and branch
// on the value returned by readHeader rather than reinterpreting old offsets.
constexpr int kVersion = 1;

// features.txt version 2 adds weapon proficiencies after the languages.
constexpr int kFeaturesVersion = 2;
// Sanity ceiling for any "how many records follow" field. A corrupt count used
// to be trusted, so a damaged file could spin the reader over stale data.
constexpr int kMaxRecords = 100000;

// Writes "#<tag> <version>\n".
void writeHeader(std::ostream& file, const std::string& tag, int version = kVersion);

// Reads the header if present and returns its version. Returns 0 for a
// headerless (legacy) file, rewinding so the caller can read the old layout.
// Throws LoadError if the header is malformed or newer than `current`.
int readHeader(std::istream& file, const std::string& tag,
               const std::string& what, int current = kVersion);

// Reads one line, stripping a trailing CR. False at end of input.
bool readLine(std::istream& file, std::string& line);

// Parses a complete integer, rejecting trailing junk ("12abc"), which
// operator>> would have accepted as 12.
bool parseWholeInt(const std::string& text, int& out);

// Splits a line into exactly `count` integers. False on any mismatch, so a
// short or long line is caught instead of leaving later fields stale.
bool parseInts(const std::string& line, int count, std::vector<int>& out);

// Reads a record-count line, rejecting anything negative, unparseable, or
// beyond kMaxRecords. Throws LoadError naming `what`.
int readCount(std::istream& file, const std::string& what);

// Clamps `value` into [lo, hi], appending a note to `repairs` if it changed.
int repairRange(int value, int lo, int hi, int fallback,
                const std::string& field, std::vector<std::string>* repairs);

} // namespace SaveFormat

#endif
