#include "m8/macros_custom.hh"

#include "m8/m8.hh"

#include <string>
#include <sstream>
#include <iostream>

#include <filesystem>
namespace fs = std::filesystem;

namespace Macros
{

void macros_custom(M8& m8)
{
  // define macros

  // single regex and func
  // M8::set_macro(name, info, usage, regex, func)

  // m8.set_macro("",
  //   "",
  //   "",
  //   "^(.*)$",
  //   [&](auto& ctx) {
  //   return 0;
  //   }
  // );

  // overloaded regex and func
  // M8::set_macro(name, info, usage, {{regex, func}, {regex, func}})

  // m8.set_macro("",
  //   "",
  //   "",
  //   {
  //     {"",
  //       [&](auto& ctx) {
  //       return 0;
  //       }
  //     },
  //     {"^(.+)$",
  //       [&](auto& ctx) {
  //       return 0;
  //       }
  //     },
  //   }
  // );
}

} // namespace Macros
