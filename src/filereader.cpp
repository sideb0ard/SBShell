#include <unistd.h>

#include <filereader.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

// Returns true for lines like "p10 # comp_name;" where '#' is process syntax.
bool is_process_statement(const std::string& line) {
  size_t i = line.find_first_not_of(" \t");
  if (i == std::string::npos || line[i] != 'p') return false;
  i++;
  if (i >= line.size() || !isdigit(line[i])) return false;
  while (i < line.size() && isdigit(line[i])) i++;
  return i < line.size() && isspace(line[i]);
}

bool starts_with_comment(const std::string& line) {
  std::size_t first_char_pos = line.find_first_not_of(" \t");
  return first_char_pos != std::string::npos && line[first_char_pos] == '#';
}

// Strip inline comment from a single line (does not handle leading # comments).
// '//' is always a comment marker.
// '#' is a comment marker unless the line is a process statement (p10 # name).
std::string strip_inline_comment(const std::string& line) {
  if (is_process_statement(line)) return line;

  bool in_string = false;
  char string_char = '\0';

  for (size_t i = 0; i < line.length(); i++) {
    char c = line[i];

    if (!in_string && (c == '"' || c == '\'')) {
      in_string = true;
      string_char = c;
    } else if (in_string && c == string_char &&
               (i == 0 || line[i - 1] != '\\')) {
      in_string = false;
      string_char = '\0';
    }

    if (!in_string && c == '/' && i + 1 < line.length() && line[i + 1] == '/')
      return line.substr(0, i);

    if (!in_string && c == '#') return line.substr(0, i);
  }

  return line;
}
}  // namespace

std::string ReadFileContents(std::string filepath) {
  std::ifstream ifs(filepath);
  if (ifs.is_open()) {
    std::stringstream buffer;

    std::string line;
    while (getline(ifs, line)) {
      if (!starts_with_comment(line)) {
        buffer << strip_inline_comment(line);
      }
    }

    return buffer.str();
  }
  return "";
}
