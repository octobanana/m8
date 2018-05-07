#include "m8.hh"

#include "ast.hh"
using Tmacro = OB::Tmacro;
using Ast = OB::Ast;

#include "reader.hh"
using Reader = OB::Reader;

#include "writer.hh"
using Writer = OB::Writer;

#include "lexer.hh"
using Lexer = OB::Lexer;

#include "parser.hh"
using Parser = OB::Parser;

// #include "cache.hh"
// using Cache = OB::Cache;

#include "sys_command.hh"
#include "crypto.hh"
#include "http.hh"

#include "string.hh"
namespace String = OB::String;

#include "color.hh"
namespace Cl = OB::Color;

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include "json.hh"
using Json = nlohmann::json;

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <unordered_map>
#include <vector>
#include <regex>
#include <functional>
#include <ctime>
#include <stdexcept>
#include <future>
#include <iterator>
#include <stack>
#include <algorithm>
#include <cctype>
#include <deque>
#include <optional>

M8::M8()
{
}

M8::~M8()
{
}

void M8::set_debug(bool val)
{
  debug_ = val;
}

void M8::set_copy(bool val)
{
  copy_ = val;
}

void M8::set_readline(bool val)
{
  readline_ = val;
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex)
{
  regex = String::format(regex, rx_grammar);
  macros[name] = {Mtype::external, name, info, usage, {{regex, nullptr}}, {}};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex, std::string const& url)
{
  regex = String::format(regex, rx_grammar);
  macros[name] = {Mtype::remote, name, info, usage, {{regex, nullptr}}, url};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex, macro_fn func)
{
  regex = String::format(regex, rx_grammar);
  macros[name] = {Mtype::internal, name, info, usage, {{regex, func}}, {}};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage,
  std::vector<std::pair<std::string, macro_fn>> rx_fn)
{
  for (auto& e : rx_fn)
  {
    e.first = String::format(e.first, rx_grammar);
  }
  macros[name] = {Mtype::internal, name, info, usage, rx_fn, {}};
}

void M8::set_delimits(std::string const& delim_start, std::string const& delim_end)
{
  if (delim_start == delim_end)
  {
    throw std::runtime_error("start and end delim cannot be the same value");
  }
  delim_start_ = delim_start;
  delim_end_ = delim_end;
  rx_grammar["DS"] = delim_start_;
  rx_grammar["DE"] = delim_end_;
}

std::string M8::list_macros() const
{
  std::stringstream ss;

  ss << "M8:\n\n";

  for (auto const& e : macros)
  {
    ss
    << e.second.name << "\n"
    << "  " << e.second.usage << "\n"
    << "  " << e.second.info << "\n";
    for (auto const& e : e.second.rx_fn)
    {
      ss << "  " << e.first << "\n";
    }
  }

  return ss.str();
}

std::string M8::macro_info(std::string const& name) const
{
  std::stringstream ss;
  if (macros.find(name) == macros.end())
  {
    ss << AEC::wrap("Error: ", AEC::fg_red);
    ss << "Undefined name '" << AEC::wrap(name, AEC::fg_white) << "'\n";

    // lookup similar macro suggestion
    ss << "  Looking for similar names...\n";
    auto similar_names = suggest_macro(name);
    if (similar_names.size() > 0)
    {
      ss << "  Did you mean: " << AEC::fg_green;
      for (auto const& e : similar_names)
      {
        ss << e << " ";
      }
      ss << AEC::reset << "\n";
    }
    else
    {
      ss << Cl::fg_magenta << "  No suggestions found.\n" << Cl::reset;
    }
  }
  else
  {
    auto const& e = macros.at(name);
    ss
    << e.name << "\n"
    << "  " << e.info << "\n"
    << "  " << e.usage << "\n";
    for (auto const& e : e.rx_fn)
    {
      ss << "  " << e.first << "\n";
    }
  }
  return ss.str();
}

