#ifndef SRC_BASE_COMMAND_LINE_H_
#define SRC_BASE_COMMAND_LINE_H_

#include <map>
#include <optional>
#include <string>

class CommandLine {
 public:
  static CommandLine& instance();
  static void initializeFromArgv(int argc, char** argv);

  std::optional<const char*> getSwitchValueStr(const std::string& key) const;

 private:
  CommandLine() = default;
  std::map<std::string, std::string> values_;
};

#endif  // SRC_BASE_COMMAND_LINE_H_
