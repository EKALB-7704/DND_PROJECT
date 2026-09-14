#pragma once
#include <stdexcept>
#include <string>

struct DndException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct SaveError : DndException {
    using DndException::DndException;
};

struct LoadError : DndException {
    using DndException::DndException;
};

// Input stream closed (EOF) while a value was still required. Lets the readers
// in ConsoleIO abandon a re-prompt loop instead of spinning on a dead stream.
struct EndOfInput : DndException {
    using DndException::DndException;
};