void M8::set_config(std::string file_name)
{
  // add external macros
  if (file_name.empty())
  {
    // default config file location -> ~/.m8.json
    file_name = env_var("HOME") + "/.m8.json";
  }

  // open the config file
  std::ifstream file {file_name};
  if (! file.is_open())
  {
    if (debug_)
    {
      std::cerr << "Debug: could not open config file\n";
    }
    return;
  }

  // read in the config file into memory
  file.seekg(0, std::ios::end);
  size_t size (file.tellg());
  std::string content (size, ' ');
  file.seekg(0);
  file.read(&content[0], size);

  // parse the contents
  Json j = Json::parse(content);
  for (auto const& e : j["macros"])
  {
    // add remote macro
    if (e.count("url") == 1)
    {
      set_macro(
        e["name"].get<std::string>(),
        e["info"].get<std::string>(),
        e["usage"].get<std::string>(),
        e["regex"].get<std::string>(),
        e["url"].get<std::string>());
    }

    // add external macro
    else
    {
      set_macro(
        e["name"].get<std::string>(),
        e["info"].get<std::string>(),
        e["usage"].get<std::string>(),
        e["regex"].get<std::string>());
    }
  }
}

std::string M8::summary() const
{
  std::stringstream ss; ss
  << AEC::wrap("\nSummary\n", AEC::fg_magenta)
  << AEC::wrap("  Total      ", AEC::fg_magenta) << AEC::wrap(stats.macro, AEC::fg_green) << "\n"
  << AEC::wrap("    Internal ", AEC::fg_magenta) << AEC::wrap(stats.internal, AEC::fg_green) << "\n"
  << AEC::wrap("    External ", AEC::fg_magenta) << AEC::wrap(stats.external, AEC::fg_green) << "\n"
  << AEC::wrap("    Remote   ", AEC::fg_magenta) << AEC::wrap(stats.remote, AEC::fg_green) << "\n"
  << AEC::wrap("  passes     ", AEC::fg_magenta) << AEC::wrap(stats.pass, AEC::fg_green) << "\n"
  << AEC::wrap("  warnings   ", AEC::fg_magenta) << AEC::wrap(stats.warning, AEC::fg_green) << "\n";
  return ss.str();
}

std::vector<std::string> M8::suggest_macro(std::string const& name) const
{
  std::vector<std::string> similar_names;

  std::smatch similar_match;
  int len = (name.size() / 1.2);
  // if (len < 2) len = name.size();
  std::stringstream escaped_name;
  for (auto const& e : name)
  {
    if (std::isalnum(e))
    {
      escaped_name << e;
    }
    else
    {
      escaped_name << "\\" << e;
    }
  }
  std::string similar_regex {"^.*[" + escaped_name.str() + "]{" + std::to_string(len) + "}.*$"};

  for (auto const& e : macros)
  {
    if (std::regex_match(e.first, similar_match, std::regex(similar_regex, std::regex::icase)))
    {
      similar_names.emplace_back(similar_match[0]);
    }
  }

  std::sort(similar_names.begin(), similar_names.end(),
  [](std::string const& lhs, std::string const& rhs) {
    return lhs.size() < rhs.size();
  });

  if (similar_names.size() > 8)
  {
    similar_names.erase(similar_names.begin() + 8, similar_names.end());
  }

  return similar_names;
}

void M8::set_hook(Htype t, Hook h)
{
  auto const insert_hook = [&](auto& hooks) {
    for (auto& e : hooks)
    {
      if (e.key == h.key)
      {
        e.val = h.val;
        return;
      }
    }
    hooks.emplace_back(h);
  };

  switch (t)
  {
    case Htype::begin:
      insert_hook(h_begin);
      return;
    case Htype::macro:
      insert_hook(h_macro);
      return;
    case Htype::res:
      insert_hook(h_res);
      return;
    case Htype::end:
      insert_hook(h_end);
      return;
    default:
      return;
  }
}

std::optional<M8::Hooks> M8::get_hooks(Htype t) const
{
  switch (t)
  {
    case Htype::begin: return h_begin;
    case Htype::macro: return h_macro;
    case Htype::res: return h_res;
    case Htype::end: return h_end;
    default: return {};
  }
}

