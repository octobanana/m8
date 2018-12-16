#ifndef M8_READER_HH
#define M8_READER_HH

#include <cstdint>
#include <cstddef>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

class Reader
{
public:

  Reader();
  ~Reader();

  void open(std::string const& file_name);
  bool next(std::string& str);
  std::uint32_t row();
  std::uint32_t col();
  std::string line();

private:

  bool readline_ {true};

  std::string history_ {"~/.m8-history"};
  std::string prompt_;
  std::vector<std::string> examples {"floor", "find", "read", "round", "print!"};

  // input file stream
  std::ifstream ifile_;

  // current row number
  std::uint32_t row_ {0};
  // current column number
  std::uint32_t col_ {0};
  // current line
  std::string line_;

  // stores file position to line number
  std::map<std::size_t, std::uint32_t> lines_;
}; // class Reader

#endif // M8_READER_HH
