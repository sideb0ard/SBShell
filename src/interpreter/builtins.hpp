#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "evaluator.hpp"
#include "object.hpp"

namespace builtin {

extern std::unordered_map<std::string, std::shared_ptr<object::BuiltIn>>
    built_ins;

// Set the bar-start tick anchor so note_on_at() schedules relative to it
// rather than when the audio thread dequeues the action. Call before running
// a computation, reset to -1 after. Thread-local so safe for the process
// worker.
void SetNoteBarAnchor(int tick);

}  // namespace builtin