void M8::rm_hook(Htype t, std::string key)
{
  auto const rm_key = [&](auto& m) {
    size_t i {0};
    for (auto& e : m)
    {
      if (e.key == key)
      {
        m.erase(m.begin() + i);
      }
      ++i;
    }
  };

  switch (t)
  {
    case Htype::begin:
      rm_key(h_begin);
      break;
    case Htype::macro:
      rm_key(h_macro);
      break;
    case Htype::res:
      rm_key(h_res);
      break;
    case Htype::end:
      rm_key(h_end);
      break;
    default:
      break;
  }
}

void M8::run_hooks(Hooks const& h, std::string& s)
{
  for (auto const& e : h)
  {
    std::string str = s;
    s.clear();
    std::smatch match;

    while (std::regex_search(str, match, std::regex(e.key)))
    {
      std::unordered_map<std::string, std::string> m;
      for (size_t i = 1; i < match.size(); ++i)
      {
        m[std::to_string(i)] = match[i];
        m["DS"] = delim_start_;
        m["DE"] = delim_end_;
      }
      std::string ns {e.val};
      ns = String::format(ns, m);
      s += std::string(match.prefix()) + ns;
      if (str == match.suffix())
      {
        str.clear();
        break;
      }
      str = match.suffix();
      if (str.empty()) break;
    }
    s += str;
  }
}

