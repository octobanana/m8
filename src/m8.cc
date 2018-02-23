#include "m8.hh"
#include "sys_command.hh"
#include "http.hh"

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
  macros[name] = {Mtype::external, info, usage, regex, "", nullptr};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex, std::string const& url)
{
  if (macros.find(name) != macros.end())
  {
    throw std::logic_error("multiple definitions of macro: " + name);
  }
  macros[name] = {Mtype::remote, info, usage, regex, url, nullptr};
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string const& regex, macro_fn fn)
{
  if (macros.find(name) != macros.end())
  {
    throw std::logic_error("multiple definitions of macro: " + name);
  }
  macros[name] = {Mtype::internal, info, usage, regex, "", fn};
}

void M8::delimit(std::string const& delim_start, std::string const& delim_end)
{
  start_ = delim_start;
  end_ = delim_end;
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
  << "  passes   " << pass_count_ << "\n"
  << "  warnings " << warning_count_ << "\n"
  << "  Macros   " << macro_count_ << "\n";
  return ss.str();
}

void M8::run(std::string const& ifile, std::string const& ofile)
{
  // check the input file
  ifile_.open(ifile);
  if (! ifile_.is_open())
  {
    throw std::runtime_error("could not open the input file");
  }

  // TODO incrementally read file
  // TODO keep track of line for errors/warnings
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

  // find and replace macros
  size_t len_start {start_.length()};
  size_t len_end {end_.length()};
  size_t len_total {len_start + len_end};

  bool done {true};

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

        pos_start = text.find(start_, pos_start);
        pos_end = text.find(end_, pos_start);

        if (pos_start == std::string::npos || pos_end == std::string::npos)
        {
          break;
        }

        // if additional start delim is found before end delim, recurse
        if (text.substr(pos_start + 1, pos_end).find(start_, 0) != std::string::npos)
        {
          find_and_replace(pos_start + 1, pos_end, depth + 1);

          // find the new end delim
          pos_end = text.find(end_, pos_start);
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
            // internal macro
            auto& func = it->second.fn;
            ec = func(res, match);
          }
          else if (it->second.type == Mtype::remote)
          {
            // remote macro
            Http api;
            api.req.method = "POST";
            api.req.headers.emplace_back("content-type: application/json");
            api.req.url = it->second.url;

            Json data;
            data["name"] = it->first;
            data["args"] = match;
            api.req.data = data.dump();

            std::cout << "Remote macro call -> " << it->first << "\n";
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

            ec = send.get();
            if (ec == 0)
              std::cout << "\033[2K\r" << "Success: remote call\n";
            else
              std::cout << "\033[2K\r" << "Error: remote call\n";
          }
          else
          {
            // external macro
            std::string m_args;
            for (size_t i = 1; i < match.size(); ++i)
            {
              m_args += match[i];
              if (i < match.size() - 1)
                m_args += " ";
            }
            ec = exec(res, name + " " + m_args);
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

std::string M8::env_var(std::string const& var) const
{
  std::string res;
  if (const char* envar = std::getenv(var.c_str()))
  {
    res = envar;
  }
  return res;
}
