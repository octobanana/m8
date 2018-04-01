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

#include "cache.hh"
using Cache = OB::Cache;

#include "sys_command.hh"
#include "crypto.hh"
#include "http.hh"

#include "color.hh"
namespace Cl = OB::Color;

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include "json.hh"
using Json = nlohmann::json;

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
#include <future>
#include <iterator>
#include <stack>

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
  std::string const& usage, std::string const& regex)
{
  // if (macros.find(name) != macros.end())
  // {
  //   throw std::logic_error("multiple definitions of macro: " + name);
  // }
  macros[name] = {Mtype::external, name, info, usage, regex, "", nullptr};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex, std::string const& url)
{
  // if (macros.find(name) != macros.end())
  // {
  //   throw std::logic_error("multiple definitions of macro: " + name);
  // }
  macros[name] = {Mtype::remote, name, info, usage, regex, url, nullptr};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex, macro_fn fn)
{
  // if (macros.find(name) != macros.end())
  // {
  //   throw std::logic_error("multiple definitions of macro: " + name);
  // }
  macros[name] = {Mtype::internal, name, info, usage, regex, "", fn};
}

void M8::set_delimits(std::string const& delim_start, std::string const& delim_end)
{
  delim_start_ = delim_start;
  delim_end_ = delim_end;
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
    << "  " << e.second.info << "\n"
    << "  " << e.second.regex << "\n\n";
  }

  return ss.str();
}

