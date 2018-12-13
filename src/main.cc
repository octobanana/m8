#include "m8.hh"
#include "m8_macros.hh"
#include "user_macros.hh"
#include "timer.hh"
#include "crypto.hh"

#include "string.hh"
namespace String = OB::String;

#include "term.hh"
using Term = OB::Term;

#include "parg.hh"
using Parg = OB::Parg;

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include <filesystem>
namespace fs = std::filesystem;

#include <string>
#include <iomanip>
#include <iostream>
#include <algorithm>

int program_options(Parg& pg);
int start_m8(Parg& pg);

struct Version
{
  std::string const major {};
  std::string const minor {};
  std::string const patch {};
}; // struct Version

int program_options(Parg& pg)
{
  Version v {"0", "5", "10"};
  std::string const date {"30.10.2018"};
  std::string const author {"Brett Robinson (octobanana) <octobanana.dev@gmail.com>"};

  pg.name("m8").version(v.major + "." + v.minor + "." + v.patch + " (" + date + ")");

  pg.description("a meta programming tool");
  pg.usage("[flags] [options] [--] [arguments]");
  pg.usage("['input_file'] [-o 'output_file'] [-c 'config file'] [-s 'start_delim' -e 'end_delim' | -m 'mirror_delim'] [d]");
  pg.usage("[-v|--version]");
  pg.usage("[-h|--help]");
  pg.info("Exit Codes", {"0 -> normal", "1 -> error"});
  pg.info("Examples", {
    pg.name() + "input_file -o ouput_file",
    pg.name() + "input_file -o ouput_file -c ~/custom_path/.m8",
    pg.name() + "input_file -o ouput_file -s '[[' -e ']]'",
    pg.name() + "input_file -o ouput_file -m '[['",
    pg.name() + "--list",
    pg.name() + "--help",
    pg.name() + "--version",
  });
  pg.author(author);

  // singular flags
  pg.set("help,h", "print the help output");
  pg.set("version,v", "print the program version");
  pg.set("list", "list all defined macros");

  // combinable flags
  pg.set("debug,d", "print out debug information, useful for debugging macro regexes");
  pg.set("interpreter,i", "start the interpreter in readline mode");
  pg.set("no-copy", "do not copy outside text");
  pg.set("summary", "print out summary at end");
  pg.set("timer,t", "print out execution time in milliseconds");
  pg.set("color", "print output in color");
  // TODO add flag to ignore empty lines
  // pg.set("ignore-empty", "ignore empty lines");

  // singular options
  pg.set("info", "", "str", "view informatin on specific macro");

  // combinable options
  pg.set("config,c", "", "file_name", "the config file");
  pg.set("file,f", "", "file_name", "the input file");
  pg.set("output,o", "", "file_name", "the output file");
  pg.set("start,s", "", "str", "the starting delimiter");
  pg.set("end,e", "", "str", "the ending delimiter");
  pg.set("mirror,m", "", "str", "mirror the delimiter");
  pg.set("ignore", "", "regex", "regex to ignore matching names");
  pg.set("comment", "", "str", "comment symbol");
  // TODO add option to define variable
  // pg.set("define,D", "", "str", "code to exec before parsing");

  pg.set_pos();
  // pg.set_stdin();

  int status {pg.parse()};
  // uncomment if at least one argument is expected
  if (status > 0 && pg.get_stdin().empty())
  {
    std::cout << pg.help() << "\n";
    std::cout << "Error: " << "expected arguments" << "\n";
    return -1;
  }
  if (status < 0)
  {
    std::cout << pg.help() << "\n";
    std::cout << "Error: " << pg.error() << "\n";
    return -1;
  }
  if (pg.get<bool>("help"))
  {
    std::cout << pg.help();
    return 1;
  }
  if (pg.get<bool>("version"))
  {
    std::cout << pg.name() << " v" << pg.version() << "\n";
    return 1;
  }

  if (pg.get<bool>("color") && (! Term::is_term(STDOUT_FILENO) || ! Term::is_term(STDERR_FILENO)))
  {
    std::cout << "Error: flag 'color' can not be used on a non-interactive terminal\n";
    return -1;
  }

  return 0;
}

