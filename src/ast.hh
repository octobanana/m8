#ifndef OB_AST_HH
#define OB_AST_HH

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

namespace OB
{

struct Tmacro
{
  uint32_t line_start {0};
  uint32_t line_end {0};
  size_t begin {0};
  size_t end {0};
  std::string str;
  std::string name;
  std::string args;
  std::vector<Tmacro> children;
  std::vector<std::string> match;
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
  std::string recurse_tmacro(Tmacro const& t, size_t depth);

}; // class Ast

} // namespace OB

#endif // OB_AST_HH
