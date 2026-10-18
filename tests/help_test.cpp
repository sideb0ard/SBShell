#include "help.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "help_commands.h"
#include "interpreter/builtins.hpp"

namespace {

// Every builtin needs a `help <name>` entry. If this fails after adding a
// builtin, add it to kCommands in src/help_commands.cpp.
TEST(HelpTest, EveryBuiltinHasCommandHelp) {
  std::vector<std::string> missing;
  for (const auto& [name, fn] : builtin::built_ins) {
    if (!FindCommandHelp(name)) missing.push_back(name);
  }
  EXPECT_TRUE(missing.empty()) << "Builtins without help: " << [&] {
    std::string s;
    for (const auto& m : missing) s += m + " ";
    return s;
  }();
}

// Entries for things that aren't builtins would be stale docs.
TEST(HelpTest, EveryHelpEntryIsABuiltin) {
  for (const auto& c : CommandHelpTable()) {
    EXPECT_TRUE(builtin::built_ins.count(c.name)) << "No builtin: " << c.name;
  }
}

TEST(HelpTest, HelpSchedShowsArgumentOrder) {
  std::string out = build_help("sched");
  EXPECT_NE(out.find("sched(when, start_val, end_val, time"),
            std::string::npos);
}

TEST(HelpTest, UnknownTopicSuggestsSimilarCommands) {
  std::string out = build_help("note");
  EXPECT_NE(out.find("note_on_at"), std::string::npos);
}

}  // namespace
