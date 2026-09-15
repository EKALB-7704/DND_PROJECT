# DND Character Manager

A command-line D&D character management system written in C++. Tracks everything on a character sheet — stats, inventory, spells, currency, skills, and more — and saves each character to disk so nothing is lost between sessions.

## Features

- **Character sheet**: All core D&D attributes — ability scores, HP (current/max/temporary), death saving throws, initiative, AC, speed, conditions, and inspiration
- **Inventory system**: Polymorphic item hierarchy supporting weapons, armor, and generic gear with full D&D properties (damage dice, AC calculation, attunement, rarity, etc.)
- **Spellcasting**: Spell registry, per-character spellbook, and spell slot tracking (levels 1–9 with independent max and current counts)
- **Character features**: Skills (18 standard skills with proficiency/expertise), saving throw proficiencies, feats, racial traits, and language tracking
- **Wallet**: Five-denomination currency tracking (cp, sp, ep, gp, pp)
- **Dice roller**: Generic dN rolling plus D20 advantage/disadvantage modes
- **Checks, saves and attacks**: Roll skill checks, saving throws, ability checks, initiative and weapon attacks (with damage) straight from the character editor, with the sheet's modifiers applied
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
│                      # P_ManageFeatures, P_EditDetails, P_Rest, P_RollChecks,
│                      # P_GlobalSpells)
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
| `character.txt` | Core attributes (name, race, class, ability scores, HP, etc.) |
| `features.txt` | Feats, racial traits, languages, skills, saving throws |
| `inventory.txt` | Items with type tags for polymorphic reconstruction |
| `spells.txt` | Known spells |
| `spellslots.txt` | Spell slot availability per level |
| `wallet.txt` | Currency denominations |

Every save file starts with a version line — `#DNDCHAR 1`, `#DNDFEATS 1`,
`#DNDINV 1`, `#DNDWALLET 1`, `#DNDSLOTS 1`, `#DNDSPELLS 1`. Files saved before
versioning have no header, are read as version 0, and are rewritten with one
the next time the character is saved, so older saves keep working.

Fields are validated on load rather than trusted: values outside their legal
range are repaired to a safe default and reported, record counts are bounded
and must parse completely, and a malformed or truncated file is refused rather
than partially read. The shared machinery lives in `H_SaveFormat.h`.

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
- `test_save_format` — save versioning across all six files, legacy reads, field repair, malformed files

## Future Plans

The codebase cleanup pass is **complete**:

1. ~~Repo hygiene — untrack build artifacts and runtime save data~~
2. ~~Consolidate console input behind one validated I/O layer~~ (`ConsoleIO`;
   no raw `std::cin` remains in `src/`)
3. ~~Split `CharacterManager` into per-section menu units~~ (eight files, none
   over 220 lines; `editCharacter` is a 54-line dispatcher)
4. ~~Version every save file, with validation on load~~ (see Data Format)
5. ~~Consistency pass~~ — one header-guard style, PascalCase types, camelCase
   members, filenames matching the type they define

Since the cleanup, the editor UI has been made consistent: every submenu now
loops until `0`, and short/long rest moved out of the Spells submenu into their
own top-level "Rest" entry, since a rest restores HP and hit dice as well as
spell slots.

The dice roller is now wired into the character sheet: editor option 8,
"Roll checks, saves and attacks", rolls skill checks, saving throws, ability
checks and initiative with advantage/disadvantage, adding the modifier the sheet
already computes (ability modifier plus proficiency or expertise). A short
rest now rolls the hit dice it spends (each die plus the CON modifier, never
below 0) instead of asking for a total, and every roll in the program --
editor, rests and the main-menu dice roller -- comes from one shared roller.

Weapon attacks pick a weapon from the inventory and roll to hit with the
weapon's ability (STR for melee, DEX for ranged, the higher for finesse) plus
proficiency if the player says they are proficient. On a hit, the weapon's
damage dice (`1d8`, `2d6+1`, `d4`, or a flat `1`) are rolled with the same
ability modifier; a natural 20 doubles the dice and a natural 1 misses.
Damage the parser cannot read, like a versatile `1d8/1d10`, is typed in.

Planned features, once the above lands:

- Dungeon Master mode: manage both player characters and NPCs within a campaign
- Track weapon proficiencies (Simple / Martial) on the sheet, so attacks
  stop asking "Proficient with X?" each time
- Versatile weapons: store the one- and two-handed damage separately and
  ask which grip, instead of falling back to a typed-in total
- Short rest: offer "roll for me" or "enter my own roll" when spending hit
  dice, so players rolling physical dice at the table can type in their total
  (the rest currently always rolls for you)
- XP tracking and level-up (proficiency bonus is derivable from level)
- Character deletion

The `UI-REWORK`, `Save-system-overhaul`, and `Linux-Windows_cross_compatibility`
branches have all been merged into `main`.




