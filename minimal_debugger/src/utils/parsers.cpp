#include "utils/parsers.h"
#include <regex>

vector<string> get_tokens(const string &command) {
  const std::regex ws(R"([\s]+)");
  std::sregex_token_iterator iter(command.begin(), command.end(), ws, -1);
  std::sregex_token_iterator end;
  std::vector<std::string> tokens(iter, end);

  tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
                              [](const std::string &s) { return s.empty(); }),
               tokens.end());
  return tokens;
}

addr_t string_to_address(const string &addr_str) {
  static const std::regex hexRegex(R"(^0x([0-9a-fA-F]+)|^([0-9a-fA-F]+))");
  std::smatch match;
  if (!std::regex_match(addr_str, match, hexRegex))
    throw std::invalid_argument("Invalid hex string");
  string hexPart = match.length(1) ? match[1].str() : match[2].str();
  int base = match.length(1) ? 16 : 0;
  return (addr_t)std::stoul(hexPart, nullptr, base);
}

string address_to_string(addr_t address) {
  char buffer[32];
  sprintf(buffer, "0x%lx", address);
  return string(buffer);
}
