#include "m8/m8.hh"

#include "m8/ast.hh"
#include "m8/reader.hh"
#include "m8/writer.hh"

#include "ob/sys_command.hh"
#include "ob/string.hh"
#include "ob/http.hh"

#include "ob/term.hh"
namespace aec = OB::Term::ANSI_Escape_Codes;

#include "lib/json.hh"
using Json = nlohmann::json;

#include <ctime>
#include <cctype>
#include <cstddef>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <regex>
#include <functional>
#include <stdexcept>
#include <future>
#include <iterator>
#include <stack>
#include <algorithm>
#include <deque>
#include <optional>

#include <filesystem>
namespace fs = std::filesystem;

M8::M8()
{
  core_macros();
}

M8::~M8()
{
}

void M8::core_macros()
{
  set_core("m8:include_once",
    "include a files contents once",
    "{str}",
    "{b}{!str_s}{e}",
    [&](auto& ctx) {
    auto str = ctx.args.at(1);
    if (str.empty())
    {
      return -1;
    }
    try
    {
      auto const& name = ctx.args.at(1);

      if (includes_.find(name) != includes_.end())
      {
        return 0;
      }
      includes_.emplace(name);

      ctx.core->w.write(ctx.core->buf);
      ctx.core->w.flush();
      ctx.core->buf.clear();
      parse(name, ctx.core->ofile);
    }
    catch (std::exception const& e)
    {
      ctx.err_msg = "an error occurred while parsing the included file";
      return -1;
    }
    return 0;
    });

  set_core("m8:include",
    "include a files contents",
    "{str}",
    "{b}{!str_s}{e}",
    [&](auto& ctx) {
    auto str = ctx.args.at(1);
    if (str.empty())
    {
      return -1;
    }
    try
    {
      ctx.core->w.write(ctx.core->buf);
      ctx.core->w.flush();
      ctx.core->buf.clear();
      parse(ctx.args.at(1), ctx.core->ofile);
    }
    catch (std::exception const& e)
    {
      ctx.err_msg = "an error occurred while parsing the included file";
      return -1;
    }
    return 0;
    });

  set_core("m8:file",
    "current file name",
    "[void]",
    "{void}",
    [&](auto& ctx) {
    ctx.str = ctx.core->ifile;
    return 0;
    });

  set_core("m8:line",
    "current line number",
    "[void]",
    "{void}",
    [&](auto& ctx) {
    ctx.str = std::to_string(ctx.core->r.row());
    return 0;
    });

  set_core("m8:ns+",
    "namespace block",
    "[void]",
    "{void}",
    [&](auto& ctx) {
    macros_.add_scope();

    return 0;
    });

  set_core("m8:ns-",
    "namespace block",
    "[void]",
    "{void}",
    [&](auto& ctx) {
    macros_.rm_scope();

    return 0;
    });

  auto const fn_test_hook_info = [&](auto& ctx) {
    std::stringstream st;
    st << ctx.args.at(1);
    char t;
    st >> t;

    std::stringstream ss;

    auto const stringify_hooks = [&](auto t) {
      if (auto h = get_hooks(t))
      {
        for (auto const& e : *h)
        {
          ss << "k: " << e.key << "\nv: " << e.val << "\n";
        }
      }
    };

    if (t == 'b')
    {
      stringify_hooks(M8::Htype::begin);
    }
    else if (t == 'm')
    {
      stringify_hooks(M8::Htype::macro);
    }
    else if (t == 'r')
    {
      stringify_hooks(M8::Htype::res);
    }
    else if (t == 'e')
    {
      stringify_hooks(M8::Htype::end);
    }
    else
    {
      return -1;
    }

    ctx.str = ss.str();
    return 0;
  };
  auto const fn_test_hook_rm = [&](auto& ctx) {
    std::stringstream ss;
    ss << ctx.args.at(1);
    char t;
    ss >> t;
    auto s1 = ctx.args.at(2);

    if (t == 'b')
    {
      rm_hook(M8::Htype::begin, s1);
    }
    else if (t == 'm')
    {
      rm_hook(M8::Htype::macro, s1);
    }
    else if (t == 'r')
    {
      rm_hook(M8::Htype::res, s1);
    }
    else if (t == 'e')
    {
      rm_hook(M8::Htype::end, s1);
    }
    else
    {
      return -1;
    }
    return 0;
  };
  auto const fn_test_hook_add = [&](auto& ctx) {
    std::stringstream ss;
    ss << ctx.args.at(1);
    char t;
    ss >> t;
    auto s1 = ctx.args.at(2);
    auto s2 = ctx.args.at(3);

    if (t == 'b')
    {
      set_hook(M8::Htype::begin, {s1, s2});
    }
    else if (t == 'm')
    {
      set_hook(M8::Htype::macro, {s1, s2});
    }
    else if (t == 'r')
    {
      set_hook(M8::Htype::res, {s1, s2});
    }
    else if (t == 'e')
    {
      set_hook(M8::Htype::end, {s1, s2});
    }
    else
    {
      return -1;
    }
    return 0;
  };
  set_macro("m8:hook+",
    "test add hook macro",
    {
      M8::macro_t("1[bre] str str", "{b}([bmre]{1}){ws}{!str_s}{ws}{!str_s}{e}", fn_test_hook_add),
    });
  set_macro("m8:hook-",
    "test remove hook macro",
    {
      M8::macro_t("1[bre] str", "{b}([bmre]{1}){ws}{!str_s}{e}", fn_test_hook_rm),
    });
  set_macro("m8:hook:info",
    "test info hook macro",
    {
      M8::macro_t("1[bre]", "{b}([bmre]{1}){e}", fn_test_hook_info),
    });
}

