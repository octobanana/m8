#include "m8.hh"
#include "ast.hh"
using Tmacro = OB::Tmacro;
using Ast = OB::Ast;
#include "parser.hh"
using Parser = OB::Parser;
#include "sys_command.hh"
#include "http.hh"
#include "color.hh"
namespace Cl = OB::Color;

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

void M8::set_debug(bool const& val)
{
  debug_ = val;
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex)
{
  if (macros.find(name) != macros.end())
  {
    throw std::logic_error("multiple definitions of macro: " + name);
  }
  macros[name] = {Mtype::external, name, info, usage, regex, "", nullptr};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex, std::string const& url)
{
  if (macros.find(name) != macros.end())
  {
    throw std::logic_error("multiple definitions of macro: " + name);
  }
  macros[name] = {Mtype::remote, name, info, usage, regex, url, nullptr};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex, macro_fn fn)
{
  if (macros.find(name) != macros.end())
  {
    throw std::logic_error("multiple definitions of macro: " + name);
  }
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
    // return with no error if external macros do not matter
    // or throw exception if config file is mandatory

    // throw std::runtime_error("could not open the config file");
    return;
  }

  // read in the config file into memory
  std::string content;
  content.assign((std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

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
  file.close();
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

void M8::write(std::string const& _ifile, std::string const& _ofile)
{
  std::ifstream ifile {_ifile};
  if (! ifile.is_open())
  {
    throw std::runtime_error("could not open input file");
  }

  std::ifstream ofile {_ofile};
  if (! ofile.is_open())
  {
    throw std::runtime_error("could not open output file");
  }

  auto& ast = ast_.ast;

  for (auto const& e : ast)
  {
  }
}

void M8::parse(std::string const& ifile)
{
  auto& ast = ast_.ast;

  // init the parsing stack
  std::stack<Tmacro> stk;

  // init the parser with the input file
  Parser p {ifile};

  std::string line;
  while(p.get_next(line))
  {
    if (line.empty()) continue;

    // debug
    // std::cout << p.current_line() << ".\n" << line << "\n";
    std::string mark_line;
    mark_line.reserve(line.size() + (Cl::fg_black.size() * 2));
    for (size_t i = 0; i < line.size(); ++i)
    {
      mark_line += " ";
    }
    mark_line = Cl::wrap(mark_line, {Cl::fg_green});

    // parse line char by char for either start or end delim
    for (size_t i = 0; i < line.size(); ++i)
    {

      // check for comment ';'
      if (line.at(i) == ';')
      {
        break;
      }

      // start delimiter
      if (line.at(i) == delim_start_.at(0))
      {
        size_t pos_start = line.find(delim_start_, i);
        if (pos_start != std::string::npos)
        {
          //debug
          mark_line.replace(pos_start + Cl::fg_black.size(), 1, "^");

          // stack operations
          auto t = Tmacro();
          t.line_start = p.current_line();
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

      // end delimiter
      if (line.at(i) == delim_end_.at(0))
      {
        size_t pos_end = line.find(delim_end_, i);
        if (pos_end != std::string::npos)
        {
          //debug
          mark_line.replace(pos_end + Cl::fg_black.size(), 1, "^");

          // stack operations
          if (stk.empty())
          {
            throw std::runtime_error("missing opening delimiter");
          }
          else
          {
            auto t = stk.top();
            stk.pop();

            t.line_end = p.current_line();
            t.end = pos_end;

            // parse str into name and args
            {
              std::smatch match;
              std::string name_args {"^\\s*([a-zA-Z./*\\-+!?]+)\\s*([^\\r]*?)\\s*$"};
              if (std::regex_match(t.str, match, std::regex(name_args)))
              {
                t.name = match[1];
                t.args = match[2];
                // std::cerr << "name: =" << t.name << "=" << std::endl;
                // std::cerr << "args: =" << t.args << "=" << std::endl;
              }
              else
              {
                throw std::runtime_error("invalid macro format");
              }
            }

            // validate name and args
            {
              auto const it = macros.find(t.name);
              if (it == macros.end())
              {
                std::cout << Cl::fg_red << "Undefined name: " << Cl::reset;
                std::cout << ifile << ":"
                  << t.line_start << ":" <<
                  (t.begin + delim_start_.size() + 1) << "\n";
                std::cout << Cl::fg_magenta << "macro: " << Cl::fg_green << t.str << "\n";
                std::cout << Cl::fg_magenta << "       ^" << Cl::reset << "\n";

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

                throw std::runtime_error("undefined name");
              }
              std::smatch match;
              if (std::regex_match(t.args, match, std::regex(it->second.regex)))
              {
                for (auto const& e : match)
                {
                  t.match.emplace_back(std::string(e));
                }

                // process macro
                int ec {0};
                if (it->second.type == Mtype::internal)
                {
                  ec = run_internal(it->second, t.res, match, it->second.fn);
                }
                else if (it->second.type == Mtype::remote)
                {
                  ec = run_remote(it->second, t.res, match, it->second.fn);
                }
                else
                {
                  ec = run_external(it->second, t.res, match, it->second.fn);
                }

                if (ec != 0)
                {
                  throw std::runtime_error("macro " + t.name + " failed");
                }

                // debug
                // std::cerr << t.res << std::endl;

                if (! stk.empty())
                {
                  stk.top().str += t.res;
                }

              }
              else
              {
                std::cout << Cl::fg_red << "Invalid regex: " << Cl::reset;
                std::cout << ifile << ":"
                  << t.line_start << ":" <<
                  (t.begin + delim_start_.size() + 1) << "\n";
                std::cout << Cl::fg_magenta << "macro: " << Cl::fg_green << t.str << "\n";
                std::cout << Cl::fg_magenta << "regex: " << Cl::fg_green << it->second.regex << "\n" << Cl::reset;
                throw std::runtime_error("invalid regex");
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

      // append inner
      if (! stk.empty())
      {
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

    }

    // debug
    // std::cout << mark_line << "\n\n";

  }

  if (! stk.empty())
  {
    throw std::runtime_error("missing closing delimiter");
  }

  // debug
  // std::cerr << ast_.str() << std::endl;
}

// void M8::run_(std::string const& ifile, std::string const& ofile)
// {
//   // check the input file
//   ifile_.open(ifile);
//   if (! ifile_.is_open())
//   {
//     throw std::runtime_error("could not open the input file");
//   }

//   // TODO incrementally read file

//   // getline
//   // store line number
//   // search for start
//   // if found store start line:column
//   // search for additional start
//   // if found recurse
//   // search for end
//   // if found store end line:column
//   // read macro call into string buffer
//   // perform macro
//   // store result in struct
//   // repeat until eof

//   // back to start of file
//   // getline to output file
//   // until macro pos found
//   // output macro result
//   // continue until eof

//   // read entire file into string
//   std::string text;
//   text.assign((std::istreambuf_iterator<char>(ifile_)),
//     (std::istreambuf_iterator<char>()));

//   // close input file
//   ifile_.close();

//   // check the output file
//   if (ofile.empty())
//   {
//     use_stdout_ = true;
//   }
//   else
//   {
//     ofile_.open(ofile);
//     if (! ofile_.is_open())
//     {
//       throw std::runtime_error("could not open the output file");
//     }
//   }

//   // bool done {true};

//   // loop for multiple passes to make sure all macros were found
//   // do
//   // {
//   //   done = true;

//     std::string::size_type str_start {0};
//     std::string::size_type str_end {0};

//     ++pass_count_;
//     if (debug_)
//     {
//       std::cerr << "Debug: pass -> " << pass_count_ << "\n";
//     }

//     std::function<int(std::string::size_type pos_start, std::string::size_type pos_end, int depth)> const
//       find_and_replace = [&](std::string::size_type pos_start, std::string::size_type pos_end, int depth) {
//       for (/* */; pos_start != std::string::npos; ++pos_start)
//       {
//         if (debug_)
//         {
//           std::cerr << "Debug: depth -> " << depth << "\n";
//         }

//         pos_start = text.find(delim_start_, pos_start);
//         pos_end = text.find(delim_end_, pos_start);

//         if (pos_start == std::string::npos || pos_end == std::string::npos)
//         {
//           break;
//         }

//         // if additional start delim is found before end delim, recurse
//         if (text.substr(pos_start + 1, pos_end).find(delim_start_, 0) != std::string::npos)
//         {
//           find_and_replace(pos_start + 1, pos_end, depth + 1);

//           // find the new end delim
//           pos_end = text.find(delim_end_, pos_start);
//           if (pos_end == std::string::npos)
//           {
//             break;
//           }
//         }

//         size_t len = (pos_end - pos_start + len_end);
//         std::string fn = text.substr((pos_start + len_start), (len - len_start - len_end));
//         std::string name = fn.substr(0, fn.find_first_of(" ", 0));
//         if (debug_)
//         {
//           std::cerr << "Debug: depth -> " << depth << "\n";
//           std::cerr << "Debug: fn -> " << fn << "\n";
//           std::cerr << "Debug: name -> " << name << "\n";
//         }

//         // check if macro exists
//         auto const it = macros.find(name);
//         if (it == macros.end())
//         {
//           // TODO how should undefined macro be treated
//           ++warning_count_;
//           std::cout << "Warning: undefined macro '" << name << "'\n";
//           text.replace(pos_start, len, "");
//           // text.replace(pos_start, len, "/* " + fn + " */");
//           // skip to end of invalid macro
//           pos_start = pos_end;
//           continue;
//         }

//         std::string args = fn.substr(fn.find_first_of(" ") + 1, fn.size());
//         if (debug_)
//         {
//           std::cerr << "Debug: args -> " << args << "\n";
//         }

//         // match the defined regex
//         std::smatch match;
//         if (std::regex_match(args, match, std::regex(it->second.regex)))
//         {
//           int ec {0};
//           std::string res;

//           if (it->second.type == Mtype::internal)
//           {
//             ec = run_internal(it->second, res, match, it->second.fn);
//           }
//           else if (it->second.type == Mtype::remote)
//           {
//             ec = run_remote(it->second, res, match, it->second.fn);
//           }
//           else
//           {
//             ec = run_external(it->second, res, match, it->second.fn);
//           }

//           // TODO handle when function return error
//           // replace text with what?
//           if (ec != 0)
//           {
//             // std::cout << "Error: macro '" << it->first << "' failed\n";
//             // std::cout << "Info: " << it->second.info << "\n";
//             // std::cout << "Usage: " << it->second.usage << "\n";
//             // std::cout << "Regex: " << it->second.regex << "\n";
//             // std::cout << "Res: " << res << "\n";
//             return 1;
//           }
//           else
//           {
//             text.replace(pos_start, len, res);
//             ++macro_count_;
//             // done = false;

//             // if (depth > 0) break;
//           }
//         }
//         else
//         {
//           // TODO how should invalid regex be treated
//           ++warning_count_;
//           std::cout << "Warning: macro '" << name << "' has invalid regex\n";
//           text.replace(pos_start, len, "");
//           // text.replace(pos_start, len, "/* " + fn + " */");
//           // skip to end of invalid macro
//           pos_start = pos_end;
//         }
//       }

//       return 0;
//     };

//     find_and_replace(str_start, str_end, 0);

//   // }
//   // while (! done);

//   if (use_stdout_)
//   {
//     std::cout << text << std::flush;
//   }
//   else
//   {
//     // write to output file
//     ofile_ << text;

//     // close output file
//     ofile_.close();
//   }
// }

int M8::run_internal(Macro macro, std::string& res, std::smatch const match, macro_fn fn)
{
  ++internal_count_;

  return fn(res, match);
}

int M8::run_external(Macro macro, std::string& res, std::smatch const match, macro_fn fn)
{
  ++external_count_;

  std::string m_args;
  for (size_t i = 1; i < match.size(); ++i)
  {
    m_args += match[i];
    if (i < match.size() - 1)
      m_args += " ";
  }
  return exec(res, macro.name + " " + m_args);
}

int M8::run_remote(Macro macro, std::string& res, std::smatch const match, macro_fn fn)
{
  ++remote_count_;

  Http api;
  api.req.method = "POST";
  api.req.headers.emplace_back("content-type: application/json");
  api.req.url = macro.url;

  Json data;
  data["name"] = macro.name;
  data["args"] = match;
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
      res = api.res.body;
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
    std::cout << "\033[2K\r" << "Success: remote call\n";
  else
    std::cout << "\033[2K\r" << "Error: remote call\n";

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