std::string mirror_delim(std::string str)
{
  if (str.empty())
  {
    return {};
  }

  std::reverse(std::begin(str), std::end(str));

  size_t const len {1};
  for (size_t pos = 0;; ++pos)
  {
    pos = str.find_first_of("()[]{}<>", pos);

    if (pos == std::string::npos)
    {
      break;
    }

    switch (str.at(pos))
    {
      case '(':
        str.replace(pos, len, ")");
        break;
      case ')':
        str.replace(pos, len, "(");
        break;

      case '[':
        str.replace(pos, len, "]");
        break;
      case ']':
        str.replace(pos, len, "[");
        break;

      case '{':
        str.replace(pos, len, "}");
        break;
      case '}':
        str.replace(pos, len, "{");
        break;

      case '<':
        str.replace(pos, len, ">");
        break;
      case '>':
        str.replace(pos, len, "<");
        break;

      default:
        break;
    }
  }
  return str;
}

int start_m8(Parg& pg)
{
  try
  {
    // init M8 object
    M8 m8;

    // add internal macros
    Macros::m8_macros(m8);

    // add user macros
    Macros::user_macros(m8);

    // list out all macros if --list option given
    if (pg.get<bool>("list"))
    {
      std::cout << m8.list_macros();
      return 0;
    }

    // output info on macro if --info option given
    if (pg.find("info"))
    {
      std::cout << m8.macro_info(pg.get("info"));
      return 0;
    }

    // set debug option
    m8.set_debug(pg.get<bool>("debug"));

    // set comment option
    m8.set_comment(pg.get("comment"));

    // set ignore option
    if (pg.find("ignore"))
    {
      m8.set_ignore(pg.get("ignore"));
    }

    // set readline option
    m8.set_readline(pg.get<bool>("interpreter"));

    // set config file
    m8.set_config(pg.get("config"));

    // set no-copy option
    m8.set_copy(! pg.get<bool>("no-copy"));

    // set start and end delimiters
    if (pg.find("mirror"))
    {
      auto delim = pg.get("mirror");
      if (delim.size() <= 1)
      {
        throw std::runtime_error("delimiter must be at least 2 chars long");
      }
      auto rdelim = mirror_delim(delim);
      m8.set_delimits(delim, rdelim);
      Macros::m8_delim_start = delim;
      Macros::m8_delim_end = rdelim;
    }
    else if (pg.find("start") && pg.find("end"))
    {
      auto delim_start = pg.get("start");
      auto delim_end = pg.get("end");
      if (delim_start.size() <= 1)
      {
        throw std::runtime_error("start delimiter must be at least 2 chars long");
      }
      if (delim_end.size() <= 1)
      {
        throw std::runtime_error("end delimiter must be at least 2 chars long");
      }
      m8.set_delimits(delim_start, delim_end);
      Macros::m8_delim_start = delim_start;
      Macros::m8_delim_end = delim_end;
    }

    // parse
    if (pg.get<bool>("interpreter"))
    {
      m8.parse();
    }
    else
    {
      auto positionals = pg.get_pos_vec();
      if (positionals.empty())
      {
        throw std::runtime_error("expected input file");
      }
      for (auto const& e : positionals)
      {
        m8.parse(e, pg.get("output"));
      }
    }

    if (! pg.get("output").empty())
    {
      std::string ofile {pg.get("output")};
      fs::path fp {ofile};
      std::string otmp {".m8/swp/" + Crypto::sha256(fp) + ".swp.m8"};
      fs::path p1 {otmp};
      fs::path p2 {ofile};
      if (! p2.parent_path().empty())
      {
        fs::create_directories(p2.parent_path());
      }
      fs::rename(p1, p2);
    }

    // print out summary
    if (pg.get<bool>("summary"))
    {
      if (! pg.get<bool>("interpreter"))
      {
        std::cerr << AEC::wrap("File: ", AEC::fg_magenta) << AEC::wrap(pg.get("output"), AEC::fg_green) << "\n";
      }
      std::cerr << m8.summary();
    }

    return 0;
  }
  catch (std::exception const& e)
  {
    std::cerr << AEC::wrap("Error: ", AEC::fg_red) << e.what() << "\n";
    return 1;
  }
}

int main(int argc, char *argv[])
{
  Parg pg {argc, argv};
  int pstatus {program_options(pg)};
  if (pstatus > 0) return 0;
  if (pstatus < 0) return 1;

  auto show_time = pg.get<bool>("timer");
  OB::Timer t;
  if (show_time)
  {
    t.start();
  }

  auto status = start_m8(pg);

  if (show_time)
  {
    t.stop();
    long double time {static_cast<long double>(t.time<std::chrono::nanoseconds>())};
    time /= 1000000000;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << time;
    std::cout << AEC::wrap("time: ", AEC::fg_magenta) << AEC::wrap(ss.str(), AEC::fg_green) << "\n\n";
  }

  return status;
}
