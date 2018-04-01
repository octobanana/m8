#include "macros.hh"
#include "m8.hh"
#include "sys_command.hh"
#include "http.hh"
#include "crypto.hh"

#include "color.hh"
namespace Cl = OB::Color;

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include "json.hh"
using Json = nlohmann::json;

#define FMT_HEADER_ONLY
#include <fmt/format.h>
// #include <fmt/ostream.h>
// using namespace fmt::literals;

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
#include <cmath>
#include <utility>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace Macros
{

// cpp macros
#define var auto
#define let auto const
#define fn auto

// shared database for key value pairs
var db = std::map<std::string, std::string>();

// prototypes
fn repeat(std::string const& str, size_t num) -> std::string;
fn replace_all(std::string& str, std::string const& key, std::string const& val) -> void;
fn unescape_str(std::string& str) -> void;
fn delimit(std::string const& str, std::string const& delim) -> std::vector<std::string>;
fn delimit_first(std::string const& str, std::string const& delim) -> std::vector<std::string>;
fn format(std::string str, std::string const key, std::vector<std::string> const& args) -> std::string;
fn str_count(std::string const& str, std::string const& s) -> size_t;

// helper functions
fn repeat(std::string const& str, size_t num) -> std::string
{
  if (num < 2) return str;
  auto res = std::string();
  for (size_t i = 0; i < num; ++i)
  {
    res += str;
  }
  return res;
}

void unescape_str(std::string& str)
{
  size_t pos {0};

  for (;; ++pos)
  {
    pos = str.find("\\", pos);
    if (pos == std::string::npos || pos + 1 == std::string::npos) break;
    if (str.at(pos + 1) == 'n')
    {
      str.replace(pos, 2, "\n");
    }
    else if (str.at(pos + 1) == 't')
    {
      str.replace(pos, 2, "\t");
    }
    else if (str.at(pos + 1) == 'r')
    {
      str.replace(pos, 2, "\r");
    }
    else if (str.at(pos + 1) == 'a')
    {
      str.replace(pos, 2, "\a");
    }
    else if (str.at(pos + 1) == 'b')
    {
      str.replace(pos, 2, "\b");
    }
    else if (str.at(pos + 1) == 'f')
    {
      str.replace(pos, 2, "\f");
    }
    else if (str.at(pos + 1) == 'v')
    {
      str.replace(pos, 2, "\v");
    }
    else if (str.at(pos + 1) == '\\')
    {
      str.replace(pos, 2, "\\");
    }
    else if (str.at(pos + 1) == '"')
    {
      str.replace(pos, 2, "\"");
    }
    else if (str.at(pos + 1) == '\'')
    {
      str.replace(pos, 2, "'");
    }
    else if (str.at(pos + 1) == '?')
    {
      str.replace(pos, 2, "\?");
    }
  }
}

void replace_all(std::string& str, std::string const& key, std::string const& val)
{
  size_t pos {0};

  for (;;)
  {
    pos = str.find(key, pos);
    if (pos == std::string::npos) break;
    str.replace(pos, key.size(), val);
    ++pos;
  }
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

std::vector<std::string> delimit_first(std::string const& str, std::string const& delim)
{
  auto vtok = std::vector<std::string>();
  auto pos = str.find(delim);
  if (pos != std::string::npos) {
    vtok.emplace_back(str.substr(0, pos));
    if (pos + delim.size() != std::string::npos)
    {
      vtok.emplace_back(str.substr(pos + delim.size()));
    }
  }
  return vtok;
}

std::string format(std::string str, std::string const key, std::vector<std::string> const& args)
{
  size_t pos {0};

  for (auto const& e : args)
  {
    pos = str.find(key, pos);
    if (pos == std::string::npos) break;
    str.replace(pos, key.size(), e);
    ++pos;
  }
  return str;
}

size_t str_count(std::string const& str, std::string const& s)
{
  size_t count {0};
  size_t pos {0};

  for (;;)
  {
    pos = str.find(s, pos);
    if (pos == std::string::npos) break;
    ++count;
    ++pos;
  }
  return count;
}

// macro functions
// std::function<std::string(std::string& response, std::smatch const& match)>

// auto const fn_main = [](auto& ctx) {
//   auto str = ctx.args.at(1);
//   std::stringstream ss;
//   ss << "int main(int argc, char* argv[])\n"
//      << "{"
//      << str
//      << "}";
//   ctx.str = ss.str();
//   return 0;
// };

auto const fn_repeat = [](auto& ctx) {
  auto res = repeat(ctx.args.at(1), std::stoul(ctx.args.at(2)));
  std::stringstream ss;
  ss << res;
  ctx.str = ss.str();
  return 0;
};

// auto const fn_add = [](auto& ctx) {
//   auto type = std::string(m[1]);
//   std::stringstream ss;
//   ss << type << " add(" << type << " x, " << type << " y)\n"
//      << "{\n"
//      << "  return x + y;\n"
//      << "}\n";
//   s = ss.str();
//   return 0;
// };

auto const fn_comment_header = [](auto& ctx) {
  std::stringstream ss; ss
  << "// timestamp:   " << std::time(nullptr) << "\n"
  << "// Version:     " << ctx.args.at(1) << "\n"
  << "// Author:      " << ctx.args.at(2) << "\n"
  << "// Description: " << ctx.args.at(3) << "\n";
  ctx.str = ss.str();
  return 0;
};

auto const fn_license = [](auto& ctx) {
  auto license = ctx.args.at(1);
  auto author = ctx.args.at(2);
  auto year = ctx.args.at(3);

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
    << "//\n";
    ctx.str = ss.str();
  }
  else
  {
    return -1;
  }

  return 0;
};

auto const fn_sh = [](auto& ctx) {
  return exec(ctx.str, ctx.args.at(1));
};

auto const fn_file = [](auto& ctx) {
  var file_path = ctx.args.at(1);
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

  ctx.str = content;

  return 0;
};

auto const fn_env = [](auto& ctx) {
  std::string str {std::getenv(ctx.args.at(1).c_str())};
  ctx.str = str;
  return 0;
};

auto const fn_headerpp = [](auto& ctx) {
  auto str = ctx.args.at(1);

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
  ctx.str = ss.str();
  return 0;
};

auto const fn_sourcepp = [](auto& ctx) {
  auto str = ctx.args.at(1);

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
  ctx.str = ss.str();
  return 0;
};

auto const fn_str = [](auto& ctx) {
  auto str = ctx.args.at(1);
  ctx.str = "\"" + str + "\"";
  return 0;
};

auto const fn_prt = [](auto& ctx) {
  auto str = ctx.args.at(1);
  unescape_str(str);
  std::cout << str << std::flush;
  return 0;
};

auto const fn_print = [](auto& ctx) {
  std::stringstream ss;
  for (size_t i = 1; i < ctx.args.size(); ++i)
  {
    ss << ctx.args.at(i);
  }
  std::string str = ss.str();
  unescape_str(str);
  std::cout << str << std::flush;
  return 0;
};

auto const fn_stdout_nl = [](auto& ctx) {
  std::cout << "\n" << std::flush;
  return 0;
};

auto const fn_nl = [](auto& ctx) {
  ctx.str = "\n";
  return 0;
};

auto const fn_math_pow = [](auto& ctx) {
  auto n1 = ctx.args.at(1);
  auto n2 = ctx.args.at(2);
  auto total = std::pow(std::stod(n1), std::stod(n2));
  std::stringstream ss; ss << total;
  ctx.str = ss.str();
  return 0;
};

auto const fn_math_mod = [](auto& ctx) {
  auto n1 = ctx.args.at(1);
  auto n2 = ctx.args.at(2);
  double total {std::fmod(std::stod(n1), std::stod(n2))};
  std::stringstream ss; ss << total;
  ctx.str = ss.str();
  return 0;
};

auto const fn_math_add = [](auto& ctx) {
  auto n1 {ctx.args.at(1)};
  auto n2 {ctx.args.at(2)};
  double total {std::stod(n1) + std::stod(n2)};
  std::stringstream ss; ss << total;
  ctx.str = ss.str();
  return 0;
};

auto const fn_math_subtract = [](auto& ctx) {
  auto n1 {ctx.args.at(1)};
  auto n2 {ctx.args.at(2)};
  double total {std::stod(n1) - std::stod(n2)};
  std::stringstream ss; ss << total;
  ctx.str = ss.str();
  return 0;
};

auto const fn_math_multiply = [](auto& ctx) {
  auto n1 {ctx.args.at(1)};
  auto n2 {ctx.args.at(2)};
  double total {std::stod(n1) * std::stod(n2)};
  std::stringstream ss; ss << total;
  ctx.str = ss.str();
  return 0;
};

auto const fn_math_divide = [](auto& ctx) {
  auto n1 {ctx.args.at(1)};
  auto n2 {ctx.args.at(2)};
  double total {std::stod(n1) / std::stod(n2)};
  std::stringstream ss; ss << total;
  ctx.str = ss.str();
  return 0;
};

auto const fn_cat = [](auto& ctx) {
  auto str = ctx.args.at(1);
  std::stringstream ss;

  std::regex r {"\"([^//r]+?)\""};
  std::smatch sm;
  while(regex_search(str, sm, r))
  {
    ss << sm[1];
    str = sm.suffix();
  }
  ctx.str = "\"" + ss.str() + "\"";

  return 0;
};

auto const fn_term_width = [](auto& ctx) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int width = w.ws_col;
  ctx.str = std::to_string(width);
  return 0;
};

