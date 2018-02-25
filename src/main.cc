#include "m8.hh"
#include "macros.hh"

#include "parg.hh"
using Parg = OB::Parg;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <regex>
#include <functional>

int program_options(Parg& pg);
int start_m8(Parg& pg);

int program_options(Parg& pg)
{
  pg.name("m8").version("0.3.0 (02.25.2018)");
  pg.description("a meta programming tool");
  pg.usage("[flags] [options] [--] [arguments]");
  pg.usage("[-f 'input_file'] [-o 'output_file'] [-c 'config file'] [-s 'start_delim'] [-e 'end_delim'] [d]");
  pg.usage("[-v|--version]");
  pg.usage("[-h|--help]");
  pg.info("Exit Codes", {"0 -> normal", "1 -> error"});
  pg.info("Examples", {
    "m8 -f input_file -o ouput_file",
    "m8 -f input_file -o ouput_file -c ~/custom_path/.m8",
    "m8 -f input_file -o ouput_file -s '[[' -e ']]'",
    "m8 --list",
    "m8 --help",
    "m8 --version",
  });
  pg.author("Brett Robinson (octobanana) <octobanana.dev@gmail.com>");

  // singular flags
  pg.set("help,h", "print the help output");
  pg.set("version,v", "print the program version");
  pg.set("list", "list all defined macros");

  // combinable flags
  pg.set("debug,d", "print out debug information, useful for debugging macro regexes");

  // singular options
  pg.set("info", "", "str", "view informatin on specific macro");

  // combinable options
  pg.set("config,c", "", "file_name", "the config file");
  pg.set("file,f", "", "file_name", "the input file");
  pg.set("output,o", "", "file_name", "the output file");
  pg.set("start,s", "", "str", "the starting delimiter");
  pg.set("end,e", "", "str", "the ending delimiter");

  // pg.set_pos();
  // pg.set_stdin();

  int status {pg.parse()};
  // uncomment if at least one argument is expected
  if (status > 0 && pg.get_stdin().empty())
  {
    std::cout << pg.print_help() << "\n";
    std::cout << "Error: " << "expected arguments" << "\n";
    return -1;
  }
  if (status < 0)
  {
    std::cout << pg.print_help() << "\n";
    std::cout << "Error: " << pg.error() << "\n";
    return -1;
  }
  if (pg.get<bool>("help"))
  {
    std::cout << pg.print_help();
    return 1;
  }
  if (pg.get<bool>("version"))
  {
    std::cout << pg.print_version();
    return 1;
  }
  return 0;
}

int start_m8(Parg& pg)
{
  try
  {
    // init M8 object
    M8 m8;

    // set debug option
    m8.set_debug(pg.get<bool>("debug"));

    // set config file
    m8.set_config(pg.get("config"));

    // set start and end delimiters
    if (pg.find("start") && pg.find("end"))
    {
      m8.delimit(pg.get("start"), pg.get("end"));
    }

    // add internal macros
    Macros::macros(m8);

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

    // check to see if ifile and ofile args were given
    // if (! pg.find("file") || ! pg.find("output"))
    if (! pg.find("file"))
    {
      std::cout << "Error: missing required 'file' argument\n";
      return 1;
    }

    // run m8
    m8.run(pg.get("file"), pg.get("output"));

    // print out summary
    std::cerr << m8.summary();
    return 0;
  }
  catch (std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return -1;
  }
}

int main(int argc, char *argv[])
{
  Parg pg {argc, argv};
  int pstatus {program_options(pg)};
  if (pstatus > 0)
  {
    return 0;
  }
  else if (pstatus < 0)
  {
    return 1;
  }

  return start_m8(pg);
}
