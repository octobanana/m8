#include "m8.hh"
#include "m8_macros.hh"
#include "user_macros.hh"

#include "parg.hh"
using Parg = OB::Parg;

#include <string>
#include <iostream>
#include <algorithm>

int program_options(Parg& pg);
void replace_all(std::string& str, std::string const& key, std::string const& val);
int start_m8(Parg& pg);

int program_options(Parg& pg)
{
  std::string const v_major {"0"};
  std::string const v_minor {"5"};
  std::string const v_patch {"2"};
  std::string const v_date {"21.04.2018"};
  pg.name("m8").version(v_major + "." + v_minor + "." + v_patch + " (" + v_date + ")");

  pg.description("a meta programming tool");
  pg.usage("[flags] [options] [--] [arguments]");
  pg.usage("[-f 'input_file'] [-o 'output_file'] [-c 'config file'] [-s 'start_delim'] [-e 'end_delim'] [d]");
  pg.usage("[-v|--version]");
  pg.usage("[-h|--help]");
  pg.info("Exit Codes", {"0 -> normal", "1 -> error"});
  pg.info("Examples", {
    pg.name() + "-f input_file -o ouput_file",
    pg.name() + "-f input_file -o ouput_file -c ~/custom_path/.m8",
    pg.name() + "-f input_file -o ouput_file -s '[[' -e ']]'",
    pg.name() + "--list",
    pg.name() + "--help",
    pg.name() + "--version",
  });
  pg.author("Brett Robinson (octobanana) <octobanana.dev@gmail.com>");

  // singular flags
  pg.set("help,h", "print the help output");
  pg.set("version,v", "print the program version");
  pg.set("list", "list all defined macros");

  // combinable flags
  pg.set("debug,d", "print out debug information, useful for debugging macro regexes");
  pg.set("interpreter,i", "start the interpreter in readline mode");
  pg.set("no-copy", "do not copy outside text");
  pg.set("summary", "print out summary at end");

  // singular options
  pg.set("info", "", "str", "view informatin on specific macro");

  // combinable options
  pg.set("config,c", "", "file_name", "the config file");
  pg.set("file,f", "", "file_name", "the input file");
  pg.set("output,o", "", "file_name", "the output file");
  pg.set("start,s", "", "str", "the starting delimiter");
  pg.set("end,e", "", "str", "the ending delimiter");
  pg.set("mirror,m", "", "str", "mirror the delimiter");

  // pg.set_pos();
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
  return 0;
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

    // set readline option
    m8.set_readline(pg.get<bool>("interpreter"));

    // set config file
    m8.set_config(pg.get("config"));

    // set no-copy option
    m8.set_copy(! pg.get<bool>("no-copy"));

    // set start and end delimiters
    if (pg.find("mirror"))
    {
      auto delim = pg.get<std::string>("mirror");
      auto rdelim = delim;
      std::reverse(std::begin(rdelim), std::end(rdelim));
      replace_all(rdelim, "(", ")");
      replace_all(rdelim, "[", "]");
      replace_all(rdelim, "{", "}");
      replace_all(rdelim, "<", ">");
      m8.set_delimits(delim, rdelim);
      Macros::m8_delim_start = delim;
      Macros::m8_delim_end = rdelim;
    }
    else if (pg.find("start") && pg.find("end"))
    {
      m8.set_delimits(pg.get("start"), pg.get("end"));
      Macros::m8_delim_start = pg.get("start");
      Macros::m8_delim_end = pg.get("end");
    }

    // parse
    m8.parse(pg.get("file"), pg.get("output"));

    // print out summary
    if (pg.get<bool>("summary"))
    {
      std::cerr << m8.summary();
    }

    return 0;
  }
  catch (std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

int main(int argc, char *argv[])
{
  Parg pg {argc, argv};
  int pstatus {program_options(pg)};
  if (pstatus > 0) return 0;
  if (pstatus < 0) return 1;

  return start_m8(pg);
}
