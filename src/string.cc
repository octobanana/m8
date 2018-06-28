#include "string.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <regex>

#define var auto
#define let auto const
#define fn auto

namespace OB
{

namespace String
{

fn repeat(std::string const& str, size_t num)
-> std::string
{
  if (num < 2) return str;
  var res = std::string();
  res.reserve(str.size() * num);
  for (size_t i = 0; i < num; ++i)
  {
    res += str;
  }
  return res;
}

fn count(std::string const& str, std::string const& val)
-> size_t
{
  var pos = size_t(0);
  var count = size_t(0);
  for (;;)
  {
    pos = str.find(val, pos);
    if (pos == std::string::npos) break;
    ++count;
    ++pos;
  }
  return count;
}

fn escape(std::string str)
-> std::string
{
  var pos = size_t(0);
  for (;; pos += 2)
  {
    pos = str.find_first_of("\n\t\r\a\b\f\v\\\"\'\?", pos);
    if (pos == std::string::npos || pos + 1 == std::string::npos) break;
    if (str.at(pos) == '\n')
    {
      str.replace(pos, 1, "\\\n");
    }
    else if (str.at(pos) == '\t')
    {
      str.replace(pos, 1, "\\\t");
    }
    else if (str.at(pos) == '\r')
    {
      str.replace(pos, 1, "\\\r");
    }
    else if (str.at(pos) == '\a')
    {
      str.replace(pos, 1, "\\\a");
    }
    else if (str.at(pos) == '\b')
    {
      str.replace(pos, 1, "\\\b");
    }
    else if (str.at(pos) == '\f')
    {
      str.replace(pos, 1, "\\\f");
    }
    else if (str.at(pos) == '\v')
    {
      str.replace(pos, 1, "\\\v");
    }
    else if (str.at(pos) == '\\')
    {
      str.replace(pos, 1, "\\\\");
    }
    else if (str.at(pos) == '\"')
    {
      str.replace(pos, 1, "\\\"");
    }
    else if (str.at(pos) == '\'')
    {
      str.replace(pos, 1, "\\'");
    }
    else if (str.at(pos) == '\?')
    {
      str.replace(pos, 1, "\\\?");
    }
  }
  return str;
}

fn unescape(std::string str)
-> std::string
{
  var pos = size_t(0);
  for (;; ++pos)
  {
    pos = str.find("\\", pos);
    if (pos == std::string::npos || pos + 1 == std::string::npos) break;
    if (str.at(pos + 1) == 'n')
    {
      str.replace(pos, 2, "\n");
    }
    else if (str.at(pos + 1) == 't')
    {
      str.replace(pos, 2, "\t");
    }
    else if (str.at(pos + 1) == 'r')
    {
      str.replace(pos, 2, "\r");
    }
    else if (str.at(pos + 1) == 'a')
    {
      str.replace(pos, 2, "\a");
    }
    else if (str.at(pos + 1) == 'b')
    {
      str.replace(pos, 2, "\b");
    }
    else if (str.at(pos + 1) == 'f')
    {
      str.replace(pos, 2, "\f");
    }
    else if (str.at(pos + 1) == 'v')
    {
      str.replace(pos, 2, "\v");
    }
    else if (str.at(pos + 1) == '\\')
    {
      str.replace(pos, 2, "\\");
    }
    else if (str.at(pos + 1) == '"')
    {
      str.replace(pos, 2, "\"");
    }
    else if (str.at(pos + 1) == '\'')
    {
      str.replace(pos, 2, "'");
    }
    else if (str.at(pos + 1) == '?')
    {
      str.replace(pos, 2, "\?");
    }
  }
  return str;
}

fn replace_first(std::string str, std::string const& key, std::string const& val)
-> std::string
{
  var pos = size_t(0);
  pos = str.find(key);
  if (pos == std::string::npos) return str;
  str.replace(pos, key.size(), val);
  return str;
}

fn replace_last(std::string str, std::string const& key, std::string const& val)
-> std::string
{
  var pos = size_t(0);
  pos = str.rfind(key);
  if (pos == std::string::npos) return str;
  str.replace(pos, key.size(), val);
  return str;
}

fn replace_all(std::string str, std::string const& key, std::string const& val)
-> std::string
{
  var pos = size_t(0);
  for (;;)
  {
    pos = str.find(key, pos);
    if (pos == std::string::npos) break;
    str.replace(pos, key.size(), val);
    pos += val.size();
  }
  return str;
}

fn delimit(std::string const& str, std::string const& delim)
-> std::vector<std::string>
{
  var vtok = std::vector<std::string>();
  var start = size_t(0);
  var end = str.find(delim);
  while (end != std::string::npos) {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.length();
    end = str.find(delim, start);
  }
  vtok.emplace_back(str.substr(start, end));
  return vtok;
}

fn delimit_first(std::string const& str, std::string const& delim)
-> std::vector<std::string>
{
  var vtok = std::vector<std::string>();
  var pos = str.find(delim);
  if (pos != std::string::npos) {
    vtok.emplace_back(str.substr(0, pos));
    if (pos + delim.size() != std::string::npos)
    {
      vtok.emplace_back(str.substr(pos + delim.size()));
    }
  }
  return vtok;
}

fn format(std::string str, std::unordered_map<std::string, std::string> args)
-> std::string
{
  if (args.empty())
  {
    return str;
  }

  var res = str;
  var pos = size_t(0);
  var match = std::smatch();
  let rx = std::regex("\\{([^\\:]+?)(?:(:[^\\r]*?)?)\\}");

  while (std::regex_search(res, match, rx))
  {
    var m = std::string(match[0]);
    var first = std::string(match[1]);
    pos += std::string(match.prefix()).size();

    if (args.find(first) == args.end())
    {
      pos += m.size();
      res = match.suffix();
      continue;
    }

    str.replace(pos, m.size(), args[first]);
    pos += args[first].size();
    res = match.suffix();
  }

  return str;
}

fn xformat(std::string str, std::unordered_map<std::string, std::string> args)
-> std::string
{
  if (args.empty())
  {
    return str;
  }

  var res = str;
  var pos = size_t(0);
  var match = std::smatch();
  let rx = std::regex("\\{(\\w+)(?:(:[^\\r]*?:\\1)?)\\}");

  while (std::regex_search(res, match, rx))
  {
    bool is_simple {true};
    var m = std::string(match[0]);
    var first = std::string(match[1]);
    var second = std::string(match[2]);
    pos += std::string(match.prefix()).size();

    // std::cerr << "m: " << m << "\n";
    // std::cerr << "first: " << first << "\n";
    // std::cerr << "second: " << second << "\n";

    if (second.size() > 0)
    {
      is_simple = false;
    }

    if (args.find(first) == args.end())
    {
      pos += m.size();
      res = match.suffix();
      continue;
    }

    if (is_simple)
    {
      str.replace(pos, m.size(), args[first]);
      pos += args[first].size();
      res = match.suffix();
    }
    else
    {
      std::smatch match_complex;

      std::stringstream first_esc;
      for (auto const& e : first)
      {
        if (std::isalnum(e))
        {
          first_esc << e;
        }
        else
        {
          first_esc << "\\" << e;
        }
      }
      // {0:*;\n;i:${BUILD_DIR}/[i]\n:0}
      std::regex rx {"^:([*]{1});([^;]+?);([a-z0-9]{1}):([^:]+?):" + first_esc.str() + "$"};

      if (std::regex_match(second, match_complex, rx))
      {
        std::string type {match_complex[1]};
        std::string delim {match_complex[2]};
        std::string index_char {match_complex[3]};
        std::string rstr {match_complex[4]};

        rstr = String::unescape(rstr);
        delim = String::unescape(delim);

        // std::cerr << "match: " << match_complex[0] << "\n";
        // std::cerr << "type: " << type << "\n";
        // std::cerr << "delim: " << delim << "\n";
        // std::cerr << "index_char: " << index_char << "\n";
        // std::cerr << "rstr: " << rstr << "\n";

        auto vec = String::delimit(args[first], delim);
        for (size_t i = 0; i < vec.size(); ++i)
        {
          if (vec.at(i).empty())
          {
            vec.erase(vec.begin() + i);
          }
        }
        std::stringstream ss;
        for (auto const& e : vec)
        {
          ss << String::replace_all(rstr, "[" + index_char + "]", e);
        }

        auto nstr = String::xformat(ss.str(), args);
        str.replace(pos, m.size(), nstr);
        pos += nstr.size();
        res = match.suffix();
      }
      else
      {
        pos += m.size();
        res = match.suffix();
      }
    }
  }

  return str;
}

fn correct(std::string const& str, std::vector<std::string> const& lst)
-> std::vector<std::string>
{
  std::vector<std::string> matches;

  let len = (str.size() / 1.2);
  std::stringstream estr;
  for (let& e : str)
  {
    if (std::isalnum(e))
    {
      estr << e;
    }
    else
    {
      estr << "\\" << e;
    }
  }
  std::string rx {"^.*[" + estr.str() + "]{" + std::to_string(len) + "}.*$"};

  for (let& e : lst)
  {
    std::smatch m;
    if (std::regex_match(e, m, std::regex(rx, std::regex::icase)))
    {
      matches.emplace_back(m[0]);
    }
  }

  std::sort(matches.begin(), matches.end(),
  [](std::string const& lhs, std::string const& rhs) {
    return lhs.size() < rhs.size();
  });

  if (matches.size() > 8)
  {
    matches.erase(matches.begin() + 8, matches.end());
  }

  return matches;
}

fn starts_with(std::string const& str, std::string const& val)
-> bool
{
  if (str.empty() || str.size() < val.size())
  {
    return false;
  }
  if (str.compare(0, val.size(), str) == 0)
  {
    return true;
  }
  return false;
}

fn ends_with(std::string const& str, std::string const& val)
-> bool
{
  if (str.empty() || str.size() < val.size())
  {
    return false;
  }
  if (str.compare(str.size() - val.size(), val.size(), str) == 0)
  {
    return true;
  }
  return false;
}

} // namespace String

#undef var
#undef let
#undef fn

} // namespace OB