auto const fn_http_get = [&](auto& ctx) {
  auto url = ctx.args.at(1);
  Http api;
  api.req.method = "GET";
  api.req.url = url;
  api.run();
  ctx.str = api.res.body;
  if (api.res.status != 200) return -1;
  return 0;
};

auto const fn_http_post = [&](auto& ctx) {
  auto url = ctx.args.at(1);
  auto data = ctx.args.at(2);
  unescape_str(url);
  unescape_str(data);
  Http api;
  api.req.method = "POST";
  api.req.url = url;
  api.req.data = data;
  api.run();
  ctx.str = api.res.body;
  if (api.res.status != 200) return -1;
  return 0;
};

auto const fn_get = [&](auto& ctx) {
  auto key = ctx.args.at(1);
  std::string val;
  if (db.find(key) != db.end())
  {
    val = db[key];
  }
  ctx.str = val;
  return 0;
};

auto const fn_set = [&](auto& ctx) {
  auto key = ctx.args.at(1);

  auto str = std::string();
  auto ss = std::stringstream();
  for (size_t i = 2; i < ctx.args.size(); ++i)
  {
    ss << ctx.args.at(i);
  }
  str = ss.str();
  unescape_str(str);

  db[key] = str;
  return 0;
};

auto const fn_sha256 = [&](auto& ctx) {
  ctx.str = Crypto::sha256(ctx.args.at(1));
  return 0;
};

