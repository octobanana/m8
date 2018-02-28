#include "parser.hh"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

namespace OB
{

Parser::Parser(std::string const& file_name):
  file_name_ {file_name}
{
  file_.open(file_name_);
  if (! file_.is_open())
  {
    throw std::runtime_error("could not open the input file");
  }
  lines_[0] = 0;
}

Parser::~Parser()
{
  file_.close();
}

bool Parser::get_next(std::string& line)
{
  // file_.seekg(lines_.at(line_));
  ++line_;
  lines_[line_] = file_.tellg();
  std::cout << "\n[debug][parser][get_next][line_] " << line_ << "\n";
  bool status {false};
  if (std::getline(file_, line))
  {
    status = true;
  }
  else
  {
    status = false;
    std::cout << "[debug][parser][get_next] EOF\n";
  }
  return status;
}

uint32_t Parser::current_line()
{
  return line_;
}

} // namespace OB
