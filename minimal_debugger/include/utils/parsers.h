#pragma once
#include "types.h"
#include <string>
#include <vector>
using std::string, std::vector;

vector<string> get_tokens(const string &command);
addr_t string_to_address(const string &addr_str);
string address_to_string(addr_t address);
