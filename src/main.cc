#include "m8.hh"

#include <ob/parg.hh>
using Parg = OB::Parg;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <regex>
#include <functional>

std::string M8_list();
int program_options(Parg& pg);

std::string M8_list()
{
  std::stringstream ss;

  ss << "M8:\n\n";

  for (auto const& e : M8)
  {
    ss << e.second.usage << "\n";
    ss << "  " << e.second.info << "\n";
    ss << "  " << e.second.regex << "\n\n";
  }

  return ss.str();
}

int program_options(Parg& pg)
{
  pg.name("m8").version("0.1.0 (02.03.2018)");
  pg.description("a c++ meta programming tool");
  pg.usage("[flags] [options] [--] [arguments]");
  pg.usage("[-v|--version]");
  pg.usage("[-h|--help]");
  pg.usage("[-f 'input_file' -o 'output_file']");
  pg.info("Exit Codes", {"0 -> normal", "1 -> error"});
  pg.info("Examples", {"app -v", "app -h", "app -help", "app --version"});
  pg.author("Brett Robinson (octobanana) <octobanana.dev@gmail.com>");

  pg.set("help,h", "print the help output");
  pg.set("version,v", "print the program version");
  pg.set("file,f", "", "file_name", "the input file");
  pg.set("output,o", "", "file_name", "the output file");
  pg.set("list,l", "list all defined macros");

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
  if (pg.get<bool>("list"))
  {
    std::cout << M8_list();
    return 1;
  }
  return 0;
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

  if (! pg.find("file") || ! pg.find("output"))
  {
    std::cout << "Error: missing required arguments\n";
    return 1;
  }

  // open input file
  std::ifstream ifile {pg.get("file")};
  if (! ifile.is_open())
  {
    std::cout << "Error: could not open the input file\n";
    return 1;
  }

  // open output file
  std::ofstream ofile {pg.get("output")};
  if (! ofile.is_open())
  {
    std::cout << "Error: could not open the output file\n";
    return 1;
  }

  // read entire file into string
  std::string text;
  text.assign((std::istreambuf_iterator<char>(ifile)),
    (std::istreambuf_iterator<char>()));

  // find and replace macros
  std::string start {"#[M8["};
  std::string end {"]]"};

  int macro_count {0};

  // bool done {true};

  // do
  // {
  //   done = true;

    std::string::size_type pos_start = 0;
    std::string::size_type pos_end = 0;
    size_t len_start {start.length()};
    size_t len_end {end.length()};
    size_t len_total {len_start + len_end};

    do
    {
      pos_start = text.find(start, ++pos_start);
      if (pos_start != std::string::npos)
      {
        pos_end = text.find(end, pos_start);
        if (pos_end != std::string::npos)
        {
          size_t len = (pos_end - pos_start + len_end);
          std::string fn = text.substr((pos_start + len_start), (len - len_start - len_end));
          std::string name = fn.substr(0, fn.find("(", 0));
          // std::cerr << "fn -> " << fn << "\n";
          // std::cerr << "name -> " << name << "\n";

          auto const it = M8.find(name);
          if (it == M8.end())
          {
            std::cout << "Warning: undefined macro '" << name << "'\n";
            continue;
          }

          std::smatch match;
          if (std::regex_match(fn, match, std::regex(it->second.regex)))
          {
            auto& func = it->second.fn;
            std::string res {};
            int status = func(res, match);

            if (status != 0)
            {
              std::cout << "Error: macro '" << it->first << "' failed\n";
              std::cout << "Info: " << it->second.info << "\n";
              std::cout << "Usage: " << it->second.usage << "\n";
              std::cout << "Regex: " << it->second.regex << "\n";
              std::cout << "Res: " << res << "\n";
              return 1;
            }
            else
            {
              text.replace(pos_start, len, res);
              ++macro_count;
              // done = false;
            }
          }
        }
      }
    }
    while (pos_start != std::string::npos);

  // }
  // while (! done);

  ofile << text;

  // close files
  ifile.close();
  ofile.close();

  // print summary
  std::cout << "Total: " << macro_count << "\n";

  return 0;
}