std::string M8::error(error_t type, Tmacro const& macro, std::string const& ifile, std::string const& line) const
{
  std::ostringstream oss;
  OB::Term::ostream ss {oss, 2};
  if (OB::Term::is_term(STDERR_FILENO))
  {
    ss.width(OB::Term::width(STDERR_FILENO));
  }
  else
  {
    ss.line_wrap(false);
    ss.escape_codes(false);
  }

  ss
  << aec::wrap(fs::canonical(ifile).string(), aec::fg_white)
  << ":" << aec::wrap(macro.line_start, aec::fg_white);

  if (macro.line_start != macro.line_end)
  {
    ss << "-" << aec::wrap(macro.line_end, aec::fg_white);
  }

  switch (type)
  {
    case error_t::missing_opening_delimiter:
    {
      ss
      << ": " << aec::wrap("error", aec::fg_red) << ": "
      << "missing opening delimiter"
      << " '" << aec::wrap(macro.name, aec::fg_white) << "'"
      << "\n"
      << line.substr(0, line.size() - (line.size() - macro.begin))
      << aec::wrap(delim_end_, aec::fg_red)
      << line.substr(macro.begin + delim_end_.size())
      // << "\n"
      // << std::string(line.size() - (line.size() - macro.begin), ' ')
      // << aec::wrap("^" + std::string(delim_end_.size() - 1, '~'), aec::fg_red)
      << "\n";

      break;
    }

    case error_t::missing_closing_delimiter:
    {
      ss
      << ": " << aec::wrap("error", aec::fg_red) << ": "
      << "missing closing delimiter"
      << " '" << aec::wrap(delim_end_, aec::fg_white) << "'"
      << "\n"
      << line
      // << "\n"
      // << std::string(macro.begin, ' ')
      // << aec::wrap("^", aec::fg_red)
      << "\n";

      break;
    }

    case error_t::invalid_format:
    {
      ss
      << ": " << aec::wrap("error", aec::fg_red) << ": "
      << "invalid format";

      if (! macro.name.empty())
      {
       ss
       << " '" << aec::wrap(macro.name, aec::fg_white) << "'";
      }

      ss
      << "\n"
      << delim_start_ << " "
      << aec::wrap(macro.str, aec::fg_red) << " "
      << delim_end_
      // << "\n"
      // << std::string(delim_start_.size(), ' ')
      // << aec::wrap("^~" + std::string(macro.str.size(), '~'), aec::fg_red)
      << "\n";

      break;
    }

    case error_t::undefined_name:
    {
      ss
      << ": " << aec::wrap("error", aec::fg_red) << ": "
      << "undefined name"
      << " '" << aec::wrap(macro.name, aec::fg_white) << "'\n"
      << delim_start_ << " "
      << aec::wrap(macro.name, aec::fg_red) << " ";
      if (! macro.args.empty())
      {
        ss << macro.args << " ";
      }
      ss
      << delim_end_
      // << "\n"
      // << std::string(delim_start_.size() + 1, ' ')
      // << aec::wrap("^" + std::string(macro.name.size() - 1, '~'), aec::fg_red)
      << "\n";

      auto similar_names = suggest_macro(macro.name);
      if (similar_names.size() > 0)
      {
        ss
        << "did you mean:"
        << aec::fg_green
        << OB::Term::iomanip::push();

        for (auto const& e : similar_names)
        {
          ss << e << "\n";
        }

        ss
        << aec::reset
        << OB::Term::iomanip::pop();
      }

      break;
    }

    case error_t::invalid_arg:
    {
      ss
      << ": " << aec::wrap("error", aec::fg_red) << ": "
      << "invalid argument"
      << " in '" << aec::wrap(macro.name, aec::fg_white) << "'"
      << "\n"
      << delim_start_
      << " " << macro.name << " ";
      if (! macro.args.empty())
      {
        ss << aec::wrap(macro.args, aec::fg_red) << " ";
      }
      ss
      << delim_end_
      // << "\n"
      // << std::string(delim_start_.size() + 1 + macro.name.size() + 1, ' ')
      // << aec::wrap("^" + std::string((macro.args.size() ? macro.args.size() - 1 : macro.args.size()), '~'), aec::fg_red)
      << "\n";

      auto const info = macros_.find(macro.name)->second;

      ss
      << "expected usage/regex "
      << OB::String::plural("pair", info.impl.size())
      << " for '"
      << aec::wrap(macro.name, aec::fg_white)
      << "':\n";

      for (auto const& e : info.impl)
      {
        ss
        << OB::Term::iomanip::push()
        << aec::wrap(e.usage, aec::fg_white) << "\n"
        << OB::Term::iomanip::push()
        << aec::wrap(e.regex, aec::fg_green) << "\n"
        << OB::Term::iomanip::pop(2);
      }

      break;
    }

    case error_t::failed:
    {
      ss
      << ": " << aec::wrap("error", aec::fg_red) << ": "
      << "macro "
      << "'" << aec::wrap(macro.name, aec::fg_white) << "'"
      << " failed"
      << "\n"
      << "reason: "
      << line
      << "\n";

      break;
    }

    default:
    {
      break;
    }
  }

  return oss.str();
}

