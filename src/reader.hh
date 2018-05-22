#ifndef OB_READER_HH
#define OB_READER_HH

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

namespace OB
{

class Reader
{
public:
  Reader();
  ~Reader();

  void open(std::string const& file_name);
  bool next(std::string& str);
  uint32_t row();
  uint32_t col();
  std::string line();

private:
  bool readline_ {true};

  std::string history_ {"~/.m8-history"};
  std::string prompt_;
  std::vector<std::string> examples {"floor", "find", "read", "round", "print!"};

  // input file stream
  std::ifstream ifile_;

  // current row number
  uint32_t row_ {0};
  // current column number
  uint32_t col_ {0};
  // current line
  std::string line_;

  // stores file position to line number
  std::map<size_t, uint32_t> lines_;

  inline size_t str_count(std::string const& str, std::string const& s);

}; // class Reader

} // namespace OB

#endif // OB_READER_HH
