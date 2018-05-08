#include "writer.hh"
#include "crypto.hh"

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
  fs::create_directories(".m8/swp");
}

Writer::~Writer()
{
  // if (fs::path(".m8/swp").empty())
  // {
  //   fs::remove_all(".m8/swp");
  // }

  // if (fs::path(".m8").empty())
  // {
  //   fs::remove_all(".m8");
  // }
}

void Writer::open()
{
  fs::path fp {file_name_};
  file_tmp_ = ".m8/swp/" + Crypto::sha256(fp) + file_ext_;
  fs::path p {file_tmp_};
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
  fs::path p1 {file_tmp_};
  fs::path p2 {file_name_};
  if (! p2.parent_path().empty())
  {
    fs::create_directories(p2.parent_path());
  }
  fs::rename(p1, p2);
}

} // namespace OB
