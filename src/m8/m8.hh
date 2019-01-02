#ifndef M8_HH
#define M8_HH

#include "ob/ordered_map.hh"
#include "ob/scoped_map.hh"

#include "m8/ast.hh"
#include "m8/reader.hh"
#include "m8/writer.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <regex>
#include <functional>
#include <utility>
#include <deque>
#include <optional>

class M8
{
  struct Core_Ctx
  {
    Core_Ctx(std::string& buf_, Reader& r_, Writer& w_, std::string ifile_, std::string ofile_):
      buf {buf_},
      r {r_},
      w {w_},
      ifile {ifile_},
      ofile {ofile_}
    {
    }
    std::string& buf;
    Reader& r;
    Writer& w;
    std::string ifile;
    std::string ofile;
  }; // struct Core_Ctx

  using Args = std::vector<std::string>;

  struct Ctx
  {
    std::string& str;
    Args const& args;
    std::string err_msg;
    std::unique_ptr<Core_Ctx> core;
    // Cache& cache;
  }; // struct Ctx

public:

  using macro_fn = std::function<int(Ctx& ctx)>;

  struct macro_t
  {
    macro_t(std::string const& usage_, std::string const& regex_, macro_fn const& func_) :
      usage {usage_},
      regex {regex_},
      func {func_}
    {
    }

    std::string usage;
    std::string regex;
    macro_fn func;
  };

private:

  enum class Mtype
  {
    core,
    internal,
    external,
    remote
  };

  struct Macro
  {
    Mtype type;
    std::string name;
    std::string info;
    // std::string usage;
    // std::vector<std::pair<std::string, macro_fn>> rx_fn;
    std::vector<macro_t> impl;
    std::string url;
  }; // struct Macro
  OB::Scoped_Map<std::string, Macro> macros_;

public:

  M8();
  ~M8();

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
  void set_macro(std::string const& name, std::string const& info, std::vector<M8::macro_t> impl);

  // unset internal macro
  void unset_macro(std::string const& name);

  // unset internal macro
  void unset_macro(std::string const& name, std::string regex);

  // set core macro
  void set_core(std::string const& name, std::string const& info,
    std::string const& usage, std::string regex, macro_fn func);

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

  void set_debug(bool val);
  void set_comment(std::string str);
  void set_ignore(std::string str);
  void set_copy(bool val);
  void set_config(std::string file_name);
  void set_delimits(std::string const& delim_start, std::string const& delim_end);
  void set_readline(bool val);

  std::string summary() const;
  std::string list_macros() const;
  std::string macro_info(std::string const& name) const;

  void parse(std::string const& _ifile = {}, std::string const& _ofile = {});

private:

  enum class error_t
  {
    missing_opening_delimiter,
    missing_closing_delimiter,
    invalid_format,
    undefined_name,
    invalid_arg,
    failed,
  };

  std::string error(error_t type, Tmacro const& macro, std::string const& ifile, std::string const& line = {}) const;

  struct Settings
  {
    bool readline {false};
    bool use_stdout {false};
    bool debug {false};
    bool copy {false};
    bool summary {false};
    bool ignore {false};
  }; // struct Settings
  Settings settings_;

  struct Stats
  {
    // general stats
    int macro {0};
    int ignored {0};
    int warning {0};
    int error {0};
    int pass {0};

    // macro stats
    int core {0};
    int internal {0};
    int external {0};
    int remote {0};
  }; // struct Stats
  Stats stats_;

  std::string delim_start_ {"[M8["};
  std::string delim_end_ {"]8M]"};
  std::string ignore_;
  std::string comment_;

  std::unordered_set<std::string> includes_;

  std::unordered_map<std::string, std::string> rx_grammar_ {
    {"b", "^"},
    {"e", "$"},
    {"ws", "\\s+"},
    {"empty", "^$"},
    {"void", "^$"},
    {"!all", "([^\\r]*?)"},
    {"!wrd", "([^\\s]+?)"},
    {"!num", "([\\-+]{0,1}[0-9]+(?:\\.[0-9]+)?(?:e[\\-+]{0,1}[0-9]+)?)"},
    // {"!int", "([0-9]+)"},
    // {"!dec", "([0-9]+\.[0-9]+)"},
    {"!str_s", "'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)'"},
    {"!str_d", "\"([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)\""},
  };

  // abstract syntax tree
  Ast ast_;

  // hooks
  Hooks h_begin_;
  Hooks h_macro_;
  Hooks h_res_;
  Hooks h_end_;

  void core_macros();

  void run_hooks(Hooks const& h, std::string& s);

  int run_internal(macro_fn const& func, Ctx& ctx);
  int run_external(Macro const& macro, Ctx& ctx);
  int run_remote(Macro const& macro, Ctx& ctx);

  std::string env_var(std::string const& str) const;
  std::vector<std::string> suggest_macro(std::string const& name) const;
}; // class M8

#endif // M8_HH
