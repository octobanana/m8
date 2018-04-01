#ifndef OB_READER_HH
#define OB_READER_HH

// #include <replxx.hxx>
// using Replxx = replxx::Replxx;

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

  void open_file(std::string file);
  // bool read();
  bool next(std::string& str);
  uint32_t row();
  uint32_t col();

private:
  bool readline_ {true};

  // replxx
  // Replxx rx_;
  std::string history_ {".m8/history.m8"};
  std::string prompt_;
  std::vector<std::string> examples {"floor", "find", "read", "round", "print!"};

  // input file stream
  std::ifstream ifile_;

  // input buffer
  // std::vector<char> buf_;

  // input buffer length
  // size_t buf_len_ {0};

  // current byte index into input buffer
  // size_t pnt_ {0};

  // current row number
  uint32_t row_ {0};
  // current column number
  uint32_t col_ {0};

  // stores file position to line number
  std::map<size_t, uint32_t> lines_;

  inline size_t str_count(std::string const& str, std::string const& s);

}; // class Reader

} // namespace OB

#endif // OB_READER_HH
