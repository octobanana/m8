#include "m8/ast.hh"

#include "ob/term.hh"
namespace aec = OB::Term::ANSI_Escape_Codes;

#include <cstdint>
#include <cstddef>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <functional>

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

void Ast::clear()
{
  ast = {};
}

std::string Ast::recurse_tmacro(Tmacro const& t, std::size_t depth)
{
  std::string indent_base {aec::fg_blue + ". " + aec::reset};
  std::string indent;
  for (std::size_t i = 0; i < depth; ++i)
  {
    indent += indent_base;
  }

  std::stringstream ss; ss
  << indent << "(\n"
  << indent << indent_base << aec::fg_magenta << "s-line   : " << aec::fg_green << t.line_start << "\n" << aec::reset
  << indent << indent_base << aec::fg_magenta << "begin    : " << aec::fg_green << t.begin << "\n" << aec::reset
  << indent << indent_base << aec::fg_magenta << "e-line   : " << aec::fg_green << t.line_end << "\n" << aec::reset
  << indent << indent_base << aec::fg_magenta << "end      : " << aec::fg_green << t.end << "\n" << aec::reset
  << indent << indent_base << aec::fg_magenta << "str      : " << aec::fg_green << t.str << "\n" << aec::reset
  << indent << indent_base << aec::fg_magenta << "name     : " << aec::fg_green << t.name << "\n" << aec::reset
  << indent << indent_base << aec::fg_magenta << "args     : " << aec::fg_green << t.args << "\n" << aec::reset;

  ss << indent << indent_base << aec::fg_magenta << "match    : " << aec::fg_green;

  if (t.match.size() > 1)
  {
    for (std::size_t i = 0; i < t.match.size() - 1; ++i)
    {
      ss << "[" << t.match.at(i) << "]" << aec::fg_magenta << ", " << aec::fg_green;
    }
    ss << "[" << t.match.back() << "]";
  }
  else if (t.match.size() == 1)
  {
    ss << "[" << t.match.back() << "]";
  }
  ss << "\n" << aec::reset;

  if (t.children.empty())
  {
    ss << indent << indent_base << aec::fg_magenta << "children : " << aec::fg_green << "0\n" << aec::reset;
  }
  else
  {
    ss << indent << indent_base << aec::fg_magenta << "children : " << aec::fg_green << t.children.size() << "\n" << aec::reset;
    for (auto const& c : t.children)
    {
      ss << recurse_tmacro(c, depth + 1);
    }
  }

  ss << indent << ")\n";

  return ss.str();
}
