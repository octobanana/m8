#include "sys_command.hh"

#include <string>
#include <array>
#include <memory>

int exec(std::string& result, std::string const& command)
{
  const char* cmd {command.c_str()};

  int const buf_len {1024};
  std::array<char, buf_len> buf;

  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);

  if (!pipe)
  {
    return -1;
  }

  while (!feof(pipe.get()))
  {
    if (fgets(buf.data(), buf_len, pipe.get()) != nullptr)
    {
      result += buf.data();
    }
  }

  return 0;
}
