#include "m8/macros.hh"

#include "m8/m8.hh"

#include "ob/sys_command.hh"
#include "ob/crypto.hh"
#include "ob/string.hh"
#include "ob/http.hh"

#include "ob/term.hh"
namespace aec = OB::Term::ANSI_Escape_Codes;

#include "lib/json.hh"
using Json = nlohmann::json;

#include <unistd.h>
#include <sys/ioctl.h>

#include <ctime>
#include <cmath>
#include <cctype>
#include <cstddef>
#include <cstdlib>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <unordered_map>
#include <vector>
#include <regex>
#include <functional>
#include <stdexcept>
#include <utility>
#include <random>

#include <filesystem>
namespace fs = std::filesystem;

namespace Macros
{

// variables
std::unordered_map<std::string, std::string> db;
std::string m8_delim_start;
std::string m8_delim_end;

// prototypes
int ftostr(std::string f, std::string& s);

int ftostr(std::string f, std::string& s)
{
  std::ifstream file {f};
  if (! file.is_open()) return -1;
  file.seekg(0, std::ios::end);
  std::size_t size (static_cast<std::size_t>(file.tellg()));
  std::string content (size, ' ');
  file.seekg(0);
  file.read(&content[0], static_cast<std::streamsize>(size));
  s = content;
  return 0;
}

void macros(M8& m8)
{

// macro functions

auto const fn_repeat = [](auto& ctx) {
  auto res = OB::String::repeat(ctx.args.at(1), std::stoul(ctx.args.at(2)));
  std::stringstream ss;
  ss << res;
  ctx.str = ss.str();
  return 0;
};

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
  // TODO return error on failed shell command
  return OB::exec(ctx.str, ctx.args.at(1));
};

auto const fn_file = [](auto& ctx) {
  auto file_path = ctx.args.at(1);
  if (file_path.empty())
  {
    ctx.err_msg = "file name is empty";
    return -1;
  }
  if (file_path.front() == '~')
  {
    file_path.replace(0, 1, std::getenv("HOME"));
  }

  std::ifstream file {file_path};
  if (! file.is_open())
  {
    ctx.err_msg = "could not open file";
    return -1;
  }

  std::string content;
  content.assign((std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  ctx.str = content;

  return 0;
};

auto const fn_env = [](auto& ctx) {
  const char *e = std::getenv(ctx.args.at(1).c_str());
  if (e)
  {
    ctx.str = e;
  }
  else
  {
    ctx.str = "0";
  }
  return 0;
};

auto const fn_headerpp = [](auto& ctx) {
  auto str = ctx.args.at(1);

  std::string str_upper;
  for (auto const& e : str)
  {
    str_upper += std::toupper(e);
  }

  auto tnano = std::chrono::system_clock::now().time_since_epoch();
  long int uuid = std::chrono::duration_cast<std::chrono::nanoseconds>(tnano).count();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::string sid {OB::Crypto::sha256(std::to_string(uuid) + std::to_string(gen()))};
  sid = sid.substr(0, 8);

  std::stringstream ss; ss
  << "#ifndef OB_" << str_upper << "_HH_" << sid << "\n"
  << "#define OB_" << str_upper << "_HH_" << sid << "\n"
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
  str = OB::String::unescape(str);
  std::cout << str << std::flush;
  return 0;
};

auto const fn_print = [](auto& ctx) {
  std::stringstream ss;
  for (std::size_t i = 1; i < ctx.args.size(); ++i)
  {
    ss << ctx.args.at(i);
  }
  std::string str = ss.str();
  str = OB::String::unescape(str);
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

  if (api.res.status != 200)
  {
    return -1;
  }

  return 0;
};

auto const fn_http_post = [&](auto& ctx) {
  auto url = ctx.args.at(1);
  auto data = ctx.args.at(2);
  url = OB::String::unescape(url);
  data = OB::String::unescape(data);
  Http api;
  api.req.method = "POST";
  api.req.url = url;
  api.req.data = data;
  api.run();
  ctx.str = api.res.body;

  if (api.res.status != 200)
  {
    return -1;
  }

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
  for (std::size_t i = 2; i < ctx.args.size(); ++i)
  {
    ss << ctx.args.at(i);
  }
  str = ss.str();
  // str = OB::String::unescape(str);

  db[key] = str;
  return 0;
};

auto const fn_sha256 = [&](auto& ctx) {
  ctx.str = OB::Crypto::sha256(ctx.args.at(1));
  return 0;
};

auto const fn_template = [&](auto& ctx) {
  auto key = ctx.args.at(1);
  if (db.find(key) == db.end()) return -1;
  auto tmp = db[key];

  // template args start at 2
  for (std::size_t i = 2; i < ctx.args.size(); ++i)
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

  tmp = OB::String::unescape(tmp);
  tmp = OB::String::replace_all(tmp, "[[", "((");
  tmp = OB::String::replace_all(tmp, "]]", "))");
  ctx.str = tmp;

  return 0;
};

auto const fn_math_abs = [&](auto& ctx) {
  auto n = std::stod(ctx.args.at(1));
  n = std::fabs(n);
  std::stringstream ss; ss << n;
  ctx.str = ss.str();
  return 0;
};

auto const fn_date = [&](auto& ctx) {
  std::stringstream ss;
  std::time_t t {std::time(nullptr)};
  std::tm tm = *std::localtime(&t);
  if (ctx.args.size() == 2)
  {
    auto str = ctx.args.at(1);
    ss << std::put_time(&tm, str.c_str());
    ctx.str = ss.str();
  }
  else
  {
    ss << std::asctime(&tm);
    ctx.str = ss.str();
    ctx.str.pop_back();
  }
  return 0;
};

auto const fn_math_round = [&](auto& ctx) {
  auto n = std::stod(ctx.args.at(1));
  n = std::round(n);
  std::stringstream ss; ss << n;
  ctx.str = ss.str();
  return 0;
};

auto const fn_in = [&](auto& ctx) {
  auto str = std::string();
  std::cout << "> ";
  std::getline(std::cin, str);
  ctx.str = str;
  return 0;
};

auto const fn_math_floor = [&](auto& ctx) {
  auto n = std::stod(ctx.args.at(1));
  n = std::floor(n);
  std::stringstream ss; ss << n;
  ctx.str = ss.str();
  return 0;
};

auto const fn_info = [&](auto& ctx) {
  auto str = ctx.args.at(1);
  ctx.str = m8.macro_info(str);
  return 0;
};

auto const fn_cpp_enum = [&](auto& ctx) {
  auto str = ctx.args.at(1);
  std::stringstream ss;
  ss.str(str);

  std::string name;
  ss >> name;

  std::vector<std::string> ve;
  std::string elem;
  while (ss >> elem)
  {
    ve.emplace_back(elem);
  }

  auto const list_enum = [&ve]() {
    std::string s;
    if (ve.size() > 1)
    {
      for (std::size_t i = 0; i < ve.size() - 1; ++i)
      {
        s += ve.at(i) + ", ";
      }
    }
    s += ve.back();
    return s;
  };

  auto const str_enum = [&ve]() {
    std::string s;
    if (ve.size() > 1)
    {
      for (auto const& e : ve)
      {
        s += "case " + e + ": return \"" + e +  "\";";
      }
    }
    return s;
  };

  std::stringstream res;
  res
  << "struct " << name
  << " { enum en { " << list_enum() << " } e; "
  << "std::string s() const { switch (e) { " << str_enum()
  << " default: return {}; } } "
  << name + "& operator=(" << name << "::en rhs) { std::swap(e, rhs); return *this; }"
  << " };"
  // << "friend std::ostream& operator<<(std::ostream& os, const " << name << "& obj); }; "
  // << "inline bool operator==(" << name << " const& lhs, "<< name << " const& rhs) { return lhs == rhs; }; "
  // << "std::ostream& operator<<(std::ostream& os, " << name << " const& obj) { os << obj.s(); return os; }"
  << "\n";

  ctx.str = res.str();
  return 0;
};

auto const fn_version = [&](auto& ctx) {
  auto name = ctx.args.at(1);
  std::string str;
  if (ftostr(name, str) != 0) return -1;
  auto num = std::stoi(str);
  ++num;
  ctx.str = std::to_string(num);
  std::ofstream ofile {name};
  if (! ofile.is_open()) return -1;
  ofile << num;
  return 0;
};

auto const fn_eq = [&](auto& ctx) {
  auto n1 = ctx.args.at(1);
  auto n2 = ctx.args.at(2);
  std::string res {"0"};
  if (n1 == n2)
  {
    res = "1";
  }
  ctx.str = res;
  return 0;
};

auto const fn_if_else = [&](auto& ctx) {
  auto cond = std::stoi(ctx.args.at(1));
  if (cond)
  {
    ctx.str = ctx.args.at(2);
  }
  else
  {
    ctx.str = ctx.args.at(3);
  }
  return 0;
};

auto const fn_if_else_s = [&](auto& ctx) {
  auto cond = std::stoi(ctx.args.at(1));
  if (cond)
  {
    ctx.str = ctx.args.at(2);
  }
  return 0;
};

auto const fn_nop = [&](auto& ctx) {
  ctx.str = ctx.args.at(1);
  return 0;
};

auto const fn_if = [&](auto& ctx) {
  std::string cond {ctx.args.at(1)};
  std::string first {ctx.args.at(2)};
  std::string second {ctx.args.at(3)};

  if (std::stoi(cond) != 0)
  {
    ctx.str = first;
  }
  else
  {
    ctx.str = second;
  }

  return 0;
};

auto const fn_printc = [&](auto& ctx) {
  std::string col {ctx.args.at(1)};
  std::stringstream ss;
  for (std::size_t i = 2; i < ctx.args.size(); ++i)
  {
    ss << ctx.args.at(i);
  }
  std::string str = ss.str();
  str = OB::String::unescape(str);
  std::cout << aec::fg_true(col) << str << aec::reset << std::flush;
  return 0;
};

// auto const fn_def_l = [&](auto& ctx) {
//   auto delim_start = m8_delim_start;
//   auto delim_end = m8_delim_end;

//   auto name = ctx.args.at(1);
//   auto str = ctx.args.at(2);

//   db[name] = str;

//   m8.set_macro(name, "def-macro", "", "", [&, name, delim_start, delim_end](auto& ctx) {
//     if (db.find(name) == db.end()) return -1;
//     auto tmp = db[name];

//     auto arg_map = std::unordered_map<std::string, std::string>();
//     for (std::size_t i = 1; i < ctx.args.size(); ++i)
//     {
//       auto pos = ctx.args.at(i).find(":");
//       if (pos == std::string::npos) continue;
//       auto key = ctx.args.at(i).substr(0, pos);
//       auto str = ctx.args.at(i).substr(pos + 1);
//       arg_map[key] = str;
//     }

//     tmp = OB::String::xformat(tmp, arg_map);
//     if (OB::String::ends_with(tmp, "\n\n"))
//     {
//       tmp = OB::String::replace_last(tmp, "\n\n", "\n");
//     }
//     tmp = OB::String::replace_first(tmp, "`" + delim_start, delim_start);
//     tmp = OB::String::replace_last(tmp, delim_end + "`", delim_end);
//     ctx.str = tmp;
//     return 0;
//   });

//   return 0;
// };

auto const fn_def = [&](auto& ctx) {
  auto delim_start = m8_delim_start;
  auto delim_end = m8_delim_end;

  auto name = ctx.args.at(1);
  auto info = ctx.args.at(2);
  // TODO move this to set_macro()
  std::string rx = "^" + ctx.args.at(3) + "$";
  auto str = ctx.args.at(4);
  db[name] = str;

  m8.set_macro(name, "def-macro", info, rx, [&, name, delim_start, delim_end](auto& ctx) {
    if (db.find(name) == db.end()) return -1;
    auto tmp = db[name];

    auto arg_map = std::unordered_map<std::string, std::string>();
    for (std::size_t i = 0; i < ctx.args.size(); ++i)
    {
      arg_map[std::to_string(i)] = ctx.args.at(i);
    }

    tmp = OB::String::xformat(tmp, arg_map);
    // if (OB::String::ends_with(tmp, "\n\n"))
    // {
    //   tmp = OB::String::replace_last(tmp, "\n\n", "\n");
    // }
    // TODO should unescape be removed?
    // tmp = OB::String::unescape(tmp);
    tmp = OB::String::replace_all(tmp, "`" + delim_start, delim_start);
    tmp = OB::String::replace_all(tmp, delim_end + "`", delim_end);
    ctx.str = tmp;
    return 0;
  });

  return 0;
};

auto const fn_def_s = [&](auto& ctx) {
  auto delim_start = m8_delim_start;
  auto delim_end = m8_delim_end;

  auto name = ctx.args.at(1);
  std::string info {"{args:all}"};
  std::string rx {"^{!all}$"};
  auto str = ctx.args.at(2);
  db[name] = str;

  m8.set_macro(name, "def-macro", info, rx, [&, name, delim_start, delim_end](auto& ctx) {
    if (db.find(name) == db.end()) return -1;
    auto tmp = db[name];

    auto arg_map = std::unordered_map<std::string, std::string>();
    for (std::size_t i = 0; i < ctx.args.size(); ++i)
    {
      arg_map[std::to_string(i)] = ctx.args.at(i);
    }

    tmp = OB::String::xformat(tmp, arg_map);
    // if (OB::String::ends_with(tmp, "\n\n"))
    // {
    //   tmp = OB::String::replace_last(tmp, "\n\n", "\n");
    // }
    // TODO should unescape be removed?
    // tmp = OB::String::unescape(tmp);
    tmp = OB::String::replace_all(tmp, "`" + delim_start, delim_start);
    tmp = OB::String::replace_all(tmp, delim_end + "`", delim_end);
    ctx.str = tmp;
    return 0;
  });

  return 0;
};

auto const fn_c = [&](auto& ctx) {
  std::string flags;
  if (db.find("c-flags") != db.end())
  {
    flags = db["c-flags"];
  }
  std::string headers;
  if (db.find("c-headers") != db.end())
  {
    headers = db["c-headers"];
  }

  std::stringstream code; code
  << headers
  << "\n\nint main(){\n"
  << ctx.args.at(1)
  << "\n}";

  std::string const path {".m8/.m8-c.c"};
  std::ofstream ofile {path};
  ofile << code.str();
  ofile.close();

  std::string out;
  std::string const compile {"gcc .m8/.m8-c.c -o .m8/.m8-c " + flags};
  if (OB::exec(out, compile) != 0)
  {
    return -1;
  }

  if (OB::exec(ctx.str, ".m8/.m8-c") != 0)
  {
    return -1;
  }

  fs::remove(fs::path(".m8/.m8-c"));
  fs::remove(fs::path(".m8/.m8-c.c"));

  return 0;
};

auto const fn_cpp = [&](auto& ctx) {
  std::string flags;
  if (db.find("cpp-flags") != db.end())
  {
    flags = db["cpp-flags"];
  }
  std::string headers;
  if (db.find("cpp-headers") != db.end())
  {
    headers = db["cpp-headers"];
  }

  std::stringstream code; code
  << headers
  << "\n\nint main(){\n"
  << ctx.args.at(1)
  << "\n}";

  std::string const path {".m8/.m8-cpp.cc"};
  std::ofstream ofile {path};
  ofile << code.str();
  ofile.close();

  std::string out;
  std::string const compile {"g++ .m8/.m8-cpp.cc -o .m8/.m8-cpp " + flags};
  if (OB::exec(out, compile) != 0)
  {
    return -1;
  }

  if (OB::exec(ctx.str, ".m8/.m8-cpp") != 0)
  {
    return -1;
  }

  fs::remove(fs::path(".m8/.m8-cpp"));
  fs::remove(fs::path(".m8/.m8-cpp.cc"));

  return 0;
};

auto const fn_script = [&](auto& ctx) {
  auto str = ctx.args.at(1);
  std::string const path {".m8/.m8-script.tmp.m8"};
  std::ofstream ofile {path};
  ofile << str;
  ofile.close();
  fs::permissions(path, fs::perms::owner_exec, fs::perm_options::add);
  auto const status = OB::exec(ctx.str, path);
  fs::remove(fs::path(path));

  return status;
};

auto const fn_mod = [&](auto& ctx) {
  // auto version = ctx.args.at(1);
  // auto name = ctx.args.at(2);
  // std::string url {
  //   "https://raw.githubusercontent.com/octobanana/m8-modules/" +
  //   version + "/" + name
  // };
  // Http api;
  // api.req.url = url;

  // api.run();
  // if (api.res.status != 200)
  // {
  //   return -1;
  // }
  // ctx.str = api.res.body;

  return 0;
};

auto const fn_count = [&](auto& ctx) {
  auto search = ctx.args.at(1);
  auto str = ctx.args.at(2);
  auto count = OB::String::count(str, search);
  ctx.str = std::to_string(count);
  return 0;
};

auto const fn_cmp = [&](auto& ctx) {
  auto n1 = std::stod(ctx.args.at(1));
  auto n2 = std::stod(ctx.args.at(2));
  int res {0};
  if (n1 > n2) res = 1;
  else if (n1 == n2) res = 0;
  else if (n1 < n2) res = -1;
  ctx.str = std::to_string(res);
  return 0;
};

auto const fn_test_regex = [&](auto& ctx) {
  auto str = ctx.args.at(1);
  ctx.str = str;
  return 0;
};

auto const fn_file_write = [&](auto& ctx) {
  auto const file = ctx.args.at(1);
  auto const str = ctx.args.at(2);
  std::ofstream ofile {file};
  if (! ofile.is_open()) return -1;
  ofile << str << "\n";
  return 0;
};

auto const fn_file_append = [&](auto& ctx) {
  auto const file = ctx.args.at(1);
  auto const str = ctx.args.at(2);
  std::ofstream ofile {file, std::ios::app};
  if (! ofile.is_open()) return -1;
  ofile << str << "\n";
  return 0;
};

// define macros
// single regex and func
// M8::set_macro(name, info, usage, regex, func)
// overloaded regex and func
// M8::set_macro(name, info, usage, {{regex, func}, {regex, func}})

// m8.set_macro("",
//   "",
//   "",
//   "^(.*)$",
//   [&](auto& ctx) {
//   auto str = ctx.args.at(1);
//   ctx.str = str;
//   return 0;
//   });

m8.set_macro("for",
  "for each loop",
  "{!all}",
  "{b}{!all}{e}",
  [&](auto& ctx) {
  return 0;
  });

m8.set_macro("null",
  "/dev/null",
  "{!all}",
  "{b}{!all}{e}",
  [&](auto& ctx) {
  return 0;
  });

m8.set_macro("assert",
  "static assert",
  "{!all}",
  "{b}{!all}{e}",
  [&](auto& ctx) {
  auto lhs = ctx.args.at(1);
  auto rhs = ctx.args.at(2);

  return 0;
  });

m8.set_macro("lowercase",
  "lower the case of a string",
  "(\\d+){ws}(\\d+){ws}{!all}",
  "{b}(\\d+){ws}(\\d+){ws}{!all}{e}",
  [&](auto& ctx) {
  auto start = std::stoul(ctx.args.at(1));
  auto end = std::stoul(ctx.args.at(2));
  auto str = ctx.args.at(3);
  if (start > str.size())
  {
    return -1;
  }
  if (end == 0) end = str.size() - 1;
  ctx.str = str.replace(start, end, OB::String::lowercase(str.substr(start, end)));
  return 0;
  });

m8.set_macro("uppercase",
  "upper the case of a string",
  "(\\d+){ws}(\\d+){ws}{!all}",
  "{b}(\\d+){ws}(\\d+){ws}{!all}{e}",
  [&](auto& ctx) {
  auto start = std::stoul(ctx.args.at(1));
  auto end = std::stoul(ctx.args.at(2));
  auto str = ctx.args.at(3);
  if (start > str.size())
  {
    return -1;
  }
  if (end == 0) end = str.size() - 1;
  ctx.str = str.replace(start, end, OB::String::uppercase(str.substr(start, end)));
  return 0;
  });

m8.set_macro("rand",
  "generate random number",
  "",
  "{empty}",
  [&](auto& ctx) {
  std::random_device rd;
  std::mt19937 gen(rd());
  ctx.str = std::to_string(gen());
  return 0;
  });

m8.set_macro("nano",
  "epoch time in nanoseconds",
  "",
  "{empty}",
  [&](auto& ctx) {
  auto tnano = std::chrono::system_clock::now().time_since_epoch();
  long int uuid = std::chrono::duration_cast<std::chrono::nanoseconds>(tnano).count();
  ctx.str = std::to_string(uuid);
  return 0;
  });

m8.set_macro("substr",
  "get substring of a string",
  "(\\d+){ws}(\\d+){ws}{!all}",
  "{b}(\\d+){ws}(\\d+){ws}{!all}{e}",
  [&](auto& ctx) {
  auto start = std::stoul(ctx.args.at(1));
  auto end = std::stoul(ctx.args.at(2));
  auto str = ctx.args.at(3);
  if (start > str.size())
  {
    return -1;
  }
  if (end > str.size())
  {
    ctx.str = str.substr(start);
    return 0;
  }
  ctx.str = str.substr(start, end);
  return 0;
  });

m8.set_macro("info",
  "",
  "",
  "^(.+)$",
  fn_info);

m8.set_macro("cpp:enum",
  "",
  "",
  "^([^\\r]+)$",
  fn_cpp_enum);

m8.set_macro("version",
  "",
  {
    M8::macro_t("", "{b}{!str_s}{e}", fn_version),
    M8::macro_t("", "{b}{!str_d}{e}", fn_version),
  });

m8.set_macro("eq",
  "compare two values",
  {
    M8::macro_t("{lhs} {rhs}", R"(^{!num}{ws}{!num}$)", fn_eq),
    M8::macro_t("{lhs} {rhs}", R"(^{!str_s}{ws}{!str_s}$)", fn_eq),
    M8::macro_t("{lhs} {rhs}", R"(^{!str_d}{ws}{!str_d}$)", fn_eq),
  });

m8.set_macro("m8:if",
  "if else conditional statement",
  {
    M8::macro_t("m8:if {0|1} {...} m8:else {...?} m8:end", R"(^([01]{1})\n([^\r]*)\nm8:else(?:\n([^\r]*))?\nm8:end$)", fn_if_else),
    M8::macro_t("m8:if {0|1} {...} m8:else {...?} m8:end", R"(^([01]{1})\n([^\r]*)\nm8:end$)", fn_if_else_s),
  });

m8.set_macro("nop",
  "returns input untouched",
  "{!all}",
  "{b}{!all}{e}",
  fn_nop);

m8.set_macro("if",
  "if cond true false",
  "",
  "",
  fn_if);

m8.set_macro("printc!",
  "",
  "",
  "",
  fn_printc);

m8.set_macro("def",
  "define a macro",
  {
    M8::macro_t("{name:str_s} {info:str_s} {regex:str_s} {body:all}", "{b}{!str_s}{ws}{!str_s}{ws}{!str_s}{ws}(?:M8!|)([^\\r]+?)(?:!8M|{e})", fn_def),
    M8::macro_t("{name:wrd} {info:str_s} {regex:str_s} {body:all}", "{b}{!wrd}{ws}{!str_s}{ws}{!str_s}{ws}(?:M8!|)([^\\r]+?)(?:!8M|{e})", fn_def),
    M8::macro_t("{name:wrd} {body:all}", "{b}{!wrd}{ws}(?:M8!|){!all}(?:!8M|{e})", fn_def_s),
    // {"{b}{!str_s}{ws}(?:M8!|)([^\\r]+?)(?:!8M|{e})", fn_def_s},
    // {"^(.+?)\\s+(?:M8!|)([^\\r]+?)(?:!8M|$)", fn_def_l},
  });

m8.set_macro("c",
  "run c code snippet",
  "c <str>",
  "^([^\\r]+)$",
  fn_c);

m8.set_macro("cpp",
  "run cpp code snippet",
  "cpp <str>",
  "^([^\\r]+)$",
  fn_cpp);

m8.set_macro("script",
  "run a script",
  "script <str>",
  "^([^\\r]+)$",
  fn_script);

m8.set_macro("mod",
  "insert module from github",
  "mod file-url",
  "",
  fn_mod);

m8.set_macro("tmp",
  "call a macro template",
  "template <args...>",
  "",
  fn_template);

m8.set_macro("cmp",
  "compare two values",
  {
    M8::macro_t("{lhs} {rhs}", R"(^{!num}{ws}{!num}$)", fn_cmp),
    // {R"(^{!str_s}{ws}{!str_s}$)", fn_cmp},
    // {R"(^{!str_d}{ws}{!str_d}$)", fn_cmp},
  });

m8.set_macro("count",
  "count",
  "count",
  "^\"(.+)\"\\s+\"(.*)\"$",
  fn_count);

 m8.set_macro("sha256",
   "returns an sha256 hash of input string",
   "{!all}",
   "{b}{!all}{e}",
   fn_sha256);

m8.set_macro("get",
  "get value of key from db",
  "get <key>",
  "^(.+)$",
  fn_get);

m8.set_macro("set",
  "set key to value in db",
  "set <key> <val>",
  "^(.+?)\\s+(?:M8!|)([^\\r]+?)(?:!8M|$)",
  fn_set);

m8.set_macro("http-get",
  "http get request",
  "get \"url\"",
  "",
  fn_http_get);

m8.set_macro("http-post",
  "http post request",
  "post \"url\" \"data\"",
  "",
  fn_http_post);

m8.set_macro("floor",
  "floor a decimal",
  "floor n",
  "",
  fn_math_floor);

m8.set_macro("file:write",
  "send output to file",
  {
    M8::macro_t("{str} {all}", "{b}{!str_s}{ws}{!all}{e}", fn_file_write),
    M8::macro_t("{str} {all}", "{b}{!str_d}{ws}{!all}{e}", fn_file_write),
  });

m8.set_macro("file:append",
  "send output to file",
  {
    M8::macro_t("{str} {all}", "{b}{!str_s}{ws}{!all}{e}", fn_file_append),
    M8::macro_t("{str} {all}", "{b}{!str_d}{ws}{!all}{e}", fn_file_append),
  });

m8.set_macro("in",
  "get input from stdin",
  "in val",
  "",
  fn_in);

m8.set_macro("round",
  "round a number",
  "{num}",
  "{b}{!num}{e}",
  fn_math_round);

m8.set_macro("date",
  "the current date timestamp",
  {
    M8::macro_t("date", "{empty}", fn_date),
    M8::macro_t("date", "{b}{!str_s}{e}", fn_date),
    M8::macro_t("date", "{b}{!str_d}{e}", fn_date),
  });

m8.set_macro("abs",
  "absolute value of number",
  "{num}",
  "{b}{!num}{e}",
  fn_math_abs);

m8.set_macro("^",
  "the exponent operator",
  "{num} {num}",
  "{b}{!num}{ws}{!num}{e}",
  fn_math_pow);

m8.set_macro("%",
  "the modulo operator",
  "{num} {num}",
  "{b}{!num}{ws}{!num}{e}",
  fn_math_mod);

m8.set_macro("/",
  "the division operator",
  "{num} {num}",
  "{b}{!num}{ws}{!num}{e}",
  fn_math_divide);

m8.set_macro("*",
  "the multiplication operator",
  "{num} {num}",
  "{b}{!num}{ws}{!num}{e}",
  fn_math_multiply);

m8.set_macro("-",
  "the subtraction operator",
  "{num} {num}",
  "{b}{!num}{ws}{!num}{e}",
  fn_math_subtract);

m8.set_macro("+",
  "the addition operator",
  "{num} {num}",
  "{b}{!num}{ws}{!num}{e}",
  fn_math_add);

m8.set_macro("nl",
  "returns a newline",
  "{empty}",
  "{empty}",
  fn_nl);

m8.set_macro("nl!",
  "print a newline to stdout",
  "{empty}",
  "{empty}",
  fn_stdout_nl);

m8.set_macro("print!",
  "print a string to stdout",
  "print! str",
  "",
  fn_print);

m8.set_macro("term-width",
  "returns width of terminal",
  "{empty}",
  "{empty}",
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
  "^(.+)$",
  fn_sourcepp);

m8.set_macro("headerpp",
  "templates a c++ header file structure",
  "headerpp str",
  "^(.+)$",
  fn_headerpp);

m8.set_macro("env",
  "gets an environment variable",
  "env str",
  "^(.+)$",
  fn_env);

m8.set_macro("sh",
  "execute a shell command",
  "sh (str)",
  "^([^\\r]+)$",
  fn_sh);

m8.set_macro("file",
  "read in a file",
  "{b}{!str_s}{e}",
  "{b}{!str_s}{e}",
  fn_file);

m8.set_macro("license",
  "insert a license header",
  "license <license> <author> <year>)",
  "",
  fn_license);

m8.set_macro("repeat",
  "repeats the given string 'n' times",
  "repeat \"str\", int",
  "^\"([^\\r]+?)\", ([0-9]+)$",
  fn_repeat);

m8.set_macro("comment_header",
  "outputs the authors name, timestamp, version, and description in a c++ comment block",
  "comment_header (version, author, description)",
  "^\\(([.0-9]+?), \"(.+?)\", \"(.+?)\"\\)$",
  fn_comment_header);

}

} // namespace Macros
