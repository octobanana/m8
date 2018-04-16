#include "reader.hh"

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include "linenoise.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cctype>

namespace OB
{

Reader::Reader()
{
  if (readline_)
  {
    linenoise::LoadHistory(history_.c_str());
    // linenoise::SetMultiLine(true);
    linenoise::SetHistoryMaxLen(10);
    linenoise::SetCompletionCallback([](const char* editBuffer, std::vector<std::string>& completions) {
      if (editBuffer[0] == 'a')
      {
        completions.push_back("all");
        completions.push_back("alli");
        completions.push_back("alligator");
      }
    });
  }
}

Reader::~Reader()
{
  if (readline_)
  {
    linenoise::SaveHistory(history_.c_str());
  }
}

void Reader::open_file(std::string file)
{
  ifile_.open(file);
  if (! ifile_.is_open())
  {
    throw std::runtime_error("could not open the input file");
  }
  lines_[0] = 0;
  readline_ = false;
}

bool Reader::next(std::string& str)
{
  ++row_;
  lines_[row_] = ifile_.tellg();
  bool status {false};
  if (readline_)
  {
    bool quit {false};
    prompt_ = AEC::fg_magenta + "M8>" + AEC::reset + " ";
    std::string input = linenoise::Readline(prompt_.c_str(), quit);
    if (input == ".quit" || input == ".quit" || quit)
    {
      std::cout << "\n";
      status = false;
    }
    else
    {
      linenoise::AddHistory(input.c_str());
      str = input;
      status = true;
    }
    return status;
  }
  else
  {
    if (std::getline(ifile_, str))
    {
      status = true;
    }
    else
    {
      status = false;
    }
    return status;
  }
}

uint32_t Reader::row()
{
  return row_;
}

uint32_t Reader::col()
{
  return col_;
}

size_t Reader::str_count(std::string const& str, std::string const& s)
{
  size_t count {0};
  size_t pos {0};

  for (;;)
  {
    pos = str.find(s, pos);
    if (pos == std::string::npos) break;
    ++count;
    ++pos;
  }
  return count;
}

} // namespace OB
