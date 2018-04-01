#include "reader.hh"

#include "ansi_escape_codes.hh"
namespace AEC = OB::ANSI_Escape_Codes;

#include "linenoise.hh"

// #include <replxx.hxx>
// using Replxx = replxx::Replxx;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cctype>

namespace OB
{

// prototypes
// Replxx::completions_t hook_completion(std::string const& prefix, int, void* ud);
// Replxx::hints_t hook_hint(std::string const& prefix, int, Replxx::Color&, void* ud);
// void hook_color(std::string const& str, Replxx::colors_t& colors, void*);

// Replxx::completions_t hook_completion(std::string const& prefix, int, void* ud)
// {
//   auto* examples = static_cast<std::vector<std::string>*>(ud);
//   Replxx::completions_t completions;

//   for (auto const& e : *examples)
//   {
//     if (e.compare(0, prefix.size(), prefix) == 0)
//     {
//       completions.emplace_back(e.c_str());
//     }
//   }

//   return completions;
// }

// Replxx::hints_t hook_hint(std::string const& prefix, int, Replxx::Color&, void* ud)
// {
//   auto* examples = static_cast<std::vector<std::string>*>(ud);
//   Replxx::hints_t hints;

//   // only show hint after 'n' numbers of chars received
//   // or if prefix begins with a specific character
//   if (prefix.size() >= 2 || (! prefix.empty() && prefix.at(0) == '.'))
//   {
//     for (auto const& e : *examples)
//     {
//       if (e.compare(0, prefix.size(), prefix) == 0)
//       {
//         hints.emplace_back(e.c_str() + prefix.length());
//       }
//     }
//   }

//   return hints;
// }

// void hook_color(std::string const& str, Replxx::colors_t& colors, void*)
// {
//   for (size_t i = 0; i < str.size(); ++i)
//   {
//     if (std::isdigit(str.at(i)))
//     {
//       colors.at(i) = Replxx::Color::BRIGHTMAGENTA;
//     }
//     else if (str.at(i) == '(' || str.at(i) == ')')
//     {
//       colors.at(i) = Replxx::Color::BRIGHTCYAN;
//     }
//     else if (str.at(i) == '"')
//     {
//       colors.at(i) = Replxx::Color::GREEN;
//     }
//   }
// }

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

  // rx_.install_window_change_handler();
  // rx_.history_load(rx_history_);
  // rx_.set_highlighter_callback(hook_color, nullptr);
  // rx_.set_completion_callback(hook_completion, static_cast<void*>(&examples));
  // rx_.set_hint_callback(hook_hint, static_cast<void*>(&examples));

  // buf_.reserve(1024);
}

Reader::~Reader()
{
  if (readline_)
  {
    linenoise::SaveHistory(history_.c_str());
  }

  // rx_.history_save(rx_history_);
}

void Reader::open_file(std::string file)
{
  ifile_.open(file);
  if (! ifile_.is_open())
  {
    throw std::runtime_error("could not open the input file");
  }
  lines_[0] = 0;
  // read();
  readline_ = false;
}

// bool Reader::read()
// {
//   pnt_ = 0;

//   if (ifile_.read(buf_.data(), buf_.size()))
//   {
//     buf_len_ = ifile_.gcount();
//     return true;
//   }

//   return false;
// }

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

    // rx_prompt_ = AEC::fg_magenta + "M8>" + AEC::reset + " ";
    // char const* input = rx_.input(rx_prompt_);
    // if (input == nullptr)
    // {
    //   std::cout << "\n";
    //   status = false;
    // }
    // else
    // {
    //   rx_.history_add(input);
    //   str = input;
    //   status = true;
    // }
    // return status;

    // std::string prompt {AEC::fg_magenta + "M8>" + AEC::reset + " "};
    // std::cout << prompt;
    // if (std::getline(std::cin, str))
    // {
    //   status = true;
    // }
    // else
    // {
    //   status = false;
    // }
    // return status;
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

  // if (pnt_ == buf_len_ - 1)
  // {
  //   read();
  // }

  // if (buf_len_ == 0)
  // {
  //   return false;
  // }

  // if (readline_)
  // {
  //   // TODO
  // }
  // else
  // {
  //   str = buf_.at(pnt_);
  //   ++pnt_;
  //   return true;
  // }
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
