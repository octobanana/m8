#ifndef OB_TERM_HH
#define OB_TERM_HH

#include <unistd.h>
#include <sys/ioctl.h>

#include <cstdio>
#include <cstddef>
#include <cstdint>

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <regex>

namespace OB::Term
{

inline bool is_term(int fd)
{
  switch (fd)
  {
    case STDIN_FILENO: return isatty(STDIN_FILENO);
    case STDOUT_FILENO: return isatty(STDOUT_FILENO);
    case STDERR_FILENO: return isatty(STDERR_FILENO);
    default: return false;
  }
}

inline std::size_t width()
{
  winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  return w.ws_col;
}

inline std::size_t height()
{
  winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  return w.ws_row;
}

inline void size(std::size_t &width, std::size_t &height)
{
  winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  width = w.ws_col;
  height = w.ws_row;
}

} // namespace OB::Term

namespace OB::Term::ANSI_Escape_Codes
{

// standard escaped characters
std::string const nl {"\n"};
std::string const cr {"\r"};
std::string const tab {"\t"};
std::string const alert {"\a"};
std::string const backspace {"\b"};
std::string const backslash {"\\"};

// escape code sequence
std::string const esc {"\x1b"};

// clears all attributes
std::string const reset {esc + "[0m"};

// style
std::string const bold {esc + "[1m"};
std::string const dim {esc + "[2m"};
std::string const italic {esc + "[3m"};
std::string const underline {esc + "[4m"};
std::string const blink {esc + "[5m"};
std::string const rblink {esc + "[6m"};
std::string const reverse {esc + "[7m"};
std::string const conceal {esc + "[8m"};
std::string const cross {esc + "[9m"};

// erasing
std::string const erase_end {esc + "[K"};
std::string const erase_start {esc + "[1K"};
std::string const erase_line {esc + "[2K"};
std::string const erase_down {esc + "[J"};
std::string const erase_up {esc + "[1J"};
std::string const erase_screen {esc + "[2J"};

// cursor visibility
std::string const cursor_hide {esc + "[?25l"};
std::string const cursor_show {esc + "[?25h"};

// cursor movement
std::string const cursor_home {esc + "[H"};
std::string const cursor_up {esc + "[1A"};
std::string const cursor_down {esc + "[1B"};
std::string const cursor_right {esc + "[1C"};
std::string const cursor_left {esc + "[1D"};

// cursor position
std::string const cursor_save {esc + "7"};
std::string const cursor_load {esc + "8"};

// foreground color
std::string const fg_black {esc + "[30m"};
std::string const fg_red {esc + "[31m"};
std::string const fg_green {esc + "[32m"};
std::string const fg_yellow {esc + "[33m"};
std::string const fg_blue {esc + "[34m"};
std::string const fg_magenta {esc + "[35m"};
std::string const fg_cyan {esc + "[36m"};
std::string const fg_white {esc + "[37m"};

// background color
std::string const bg_black {esc + "[40m"};
std::string const bg_red {esc + "[41m"};
std::string const bg_green {esc + "[42m"};
std::string const bg_yellow {esc + "[43m"};
std::string const bg_blue {esc + "[44m"};
std::string const bg_magenta {esc + "[45m"};
std::string const bg_cyan {esc + "[46m"};
std::string const bg_white {esc + "[47m"};

inline std::string fg_256(std::string const& str)
{
  auto const n = std::stoi(str);
  if (n < 0 || n > 256) return {};
  std::stringstream ss;
  ss << esc << "38;5;" << str << "m";

  return ss.str();
}

inline std::string bg_256(std::string const& str)
{
  auto const n = std::stoi(str);
  if (n < 0 || n > 256) return {};
  std::stringstream ss;
  ss << esc << "48;5;" << str << "m";

  return ss.str();
}

inline std::string htoi(std::string const& str)
{
  std::stringstream ss;
  ss << str;
  unsigned int n;
  ss >> std::hex >> n;

  return std::to_string(n);
}

inline bool valid_hstr(std::string& str)
{
  std::smatch m;
  std::regex rx {"^#?((?:[0-9a-fA-F]{3}){1,2})$"};

  if (std::regex_match(str, m, rx))
  {
    std::string hstr {m[1]};

    if (hstr.size() == 3)
    {
      std::stringstream ss;
      ss << hstr[0] << hstr[0] << hstr[1] << hstr[1] << hstr[2] << hstr[2];
      hstr = ss.str();
    }

    str = hstr;

    return true;
  }

  return false;
}

inline std::string fg_true(std::string str)
{
  if (! valid_hstr(str))
  {
    return {};
  }

  std::string const h1 {str.substr(0, 2)};
  std::string const h2 {str.substr(2, 2)};
  std::string const h3 {str.substr(4, 2)};

  std::stringstream ss; ss
  << esc << "38;2;"
  << htoi(h1) << ";"
  << htoi(h2) << ";"
  << htoi(h3) << "m";

  return ss.str();
}

inline std::string bg_true(std::string str)
{
  if (! valid_hstr(str))
  {
    return {};
  }

  std::string const h1 {str.substr(0, 2)};
  std::string const h2 {str.substr(2, 2)};
  std::string const h3 {str.substr(4, 2)};

  std::stringstream ss; ss
  << esc << "48;2;"
  << htoi(h1) << ";"
  << htoi(h2) << ";"
  << htoi(h3) << "m";

  return ss.str();
}

inline std::string cursor_set(std::size_t width, std::size_t height)
{
  std::stringstream ss;
  ss << esc << height << ";" << width << "H";

  return ss.str();
}

inline int cursor_get(std::size_t& width, std::size_t& height)
{
  std::cout << (esc + "[6n") << std::flush;

  char buf[32];
  std::uint8_t i {0};

  for (;i < sizeof(buf) -1; ++i)
  {
    while (read(STDIN_FILENO, &buf[i], 1) != 1)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (buf[i] == 'R')
    {
      break;
    }
  }

  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
  {
    return -1;
  }

  int x;
  int y;
  if (std::sscanf(&buf[2], "%d;%d", &y, &x) != 2)
  {
    return -1;
  }

  width = static_cast<std::size_t>(x);
  height = static_cast<std::size_t>(y);

  return 0;
}

template<class T>
std::string wrap(T const val, std::string const col, bool color = true)
{
  std::stringstream ss;

  if (color)
  {
    ss
    << col
    << val
    << reset;
  }
  else
  {
    ss << val;
  }

  return ss.str();
}

template<class T>
std::string wrap(T const val, std::vector<std::string> const col, bool color = true)
{
  std::stringstream ss;

  if (color)
  {
    for (auto const& e : col)
    {
      ss << e;
    }
    ss << val << reset;
  }
  else
  {
    ss << val << reset;
  }

  return ss.str();
}

} // namespace OB::Term::ANSI_Escape_Codes

#endif // OB_TERM_HH
