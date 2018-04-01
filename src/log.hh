//
// Minimal Logging Class
//
// Author:
// Octobanana (Brett Robinson)
// octo.banana93@gmail.com
//
// License:
// MIT License
//
// Usage:
// appends data to selected log file
// OB::Log(Log_Level log_level, std::string file_name) << msg << "hi" << 20;
// clears the selected log file
// OB::Log::clear(std::string file_name);
//
// Example:
// #include "log.hh"
// #define LOG_INFO "info.log"
// #define LOG_DBUG "debug.log"
// int main(int argc, char *argv[])
// {
//   OB::Log::clear(LOG_INFO);
//   OB::Log::clear(LOG_DBUG);
//   OB::Log(OB::Log::INFO, LOG_INFO) << "Begin Info Log";
//   OB::Log(OB::Log::DBUG, LOG_DBUG) << "Begin Debug Log";
//   return 0;
// }
//
// File Output:
// Year.Month.Day Hour:Min:Sec [Log_Level] <msg here>
// 17.05.10 12:08:39 [INFO] Begin Info Log
// 17.05.10 12:08:39 [DBUG] Begin Debug Log
//

#ifndef LOG_HH
#define LOG_HH

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace OB
{
class Log
{
public:

  enum Log_Level
  {
    INFO = 0,
    DBUG,
    WARN,
    EROR
  };

  struct Log_Type
  {
    std::string info {"\x1b[1;32m" "INFO" "\x1b[0m"}; // green
    std::string dbug {"\x1b[1;35m" "DBUG" "\x1b[0m"}; // purple
    std::string warn {"\x1b[1;33m" "WARN" "\x1b[0m"}; // yellow
    std::string eror {"\x1b[0;31m" "EROR" "\x1b[0m"}; // red
  } log_type;

  Log(Log_Level log_level, std::string file_name)
  {
    set_file(file_name);
    std::time_t t = std::time(0);
    std::tm tm = *std::localtime(&t);
    operator << (std::put_time(&tm, "%y.%m.%d %H:%M:%S"));
    operator << (" [" + get_log_level(log_level) + "] ");
  }

  ~Log()
  {
    std::ofstream ofile {m_file, std::ios::app};
    if (ofile.is_open())
    {
      ofile << m_buffer.str();
      ofile << '\n';
      ofile.close();
    }
  }

  template<class T>
  Log& operator << (const T &msg)
  {
    m_buffer << msg;
    return *this;
  }

  static void clear(std::string file_name)
  {
    std::ofstream ofile {file_name, std::ios::out};
    if (ofile.is_open())
    {
      ofile << "";
      ofile.close();
    }
  }

private:

  void set_file(std::string file_name)
  {
    m_file = file_name;
  }

  std::string get_log_level(Log_Level log_level)
  {
    if (log_level == INFO)
      return log_type.info;
    else if (log_level == DBUG)
      return log_type.dbug;
    else if (log_level == WARN)
      return log_type.warn;
    else if (log_level == EROR)
      return log_type.eror;
    else
      return "    ";
  }

  std::ostringstream m_buffer;
  std::string m_file;
}; // class Log
} // namespace OB

#endif // OB_TERM_HH
