#include "ast.hh"

#include "color.hh"
namespace Cl = OB::Color;

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <functional>

namespace OB
{

Ast::Ast()
{
}

Ast::~Ast()
{
}

std::string Ast::str()
{
  std::stringstream ss;
  ss << "(\n";
  for (auto const& e : ast)
  {
    ss << recurse_tmacro(e, 1);
  }
  ss << ")\n";

  return ss.str();
}

std::string Ast::recurse_tmacro(Tmacro const& t, size_t depth)
{
  std::string indent_base {Cl::fg_blue + ". " + Cl::reset};
  std::string indent;
  for (size_t i = 0; i < depth; ++i)
  {
    indent += indent_base;
  }

  std::stringstream ss; ss
  << indent << "(\n"
  << indent << indent_base << Cl::fg_magenta << "s-line   : " << Cl::fg_green << t.line_start << "\n" << Cl::reset
  << indent << indent_base << Cl::fg_magenta << "begin    : " << Cl::fg_green << t.begin << "\n" << Cl::reset
  << indent << indent_base << Cl::fg_magenta << "e-line   : " << Cl::fg_green << t.line_end << "\n" << Cl::reset
  << indent << indent_base << Cl::fg_magenta << "end      : " << Cl::fg_green << t.end << "\n" << Cl::reset
  << indent << indent_base << Cl::fg_magenta << "str      : " << Cl::fg_green << t.str << "\n" << Cl::reset
  << indent << indent_base << Cl::fg_magenta << "name     : " << Cl::fg_green << t.name << "\n" << Cl::reset
  << indent << indent_base << Cl::fg_magenta << "args     : " << Cl::fg_green << t.args << "\n" << Cl::reset;

  // ss << indent << indent_base << Cl::fg_magenta << "match    : " << Cl::fg_green;

  // if (t.match.size() > 1)
  // {
  //   for (size_t i = 0; i < t.match.size() - 1; ++i)
  //   {
  //     ss << "[" << t.match.at(i) << "]" << Cl::fg_magenta << ", " << Cl::fg_green;
  //   }
  //   ss << "[" << t.match.back() << "]";
  // }
  // else if (t.match.size() == 1)
  // {
  //   ss << "[" << t.match.back() << "]";
  // }
  // ss << "\n" << Cl::reset;

  if (t.children.empty())
  {
    ss << indent << indent_base << Cl::fg_magenta << "children : " << Cl::fg_green << "0\n" << Cl::reset;
  }
  else
  {
    ss << indent << indent_base << Cl::fg_magenta << "children : " << Cl::fg_green << t.children.size() << "\n" << Cl::reset;
    for (auto const& c : t.children)
    {
      ss << recurse_tmacro(c, depth + 1);
    }
  }

  ss << indent << ")\n";

  return ss.str();
}

} // namespace OB
