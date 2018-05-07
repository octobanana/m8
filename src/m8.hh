#ifndef OB_M8_HH
#define OB_M8_HH

#include "ast.hh"
using Tmacro = OB::Tmacro;
using Ast = OB::Ast;

// #include "cache.hh"
// using Cache = OB::Cache;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <regex>
#include <functional>
#include <utility>
#include <deque>
#include <optional>

int const DEBUG {0};
#define debug(x) if (DEBUG) std::cerr << AEC::wrap(x, AEC::fg_true("f50")) << " ";
#define debug_nl if (DEBUG) std::cerr << "\n";
#define debugf(x) debug_format(#x, x)
#define debug_format(x, y) if (DEBUG) std::cerr << AEC::wrap(x, AEC::fg_true("f50")) << ": " << AEC::wrap(y, AEC::fg_green) << "\n";

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
    std::string file;
    uint32_t line {0};
    std::string err_msg;
    // Cache& cache;
  }; // struct Ctx

  using macro_fn = std::function<int(Ctx& ctx)>;

  // set external macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string regex);

  // set remote macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string regex, std::string const& url);

  // set internal macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string regex, macro_fn func);

  // set internal macro
  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::vector<std::pair<std::string, macro_fn>> rx_fn);

  // plain macro hooks
  enum class Htype
  {
    begin,
    macro,
    res,
    end
  };
  struct Hook
  {
    std::string key;
    std::string val;
  };
  using Hooks = std::deque<Hook>;
  void set_hook(Htype t, Hook h);
  std::optional<Hooks> get_hooks(Htype t) const;
  void rm_hook(Htype t, std::string key);
  void run_hooks(Hooks const& h, std::string& s);

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
    std::map<std::string, uint32_t> count;
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

  std::unordered_map<std::string, std::string> rx_grammar {
    {"b", "^"},
    {"e", "$"},
    {"ws", "\\s+"},
    {"!all", "([^\\r]+?)"},
    {"!int", "([0-9]+)"},
    {"!dec", "([0-9]+\.[0-9]+)"},
    {"!str_s", "'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)'"},
    {"!str_d", "\"([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)\""},
  };

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
    std::vector<std::pair<std::string, macro_fn>> rx_fn;
    std::string url;
  }; // struct Macro
  std::map<std::string, Macro> macros;

  // abstract syntax tree
  Ast ast_;

  // hooks
  Hooks h_begin;
  Hooks h_macro;
  Hooks h_res;
  Hooks h_end;

  int run_internal(macro_fn const& func, Ctx& ctx);
  int run_external(Macro const& macro, Ctx& ctx);
  int run_remote(Macro const& macro, Ctx& ctx);

  std::string env_var(std::string const& var) const;
  std::vector<std::string> suggest_macro(std::string const& name) const;

}; // class M8

#endif // OB_M8_HH