void M8::set_debug(bool val)
{
  settings_.debug = val;
}

void M8::set_comment(std::string str)
{
  comment_ = str;
}

void M8::set_ignore(std::string str)
{
  ignore_ = str;
}

void M8::set_copy(bool val)
{
  settings_.copy = val;
}

void M8::set_readline(bool val)
{
  settings_.readline = val;
}

void M8::set_core(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex, macro_fn func)
{
  regex = OB::String::format(regex, rx_grammar_);
  // macros_[name] = {Mtype::core, name, info, usage, {{regex, func}}, {}};
  macros_.insert_or_assign(name, Macro({Mtype::core, name, info, {{usage, regex, func}}, {}}));
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex)
{
  regex = OB::String::format(regex, rx_grammar_);
  // macros_[name] = {Mtype::external, name, info, usage, {{regex, nullptr}}, {}};
  macros_.insert_or_assign(name, Macro({Mtype::external, name, info, {{usage, regex, nullptr}}, {}}));
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex, std::string const& url)
{
  regex = OB::String::format(regex, rx_grammar_);
  // macros_[name] = {Mtype::remote, name, info, usage, {{regex, nullptr}}, url};
  macros_.insert_or_assign(name, Macro({Mtype::remote, name, info, {{usage, regex, nullptr}}, url}));
}

void M8::set_macro(std::string const& name, std::string const& info,
  std::string const& usage, std::string regex, macro_fn func)
{
  regex = OB::String::format(regex, rx_grammar_);
  // macros_[name] = {Mtype::internal, name, info, usage, {{regex, func}}, {}};
  macros_.insert_or_assign(name, Macro({Mtype::internal, name, info, {{usage, regex, func}}, {}}));
}

void M8::set_macro(std::string const& name, std::string const& info, std::vector<M8::macro_t> impl)
{
  for (auto& e : impl)
  {
    e.regex = OB::String::format(e.regex, rx_grammar_);
  }
  // macros_[name] = {Mtype::internal, name, info, usage, rx_fn, {}};
  macros_.insert_or_assign(name, Macro({Mtype::internal, name, info, impl, {}}));
}

void M8::set_delimits(std::string const& delim_start, std::string const& delim_end)
{
  if (delim_start == delim_end)
  {
    throw std::runtime_error("start and end delimeter can't be the same value");
  }
  delim_start_ = delim_start;
  delim_end_ = delim_end;
  rx_grammar_["DS"] = delim_start_;
  rx_grammar_["DE"] = delim_end_;
}

std::string M8::list_macros() const
{
  std::stringstream ss;

  ss << "M8:\n\n";

  for (auto const& e : macros_)
  {
    ss
    << e.second.name << "\n"
    << "  " << e.second.info << "\n";
    for (auto const& e : e.second.impl)
    {
      ss << "  " << e.usage << "\n";
      ss << "    " << e.regex << "\n";
    }
  }

  return ss.str();
}

std::string M8::macro_info(std::string const& name) const
{
  std::stringstream ss;
  if (macros_.find(name) == macros_.end())
  {
    ss << aec::wrap("Error: ", aec::fg_red);
    ss << "Undefined name '" << aec::wrap(name, aec::fg_white) << "'\n";

    // lookup similar macro suggestion
    ss << "  Looking for similar names...\n";
    auto similar_names = suggest_macro(name);
    if (similar_names.size() > 0)
    {
      ss << "  Did you mean:" << aec::fg_green;
      for (auto const& e : similar_names)
      {
        ss << " " << e;
      }
      ss << aec::reset << "\n";
    }
    else
    {
      ss << aec::fg_magenta << "  No suggestions found.\n" << aec::reset;
    }
  }
  else
  {
    auto const& e = macros_.at(name);
    ss
    << e.name << "\n"
    << "  " << e.info << "\n";
    for (auto const& e : e.impl)
    {
      ss << "  " << e.usage << "\n";
      ss << "    " << e.regex << "\n";
    }
  }
  return ss.str();
}

