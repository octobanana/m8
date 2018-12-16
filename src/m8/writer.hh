#ifndef M8_WRITER_HH
#define M8_WRITER_HH

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

class Writer
{
public:

  Writer();
  ~Writer();

  void open(std::string const& file_name);
  void write(std::string const& str);
  void close();
  void flush();

private:

  std::string file_ext_ {".swp.m8"};
  std::string file_name_;
  std::string file_tmp_;
  std::ofstream file_;
}; // class Writer

#endif // M8_WRITER_HH
