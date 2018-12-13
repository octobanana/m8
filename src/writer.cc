#include "writer.hh"
#include "crypto.hh"

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <filesystem>
namespace fs = std::filesystem;

namespace OB
{

Writer::Writer()
{
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

void Writer::open(std::string const& file_name)
{
  fs::create_directories(".m8/swp");
  file_name_ = file_name;
  fs::path fp {file_name_};
  file_tmp_ = ".m8/swp/" + Crypto::sha256(fp) + file_ext_;
  fs::path p {file_tmp_};
  file_.open(p, std::ios::app);
  if (! file_.is_open())
  {
    throw std::runtime_error("could not open the output file");
  }
}

void Writer::write(std::string const& str)
{
  if (file_name_.empty())
  {
    std::cout << str;
    if (! str.empty() && str.back() != '\n')
    {
      std::cout << AEC::wrap("%\n", AEC::reverse) << std::flush;
    }
  }
  else
  {
    file_ << str;
  }
}

void Writer::flush()
{
  if (file_name_.empty())
  {
    std::cout << std::flush;
  }
  else
  {
    file_ << std::flush;
  }
}

void Writer::close()
{
  if (file_.is_open())
  {
    file_.close();
  }
}

} // namespace OB
