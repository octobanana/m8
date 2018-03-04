#include "macros.hh"
#include "m8.hh"
#include "sys_command.hh"

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
#include <stdexcept>
#include <cstdlib>
#include <cctype>
#include <sys/ioctl.h>
#include <unistd.h>

namespace Macros
{
// prototypes
std::string repeat(std::string val, int num);
void replace_all(std::string& str, std::string const& key, std::string const& val);
std::vector<std::string> delimit(std::string const& str, std::string const& delim);

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

void replace_all(std::string& str, std::string const& key, std::string const& val)
{
  std::string::size_type pos_start = 0;
  size_t len_key {key.length()};

  do
  {
    pos_start = str.find(key, ++pos_start);
    if (pos_start != std::string::npos)
    {
      str.replace(pos_start, len_key, val);
    }
  }
  while (pos_start != std::string::npos);
}

std::vector<std::string> delimit(std::string const& str, std::string const& delim)
{
  std::vector<std::string> vtok;
  auto start = 0U;
  auto end = str.find(delim);
  while (end != std::string::npos) {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.length();
    end = str.find(delim, start);
  }
  vtok.emplace_back(str.substr(start, end));
  return vtok;
}

// macro functions
// std::function<std::string(std::string& response, std::smatch const& match)>

auto const fn_main = [](auto& s, auto const& m) {
  auto str = std::string(m[1]);
  std::stringstream ss;
  ss << "int main(int argc, char* argv[])\n"
     << "{"
     << str
     << "}";
  s = ss.str();
  return 0;
};

auto const fn_repeat = [](auto& s, auto const& m) {
  auto res = repeat(std::string(m[1]), std::stoi(std::string(m[2])));
  std::stringstream ss;
  ss << res;
  s = ss.str();
  return 0;
};

auto const fn_add = [](auto& s, auto const& m) {
  auto type = std::string(m[1]);
  std::stringstream ss;
  ss << type << " add(" << type << " x, " << type << " y)\n"
     << "{\n"
     << "  return x + y;\n"
     << "}";
  s = ss.str();
  return 0;
};

auto const fn_comment_header = [](auto& s, auto const& m) {
  std::stringstream ss; ss
  << "// timestamp:   " << std::time(nullptr) << "\n"
  << "// Version:     " << m[1] << "\n"
  << "// Author:      " << m[2] << "\n"
  << "// Description: " << m[3];
  s = ss.str();
  return 0;
};

auto const fn_license = [](auto& s, auto const& m) {
  std::string license {m[1]};
  std::string author {m[2]};
  std::string year {m[3]};

  if (license == "MIT")
  {
    std::stringstream ss; ss
    << "//\n"
    << "// MIT License\n"
    << "//\n"
    << "// Copyright (c) " << year << " " << author << "\n"
    << "//\n"
    << "// Permission is hereby granted, free of charge, to any person obtaining a copy\n"
    << "// of this software and associated documentation files (the \"Software\"), to deal\n"
    << "// in the Software without restriction, including without limitation the rights\n"
    << "// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
    << "// copies of the Software, and to permit persons to whom the Software is\n"
    << "// furnished to do so, subject to the following conditions:\n"
    << "//\n"
    << "// The above copyright notice and this permission notice shall be included in all\n"
    << "// copies or substantial portions of the Software.\n"
    << "//\n"
    << "// THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
    << "// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
    << "// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
    << "// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
    << "// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
    << "// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
    << "// SOFTWARE.\n"
    << "//";
    s = ss.str();
  }
  else
  {
    return -1;
  }

  return 0;
};

auto const fn_sh = [](auto& s, auto const& m) {
  return exec(s, std::string(m[1]));
};

auto const fn_file = [](auto& s, auto const& m) {
  std::string file_path {m[1]};
  auto const tilde = file_path.find_first_of("~");
  if (tilde != std::string::npos)
  {
    file_path.replace(tilde, 1, std::getenv("HOME"));
  }

  std::ifstream file {file_path};
  if (! file.is_open())
  {
    return -1;
  }

  std::string content;
  content.assign((std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  if (content.empty())
  {
    return -1;
  }

  if (content.back() == '\n')
  {
    content.pop_back();
  }

  s = content;

  return 0;
};

auto const fn_env = [](auto& s, auto const& m) {
  std::string str {std::getenv(std::string(m[1]).c_str())};
  s = str;
  return 0;
};

auto const fn_headerpp = [](auto& s, auto const& m) {
  auto str {std::string(m[1])};

  std::string str_upper;
  for (auto const& e : str)
  {
    str_upper += std::toupper(e);
  }

  std::stringstream ss; ss
  << "#ifndef OB_" << str_upper << "_HH\n"
  << "#define OB_" << str_upper << "_HH\n"
  << "\n"
  << "#include <string>\n"
  << "#include <sstream>\n"
  << "#include <iostream>\n"
  << "\n"
  << "namespace OB\n"
  << "{\n"
  << "\n"
  << "class " << str << "\n"
  << "{\n"
  << "public:\n"
  << "  " << str << "();\n"
  << "  ~" << str << "();\n"
  << "\n"
  << "private:\n"
  << "\n"
  << "}; // class " << str << "\n"
  << "\n"
  << "} // namespace OB\n"
  << "\n"
  << "#endif // OB_" << str_upper << "_HH";
  s = ss.str();
  return 0;
};

auto const fn_sourcepp = [](auto& s, auto const& m) {
  auto str {std::string(m[1])};

  std::string str_lower;
  for (auto const& e : str)
  {
    str_lower += std::tolower(e);
  }

  std::stringstream ss; ss
  << "#include \"" << str_lower << ".hh\"\n"
  << "\n"
  << "#include <string>\n"
  << "#include <sstream>\n"
  << "#include <iostream>\n"
  << "\n"
  << "namespace OB\n"
  << "{\n"
  << "\n"
  << str << "::" << str << "()\n"
  << "{\n"
  << "}\n"
  << "\n"
  << str << "::~" << str << "()\n"
  << "{\n"
  << "}\n"
  << "\n"
  << "} // namespace OB";
  s = ss.str();
  return 0;
};

auto const fn_str = [](auto& s, auto const& m) {
  auto str {std::string(m[1])};
  s = "\"" + str + "\"";
  return 0;
};

auto const fn_prt = [](auto& s, auto const& m) {
  auto str {std::string(m[1])};
  replace_all(str, "\\n", "\n");
  std::cout << str << std::flush;
  return 0;
};

auto const fn_print = [](auto& s, auto const& m) {
  auto str {std::string(m[1])};
  replace_all(str, "\\n", "\n");

  std::stringstream ss;

  std::regex r {"\"([^\\r]+?)\""};
  std::smatch sm;
  while(regex_search(str, sm, r))
  {
    ss << sm[1];
    str = sm.suffix();
  }
  std::cout << ss.str() << std::flush;
  return 0;
};

auto const fn_nl = [](auto& s, auto const& m) {
  std::cout << "\n" << std::flush;
  return 0;
};

auto const fn_math_add = [](auto& s, auto const& m) {
  auto n1 {std::string(m[1])};
  auto n2 {std::string(m[2])};
  double total {std::stod(n1) + std::stod(n2)};
  std::stringstream ss; ss << total;
  s = ss.str();
  return 0;
};

auto const fn_math_subtract = [](auto& s, auto const& m) {
  auto n1 {std::string(m[1])};
  auto n2 {std::string(m[2])};
  double total {std::stod(n1) - std::stod(n2)};
  std::stringstream ss; ss << total;
  s = ss.str();
  return 0;
};

auto const fn_math_multiply = [](auto& s, auto const& m) {
  auto n1 {std::string(m[1])};
  auto n2 {std::string(m[2])};
  double total {std::stod(n1) * std::stod(n2)};
  std::stringstream ss; ss << total;
  s = ss.str();
  return 0;
};

auto const fn_math_divide = [](auto& s, auto const& m) {
  auto n1 {std::string(m[1])};
  auto n2 {std::string(m[2])};
  double total {std::stod(n1) / std::stod(n2)};
  std::stringstream ss; ss << total;
  s = ss.str();
  return 0;
};

auto const fn_cat = [](auto& s, auto const& m) {
  std::string str {std::string(m[1])};
  std::stringstream ss;

  std::regex r {"\"([^//r]+?)\""};
  std::smatch sm;
  while(regex_search(str, sm, r))
  {
    ss << sm[1];
    str = sm.suffix();
  }
  s = "\"" + ss.str() + "\"";

  return 0;
};

auto const fn_stringify = [](auto& s, auto const& m) {
  auto str {std::string(m[1])};
  std::stringstream ss; ss
  << "(str " << str << ")";
  s = ss.str();
  return 0;
};

auto const fn_term_width = [](auto& s, auto const& m) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int width = w.ws_col;
  s = std::to_string(width);
  return 0;
};

// define macros
// M8::set_macro(name, info, usage, regex, func)

void macros(M8& m8)
{
  m8.set_macro("stringify",
    "returns str macro",
    "stringify arg",
    "^(.+)$",
    fn_stringify);
  m8.set_macro("/",
    "divide two values",
    "/ n1 n2",
    "^([0-9]+)\\s+([0-9]+)$",
    fn_math_divide);

  m8.set_macro("*",
    "multiply two values",
    "* n1 n2",
    "^([0-9]+)\\s+([0-9]+)$",
    fn_math_multiply);

  m8.set_macro("-",
    "subtract two values",
    "- n1 n2",
    "^([0-9]+)\\s+([0-9]+)$",
    fn_math_subtract);

  m8.set_macro("+",
    "add two values",
    "+ n1 n2",
    "^([0-9]+)\\s+([0-9]+)$",
    fn_math_add);

  m8.set_macro("nl!",
    "print a newline to stdout",
    "!nl",
    "",
    fn_nl);

  m8.set_macro("print!",
    "print a string to stdout",
    "print! str",
    "^([^\\r]+?)$",
    fn_print);

  m8.set_macro("term-width",
    "returns width of terminal",
    "term-width",
    "",
    fn_term_width);

  m8.set_macro("prt!",
    "print raw to stdout",
    "prt! str",
    "^([^\\r]+?)$",
    fn_prt);

  m8.set_macro("str",
    "wrap argument in double quotes",
    "str arg",
    "^([^\\r]+)$",
    fn_str);

  m8.set_macro("sourcepp",
    "templates a c++ source file structure",
    "sourcepp str",
    "^\\((.+)\\)$",
    fn_sourcepp);

  m8.set_macro("headerpp",
    "templates a c++ header file structure",
    "headerpp str",
    "^\\((.+)\\)$",
    fn_headerpp);

  m8.set_macro("env",
    "gets an environment variable",
    "env str",
    "^(.+)$",
    fn_env);

  m8.set_macro("main",
    "template c++ main function",
    "main (str) EOF",
    "^\\(([^\\r]*?)\\)EOF$",
    fn_main);

  m8.set_macro("sh",
    "execute a shell command",
    "sh (str)",
    "^\\(([^;]+?) DONE\\)$",
    fn_sh);

  m8.set_macro("file",
    "read in a file",
    "file file_path",
    "^(.+?)$",
    fn_file);

  m8.set_macro("license",
    "insert a license header",
    "license (license, author, year)",
    "^\\(\"(.+?)\", \"(.+?)\", \"(.+?)\"\\)$",
    fn_license);

  m8.set_macro("repeat",
    "repeats the given input by n",
    "repeat [str EOF, int]",
    "^\\[([^\\r]+?) EOF, ([0-9]+?)\\]$",
    fn_repeat);

  m8.set_macro("add",
    "template add function",
    "add type",
    "^(.+?)$",
    fn_add);

  m8.set_macro("comment_header",
    "outputs the authors name, timestamp, version, and description in a c++ comment block",
    "comment_header (version, author, description)",
    "^\\(([.0-9]+?), \"(.+?)\", \"(.+?)\"\\)$",
    fn_comment_header);
}
} // namespace Macros