void M8::set_config(std::string file_name)
{
  // add external macros
  if (file_name.empty())
  {
    // default config file location -> ~/.m8.json
    file_name = env_var("HOME") + "/.m8.json";
  }

  // open the config file
  std::ifstream file {file_name};
  if (! file.is_open())
  {
    if (settings_.debug)
    {
      std::cerr << "Debug: could not open config file\n";
    }
    return;
  }

  // read in the config file into memory
  file.seekg(0, std::ios::end);
  std::size_t size (static_cast<std::size_t>(file.tellg()));
  std::string content (size, ' ');
  file.seekg(0);
  file.read(&content[0], static_cast<std::streamsize>(size));

  // parse the contents
  Json j = Json::parse(content);
  for (auto const& e : j["macros"])
  {
    // add remote macro
    if (e.count("url") == 1)
    {
      set_macro(
        e["name"].get<std::string>(),
        e["info"].get<std::string>(),
        e["usage"].get<std::string>(),
        e["regex"].get<std::string>(),
        e["url"].get<std::string>());
    }

    // add external macro
    else
    {
      set_macro(
        e["name"].get<std::string>(),
        e["info"].get<std::string>(),
        e["usage"].get<std::string>(),
        e["regex"].get<std::string>());
    }
  }
}

std::string M8::summary() const
{
  std::stringstream ss; ss
  << aec::wrap("Summary\n", aec::fg_magenta)
  << aec::wrap("  Total      ", aec::fg_magenta) << aec::wrap(stats_.macro, aec::fg_green) << "\n"
  << aec::wrap("    Internal ", aec::fg_magenta) << aec::wrap(stats_.internal, aec::fg_green) << "\n"
  << aec::wrap("    External ", aec::fg_magenta) << aec::wrap(stats_.external, aec::fg_green) << "\n"
  << aec::wrap("    Remote   ", aec::fg_magenta) << aec::wrap(stats_.remote, aec::fg_green) << "\n"
  << aec::wrap("  passes     ", aec::fg_magenta) << aec::wrap(stats_.pass, aec::fg_green) << "\n"
  << aec::wrap("  warnings   ", aec::fg_magenta) << aec::wrap(stats_.warning, aec::fg_green) << "\n";
  return ss.str();
}

std::vector<std::string> M8::suggest_macro(std::string const& name) const
{
  int const weight_max {8};
  std::vector<std::pair<int, std::string>> dist;

  for (auto const& [key, val] : macros_)
  {
    int weight {0};

    if (OB::String::starts_with(key, name))
    {
      weight = 0;
    }
    else
    {
      weight = OB::String::damerau_levenshtein(name, key, 1, 2, 3, 0);
    }

    if (weight < weight_max)
    {
      dist.emplace_back(weight, key);
    }
  }

  std::sort(dist.begin(), dist.end(),
  [](auto const& lhs, auto const& rhs)
  {
    return (lhs.first == rhs.first) ?
      (lhs.second.size() < rhs.second.size()) :
      (lhs.first < rhs.first);
  });

  std::vector<std::string> dict;
  for (auto const& [key, val] : dist)
  {
    dict.emplace_back(val);
  }

  size_t const dict_max {3};
  if (dict.size() > dict_max)
  {
    dict.erase(dict.begin() + dict_max, dict.end());
  }

  return dict;
}

void M8::set_hook(Htype t, Hook h)
{
  auto const insert_hook = [&](auto& hooks) {
    for (auto& e : hooks)
    {
      if (e.key == h.key)
      {
        e.val = h.val;
        return;
      }
    }
    hooks.emplace_back(h);
  };

  switch (t)
  {
    case Htype::begin:
      insert_hook(h_begin_);
      return;
    case Htype::macro:
      insert_hook(h_macro_);
      return;
    case Htype::res:
      insert_hook(h_res_);
      return;
    case Htype::end:
      insert_hook(h_end_);
      return;
    default:
      return;
  }
}

std::optional<M8::Hooks> M8::get_hooks(Htype t) const
{
  switch (t)
  {
    case Htype::begin: return h_begin_;
    case Htype::macro: return h_macro_;
    case Htype::res: return h_res_;
    case Htype::end: return h_end_;
    default: return {};
  }
}

void M8::rm_hook(Htype t, std::string key)
{
  auto const rm_key = [&](auto& m) {
    long int i {0};
    for (auto& e : m)
    {
      if (e.key == key)
      {
        m.erase(m.begin() + i);
      }
      ++i;
    }
  };

  switch (t)
  {
    case Htype::begin:
      rm_key(h_begin_);
      break;
    case Htype::macro:
      rm_key(h_macro_);
      break;
    case Htype::res:
      rm_key(h_res_);
      break;
    case Htype::end:
      rm_key(h_end_);
      break;
    default:
      break;
  }
}

