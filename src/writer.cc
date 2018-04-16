#include "writer.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

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
  fs::create_directory("./.m8/swp/");
  fs::path fp {file_name_};
  fs::path p {"./.m8/swp/" + std::string(fp.filename()) + file_ext_};
  file_.open(p);
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
  fs::path fp {file_name_};
  fs::path p1 {"./.m8/swp/" + std::string(fp.filename()) + file_ext_};
  fs::path p2 {file_name_};
  fs::rename(p1, p2);
}

} // namespace OB
