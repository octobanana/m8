#include "m8.hh"
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
    ss << e.second.usage << "\n";
    ss << "  " << e.second.info << "\n";
    ss << "  " << e.second.regex << "\n\n";
  }

  return ss.str();
}

std::string M8::macro_info(std::string const& name) const
{
  if (macros.find(name) == macros.end())
  {
    return "Error: macro '" + name + "' not found\n";
  }
  auto const& e = macros.at(name);
  std::stringstream ss; ss
  << e.usage << "\n"
  << "  " << e.info << "\n"
  << "  " << e.regex << "\n";
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

void M8::run(std::string const& ifile, std::string const& ofile)
{
  struct Tmacro
  {
    uint32_t line_start {0};
    uint32_t line_end {0};
    size_t begin {0};
    size_t end {0};
    std::string name;
    std::string fn;
    std::vector<Tmacro> children;
    std::string res;
  };
  std::vector<Tmacro> ast;
  std::stack<Tmacro> stk;

  // init the parser obj with the input file
  Parser p {ifile};

  std::string line;
  while(p.get_next(line))
  {
    if (line.empty()) continue;

    // debug
    std::cout << p.current_line() << ".\n" << line << "\n";
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

      // start delimiter
      if (line.at(i) == delim_start_.at(0))
      {
        size_t pos_start = line.find(delim_start_, i);
        if (pos_start != std::string::npos)
        {
          //debug
          mark_line.replace(pos_start + Cl::fg_black.size(), 1, "^");

          auto t = Tmacro {};
          t.line_start = p.current_line();
          t.begin = pos_start;
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

          if (stk.empty())
          {
            throw std::runtime_error("missing matching delimiter");
          }
          else
          {
            auto t = stk.top();
            stk.pop();

            t.line_end = p.current_line();
            t.end = pos_end;

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

    }

    std::cout << mark_line << "\n\n";

  }

  // debug
  // print out ast
  {
    std::function<std::string(Tmacro const&, size_t depth)> const print_tmacro = [&](Tmacro const& t, size_t depth) {
      std::string indent;
      for (size_t i = 0; i < depth; ++i)
      {
        indent += " ";
      }

      std::stringstream ss; ss
        << indent << "(\n\n"
        << indent << "  " << "s-line   : " << t.line_start << "\n"
        << indent << "  " << "begin    : " << t.begin << "\n"
        << indent << "  " << "e-line   : " << t.line_end << "\n"
        << indent << "  " << "end      : " << t.end << "\n";
      if (t.children.empty())
      {
        ss << indent << "  " << "children : 0\n\n";
      }
      else
      {
        ss << indent << "  " << "children : " << t.children.size() << "\n\n";
        for (auto const& c : t.children)
        {
          ss << print_tmacro(c, depth + 2);
        }
      }
      ss << indent << ")\n\n";
      return ss.str();
    };

    std::cout << "(\n\n";
    for (auto const& e : ast)
    {
      std::cout << print_tmacro(e, 2);
    }
    std::cout << ")\n";
  }

}

void M8::run_(std::string const& ifile, std::string const& ofile)
{
  // check the input file
  ifile_.open(ifile);
  if (! ifile_.is_open())
  {
    throw std::runtime_error("could not open the input file");
  }

  // TODO incrementally read file

  // getline
  // store line number
  // search for start
  // if found store start line:column
  // search for additional start
  // if found recurse
  // search for end
  // if found store end line:column
  // read macro call into string buffer
  // perform macro
  // store result in struct
  // repeat until eof

  // back to start of file
  // getline to output file
  // until macro pos found
  // output macro result
  // continue until eof

  // read entire file into string
  std::string text;
  text.assign((std::istreambuf_iterator<char>(ifile_)),
    (std::istreambuf_iterator<char>()));

  // close input file
  ifile_.close();

  // check the output file
  if (ofile.empty())
  {
    use_stdout_ = true;
  }
  else
  {
    ofile_.open(ofile);
    if (! ofile_.is_open())
    {
      throw std::runtime_error("could not open the output file");
    }
  }

  // bool done {true};

  // loop for multiple passes to make sure all macros were found
  // do
  // {
  //   done = true;

    std::string::size_type str_start {0};
    std::string::size_type str_end {0};

    ++pass_count_;
    if (debug_)
    {
      std::cerr << "Debug: pass -> " << pass_count_ << "\n";
    }

    std::function<int(std::string::size_type pos_start, std::string::size_type pos_end, int depth)> const
      find_and_replace = [&](std::string::size_type pos_start, std::string::size_type pos_end, int depth) {
      for (/* */; pos_start != std::string::npos; ++pos_start)
      {
        if (debug_)
        {
          std::cerr << "Debug: depth -> " << depth << "\n";
        }

        pos_start = text.find(delim_start_, pos_start);
        pos_end = text.find(delim_end_, pos_start);

        if (pos_start == std::string::npos || pos_end == std::string::npos)
        {
          break;
        }

        // if additional start delim is found before end delim, recurse
        if (text.substr(pos_start + 1, pos_end).find(delim_start_, 0) != std::string::npos)
        {
          find_and_replace(pos_start + 1, pos_end, depth + 1);

          // find the new end delim
          pos_end = text.find(delim_end_, pos_start);
          if (pos_end == std::string::npos)
          {
            break;
          }
        }

        size_t len = (pos_end - pos_start + len_end);
        std::string fn = text.substr((pos_start + len_start), (len - len_start - len_end));
        std::string name = fn.substr(0, fn.find_first_of(" ", 0));
        if (debug_)
        {
          std::cerr << "Debug: depth -> " << depth << "\n";
          std::cerr << "Debug: fn -> " << fn << "\n";
          std::cerr << "Debug: name -> " << name << "\n";
        }

        // check if macro exists
        auto const it = macros.find(name);
        if (it == macros.end())
        {
          // TODO how should undefined macro be treated
          ++warning_count_;
          std::cout << "Warning: undefined macro '" << name << "'\n";
          text.replace(pos_start, len, "");
          // text.replace(pos_start, len, "/* " + fn + " */");
          // skip to end of invalid macro
          pos_start = pos_end;
          continue;
        }

        std::string args = fn.substr(fn.find_first_of(" ") + 1, fn.size());
        if (debug_)
        {
          std::cerr << "Debug: args -> " << args << "\n";
        }

        // match the defined regex
        std::smatch match;
        if (std::regex_match(args, match, std::regex(it->second.regex)))
        {
          int ec {0};
          std::string res;

          if (it->second.type == Mtype::internal)
          {
            ec = run_internal(it->second, res, match, it->second.fn);
          }
          else if (it->second.type == Mtype::remote)
          {
            ec = run_remote(it->second, res, match, it->second.fn);
          }
          else
          {
            ec = run_external(it->second, res, match, it->second.fn);
          }

          // TODO handle when function return error
          // replace text with what?
          if (ec != 0)
          {
            // std::cout << "Error: macro '" << it->first << "' failed\n";
            // std::cout << "Info: " << it->second.info << "\n";
            // std::cout << "Usage: " << it->second.usage << "\n";
            // std::cout << "Regex: " << it->second.regex << "\n";
            // std::cout << "Res: " << res << "\n";
            return 1;
          }
          else
          {
            text.replace(pos_start, len, res);
            ++macro_count_;
            // done = false;

            // if (depth > 0) break;
          }
        }
        else
        {
          // TODO how should invalid regex be treated
          ++warning_count_;
          std::cout << "Warning: macro '" << name << "' has invalid regex\n";
          text.replace(pos_start, len, "");
          // text.replace(pos_start, len, "/* " + fn + " */");
          // skip to end of invalid macro
          pos_start = pos_end;
        }
      }

      return 0;
    };

    find_and_replace(str_start, str_end, 0);

  // }
  // while (! done);

  if (use_stdout_)
  {
    std::cout << text << std::flush;
  }
  else
  {
    // write to output file
    ofile_ << text;

    // close output file
    ofile_.close();
  }
}

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
