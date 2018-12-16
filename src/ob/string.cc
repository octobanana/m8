#include "ob/string.hh"

#include <cstddef>

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <optional>
#include <limits>

namespace OB
{

namespace String
{

std::vector<std::string> split(std::string const& str, std::string const& delim, std::size_t times)
{
  std::vector<std::string> vtok;
  std::size_t start {0};
  auto end = str.find(delim);

  while ((times-- > 0) && (end != std::string::npos))
  {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.length();
    end = str.find(delim, start);
  }
  vtok.emplace_back(str.substr(start, end));

  return vtok;
}

std::string plural(std::string const& str, std::size_t num)
{
  if (num == 1) return str;
  return str + "s";
}

std::string plural(std::string const& str, std::string const& end, std::size_t num)
{
  if (num == 1) return str;
  return str + end;
}

bool assert_rx(std::string str, std::regex rx)
{
  std::smatch m;
  if (std::regex_match(str, m, rx, std::regex_constants::match_not_null))
  {
    return true;
  }
  return false;
}

std::optional<std::vector<std::string>> match(std::string const& str, std::regex rx)
{
  std::smatch m;
  if (std::regex_match(str, m, rx, std::regex_constants::match_not_null))
  {
    std::vector<std::string> v;
    for (auto const& e : m)
    {
      v.emplace_back(std::string(e));
    }
    return v;
  }
  return {};
}

std::string repeat(std::string const& str, std::size_t num)
{
  if (num < 2) return str;
  std::string res;
  res.reserve(str.size() * num);
  for (std::size_t i = 0; i < num; ++i)
  {
    res += str;
  }
  return res;
}

std::size_t count(std::string const& str, std::string const& val)
{
  std::size_t pos {0};
  std::size_t count {0};
  for (;;)
  {
    pos = str.find(val, pos);
    if (pos == std::string::npos) break;
    ++count;
    ++pos;
  }
  return count;
}

std::string escape(std::string str)
{
  std::size_t pos {0};
  for (;; ++pos)
  {
    pos = str.find_first_of("\n\t\r\a\b\f\v\\\"\'\?", pos);
    if (pos == std::string::npos) break;
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

std::string unescape(std::string str)
{
  std::size_t pos {0};
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

std::string replace_first(std::string str, std::string const& key, std::string const& val)
{
  std::size_t pos {0};
  pos = str.find(key);
  if (pos == std::string::npos) return str;
  str.replace(pos, key.size(), val);
  return str;
}

std::string replace_last(std::string str, std::string const& key, std::string const& val)
{
  std::size_t pos {0};
  pos = str.rfind(key);
  if (pos == std::string::npos) return str;
  str.replace(pos, key.size(), val);
  return str;
}

std::string replace_all(std::string str, std::string const& key, std::string const& val)
{
  std::size_t pos {0};
  for (;;)
  {
    pos = str.find(key, pos);
    if (pos == std::string::npos) break;
    str.replace(pos, key.size(), val);
    pos += val.size();
  }
  return str;
}

std::vector<std::string> delimit(std::string const& str, std::string const& delim)
{
  std::vector<std::string> vtok;
  std::size_t start {0};
  std::size_t end = str.find(delim);
  while (end != std::string::npos) {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.length();
    end = str.find(delim, start);
  }
  vtok.emplace_back(str.substr(start, end));
  return vtok;
}

std::vector<std::string> delimit_first(std::string const& str, std::string const& delim)
{
  std::vector<std::string> vtok;
  std::size_t pos = str.find(delim);
  if (pos != std::string::npos) {
    vtok.emplace_back(str.substr(0, pos));
    if (pos + delim.size() != std::string::npos)
    {
      vtok.emplace_back(str.substr(pos + delim.size()));
    }
  }
  return vtok;
}

std::pair<std::string, std::string> delimit_pair(std::string const& str, std::string const& delim)
{
  std::pair<std::string, std::string> ptok {"", ""};
  std::size_t pos = str.find(delim);
  if (pos != std::string::npos) {
    ptok.first = str.substr(0, pos);
    if (pos + delim.size() != std::string::npos)
    {
      ptok.second = str.substr(pos + delim.size());
    }
  }
  return ptok;
}

std::string format(std::string str, std::unordered_map<std::string, std::string> args)
{
  if (args.empty())
  {
    return str;
  }

  std::string res = str;
  std::size_t pos {0};
  std::smatch match;
  std::regex const rx {"\\{([^\\:]+?)(?:(:[^\\r]*?)?)\\}"};

  while (std::regex_search(res, match, rx, std::regex_constants::match_not_null))
  {
    std::string m {match[0]};
    std::string first {match[1]};
    pos += std::string(match.prefix()).size();

    if (args.find(first) == args.end())
    {
      ++pos;
      res = res.substr(std::string(match.prefix()).size() + 1);
      continue;
    }

    str.replace(pos, m.size(), args[first]);
    pos += args[first].size();
    res = match.suffix();
  }

  return str;
}

std::string xformat(std::string str, std::unordered_map<std::string, std::string> args)
{
  if (args.empty())
  {
    return str;
  }

  std::string res = str;
  std::size_t pos {0};
  std::smatch match;
  std::regex const rx {"\\{(\\w+)(?:(:[^\\r]*?:\\1)?)\\}"};

  while (std::regex_search(res, match, rx, std::regex_constants::match_not_null))
  {
    bool is_simple {true};
    std::string m {match[0]};
    std::string first {match[1]};
    std::string second {match[2]};
    pos += std::string(match.prefix()).size();

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
      std::regex rx {"^:([*]{1})(.+)([a-z0-9]{1}):([^\\r]+):" + first + "$"};
      if (std::regex_match(second, match_complex, rx, std::regex_constants::match_not_null))
      {
        std::string type {match_complex[1]};
        std::string delim {match_complex[2]};
        std::string index_char {match_complex[3]};
        std::string rstr {match_complex[4]};

        auto vec = String::delimit(args[first], delim);
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

std::vector<std::string> correct(std::string const& str, std::vector<std::string> const& lst)
{
  std::vector<std::string> matches;

  auto const len = (str.size() / 1.2);
  std::stringstream estr;
  for (auto const& e : str)
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

  for (auto const& e : lst)
  {
    std::smatch m;
    if (std::regex_match(e, m, std::regex(rx, std::regex::icase), std::regex_constants::match_not_null))
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

std::string trim(std::string str)
{
  auto start = str.find_first_not_of(" \t\n\r\f\v");
  if (start != std::string::npos)
  {
    auto end = str.find_last_not_of(" \t\n\r\f\v");
    str = str.substr(start, end - start + 1);
    return str;
  }
  return {};
}

std::string sanitize_html(std::string str)
{
  std::string val;
  std::string chars {"&<>'\""};
  std::size_t pos {0};
  for (;;)
  {
    pos = str.find_first_of(chars, pos);
    if (pos == std::string::npos) break;
    switch (str.at(pos))
    {
      case '&':
        val = "&amp;";
        break;
      case '<':
        val = "&lt;";
        break;
      case '>':
        val = "&gt;";
        break;
      case '\'':
        val = "&#39;";
        break;
      case '"':
        val = "&quot;";
        break;
      default:
        val = str.at(pos);
        break;
    }
    str.replace(pos, 1, val);
    pos += val.size();
  }
  return str;
}

std::string sanitize_sql(std::string str)
{
  str = replace_all(str, "'", "''");
  return str;
}

std::string sanitize_query(std::string str)
{
  str = replace_all(str, "'", "''");
  str = replace_all(str, "\"", "\"\"");
  return str;
}

std::string file(std::string const& str)
{
  std::ifstream file {str};
  file.seekg(0, std::ios::end);
  std::size_t size (static_cast<std::size_t>(file.tellg()));
  std::string content (size, ' ');
  file.seekg(0);
  file.read(&content[0], static_cast<std::streamsize>(size));
  return content;
}

std::string uppercase(std::string const& str)
{
  auto const to_upper = [](char& c) {
    if (c >= 'a' && c <= 'z')
    {
      c += 'A' - 'a';
    }
    return c;
  };

  std::string s {str};
  for (char& c : s)
  {
    c = to_upper(c);
  }
  return s;
}

std::string lowercase(std::string const& str)
{
  auto const to_lower = [](char& c) {
    if (c >= 'A' && c <= 'Z')
    {
      c += 'a' - 'A';
    }
    return c;
  };

  std::string s {str};
  for (char& c : s)
  {
    c = to_lower(c);
  }
  return s;
}

bool starts_with(std::string const& str, std::string const& val)
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

bool ends_with(std::string const& str, std::string const& val)
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

} // namespace OB
