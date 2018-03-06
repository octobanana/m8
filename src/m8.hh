#ifndef OB_M8_HH
#define OB_M8_HH

#include "ast.hh"
using Tmacro = OB::Tmacro;
using Ast = OB::Ast;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <regex>
#include <functional>

class M8
{
  using macro_fn = std::function<int(std::string&, std::smatch const&)>;

public:
  M8();
  ~M8();

  // set external macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex);

  // set remote macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex, std::string const& url);

  // set internal macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex, macro_fn fn);

  void set_debug(bool const& val);
  void set_config(std::string file_name);
  void set_delimits(std::string const& delim_start, std::string const& delim_end);

  std::string summary() const;
  std::string list_macros() const;
  std::string macro_info(std::string const& name) const;

  void parse(std::string const& _ifile, std::string const& _ofile);

  // void run_(std::string const& ifile, std::string const& ofile);

private:
  // general stats
  int macro_count_ {0};
  int warning_count_ {0};
  int pass_count_ {0};

  // macro stats
  int internal_count_ {0};
  int external_count_ {0};
  int remote_count_ {0};

  bool use_stdout_ {false};
  bool debug_ {false};

  std::string delim_start_ {"[M8["};
  std::string delim_end_ {"]8M]"};
  size_t len_start {delim_start_.length()};
  size_t len_end {delim_end_.length()};
  size_t len_total {len_start + len_end};

  enum class Mtype
  {
    internal,
    external,
    remote
  };

  struct Macro
  {
    Mtype type;
    std::string name;
    std::string info;
    std::string usage;
    std::string regex;
    std::string url;
    macro_fn fn;
  }; // struct Macro
  std::map<std::string, Macro> macros;

  // abstract syntax tree
  Ast ast_;

  int run_internal(Macro macro, std::string& res, std::smatch const match, macro_fn fn);
  int run_external(Macro macro, std::string& res, std::smatch const match, macro_fn fn);
  int run_remote(Macro macro, std::string& res, std::smatch const match, macro_fn fn);

  std::string env_var(std::string const& var) const;
  std::vector<std::string> suggest_macro(std::string const& name) const;

}; // class M8

#endif // OB_M8_HH
