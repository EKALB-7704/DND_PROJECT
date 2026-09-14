# DND Character Manager

A command-line D&D character management system written in C++. Tracks everything on a character sheet — stats, inventory, spells, currency, skills, and more — and saves each character to disk so nothing is lost between sessions.

## Features

- **Character sheet**: All core D&D attributes — ability scores, HP (current/max/temporary), death saving throws, initiative, AC, speed, conditions, and inspiration
- **Inventory system**: Polymorphic item hierarchy supporting weapons, armor, and generic gear with full D&D properties (damage dice, AC calculation, attunement, rarity, etc.)
- **Spellcasting**: Spell registry, per-character spellbook, and spell slot tracking (levels 1–9 with independent max and current counts)
- **Character features**: Skills (18 standard skills with proficiency/expertise), saving throw proficiencies, feats, racial traits, and language tracking
- **Wallet**: Five-denomination currency tracking (cp, sp, ep, gp, pp)
- **Dice roller**: Generic dN rolling plus D20 advantage/disadvantage modes
- **Persistence**: Each character is saved to a per-character directory of structured text files; characters survive across sessions
- **Exception handling**: Custom exception hierarchy; the program exits cleanly with error code 1 on unrecoverable errors

## Tech Stack

| Component | Technology |
|-----------|-----------|
| Language | C++17 |
| Build system | CMake 3.18+ |
| Compiler | GCC / MinGW |
| Test framework | Google Test 1.14.0 |
| Platform | Windows, Linux |

## Project Structure

```
DND_PROJECT/
├── src/               # Source files; the character editor is split by menu
│                      # section (P_EditHealth, P_EditSpells, P_ManageInventory,
│                      # P_ManageFeatures, P_EditDetails, P_GlobalSpells)
├── include/           # Header files and custom exception hierarchy
├── tests/             # Google Test unit tests (13 test suites)
├── data/
│   ├── SpellBook.txt  # Global spell registry
│   └── characters/    # Per-character save directories
├── docs/              # Additional documentation
├── CMakeLists.txt     # Top-level CMake configuration
└── build.bat          # Windows quick-build script
```

## Building

### CMake (recommended)

```bash
cmake -B build -S .
cmake --build build
```

### Run tests

```bash
ctest --test-dir build
```

### Code coverage (optional)

```bash
cmake -B build -S . -DENABLE_COVERAGE=ON
cmake --build build
cmake --build build --target coverage
```

This generates an HTML coverage report via gcovr.

### Windows quick build

```bat
build.bat
```

## Running

```bash
./build/DND_PROJECT
```

The program presents an interactive menu for creating, loading, editing, and saving characters.

## Data Format

Characters are saved under `data/characters/<name>/` with one file per subsystem:

| File | Contents |
|------|----------|
| `character.txt` | Version header, then core attributes (name, race, class, ability scores, HP, etc.) |
| `features.txt` | Feats, racial traits, languages, skills, saving throws |
| `inventory.txt` | Items with type tags for polymorphic reconstruction |
| `spells.txt` | Known spells |
| `spellslots.txt` | Spell slot availability per level |
| `wallet.txt` | Currency denominations |

`character.txt` starts with a version line, `#DNDCHAR 1`. Files saved before
versioning have no header, are read as version 0, and are rewritten with one
the next time the character is saved. Fields are validated on load: values
outside their legal range are repaired to a safe default and reported, and a
malformed file is refused rather than partially read.

`data/characters/` is intentionally **not** tracked in git — it is runtime state
written by the app, so committing it made every play session show up as a working
tree change. Your characters live on disk and persist normally; they just don't
travel with a clone. `data/SpellBook.txt` (the global spell registry) *is* tracked,
and is loaded by relative path, so run the program from the repository root.

## Tests

Thirteen test suites cover the full system:

- `test_character` — character creation, stats, HP, death saves
- `test_inventory` — item management and equipping
- `test_items` — weapon/armor/gear polymorphism
- `test_spell` — spell properties and serialization
- `test_spellslots` — slot usage and reset logic
- `test_wallet` — currency management
- `test_dice_roller` — dice rolls and advantage/disadvantage
- `test_features` — skills, saving throws, feats
- `test_persistence` — file save/load round-tripping
- `test_colours` — terminal color functionality
- `test_consoleio` — input parsing, range/format validation, and EOF handling
- `test_manager_menus` — menu flows driven end to end from a scripted stream
- `test_save_format` — save versioning, legacy reads, field repair, malformed files

## Future Plans

Current focus is a codebase cleanup pass before any new features:

1. ~~Repo hygiene — untrack build artifacts and runtime save data~~ **done**
2. ~~Consolidate console input behind one validated I/O layer~~ **done** — see
   `ConsoleIO`; no raw `std::cin` remains in `src/`
3. ~~Split `CharacterManager` into per-section menu units~~ **done** — eight
   files, none over 220 lines; `editCharacter` is now a 54-line dispatcher
4. ~~Add a version header to the character save format, with validation on
   load~~ **done** — `#DNDCHAR <n>`; headerless files read as v0 and upgrade
   on next save; out-of-range fields are repaired and reported

Planned features, once the above lands:

- Dungeon Master mode: manage both player characters and NPCs within a campaign
- Wire the dice roller into skill checks, saving throws, attacks, and initiative
- XP tracking and level-up (proficiency bonus is derivable from level)
- Character deletion

The `UI-REWORK`, `Save-system-overhaul`, and `Linux-Windows_cross_compatibility`
branches have all been merged into `main`.




