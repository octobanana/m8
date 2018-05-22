#ifndef OB_WRITER_HH
#define OB_WRITER_HH

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

namespace OB
{

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

} // namespace OB

#endif // OB_WRITER_HH
