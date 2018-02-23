#ifndef OB_M8_HH
#define OB_M8_HH

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

  void set_debug(bool const& val);

  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex);

  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex, std::string const& url);

  void set_macro(std::string const& name, std::string const& info,
    std::string const& usage, std::string const& regex, macro_fn fn);

  void delimit(std::string const& delim_start, std::string const& delim_end);

  std::string list_macros() const;

  void set_config(std::string file_name);

  std::string summary() const;

  void run(std::string const& ifile, std::string const& ofile);

private:
  std::ifstream ifile_;
  std::ofstream ofile_;

  int macro_count_ {0};
  int warning_count_ {0};
  int pass_count_ {0};

  bool use_stdout_ {false};
  bool debug_ {false};

  std::string start_ {"#[M8["};
  std::string end_ {"]]"};

  enum class Mtype
  {
    internal,
    external,
    remote
  };

  struct Macro
  {
    Mtype type;
    std::string info;
    std::string usage;
    std::string regex;
    std::string url;
    macro_fn fn;
  }; // struct Macro

  std::map<std::string, Macro> macros;

  std::string env_var(std::string const& var) const;

}; // class M8

#endif // OB_M8_HH
