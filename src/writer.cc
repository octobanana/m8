#include "writer.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <bitset>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace OB
{

Writer::Writer(std::string file_name):
  file_name_ {file_name}
{
}

Writer::~Writer()
{
}

void Writer::open()
{
  file_.open(file_name_ + file_ext_);
  if (! file_.is_open())
  {
    throw std::runtime_error("could not open the output file");
  }
}

void Writer::write(std::string const& str)
{
  file_ << str;
}

void Writer::close()
{
  if (file_.is_open())
  {
    file_.close();
  }
  fs::rename(file_name_ + file_ext_, file_name_);
}

} // namespace OB
