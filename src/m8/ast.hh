#ifndef M8_AST_HH
#define M8_AST_HH

#include <cstdint>
#include <cstddef>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

struct Tmacro
{
  std::uint32_t line_start {0};
  std::uint32_t line_end {0};
  std::size_t begin {0};
  std::size_t end {0};
  std::string str;
  std::string name;
  std::string args;
  std::vector<Tmacro> children;
  std::vector<std::string> match;
  std::size_t fn_index {0};
  std::string res;
}; // struct Tmacro

class Ast
{
public:

  Ast();
  ~Ast();

  std::string str();
  void clear();

  std::vector<Tmacro> ast;

private:

  std::string recurse_tmacro(Tmacro const& t, std::size_t depth);
}; // class Ast

#endif // M8_AST_HH
