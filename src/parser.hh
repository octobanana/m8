#ifndef OB_PARSER_HH
#define OB_PARSER_HH

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

namespace OB
{

class Parser
{
public:
  Parser(std::string const& file_name);
  ~Parser();

  std::ifstream& file();
  bool get_next(std::string& line);
  uint32_t current_line();

private:
  // input file
  std::string file_name_;
  std::ifstream file_;

  // the current line number
  uint32_t line_ {0};

  // <line_number, line_pos>
  std::map<size_t, uint32_t> lines_;

}; // class Parser

} // namespace OB

#endif // OB_PARSER_HH