std::string M8::macro_info(std::string const& name) const
{
  std::stringstream ss;
  if (macros.find(name) == macros.end())
  {
    ss <<  "Error: macro '" << name << "' not found\n";

    // lookup similar macro suggestion
    ss << "Looking for similar names...\n";
    auto similar_names = suggest_macro(name);
    if (similar_names.size() > 0)
    {
      ss << "Did you mean: ";
      for (auto const& e : similar_names)
      {
        ss << e << " ";
      }
      ss << "\n";
    }
    else
    {
      ss << Cl::fg_magenta << "No suggestions found.\n" << Cl::reset;
    }
  }
  else
  {
    auto const& e = macros.at(name);
    ss
    << e.name << "\n"
    << "  " << e.usage << "\n"
    << "  " << e.info << "\n"
    << "  " << e.regex << "\n";
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
  // std::string content;
  // content.assign((std::istreambuf_iterator<char>(file)),
  //     (std::istreambuf_iterator<char>()));
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
  << "\nSummary\n"
  << "  Total    " << macro_count_ << "\n"
  << "  passes   " << pass_count_ << "\n"
  << "  warnings " << warning_count_ << "\n"
  << "  Internal " << internal_count_ << "\n"
  << "  External " << external_count_ << "\n"
  << "  Remote   " << remote_count_ << "\n";
  return ss.str();
}

std::vector<std::string> M8::suggest_macro(std::string const& name) const
{
  std::vector<std::string> similar_names;

  std::smatch similar_match;
  int len = (name.size() / 1.2);
  if (len < 2) len = name.size();
  std::string similar_regex
    {"^.*[" + name + "]{" + std::to_string(len) + "}.*$"};

  for (auto const& e : macros)
  {
    if (std::regex_match(e.first, similar_match, std::regex(similar_regex)))
    {
      similar_names.emplace_back(similar_match[0]);
    }
  }

  return similar_names;
}

void M8::parse(std::string const& _ifile, std::string const& _ofile)
{
  auto& ast = ast_.ast;

  // init cache
  Cache cache_ {_ifile};

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

    // case comment
    if ((line.at(0) == ';'))
    {
      if (! stk.empty())
      {
        continue;
      }
    }

    // buffer to hold external chars
    std::string buf;

    // debug
    // std::cout << p.current_line() << ".\n" << line << "\n";
    // std::string mark_line;
    // mark_line.reserve(line.size() + (Cl::fg_black.size() * 2));
    // for (size_t i = 0; i < line.size(); ++i)
    // {
    //   mark_line += " ";
    // }
    // mark_line = Cl::wrap(mark_line, {Cl::fg_green});

    // parse line char by char for either start or end delim
    for (size_t i = 0; i < line.size(); ++i)
    {
      // std::cerr << "char[" << i << "] -> " << line.at(i) << "\n\n";

      // case comment
      // if ((line.at(i) == ';') && line.at(i - 1) != '\\')
      // {
      //   break;
      // }

      // case start delimiter
      if (line.at(i) == delim_start_.at(0) )
      {
        // if (i > 1 && line.at(i - 1) == '`')
        // {
        //   goto regular_char;
        // }

        size_t pos_start = line.find(delim_start_, i);
        if (pos_start != std::string::npos && pos_start == i)
        {
          // std::cerr << "\n" << line << "\n" << pos_start << "\n";

          //debug
          // mark_line.replace(pos_start + Cl::fg_black.size(), 1, "^");

          // stack operations
          auto t = Tmacro();
          t.line_start = r.row();
          t.begin = pos_start;
          if (! stk.empty())
          {
            // append placeholder
            // stk.top().str += "[%" + std::to_string(stk.top().children.size()) + "]";
          }
          stk.push(t);

          i += delim_start_.size() - 1;
          continue;
        }
      }

      // case end delimiter
      if (line.at(i) == delim_end_.at(0))
      {
        // if (i + 1 < line.size() && line.at(i + 1) == '`')
        // {
        //   goto regular_char;
        // }

        size_t pos_end = line.find(delim_end_, i);
        if (pos_end != std::string::npos && pos_end == i)
        {
          // std::cerr << "\n" << line << "\n" << pos_end << "\n";

          //debug
          // mark_line.replace(pos_end + Cl::fg_black.size(), 1, "^");

          // stack operations
          if (stk.empty())
          {
            if (readline_)
            {
              std::cerr << "Error: missing opening delimiter\n";
              i += delim_end_.size() - 1;
              stk = std::stack<Tmacro>();
              continue;
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
              if (std::regex_match(t.str, match, std::regex(name_args)))
              {
                t.name = match[1];
                t.args = match[2];
                // std::cerr << "name: =" << t.name << "=" << std::endl;
                // std::cerr << "args: =" << t.args << "=" << std::endl;
              }
              else
              {
                if (readline_)
                {
                  std::cerr << "Error: invalid macro format\n";
                  i += delim_end_.size() - 1;
                  stk = std::stack<Tmacro>();
                  continue;
                }
                throw std::runtime_error("invalid macro format");
              }
            }

            // validate name and args
            {
              auto const it = macros.find(t.name);
              if (it == macros.end())
              {
                std::cout << Cl::fg_red << "Undefined name: " << Cl::reset;
                std::cout << _ifile << ":"
                  << t.line_start << ":" <<
                  (t.begin + delim_start_.size() + 1) << "\n";
                std::cout << Cl::fg_magenta << "macro: " << Cl::fg_green << t.name << "\n";

                // lookup similar macro suggestion
                {
                  std::cout << Cl::fg_magenta << "Looking for similar names...\n" << Cl::reset;
                  auto similar_names = suggest_macro(t.name);
                  if (similar_names.size() > 0)
                  {
                    std::cout << Cl::fg_magenta << "Did you mean: " << Cl::fg_green;
                    for (auto const& e : similar_names)
                    {
                      std::cout << e << " ";
                    }
                    std::cout << "\n" << Cl::reset;
                  }
                  else
                  {
                    std::cout << Cl::fg_magenta << "No suggestions found.\n" << Cl::reset;
                  }
                }

                if (readline_)
                {
                  std::cerr << "Error: undefined name\n";
                  i += delim_end_.size() - 1;
                  stk = std::stack<Tmacro>();
                  continue;
                }
                throw std::runtime_error("undefined name");
              }

              if (it->second.regex.empty())
              {
                // args
                // num, str, macro
                // num:
                //  -+1
                //  -+.12
                //  -+1.12
                //  -+1/2
                //  1e-+10
                // str:
                //  "str\"ing\n"
                //  'str"ing\n'
                // macro:
                //  delim name args delim

                std::vector<std::string> reg_num {
                  {"^[\\-|+]{0,1}[0-9]+$"},
                  {"^[\\-|+]{0,1}[0-9]*\\.[0-9]+$"},
                  {"^[\\-|+]{0,1}[0-9]+e[\\-|+]{0,1}[0-9]+$"},
                  // {"^[\\-|+]{0,1}[0-9]+/[\\-|+]{0,1}[0-9]+$"},
                };

                std::vector<std::string> reg_str {
                  {"^\"(?:[^\\\"]+|\\.)*\"$"},
                  {"^\'(?:[^\\\']+|\\.)*\'$"},
                };

                // complete arg string as first parameter
                t.match.emplace_back(t.args);

                for (size_t j = 0; j < t.args.size(); ++j)
                {
                  std::stringstream ss; ss << t.args.at(j);
                  auto s = ss.str();
                  // std::cerr << "E:" << e << "\n";
                  if (s.find_first_of(" \n\t") != std::string::npos)
                  {
                    continue;
                  }
                  if (s.find_first_of(".-+0123456789") != std::string::npos)
                  {
                    // num
                    // std::cerr << "Num\n";
                    t.match.emplace_back(std::string());
                    for (;j < t.args.size() && t.args.at(j) != ' '; ++j)
                    {
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg\n" << t.match.back() << "\n";
                    for (auto const& e : reg_num)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        // std::cerr << "ArgValid\nnum\n" << t.match.back() << "\n\n";
                      }
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
                    // std::cerr << "Arg\n" << t.match.back() << "\n";
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
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
                    // std::cerr << "Arg\n" << t.match.back() << "\n";
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                  }
                  else
                  {
                    // invalid arg
                    if (readline_)
                    {
                      std::cerr << "Error: macro " + t.name + " has an invalid argument\n";
                      i += delim_end_.size() - 1;
                      stk = std::stack<Tmacro>();
                      continue;
                    }
                    throw std::runtime_error("macro " + t.name + " has an invalid argument");
                  }
                }
                // std::cerr << "ArgValid\ncomplete\n\n";
              }
              else
              {
                std::smatch match;
                if (std::regex_match(t.args, match, std::regex(it->second.regex)))
                {
                  for (auto const& e : match)
                  {
                    t.match.emplace_back(std::string(e));
                  }
                }
              }

                // process macro
                int ec {0};
                Ctx ctx {t.res, t.match, cache_};
                if (it->second.type == Mtype::internal)
                {
                  ec = run_internal(it->second, ctx);
                }
                else if (it->second.type == Mtype::remote)
                {
                  ec = run_remote(it->second, ctx);
                }
                else
                {
                  ec = run_external(it->second, ctx);
                }

                if (ec != 0)
                {
                  if (readline_)
                  {
                    std::cerr << "Error: macro " + t.name + " failed\n";
                    i += delim_end_.size() - 1;
                    stk = std::stack<Tmacro>();
                    continue;
                  }
                  throw std::runtime_error("macro " + t.name + " failed");
                }

                // debug
                // std::cerr << "\nRes:\n" << t.res << "\n\n";

                if (t.res.find(delim_start_) != std::string::npos)
                {
                  {
                    // remove all nl chars that follow delim_end
                    // this would normally be removed by the reader
                    size_t pos {0};
                    for (;;)
                    {
                      pos = t.res.find(delim_end_ + "\n", pos);
                      if (pos == std::string::npos) break;
                      t.res.replace(pos, delim_end_.size() + 1, delim_end_);
                      ++pos;
                    }
                  }

                  // std::cerr << "line:\n" << line << "\n\n";
                  line.insert(i + delim_end_.size(), t.res);
                  // std::cerr << "res:\n" << t.res << "\n\n";
                  // std::cerr << "line:\n" << line << "\n\n";
                  i += delim_end_.size() - 1;
                  continue;
                }

                if (! stk.empty())
                {
                  stk.top().str += t.res;
                }
                else
                {
                  buf += t.res;

                  // account for when end delim is last char on line
                  // add a newline char to buf
                  // only if response is not empty
                  if ((! t.res.empty()) && (i == line.size() - 1))
                  {
                    buf += "\n";
                  }

                  if (readline_)
                  {
                    std::cout << buf;

                    if (! buf.empty() && buf.back() != '\n')
                    {
                      std::cout
                      << AEC::reverse
                      << "%"
                      << AEC::reset
                      << "\n";
                    }
                  }
                }

              // }
              // else
              // {
              //   std::cout << Cl::fg_red << "Invalid regex: " << Cl::reset;
              //   std::cout << _ifile << ":"
              //     << t.line_start << ":" <<
              //     (t.begin + delim_start_.size() + 1) << "\n";
              //   std::cout << Cl::fg_magenta << "macro: " << Cl::fg_green << t.name << t.args << "\n";
              //   std::cout << Cl::fg_magenta << "regex: " << Cl::fg_green << it->second.regex << "\n" << Cl::reset;
              //   throw std::runtime_error("invalid regex");
              // }
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

// regular_char:

      // case else other characters
      if (! stk.empty())
      {
        // inside macro
        auto& t = stk.top();

        if (i == (line.size() - 1))
        {
          t.str += line.at(i);
          t.str += "\n";
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
            buf += line.at(i);
            buf += "\n";
          }
          else
          {
            buf += line.at(i);
          }
        }
      }

    }

    // append buf to output file
    if (! _ofile.empty())
    {
      w.write(buf);
    }

    // debug
    // std::cout << mark_line << "\n\n";

  }

  if (! stk.empty())
  {
    throw std::runtime_error("missing closing delimiter");
    std::cerr << "Error: missing closing delimiter\n";
  }

  if (! _ofile.empty())
  {
    w.close();
  }

  // debug
  // std::cerr << ast_.str() << std::endl;
  // std::ofstream ofile_ast {"ast.txt"};
  // ofile_ast << ast_.str();
  // ofile_ast.close();
}

int M8::run_internal(Macro const& macro, Ctx& ctx)
{
  ++internal_count_;

  return macro.func(ctx);
}

int M8::run_external(Macro const& macro, Ctx& ctx)
{
  ++external_count_;

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
  ++remote_count_;

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