void M8::run_hooks(Hooks const& h, std::string& s)
{
  for (auto const& e : h)
  {
    std::string str = s;
    s.clear();
    std::smatch match;

    while (std::regex_search(str, match, std::regex(e.key)))
    {
      std::unordered_map<std::string, std::string> m;
      for (std::size_t i = 1; i < match.size(); ++i)
      {
        m[std::to_string(i)] = match[i];
        m["DS"] = delim_start_;
        m["DE"] = delim_end_;
      }
      std::string ns {e.val};
      ns = OB::String::format(ns, m);
      s += std::string(match.prefix()) + ns;
      if (str == match.suffix())
      {
        str.clear();
        break;
      }
      str = match.suffix();
      if (str.empty()) break;
    }
    s += str;
  }
}

void M8::parse(std::string const& _ifile, std::string const& _ofile)
{
  // init the reader
  Reader r;
  if (! settings_.readline || ! _ifile.empty())
  {
    r.open(_ifile);
  }

  // init the writer
  Writer w;
  if (! _ofile.empty())
  {
    w.open(_ofile);
  }

  auto& ast = ast_.ast;
  std::stack<Tmacro> stk;
  // Cache cache_ {_ifile};

  std::string buf;
  std::string line;

  while(r.next(line))
  {
    buf.clear();

    // check for empty line
    if (line.empty())
    {
      // TODO add flag to ignore empty lines
      if (stk.empty())
      {
        // if (! _ofile.empty() && settings_.copy)
        if (settings_.copy)
        {
          w.write("\n");
        }
        continue;
      }
      else
      {
        auto& t = stk.top();
        t.str += "\n";
        continue;
      }
    }

    // commented out line
    if (! comment_.empty())
    {
      auto pos = line.find_first_not_of(" \t");
      if (pos != std::string::npos)
      {
        if (line.compare(pos, comment_.size(), comment_) == 0)
        {
          continue;
        }
      }
    }

    // whitespace indentation
    std::size_t indent {0};
    char indent_char {' '};
    {
      std::string e {line.at(0)};
      if (e.find_first_of(" \t") != std::string::npos)
      {
        std::size_t count {0};
        for (std::size_t i = 0; i < line.size(); ++i)
        {
          e = line.at(i);
          if (e.find_first_not_of(" \t") != std::string::npos)
          {
            break;
          }
          ++count;
        }
        indent = count;
        indent_char = line.at(0);
      }
    }

    // find and replace macro words
    run_hooks(h_begin_, line);

    // parse line char by char for either start or end delim
    for (std::size_t i = 0; i < line.size(); ++i)
    {
      // case start delimiter
      if (line.at(i) == delim_start_.at(0))
      {
        std::size_t pos_start = line.find(delim_start_, i);
        if (pos_start != std::string::npos && pos_start == i)
        {
          if (i > 0 && line.at(i - 1) == '`')
          {
            goto regular_char;
          }

          // stack operations
          auto t = Tmacro();
          t.line_start = r.row();
          t.begin = pos_start;
          // if (! stk.empty())
          // {
          //   append placeholder
          //   stk.top().str += "[%" + std::to_string(stk.top().children.size()) + "]";
          // }

          stk.push(t);

          i += delim_start_.size() - 1;
          continue;
        }
      }

      // case end delimiter
      if (line.at(i) == delim_end_.at(0))
      {
        std::size_t pos_end = line.find(delim_end_, i);
        if (pos_end != std::string::npos && pos_end == i)
        {
          if (i + delim_end_.size() < line.size() && line.at(i + delim_end_.size()) == '`')
          {
            goto regular_char;
          }

          // stack operations
          if (stk.empty())
          {
            auto t = Tmacro();
            t.name = delim_start_;
            t.line_start = r.row();
            t.line_end = r.row();
            t.begin = i;
            std::cerr << error(error_t::missing_opening_delimiter, t, _ifile, r.line());
            if (settings_.readline)
            {
              stk = std::stack<Tmacro>();
              break;
            }
            throw std::runtime_error("missing opening delimiter");
          }
          else
          {
            auto t = stk.top();
            stk.pop();

            t.line_end = r.row();
            t.end = pos_end;

            // parse str into name and args
            {
              std::smatch match;
              std::string name_args {"^\\s*([^\\s]+)\\s*([^\\r]*?)\\s*$"};
              // std::string name_args {"^\\s*([^\\s]+)\\s*(?:M8!|)([^\\r]*?)(?:!8M|$)$"};
              if (std::regex_match(t.str, match, std::regex(name_args)))
              {
                t.name = match[1];
                t.args = match[2];

                // find and replace macro words
                run_hooks(h_macro_, t.name);
                run_hooks(h_macro_, t.args);
              }
              else
              {
                std::cerr << error(error_t::invalid_format, t, _ifile);
                if (settings_.readline)
                {
                  stk = std::stack<Tmacro>();
                  break;
                }
                throw std::runtime_error("invalid format");
              }
            }

            // validate name and args
            {
              auto const it = macros_.find(t.name);
              if (it == macros_.end())
              {
                std::cerr << error(error_t::undefined_name, t, _ifile);

                if (settings_.readline)
                {
                  stk = std::stack<Tmacro>();
                  buf.clear();
                  break;
                }
                throw std::runtime_error("undefined name");
              }

              if (it->second.impl.at(0).regex.empty())
              {
                std::vector<std::string> reg_num {
                  {"^[\\-+]{0,1}[0-9]+$"},
                  {"^[\\-+]{0,1}[0-9]*\\.[0-9]+$"},
                  {"^[\\-+]{0,1}[0-9]+e[\\-+]{0,1}[0-9]+$"},
                  // {"^[\\-|+]{0,1}[0-9]+/[\\-|+]{0,1}[0-9]+$"},
                };

                std::vector<std::string> reg_str {
                  {"^([^`\\\\]*(?:\\\\.[^`\\\\]*)*)$"},
                  {"^([^'\\\\]*(?:\\\\.[^'\\\\]*)*)$"},
                  {"^([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)$"},
                };

                // complete arg string as first parameter
                t.match.emplace_back(t.args);
                std::vector<bool> valid_args;

                for (std::size_t j = 0; j < t.args.size(); ++j)
                {
                  std::stringstream ss; ss << t.args.at(j);
                  auto s = ss.str();
                  // std::cerr << "arg:" << s << "\n";
                  if (s.find_first_of(" \n\t") != std::string::npos)
                  {
                    continue;
                  }
                  // if (s.find_first_of(".") != std::string::npos)
                  // {
                  //   // macro
                  //   t.match.emplace_back(std::string());
                  //   for (;j < t.args.size() && t.args.at(j) != '.'; ++j)
                  //   {
                  //     t.match.back() += t.args.at(j);
                  //   }
                  //   std::cerr << "Arg-Macro:\n~" << t.match.back() << "~\n";
                  //   for (auto const& e : reg_num)
                  //   {
                  //     std::smatch m;
                  //     if (std::regex_match(t.match.back(), m, std::regex(e)))
                  //     {
                  //       // std::cerr << "ArgValid\nmacro\n" << t.match.back() << "\n\n";
                  //     }
                  //   }
                  // }
                  if (s.find_first_of(".-+0123456789") != std::string::npos)
                  {
                    // num
                    // std::cerr << "Num\n";
                    t.match.emplace_back(std::string());
                    for (;j < t.args.size() && t.args.at(j) != ' '; ++j)
                    {
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Num\n" << t.match.back() << "\n";
                    bool invalid {true};
                    for (auto const& e : reg_num)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nnum\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                    // if (t.match.back().find("/") != std::string::npos)
                    // {
                    //   auto n1 = t.match.back().substr(0, t.match.back().find("/"));
                    //   auto n2 = t.match.back().substr(t.match.back().find("/") + 1);
                    //   auto n = std::stod(n1) / std::stod(n2);
                    //   std::stringstream ss; ss << n;
                    //   t.match.back() = ss.str();
                    //   std::cerr << "Simplified:\n" << t.match.back() << "\n\n";
                    // }
                  }
                  else if (s.find_first_of("\"") != std::string::npos)
                  {
                    // str
                    // std::cerr << "Str\n";
                    t.match.emplace_back("");
                    ++j; // skip start quote
                    bool escaped {false};
                    for (;j < t.args.size(); ++j)
                    {
                      if (! escaped && t.args.at(j) == '\"')
                      {
                        // skip end quote
                        break;
                      }
                      if (t.args.at(j) == '\\')
                      {
                        escaped = true;
                        continue;
                      }
                      if (escaped)
                      {
                        t.match.back() += "\\";
                        t.match.back() += t.args.at(j);
                        escaped = false;
                        continue;
                      }
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Str\n" << t.match.back() << "\n";
                    bool invalid {true};
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                  }
                  else if (s.find_first_of("\'") != std::string::npos)
                  {
                    // str
                    // std::cerr << "Str\n";
                    t.match.emplace_back("");
                    ++j; // skip start quote
                    bool escaped {false};
                    for (;j < t.args.size(); ++j)
                    {
                      if (! escaped && t.args.at(j) == '\'')
                      {
                        // skip end quote
                        break;
                      }
                      if (t.args.at(j) == '\\')
                      {
                        escaped = true;
                        continue;
                      }
                      if (escaped)
                      {
                        t.match.back() += "\\";
                        t.match.back() += t.args.at(j);
                        escaped = false;
                        continue;
                      }
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Str\n" << t.match.back() << "\n";
                    bool invalid {true};
                    std::string mstr {"'" + t.match.back() + "'"};
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(mstr, m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                  }
                  else if (s.find_first_of("`") != std::string::npos)
                  {
                    // literal
                    // std::cerr << "Str\n";
                    t.match.emplace_back("");
                    ++j; // skip start quote
                    bool escaped {false};
                    for (;j < t.args.size(); ++j)
                    {
                      if (! escaped && t.args.at(j) == '`')
                      {
                        // skip end quote
                        break;
                      }
                      if (t.args.at(j) == '\\')
                      {
                        escaped = true;
                        continue;
                      }
                      if (escaped)
                      {
                        t.match.back() += "\\";
                        t.match.back() += t.args.at(j);
                        escaped = false;
                        continue;
                      }
                      t.match.back() += t.args.at(j);
                    }
                    // std::cerr << "Arg-Str\n" << t.match.back() << "\n";
                    bool invalid {true};
                    for (auto const& e : reg_str)
                    {
                      std::smatch m;
                      if (std::regex_match(t.match.back(), m, std::regex(e)))
                      {
                        invalid = false;
                        // std::cerr << "ArgValid\nstr\n" << t.match.back() << "\n\n";
                      }
                    }
                    if (invalid)
                    {
                      goto invalid_arg;
                    }
                  }
                  else
                  {
invalid_arg:
                    // invalid arg
                    if (settings_.readline)
                    {
                      std::cerr << error(error_t::invalid_arg, t, _ifile);

                      std::cerr
                      << aec::wrap(t.match.back(), aec::fg_magenta)
                      << "\n";

                      stk = std::stack<Tmacro>();
                      break;
                    }
                    throw std::runtime_error("macro " + t.name + " has an invalid argument");
                  }
                }
                // std::cerr << "ArgValid\ncomplete\n\n";
              }
              else
              {
                bool invalid_regex {true};
                if (it->second.impl.size() == 1)
                {
                  std::smatch match;
                  if (std::regex_match(t.args, match, std::regex(it->second.impl.at(0).regex)))
                  {
                    invalid_regex = false;
                    for (auto const& e : match)
                    {
                      t.match.emplace_back(std::string(e));
                    }
                  }
                }
                else
                {
                  std::size_t index {0};
                  for (auto const& rf : it->second.impl)
                  {
                    std::smatch match;
                    if (std::regex_match(t.args, match, std::regex(rf.regex)))
                    {
                      invalid_regex = false;
                      for (auto const& e : match)
                      {
                        t.match.emplace_back(std::string(e));
                        if (settings_.debug)
                        {
                          std::cerr << "arg: " << std::string(e) << "\n";
                        }
                      }
                      t.fn_index = index;
                      break;
                    }
                    ++index;
                  }
                }
                if (invalid_regex)
                {
                  // TODO fix, correct args not being listed out
                  std::cerr << error(error_t::invalid_arg, t, _ifile);

                  if (settings_.readline)
                  {
                    stk = std::stack<Tmacro>();
                    break;
                  }
                  throw std::runtime_error("invalid argument");
                }
              }

              // process macro
              int ec {0};
              Ctx ctx {t.res, t.match, "", nullptr};
              try
              {
                // ignore matching names
                std::smatch match;
                if (std::regex_match(t.name, match, std::regex(ignore_)))
                {
                  ++stats_.ignored;
                }

                // call core
                else if (it->second.type == Mtype::core)
                {
                  ++stats_.macro;
                  ctx.core = std::make_unique<Core_Ctx>(buf, r, w, _ifile, _ofile);
                  ec = run_internal(it->second.impl.at(t.fn_index).func, ctx);
                }

                // call internal
                else if (it->second.type == Mtype::internal)
                {
                  ++stats_.macro;
                  ec = run_internal(it->second.impl.at(t.fn_index).func, ctx);
                }

                // call remote
                else if (it->second.type == Mtype::remote)
                {
                  ++stats_.macro;
                  ec = run_remote(it->second, ctx);
                }

                // call external
                else if (it->second.type == Mtype::external)
                {
                  ++stats_.macro;
                  ec = run_external(it->second, ctx);
                }
              }
              catch (std::exception const& e)
              {
                std::cerr << error(error_t::failed, t, _ifile, e.what());
                if (settings_.readline)
                {
                  i += delim_end_.size() - 1;
                  stk = std::stack<Tmacro>();
                  continue;
                }
                throw std::runtime_error("macro failed");
              }
              if (ec != 0)
              {
                std::cerr << error(error_t::failed, t, _ifile, ctx.err_msg);
                if (settings_.readline)
                {
                  i += delim_end_.size() - 1;
                  stk = std::stack<Tmacro>();
                  continue;
                }
                throw std::runtime_error("macro failed");
              }

              // find and replace macro words
              run_hooks(h_res_, t.res);

              // debug
              if (settings_.debug)
              {
                std::cerr << "\nRes:\n~" << t.res << "~\n";
              }

              if (t.res.find(delim_start_) != std::string::npos)
              {
                // remove escaped nl chars
                // t.res = OB::String::replace_all(t.res, "\\\n", "");

                // remove all nl chars that follow delim_end
                // this would normally be removed by the reader
                // t.res = OB::String::replace_all(t.res, delim_end_ + "\n", delim_end_);

                // TODO handle the same as if it was read from file
                line.insert(i + delim_end_.size(), t.res);
                i += delim_end_.size() - 1;
                continue;
              }

              if (! stk.empty())
              {
                stk.top().str += t.res;

                if ((! t.res.empty()) && (i + delim_end_.size() - 1 == line.size() - 1))
                {
                  // account for when end delim is last char on line
                  // add a newline char to buf
                  // only if response is not empty
                  stk.top().str += "\n";
                }
              }
              else
              {
                // add indentation
                {
                  std::string indent_str (indent, indent_char);
                  t.res = OB::String::replace_all(t.res, "\n", "\n" + indent_str);
                  t.res = OB::String::replace_last(t.res, "\n" + indent_str, "\n");
                  t.res = OB::String::replace_all(t.res, "\n" + indent_str + "\n", "\n\n");
                }

                buf += t.res;
                // std::cerr << "t.name: " << t.name << "\n";
                // std::cerr << "i: " << i << "\n";
                // std::cerr << "t.res: " << t.res << "\n";

                if ((! t.res.empty()) && (i + delim_end_.size() - 1 == line.size() - 1))
                {
                  // account for when end delim is last char on line
                  // add a newline char to buf
                  // only if response is not empty
                  buf += "\n";
                }
              }
            }

            // debug
            if (settings_.debug)
            {
              std::cerr << "\nRes fmt:\n~" << t.res << "~\n";
              std::cerr << "\nBuf fmt:\n~" << buf << "~\n";
            }

            if (stk.empty())
            {
              ast.emplace_back(t);
            }
            else
            {
              auto& l = stk.top();
              l.children.emplace_back(t);
            }
          }

          i += delim_end_.size() - 1;
          continue;
        }
      }

regular_char:

      // case else other characters
      if (! stk.empty())
      {
        // inside macro
        auto& t = stk.top();

        if (i == (line.size() - 1))
        {
          if (line.at(i) != '\\')
          {
            t.str += line.at(i);
            t.str += "\n";
          }
        }
        else
        {
          t.str += line.at(i);
        }
      }
      else
      {
        // outside macro
        // append to output buffer
        if (settings_.copy)
        {
          if (i == (line.size() - 1))
          {
            if (line.at(i) != '\\')
            {
              buf += line.at(i);
              buf += "\n";
            }
          }
          else
          {
            buf += line.at(i);
          }
        }
      }

    }

    // find and replace macro words
    run_hooks(h_end_, buf);

    if (settings_.debug)
    {
      if (! ast_.ast.empty())
      {
        std::cerr << "AST:\n" << ast_.str() << "\n";
      }
      ast_.clear();
    }

    if (buf.empty() || buf == "\n")
    {
      continue;
    }

    // append buf to output file
    w.write(buf);
  }

  if (! stk.empty())
  {
    auto t = Tmacro();
    t.name = delim_end_;
    t.line_start = r.row();
    t.line_end = r.row();
    t.begin = r.line().size() - 1;
    std::cerr << error(error_t::missing_closing_delimiter, t, _ifile, r.line());
    throw std::runtime_error("missing closing delimiter");
  }

  if (! _ofile.empty())
  {
    w.close();
  }
}

int M8::run_internal(macro_fn const& func, Ctx& ctx)
{
  ++stats_.internal;

  return func(ctx);
}

int M8::run_external(Macro const& macro, Ctx& ctx)
{
  ++stats_.external;

  std::string m_args;
  for (std::size_t i = 1; i < ctx.args.size(); ++i)
  {
    m_args += ctx.args[i];
    if (i < ctx.args.size() - 1)
      m_args += " ";
  }
  return OB::exec(ctx.str, macro.name + " " + m_args);
}

int M8::run_remote(Macro const& macro, Ctx& ctx)
{
  ++stats_.remote;

  Http api;
  api.req.method = "POST";
  api.req.headers.emplace_back("content-type: application/json");
  api.req.url = macro.url;

  Json data;
  data["name"] = macro.name;
  data["args"] = ctx.args;
  api.req.data = data.dump();

  std::cerr << "Remote macro call -> " << macro.name << "\n";
  std::future<int> send {std::async(std::launch::async, [&]() {
    api.run();
    int status_code = api.res.status;
    if (status_code != 200)
    {
      return -1;
    }
    else
    {
      ctx.str = api.res.body;
      return 0;
    }
  })};

  std::future_status fstatus;
  do
  {
    fstatus = send.wait_for(std::chrono::milliseconds(250));
    std::cerr << "." << std::flush;
  }
  while (fstatus != std::future_status::ready);

  int ec = send.get();
  if (ec == 0)
  {
    std::cerr << aec::erase_line << aec::cr << "Success: remote call\n";
  }
  else
  {
    std::cerr << aec::erase_line << aec::cr << "Error: remote call\n";
  }

  return ec;
}

std::string M8::env_var(std::string const& str) const
{
  std::string res;
  if (const char* envar = std::getenv(str.c_str()))
  {
    res = envar;
  }
  return res;
}