auto const fn_template = [&](auto& ctx) {
  auto key = ctx.args.at(1);
  if (db.find(key) == db.end()) return -1;
  auto tmp = db[key];

  // template args start at 2
  for (size_t i = 2; i < ctx.args.size(); ++i)
  {
    auto pos = ctx.args.at(i).find(":");
    if (pos == std::string::npos)
    {
      continue;
    }
    auto name = "{" + ctx.args.at(i).substr(0, pos) + "}";
    auto str = ctx.args.at(i).substr(pos + 1);
    auto pos_name = tmp.find(name);
    if (pos_name == std::string::npos)
    {
      continue;
    }
    tmp = tmp.replace(tmp.find(name), name.size(), str);
  }

  unescape_str(tmp);
  replace_all(tmp, "[[", "((");
  replace_all(tmp, "]]", "))");
  ctx.str = tmp;

  return 0;
};

// define macros
// M8::set_macro(name, info, usage, regex, func)
void macros(M8& m8)
{
  // m8.set_macro("",
  //   "",
  //   "",
  //   "^(.*)$",
  //   [&](auto& ctx) {
  //   var str = ctx.args.at(1);
  //   ctx.str = str;
  //   return 0;
  //   });

  m8.set_macro("printc",
    "",
    "",
    "",
    [&](auto& ctx) {
    std::string col {ctx.args.at(1)};
    std::stringstream ss;
    for (size_t i = 2; i < ctx.args.size(); ++i)
    {
      ss << ctx.args.at(i);
    }
    std::string str = ss.str();
    unescape_str(str);
    std::cout << AEC::fg_true(col) << str << AEC::reset << std::flush;
    return 0;
    });

  m8.set_macro("def",
    "define a macro",
    "def <key> <val>",
    "^(.+?)\\s+(?:M8\\!|)([^\\r]+?)(?:\\!8M|$)",
    [&](auto& ctx) {
    var name = ctx.args.at(1);

    m8.set_macro(name, "", "", "", [&, name](auto& ctx) {
      if (db.find(name) == db.end()) return -1;
      auto tmp = db[name];

      auto arg_map = std::map<std::string, std::string>();
      for (size_t i = 1; i < ctx.args.size(); ++i)
      {
        auto pos = ctx.args.at(i).find(":");
        if (pos == std::string::npos) continue;
        // auto name = "{" + ctx.args.at(i).substr(0, pos) + "}";
        auto name = ctx.args.at(i).substr(0, pos);
        auto str = ctx.args.at(i).substr(pos + 1);
        arg_map[name] = str;
      }

      // size_t pos_name {0};
      // for (;;)
      // {
      //   pos_name = tmp.find(name);
      //   if (pos_name == std::string::npos) break;
      //   tmp = tmp.replace(pos_name, name.size(), str);
      //   ++pos_name;
      // }

      if (! arg_map.empty())
      {
        size_t pos {0};
        std::smatch match;
        std::regex rx {"\\{(\\w+)(?:(:\\d\\.\\d.?)?)\\}"};
        std::string str {tmp};

        while (std::regex_search(str, match, rx))
        {
          std::string m {match[0]};
          std::string first {match[1]};

          pos += std::string(match.prefix()).size();

          if (arg_map.find(first) == arg_map.end())
          {
            pos += m.size();
            str = str.substr(m.size());
            continue;
          }

          tmp.replace(pos, m.size(), arg_map[first]);
          pos += arg_map[first].size();
          str = match.suffix();
        }
      }

      unescape_str(tmp);
      replace_all(tmp, "[[", "((");
      replace_all(tmp, "]]", "))");
      ctx.str = tmp;
      // std::cerr << "run-" << name << ":\n" << tmp << "~~~\n";
      return 0;
    });

    auto str = std::string();
    auto ss = std::stringstream();
    for (size_t i = 2; i < ctx.args.size(); ++i)
    {
      ss << ctx.args.at(i);
    }
    str = ss.str();
    // unescape_str(str);

    // std::cerr << "def-" << name << ":\n" << str << "~~~\n";
    db[name] = str;

    return 0;
    });

  m8.set_macro("c",
    "run c code snippet",
    "c <str>",
    "^([^\\r]+)$",
    [&](auto& ctx) {
    // check cache for value
    var ckey = Crypto::sha256(ctx.args.at(0));
    if (ctx.cache.get(ckey, ctx.str)) return 0;

    var flags = std::string();
    if (db.find("c-flags") != db.end())
    {
      flags = db["c-flags"];
    }
    var headers = std::string();
    if (db.find("c-headers") != db.end())
    {
      headers = db["c-headers"];
    }
    var code = fmt::format("{}\n\nint main(){{\n{}\n}}", headers, ctx.args.at(1));

    let path = std::string(".m8/.m8-c.c");
    var ofile = std::ofstream(path);
    ofile << code;
    ofile.close();

    var status = int();
    var out = std::string();
    let compile = std::string("gcc .m8/.m8-c.c -o .m8/.m8-c " + flags);
    status = exec(out, compile);
    if (status != 0) return -1;

    status = exec(ctx.str, ".m8/.m8-c");
    if (status != 0) return -1;

    fs::remove(fs::path(".m8/.m8-c"));
    fs::remove(fs::path(".m8/.m8-c.c"));

    // add new value to cache
    ctx.cache.set(ckey, ctx.str);

    return 0;
    });

  m8.set_macro("cpp",
    "run cpp code snippet",
    "cpp <str>",
    "^([^\\r]+)$",
    [&](auto& ctx) {
    // check cache for value
    var ckey = Crypto::sha256(ctx.args.at(0));
    if (ctx.cache.get(ckey, ctx.str)) return 0;

    var flags = std::string();
    if (db.find("cpp-flags") != db.end())
    {
      flags = db["cpp-flags"];
    }
    var headers = std::string();
    if (db.find("cpp-headers") != db.end())
    {
      headers = db["cpp-headers"];
    }
    var code = fmt::format("{}\n\nint main(){{\n{}\n}}", headers, ctx.args.at(1));

    let path = std::string(".m8/.m8-cpp.cc");
    var ofile = std::ofstream(path);
    ofile << code;
    ofile.close();

    var status = int();
    var out = std::string();
    let compile = std::string("g++ .m8/.m8-cpp.cc -o .m8/.m8-cpp " + flags);
    status = exec(out, compile);
    if (status != 0) return -1;

    status = exec(ctx.str, ".m8/.m8-cpp");
    if (status != 0) return -1;

    fs::remove(fs::path(".m8/.m8-cpp"));
    fs::remove(fs::path(".m8/.m8-cpp.cc"));

    // add new value to cache
    ctx.cache.set(ckey, ctx.str);

    return 0;
    });

  m8.set_macro("script",
    "run a script",
    "script <str>",
    "^([^\\r]+)$",
    [&](auto& ctx) {
    // check cache for value
    var ckey = Crypto::sha256(ctx.args.at(0));
    if (ctx.cache.get(ckey, ctx.str)) return 0;

    var str = ctx.args.at(1);
    let path = std::string(".m8/.m8-script.tmp.m8");
    var ofile = std::ofstream(path);
    ofile << str;
    ofile.close();
    fs::permissions(path, fs::perms::owner_exec | fs::perms::add_perms);
    let status = exec(ctx.str, path);
    fs::remove(fs::path(path));

    // add new value to cache
    ctx.cache.set(ckey, ctx.str);

    return status;
    });

  m8.set_macro("mod",
    "insert module from github",
    "mod file-url",
    "",
    [&](auto& ctx) {
    // check cache for value
    var ckey = Crypto::sha256(ctx.args.at(0));
    if (ctx.cache.get(ckey, ctx.str)) return 0;

    var version = ctx.args.at(1);
    var name = ctx.args.at(2);
    var url = std::string(
      "https://raw.githubusercontent.com/octobanana/m8-modules/" +
      version + "/" + name);
    Http api;
    api.req.url = url;

    api.run();
    if (api.res.status != 200) return -1;
    ctx.str = api.res.body;

    // add new value to cache
    ctx.cache.set(ckey, ctx.str);

    return 0;
    });

  m8.set_macro("tmp",
    "call a macro template",
    "template <args...>",
    // "^\"(.+?)\"\\s*([^\\r]*)$",
    "",
    fn_template);

  m8.set_macro("cmp",
    "",
    "",
    "",
    [&](auto& ctx) {
      auto n1 = std::stod(ctx.args.at(1));
      auto n2 = std::stod(ctx.args.at(2));
      int res = 0;
      if (n1 > n2) res = 1;
      else if (n1 == n2) res = 0;
      else if (n1 < n2) res = -1;
      ctx.str = std::to_string(res);
      return 0;
    });

  m8.set_macro("count",
    "count",
    "count",
    "^\"(.+)\"\\s+\"(.*)\"$",
    [&](auto& ctx) {
      auto search = ctx.args.at(1);
      auto str = ctx.args.at(2);
      auto count = str_count(str, search);
      ctx.str = std::to_string(count);
      return 0;
    });

  m8.set_macro("meta",
    "meta",
    "meta",
    "^(.*)$",
    [&](auto& ctx) {
      ctx.str = "(print! \"Meta Macro Success!!!\n\")";
      return 0;
    });

   m8.set_macro("sha256",
     "returns an sha256 hash of input string",
     "sha256 \"str\"",
     "^\"([^\\r]+?)\"$",
     fn_sha256);

  m8.set_macro("get",
    "get value of key from db",
    "get <key>",
    "^\"(.+)\"$",
    fn_get);

  m8.set_macro("set",
    "set key to value in db",
    "set <key> <val>",
    "^\"(.+?)\"\\s+([^\\r]+)$",
    fn_set);

  m8.set_macro("http-get",
    "http get request",
    "get \"url\"",
    // "^\"(.+)\"$",
    "",
    fn_http_get);

  m8.set_macro("http-post",
    "http post request",
    "post \"url\" \"data\"",
    // regex is invalid
    // "^\"(.+?)\"\\s+\"((?:[^\"\\]|\\.)*)\"$",
    "",
    fn_http_post);

  m8.set_macro("floor",
    "floor a decimal",
    "floor n",
    "",
    [&](auto& ctx) {
      auto n = std::stod(ctx.args.at(1));
      n = std::floor(n);
      std::stringstream ss; ss << n;
      ctx.str = ss.str();
      return 0;
    });

  m8.set_macro("in",
    "get input from stdin",
    "in val",
    "",
    [&](auto& ctx) {
      auto str = std::string();
      std::cout << "> ";
      std::getline(std::cin, str);
      ctx.str = str;
      return 0;
    });

  m8.set_macro("round",
    "round a decimal",
    "round n",
    "",
    [&](auto& ctx) {
      auto n = std::stod(ctx.args.at(1));
      n = std::round(n);
      std::stringstream ss; ss << n;
      ctx.str = ss.str();
      return 0;
    });

  m8.set_macro("^",
    "the exponent operator",
    "/ n1 n2",
    "",
    fn_math_pow);

  m8.set_macro("%",
    "the modulo operator",
    "% n1 n2",
    "",
    fn_math_mod);

  m8.set_macro("/",
    "the division operator",
    "/ n1 n2",
    "",
    fn_math_divide);

  m8.set_macro("*",
    "the multiplication operator",
    "* n1 n2",
    "",
    fn_math_multiply);

  m8.set_macro("-",
    "the subtraction operator",
    "- n1 n2",
    "",
    fn_math_subtract);

  m8.set_macro("+",
    "the addition operator",
    "+ n1 n2",
    "",
    // "^([0-9]+)\\s+([0-9]+)$",
    fn_math_add);

  m8.set_macro("nl",
    "returns a newline",
    "nl",
    "",
    fn_nl);

  m8.set_macro("nl!",
    "print a newline to stdout",
    "!nl",
    "",
    fn_stdout_nl);

  m8.set_macro("print!",
    "print a string to stdout",
    "print! str",
    "",
    fn_print);

  m8.set_macro("term-width",
    "returns width of terminal",
    "term-width",
    "",
    fn_term_width);

  m8.set_macro("cat",
    "cat strings together",
    "cat str",
    "^([^\\r]+?)$",
    fn_cat);

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

  // m8.set_macro("main",
  //   "template c++ main function",
  //   "main (str) EOF",
  //   "^\\(([^\\r]*?)\\)EOF$",
  //   fn_main);

  m8.set_macro("sh",
    "execute a shell command",
    "sh (str)",
    "^([^\\r]+)$",
    fn_sh);

  m8.set_macro("file",
    "read in a file",
    "file file_path",
    "",
    fn_file);

  m8.set_macro("license",
    "insert a license header",
    "license (license, author, year)",
    // "^\\(\"(.+?)\", \"(.+?)\", \"(.+?)\"\\)$",
    "",
    fn_license);

  m8.set_macro("repeat",
    "repeats the given string 'n' times",
    "repeat \"str\", int",
    "^\"([^\\r]+?)\", ([0-9]+)$",
    fn_repeat);

  // m8.set_macro("add",
  //   "template add function",
  //   "add type",
  //   "^(.+?)$",
  //   fn_add);

  m8.set_macro("comment_header",
    "outputs the authors name, timestamp, version, and description in a c++ comment block",
    "comment_header (version, author, description)",
    "^\\(([.0-9]+?), \"(.+?)\", \"(.+?)\"\\)$",
    fn_comment_header);
}
} // namespace Macros
