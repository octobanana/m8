#ifndef OB_M8_HH
#define OB_M8_HH

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <vector>
#include <regex>
#include <functional>
#include <ctime>

struct macro
{
  std::string info;
  std::string usage;
  std::string regex;
  std::function<int(std::string&, std::smatch const&)> fn;
}; // struct M8

// helper functions

std::string repeat(std::string val, int num)
{
  if (num < 2)
  {
    return val;
  }
  std::string out;
  for (int i = 0; i < num; ++i)
  {
    out.append(val);
  }
  return out;
}

// fn lambdas
// where s = a reference to the output string
// and m = std::smatch of the parsed regex

// m[0] is the full regex
// m[1] is the captured string value
// m[2] is the captured integer value
auto const fn_repeat = [](auto& s, auto const& m) {
  auto res = repeat(std::string(m[1]), std::stoi(std::string(m[2])));
  std::stringstream ss;
  ss << "\"" << res << "\"";
  s = ss.str();
  return 0;
};

// m[0] is the full regex
// m[1] is the captured type as a string value
auto const fn_add = [](auto& s, auto const& m) {
  auto type = std::string(m[1]);
  std::stringstream ss;
  ss << type << "add(" << type << ") x, " << type << " y)\n"
     << "{\n"
     << "return x + y"
     << "}";
  s = ss.str();
  return 0;
};

auto const fn_comment_header = [](auto& s, auto const& m) {
  std::stringstream ss;
  ss << "// timestamp:   " << std::time(nullptr) << "\n";
  ss << "// Version:     " << m[1] << "\n";
  ss << "// Author:      " << m[2] << "\n";
  ss << "// Description: " << m[3];
  s = ss.str();
  return 0;
};

// macro map
// (name) -> (info, usage, regex, fn)
std::map<std::string, macro> M8 {
{"repeat", {"repeats the given input by n", "repeat(str, int)",
  "^repeat\\((.+),([0-9]+)\\)$", fn_repeat}},
{"add", {"template add function", "add(type)",
  "^add\\((.+)\\)$", fn_add}},
{"comment_header", {"outputs the authors name, timestamp, version, and description in a c++ comment block", "comment_header(version, author, description)",
  "^comment_header\\(([.0-9]+), \"(.+)\", \"(.+)\"\\)$", fn_comment_header}},
};

#endif // OB_M8_HH
