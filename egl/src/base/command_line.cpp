#include <cstring>
#include <optional>

#include "base/command_line.h"

namespace {}

CommandLine& CommandLine::instance() {
  static CommandLine g_instance;
  return g_instance;
}

void CommandLine::initializeFromArgv(int argc, char** argv) {
  std::map<std::string, std::string> values;

  for (int i = 1; i < argc; ++i) {
    int n = strlen(argv[i]);
    if (n < 3)
      continue;
    if (!(argv[i][0] == '-' && argv[i][1] == '-'))
      continue;
    if (i + 1 >= argc)
      continue;
    std::string key(argv[i] + 2);
    std::string value(argv[i + 1] + 2);
    values[std::move(key)] = std::move(value);
    i += 1;
  }
  CommandLine::instance().values_ = std::move(values);
}

std::optional<const char*> CommandLine::getSwitchValueStr(
    const std::string& key) const {
  auto it = values_.find(key);
  if (it == values_.end())
    return std::optional<const char*>();

  return it->second.c_str();
}