void M8::parse(std::string const& _ifile, std::string const& _ofile)
{
  auto& ast = ast_.ast;

  // init cache
  // Cache cache_ {_ifile};

  // init the parsing stack
  std::stack<Tmacro> stk;

  // init the reader
  Reader r;
  if (! readline_ || ! _ifile.empty())
  {
    r.open_file(_ifile);
  }

  // init the writer
  Writer w {_ofile};
  if (! _ofile.empty())
  {
    w.open();
  }

  std::string line;
  while(r.next(line))
  {
    // check for empty line
    if (line.empty())
    {
      if (stk.empty())
      {
        if (! _ofile.empty() && copy_)
        {
          w.write("\n");
        }
        continue;
      }
      else
      {
        auto& t = stk.top();
        t.str += "\n";
        continue;
      }
    }

    // commented out line
    {
      std::string comment_str {"//M8"};
      auto pos = line.find_first_not_of(" \t");
      if (pos != std::string::npos)
      {
        if (line.compare(pos, comment_str.size(), comment_str) == 0)
        {
          continue;
        }
      }
    }

    // whitespace indentation
    size_t indent {0};
    char indent_char {' '};
    {
      std::string e {line.at(0)};
      if (e.find_first_of(" \t") != std::string::npos)
      {
        size_t count {0};
        for (size_t i = 0; i < line.size(); ++i)
        {
          e = line.at(i);
          if (e.find_first_not_of(" \t") != std::string::npos)
          {
            break;
          }
          ++count;
        }
        indent = count;
        indent_char = line.at(0);
      }
    }

    // find and replace macro words
    run_hooks(h_begin, line);

    // buffer to hold external chars
    std::string buf;

    // warning/error marker line
    std::string mark_line (line.size(), ' ');

    // parse line char by char for either start or end delim
    for (size_t i = 0; i < line.size(); ++i)
    {
      // case start delimiter
      if (line.at(i) == delim_start_.at(0))
      {
        size_t pos_start = line.find(delim_start_, i);
        if (pos_start != std::string::npos && pos_start == i)
        {
          if (i > 0 && line.at(i - 1) == '`')
          {
            goto regular_char;
          }

          // stack operations
          auto t = Tmacro();
          t.line_start = r.row();
          t.begin = pos_start;
          // if (! stk.empty())
          // {
          //   append placeholder
          //   stk.top().str += "[%" + std::to_string(stk.top().children.size()) + "]";
          // }
          stk.push(t);

          i += delim_start_.size() - 1;
          continue;
        }
      }

      // case end delimiter
      if (line.at(i) == delim_end_.at(0))
      {
        size_t pos_end = line.find(delim_end_, i);
        if (pos_end != std::string::npos && pos_end == i)
        {
          if (i + delim_end_.size() < line.size() && line.at(i + delim_end_.size()) == '`')
          {
            goto regular_char;
          }

          // stack operations
          if (stk.empty())
          {
            if (readline_)
            {
              std::cerr << "Error: missing opening delimiter\n";
              stk = std::stack<Tmacro>();
              break;
            }
            throw std::runtime_error("missing opening delimiter");
          }
          else
          {
            auto t = stk.top();
            stk.pop();

            t.line_end = r.row();
            t.end = pos_end;

            // parse str into name and args
            {
              std::smatch match;
              std::string name_args {"^\\s*([^\\s]+)\\s*([^\\r]*?)\\s*$"};
              // std::string name_args {"^\\s*([^\\s]+)\\s*(?:M8!|)([^\\r]*?)(?:!8M|$)$"};
              if (std::regex_match(t.str, match, std::regex(name_args)))
              {
                t.name = match[1];
                t.args = match[2];

                // find and replace macro words
                run_hooks(h_macro, t.name);
                run_hooks(h_macro, t.args);

                if (debug_)
                {
                  std::cerr << "name: |" << t.name << "|" << std::endl;
                  std::cerr << "args: |" << t.args << "|" << std::endl;
                }
              }
              else
              {
                if (readline_)
                {
                  std::cerr << "Error: invalid macro format\n";
                  stk = std::stack<Tmacro>();
                  break;
                }
                throw std::runtime_error("invalid macro format");
              }
            }

            // validate name and args
            {
              auto const it = macros.find(t.name);
              if (it == macros.end())
              {
                // for (size_t j = 0; j < t.name.size(); ++j)
                // {
                //   mark_line.at(t.begin + delim_start_.size() + j) = '^';
                // }
                std::cerr
                << AEC::wrap("Error: ", AEC::fg_red)
                << "Undefined name '" << AEC::wrap(t.name, AEC::fg_white) << "' "
                << _ifile << ":"
                << t.line_start << ":" << (t.begin + delim_start_.size() + 1)
                << "\n"
                << "  " << line
                // << "\n"
                // << "  " << AEC::wrap(mark_line, AEC::fg_red)
                << "\n";

                // lookup similar macro suggestion
                {
                  std::cerr
                  << "  Looking for similar names..."
                  << "\n";
                  auto similar_names = suggest_macro(t.name);
                  if (similar_names.size() > 0)
                  {
                    std::cerr
                    << "  Did you mean: "
                    << AEC::fg_green;
                    for (auto const& e : similar_names)
                    {
                      std::cerr << e << " ";
                    }
                    std::cerr << "\n" << Cl::reset;
                  }
                  else
                  {
                    std::cerr << AEC::wrap("  No suggestions found.\n", AEC::fg_red);
                  }
                }

                // error -> undefined name
                if (readline_)
                {
                  stk = std::stack<Tmacro>();
                  break;
                }
                else
                {
                  throw std::runtime_error("undefined name");
                }
              }

              if (it->second.rx_fn.at(0).first.empty())
              {
                std::vector<std::string> reg_num {
                  {"^[\\-|+]{0,1}[0-9]+$"},
                  {"^[\\-|+]{0,1}[0-9]*\\.[0-9]+$"},
                  {"^[\\-|+]{0,1}[0-9]+e[\\-|+]{0,1}[0-9]+$"},
                  // {"^[\\-|+]{0,1}[0-9]+/[\\-|+]{0,1}[0-9]+$"},
                };

                std::vector<std::string> reg_str {
                  {"^([^`\\\\]*(?:\\\\.[^`\\\\]*)*)$"},
                  {"^([^'\\\\]*(?:\\\\.[^'\\\\]*)*)$"},
                  {"^([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)$"},
                };

                // complete arg string as first parameter
                t.match.emplace_back(t.args);
                std::vector<bool> valid_args;

                for (size_t j = 0; j < t.args.size(); ++j)
                {
                  std::stringstream ss; ss << t.args.at(j);
                  auto s = ss.str();
                  // std::cerr << "arg:" << s << "\n";
                  if (s.find_first_of(" \n\t") != std::string::npos)
                  {
                    continue;
                  }
                  // if (s.find_first_of(".") != std::string::npos)
                  // {
                  //   // macro
                  //   t.match.emplace_back(std::string());
                  //   for (;j < t.args.size() && t.args.at(j) != '.'; ++j)
                  //   {
                  //     t.match.back() += t.args.at(j);
                  //   }
                  //   std::cerr << "Arg-Macro:\n~" << t.match.back() << "~\n";
                  //   for (auto const& e : reg_num)
                  //   {
                  //     std::smatch m;
                  //     if (std::regex_match(t.match.back(), m, std::regex(e)))
                  //     {
                  //       // std::cerr << "ArgValid\nmacro\n" << t.match.back() << "\n\n";
                  //     }
                  //   }
                  // }
                  if (s.find_first_of(".-+0123456789") != std::string::npos)
                  {
                    // num
                    // std::cerr << "Num\n";
                    t.match.emplace_back(std::string());
                    for (;j < t.args.size() && t.args.at(j) != ' '; ++j)
                    {
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Num\n" << t.match.back() << "\n";
                    bool invalid {true};
                    for (auto const& e : reg_num)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nnum\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                    // if (t.match.back().find("/") != std::string::npos)
                    // {
                    //   auto n1 = t.match.back().substr(0, t.match.back().find("/"));
                    //   auto n2 = t.match.back().substr(t.match.back().find("/") + 1);
                    //   auto n = std::stod(n1) / std::stod(n2);
                    //   std::stringstream ss; ss << n;
                    //   t.match.back() = ss.str();
                    //   std::cerr << "Simplified:\n" << t.match.back() << "\n\n";
                    // }
                  }
                  else if (s.find_first_of("\"") != std::string::npos)
                  {
                    // str
                    // std::cerr << "Str\n";
                    t.match.emplace_back("");
                    ++j; // skip start quote
                    bool escaped {false};
                    for (;j < t.args.size(); ++j)
                    {
                      if (! escaped && t.args.at(j) == '\"')
                      {
                        // skip end quote
                        break;
                      }
                      if (t.args.at(j) == '\\')
                      {
                        escaped = true;
                        continue;
                      }
                      if (escaped)
                      {
                        t.match.back() += "\\";
                        t.match.back() += t.args.at(j);
                        escaped = false;
                        continue;
                      }
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Str\n" << t.match.back() << "\n";
                    bool invalid {true};
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                  }
                  else if (s.find_first_of("\'") != std::string::npos)
                  {
                    // str
                    // std::cerr << "Str\n";
                    t.match.emplace_back("");
                    ++j; // skip start quote
                    bool escaped {false};
                    for (;j < t.args.size(); ++j)
                    {
                      if (! escaped && t.args.at(j) == '\'')
                      {
                        // skip end quote
                        break;
                      }
                      if (t.args.at(j) == '\\')
                      {
                        escaped = true;
                        continue;
                      }
                      if (escaped)
                      {
                        t.match.back() += "\\";
                        t.match.back() += t.args.at(j);
                        escaped = false;
                        continue;
                      }
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Str\n" << t.match.back() << "\n";
                    bool invalid {true};
                    std::string mstr {"'" + t.match.back() + "'"};
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(mstr, m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                  }
                  else if (s.find_first_of("`") != std::string::npos)
                  {
                    // literal
                    // std::cerr << "Str\n";
                    t.match.emplace_back("");
                    ++j; // skip start quote
                    bool escaped {false};
                    for (;j < t.args.size(); ++j)
                    {
                      if (! escaped && t.args.at(j) == '`')
                      {
                        // skip end quote
                        break;
                      }
                      if (t.args.at(j) == '\\')
                      {
                        escaped = true;
                        continue;
                      }
                      if (escaped)
                      {
                        t.match.back() += "\\";
                        t.match.back() += t.args.at(j);
                        escaped = false;
                        continue;
                      }
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Str\n" << t.match.back() << "\n";
                    bool invalid {true};
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                  }
                  else
                  {
invalid_arg:
                    // invalid arg
                    if (readline_)
                    {
                      std::cerr
                      << AEC::wrap("Error: ", AEC::fg_red)
                      << "macro '"
                      << AEC::wrap(t.name, AEC::fg_white)
                      << "' has an invalid argument\n"
                      << AEC::wrap(t.match.back(), AEC::fg_magenta)
                      << "\n";
                      stk = std::stack<Tmacro>();
                      break;
                    }
                    throw std::runtime_error("macro " + t.name + " has an invalid argument");
                  }
                }
                // std::cerr << "ArgValid\ncomplete\n\n";
              }
              else
              {
                bool invalid_regex {true};
                if (it->second.rx_fn.size() == 1)
                {
                  std::smatch match;
                  if (std::regex_match(t.args, match, std::regex(it->second.rx_fn.at(0).first)))
                  {
                    invalid_regex = false;
                    for (auto const& e : match)
                    {
                      t.match.emplace_back(std::string(e));
                    }
                  }
                }
                else
                {
                  size_t index {0};
                  for (auto const& rf : it->second.rx_fn)
                  {
                    std::smatch match;
                    if (std::regex_match(t.args, match, std::regex(rf.first)))
                    {
                      invalid_regex = false;
                      for (auto const& e : match)
                      {
                        t.match.emplace_back(std::string(e));
                        if (debug_)
                        {
                          std::cerr << "arg: " << std::string(e) << "\n";
                        }
                      }
                      t.fn_index = index;
                      break;
                    }
                    ++index;
                  }
                }
                if (invalid_regex)
                {
                  std::cerr
                  << AEC::wrap("Error: ", AEC::fg_red)
                  << "Invalid regex for '"
                  << AEC::wrap(t.name, AEC::fg_white) << "' "
                  << _ifile << ":"
                  << t.line_start << ":" << (t.begin + delim_start_.size() + 1)
                  << "\n"
                  << "  " << AEC::wrap(it->second.usage, AEC::fg_green)
                  << "\n";
                  for (auto const& rf : it->second.rx_fn)
                  {
                    std::cerr << "  " << AEC::wrap(rf.first, AEC::fg_magenta) << "\n";
                  }
                  if (readline_)
                  {
                    stk = std::stack<Tmacro>();
                    break;
                  }
                  throw std::runtime_error("Invalid regex");
                }
              }

              // process macro
              int ec {0};
              Ctx ctx {t.res, t.match, _ifile, r.row()};
              try
              {
                if (it->second.type == Mtype::internal)
                {
                  ec = run_internal(it->second.rx_fn.at(t.fn_index).second, ctx);
                }
                else if (it->second.type == Mtype::remote)
                {
                  ec = run_remote(it->second, ctx);
                }
                else
                {
                  ec = run_external(it->second, ctx);
                }
                ++stats.macro;
              }
              catch (std::exception const& e)
              {
                std::stringstream ss;
                ss << AEC::wrap("Error: ", AEC::fg_red) << "macro '" + AEC::wrap(t.name, AEC::fg_white) + "' failed\n  " + AEC::wrap(e.what(), AEC::fg_magenta) + "\n";
                if (readline_)
                {
                  std::cerr << ss.str();
                  i += delim_end_.size() - 1;
                  stk = std::stack<Tmacro>();
                  continue;
                }
                throw std::runtime_error(ss.str());
              }

              if (ec != 0)
              {
                std::stringstream ss;
                ss << AEC::wrap("Error: ", AEC::fg_red) << "macro '" + AEC::wrap(t.name, AEC::fg_white) + "' failed\n  " + AEC::wrap(ctx.err_msg, AEC::fg_magenta) + "\n";
                if (readline_)
                {
                  std::cerr << ss.str();
                  i += delim_end_.size() - 1;
                  stk = std::stack<Tmacro>();
                  continue;
                }
                throw std::runtime_error(ss.str());
              }

              // find and replace macro words
              run_hooks(h_res, t.res);

              // debug
              if (debug_)
              {
                std::cerr << "\nRes:\n~" << t.res << "~\n";
              }

              if (t.res.find(delim_start_) != std::string::npos)
              {
                // remove escaped nl chars
                t.res = String::replace_all(t.res, "\\\n", "");

                // remove all nl chars that follow delim_end
                // this would normally be removed by the reader
                t.res = String::replace_all(t.res, delim_end_ + "\n", delim_end_);

                // TODO handle the same as if it was read from file
                line.insert(i + delim_end_.size(), t.res);
                i += delim_end_.size() - 1;
                continue;
              }

              if (! stk.empty())
              {
                stk.top().str += t.res;
              }
              else
              {
                // add indentation
                {
                  std::string indent_str (indent, indent_char);
                  t.res = String::replace_all(t.res, "\n", "\n" + indent_str);
                  t.res = String::replace_last(t.res, "\n" + indent_str, "\n");
                  t.res = String::replace_all(t.res, "\n" + indent_str + "\n", "\n\n");
                }

                buf += t.res;

                // account for when end delim is last char on line
                // add a newline char to buf
                // only if response is not empty
                if ((! t.res.empty()) && (i + delim_end_.size() - 1 == line.size() - 1))
                {
                  buf += "\n";
                }

                // if (readline_)
                // {
                //   std::cout << buf;

                //   if (! buf.empty() && buf.back() != '\n')
                //   {
                //     std::cout
                //     << AEC::reverse
                //     << "%"
                //     << AEC::reset
                //     << "\n";
                //   }
                // }
              }
            }

            if (stk.empty())
            {
              ast.emplace_back(t);
            }
            else
            {
              auto& l = stk.top();
              l.children.emplace_back(t);
            }
          }

          i += delim_end_.size() - 1;
          continue;
        }
      }

regular_char:

      // case else other characters
      if (! stk.empty())
      {
        // inside macro
        auto& t = stk.top();

        if (i == (line.size() - 1))
        {
          if (line.at(i) != '\\')
          {
            t.str += line.at(i);
            t.str += "\n";
          }
        }
        else
        {
          t.str += line.at(i);
        }
      }
      else
      {
        // outside macro
        // append to output buffer
        if (copy_)
        {
          if (i == (line.size() - 1))
          {
            if (line.at(i) != '\\')
            {
              buf += line.at(i);
              buf += "\n";
            }
          }
          else
          {
            buf += line.at(i);
          }
        }
      }

    }

    // find and replace macro words
    run_hooks(h_end, buf);

    if (debug_)
    {
      std::cerr << "AST:\n" << ast_.str() << "\n";
      ast_.clear();
    }

    if (buf.empty() || buf == "\n")
    {
      continue;
    }

    // append buf to output file
    if (! _ofile.empty())
    {
      w.write(buf);
    }

    if (readline_)
    {
      std::cout << buf;

      if (! buf.empty() && buf.back() != '\n')
      {
        std::cout << AEC::wrap("%\n", AEC::reverse);
      }
    }
  }

  if (! stk.empty())
  {
    throw std::runtime_error("missing closing delimiter");
  }

  if (! _ofile.empty())
  {
    w.close();
  }
}

int M8::run_internal(macro_fn const& func, Ctx& ctx)
{
  ++stats.internal;

  return func(ctx);
}

int M8::run_external(Macro const& macro, Ctx& ctx)
{
  ++stats.external;

  std::string m_args;
  for (size_t i = 1; i < ctx.args.size(); ++i)
  {
    m_args += ctx.args[i];
    if (i < ctx.args.size() - 1)
      m_args += " ";
  }
  return exec(ctx.str, macro.name + " " + m_args);
}

int M8::run_remote(Macro const& macro, Ctx& ctx)
{
  ++stats.remote;

  Http api;
  api.req.method = "POST";
  api.req.headers.emplace_back("content-type: application/json");
  api.req.url = macro.url;

  Json data;
  data["name"] = macro.name;
  data["args"] = ctx.args;
  api.req.data = data.dump();

  std::cout << "Remote macro call -> " << macro.name << "\n";
  std::future<int> send {std::async(std::launch::async, [&]() {
    api.run();
    int status_code = api.res.status;
    if (status_code != 200)
    {
      return -1;
    }
    else
    {
      ctx.str = api.res.body;
      return 0;
    }
  })};

  std::future_status fstatus;
  do
  {
    fstatus = send.wait_for(std::chrono::milliseconds(250));
    std::cout << "." << std::flush;
  }
  while (fstatus != std::future_status::ready);

  int ec = send.get();
  if (ec == 0)
  {
    std::cout << "\033[2K\r" << "Success: remote call\n";
  }
  else
  {
    std::cout << "\033[2K\r" << "Error: remote call\n";
  }

  return ec;
}

std::string M8::env_var(std::string const& var) const
{
  std::string res;
  if (const char* envar = std::getenv(var.c_str()))
  {
    res = envar;
  }
  return res;
}
