#ifndef OB_M8_HH
#define OB_M8_HH

#include "ast.hh"
using Tmacro = OB::Tmacro;
using Ast = OB::Ast;

#include "cache.hh"
using Cache = OB::Cache;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <regex>
#include <functional>
#include <utility>

class M8
{
public:
  M8();
  ~M8();

  using Args = std::vector<std::string>;

  struct Ctx
  {
    std::string& str;
    Args const& args;
    Cache& cache;
    size_t indent {0};
  }; // struct Ctx

  using macro_fn = std::function<int(Ctx& ctx)>;
  // using macro_fn = std::function<int(std::string&, Args const&)>;

  // set external macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex);

  // set remote macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex, std::string const& url);

  // set internal macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex, macro_fn fn);

  void set_debug(bool val);
  void set_copy(bool val);
  void set_config(std::string file_name);
  void set_delimits(std::string const& delim_start, std::string const& delim_end);
  void set_readline(bool val);

  std::string summary() const;
  std::string list_macros() const;
  std::string macro_info(std::string const& name) const;

  void parse(std::string const& _ifile, std::string const& _ofile);

private:
  struct Stats
  {
    // general stats
    int macro {0};
    int warning {0};
    int pass {0};

    // macro stats
    int internal {0};
    int external {0};
    int remote {0};
  }; // struct Stats
  Stats stats;

  bool readline_ {false};
  bool use_stdout_ {false};
  bool debug_ {false};
  bool copy_ {false};
  bool summary_ {false};

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
    macro_fn func;
  }; // struct Macro
  std::map<std::string, Macro> macros;

  // abstract syntax tree
  Ast ast_;

  int run_internal(Macro const& macro, Ctx& ctx);
  int run_external(Macro const& macro, Ctx& ctx);
  int run_remote(Macro const& macro, Ctx& ctx);

  std::string env_var(std::string const& var) const;
  std::vector<std::string> suggest_macro(std::string const& name) const;

}; // class M8

#endif // OB_M8_HH
