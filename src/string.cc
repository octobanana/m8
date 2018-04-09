#include "string.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
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
    ++pos;
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

fn format(std::string str, std::string const& key, std::map<std::string, std::string>& args)
-> std::string
{
  if (args.empty())
  {
    return str;
  }

  var res = str;
  var pos = size_t(0);
  var match = std::smatch();
  let rx = std::regex("\\{(\\w+)(?:(:\\d\\.\\d.?)?)\\}");

  while (std::regex_search(str, match, rx))
  {
    var m = std::string(match[0]);
    var first = std::string(match[1]);
    pos += std::string(match.prefix()).size();

    if (args.find(first) == args.end())
    {
      pos += m.size();
      res = res.substr(m.size());
      continue;
    }

    str.replace(pos, m.size(), args[first]);
    pos += args[first].size();
    res = match.suffix();
  }

  return res;
}

} // namespace String

#undef var
#undef let
#undef fn

} // namespace OB
