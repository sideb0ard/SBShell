#pragma once

#include <string>
#include <vector>

// Per-command help: `help sched`, `help note_on_at`, ...
// Every builtin function should have an entry (tests/help_test.cpp checks).

struct CommandHelp {
  const char* name;
  const char* category;
  const char*
      usage;  // call signature, e.g. sched(when, start, end, time, "cmd %")
  const char* summary;  // one line
  const char* details;  // argument notes, may be empty
  const char* example;  // may be empty
};

const std::vector<CommandHelp>& CommandHelpTable();

// Returns nullptr when there's no entry for `name`.
const CommandHelp* FindCommandHelp(const std::string& name);

std::string FormatCommandHelp(const CommandHelp& help);

// All commands grouped by category (`help commands`).
std::string FormatCommandIndex();

// Commands whose name contains `fragment`, for "did you mean" hints.
std::vector<std::string> SimilarCommands(const std::string& fragment);
