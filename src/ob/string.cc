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
#include <utility>
#include <algorithm>
#include <map>

namespace OB
{

namespace String
{

std::vector<std::string> split(std::string const& str, std::string const& delim, std::size_t size)
{
  std::vector<std::string> vtok;
  std::size_t start {0};
  auto end = str.find(delim);

  while ((size-- > 0) && (end != std::string::npos))
  {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.size();
    end = str.find(delim, start);
  }

  vtok.emplace_back(str.substr(start, end));

  return vtok;
}

std::string plural(std::string const& str, std::size_t num)
{
  if (num == 1)
  {
    return str;
  }

  return str + "s";
}

std::string plural(std::string const& str, std::string const& end, std::size_t num)
{
  if (num == 1)
  {
    return str;
  }

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
  if (num < 2)
  {
    return str;
  }

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

    if (pos == std::string::npos)
    {
      break;
    }

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

    if (pos == std::string::npos)
    {
      break;
    }

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

    if (pos == std::string::npos || pos + 1 == std::string::npos)
    {
      break;
    }

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

  if (pos == std::string::npos)
  {
    return str;
  }

  str.replace(pos, key.size(), val);

  return str;
}

std::string replace_last(std::string str, std::string const& key, std::string const& val)
{
  std::size_t pos {0};
  pos = str.rfind(key);

  if (pos == std::string::npos)
  {
    return str;
  }

  str.replace(pos, key.size(), val);

  return str;
}

std::string replace_all(std::string str, std::string const& key, std::string const& val)
{
  std::size_t pos {0};

  for (;;)
  {
    pos = str.find(key, pos);

    if (pos == std::string::npos)
    {
      break;
    }

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

  while (end != std::string::npos)
  {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.size();
    end = str.find(delim, start);
  }

  vtok.emplace_back(str.substr(start, end));

  return vtok;
}

std::vector<std::string> delimit_first(std::string const& str, std::string const& delim)
{
  std::vector<std::string> vtok;
  std::size_t pos = str.find(delim);

  if (pos != std::string::npos)
  {
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

  if (pos != std::string::npos)
  {
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

    if (pos == std::string::npos)
    {
      break;
    }

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
  auto const to_upper = [](char& c)
  {
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
  auto const to_lower = [](char& c)
  {
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

  if (str.compare(0, val.size(), val) == 0)
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

  if (str.compare(str.size() - val.size(), val.size(), val) == 0)
  {
    return true;
  }

  return false;
}

std::vector<std::string> correct(std::string const& str, std::vector<std::string> const& lst)
{
  std::vector<std::string> matches;

  std::size_t len = (str.size() / 1.5);
  if (len < 2)
  {
    len = str.size();
  }

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
  std::string rx {"^.*[" + estr.str() + "]{" + std::to_string(len) + ",}.*$"};

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

std::vector<std::string> bayes(std::vector<std::string> const& list, std::string str)
{
  std::map<std::string, std::string> dict;
  for (auto const& e : list)
  {
    dict[lowercase(e)] = e;
  }

  auto const known = [&dict](auto& res, auto& candidates)
  {
    for (auto const& e : res)
    {
      if (auto const it = dict.find(e); it != dict.end())
      {
        candidates.emplace_back(it->second);
      }
    }
  };

  auto const edits = [](auto const& str, auto& res)
  {
    // deletion
    for (std::size_t i = 0; i < str.size(); ++i)
    {
      res.emplace_back(str.substr(0, i) + str.substr(i + 1));
    }

    // transposition
    for (std::size_t i = 0; i < str.size() - 1; ++i)
    {
      res.emplace_back(str.substr(0, i) + str.at(i + 1) + str.at(i) + str.substr(i + 2));
    }

    std::string const chars {"abcdefghijklmnopqrstuvwxyz"};
    for (auto const& e : chars)
    {
      // substitution
      for (std::size_t i = 0; i < str.size(); ++i)
      {
        res.emplace_back(str.substr(0, i) + e + str.substr(i + 1));
      }

      // insertion
      for (std::size_t i = 0; i < str.size() + 1; ++i)
      {
        res.emplace_back(str.substr(0, i) + e + str.substr(i));
      }
    }
  };

  std::vector<std::string> res;
  std::vector<std::string> candidates;

  str = lowercase(str);

  if (dict.find(str) != dict.end())
  {
    candidates.emplace_back(str);
    return candidates;
  }

  edits(str, res);
  known(res, candidates);

  if (candidates.empty())
  {
    std::vector<std::string> sres;

    for (auto const& e : res)
    {
      edits(e, sres);
      known(sres, candidates);
    }
  }

  return candidates;
}

std::size_t levenshtein(std::string const& lhs, std::string const& rhs)
{
  std::size_t weight_del {3};
  std::size_t weight_ins {1};
  std::size_t weight_sub {2};

  std::vector<std::size_t> v1 (rhs.size() + 1, 0);
  std::vector<std::size_t> v2 (rhs.size() + 1, 0);

  for (std::size_t i = 0; i <= rhs.size(); ++i)
  {
    v1.at(i) = i * weight_ins;
  }

  for (std::size_t i = 0; i < lhs.size(); ++i)
  {
    v2.at(0) = (i + 1) * weight_del;
    for (std::size_t j = 0; j < rhs.size(); j++)
    {
      v2.at(j + 1) = std::min(
        // deletion
        v1.at(j + 1) + weight_del,
        // insertion
        std::min(
          v2.at(j) + weight_ins,
          // substitution
          v1.at(j) + (weight_sub * (lhs.at(i) != rhs.at(j)))));
    }

    std::swap(v1, v2);
  }

  return v1.at(rhs.size());
}

std::size_t damerau_levenshtein(std::string const& lhs, std::string const& rhs)
{
  std::size_t weight_del {3};
  std::size_t weight_ins {1};
  std::size_t weight_sub {2};
  std::size_t weight_swp {0};

  std::vector<std::size_t> v0 (rhs.size() + 1, 0);
  std::vector<std::size_t> v1 (rhs.size() + 1, 0);
  std::vector<std::size_t> v2 (rhs.size() + 1, 0);

  for (std::size_t i = 0; i <= rhs.size(); ++i)
  {
    v1.at(i) = i * weight_ins;
  }

  for (std::size_t i = 0; i < lhs.size(); ++i)
  {
    v2.at(0) = (i + 1) * weight_del;
    for (std::size_t j = 0; j < rhs.size(); j++)
    {
      v2.at(j + 1) = std::min(
        // deletion
        v1.at(j + 1) + weight_del,
        // insertion
        std::min(
          v2.at(j) + weight_ins,
          // substitution
          v1.at(j) + (weight_sub * (lhs.at(i) != rhs.at(j)))));

      if (i && j &&
        (lhs.at(i - 1) == rhs.at(j)) &&
        (lhs.at(i) == rhs.at(j - 1)))
      {
        v2.at(j + 1) = std::min(
          v0.at(j + 1),
          // swap
          v0.at(j - 1) + weight_swp);
      }
    }

    std::swap(v0, v1);
    std::swap(v1, v2);
  }

  return v1.at(rhs.size());
}

// std::size_t levenshtein(std::string lhs, std::string rhs)
// {
//   std::vector<std::vector<std::size_t>> d {lhs.size() + 1, std::vector<std::size_t>(rhs.size() + 1)};

//   for (std::size_t i = 0; i <= lhs.size(); ++i)
//   {
//     d.at(i).at(0) = i;
//   }

//   for (std::size_t i = 1; i <= rhs.size(); ++i)
//   {
//     d.at(0).at(i) = i;
//   }

//   for (std::size_t i = 1; i <= lhs.size(); ++i)
//   {
//     for (std::size_t j = 1; j <= rhs.size(); ++j)
//     {
//       std::size_t dist {0};

//       if (lhs.at(i - 1) != rhs.at(j - 1))
//       {
//         dist = 1;
//       }

//       d.at(i).at(j) = std::min(
//         // delete
//         d.at(i-1).at(j) + 1,
//         // insert
//         std::min(d.at(i).at(j-1) + 1,
//         // substitute
//         d.at(i-1).at(j-1) + dist));

//       if ((i > 1) && (j > 1) &&
//         (lhs.at(i-1) == rhs.at(j-2)) &&
//         (lhs.at(i-2) == rhs.at(j-1)))
//       {
//         d.at(i).at(j) = std::min(
//           d.at(i).at(j),
//           // transpose
//           d.at(i-2).at(j-2) + dist);
//       }
//     }
//   }

//   return d.at(lhs.size()).at(rhs.size());
// }

// std::vector<std::vector<char>> const alphakey
// {
//   {'q','w','e','r','t','y','u','i','o','p'},
//   {'a','s','d','f','g','h','j','k','l'},
//   {'z','x','c','v','b','n','m'},
// };

// std::map<char, std::string> const alphakey
// {
//   {'q', "was"},
//   {'a', "qwsxz"},
//   {'z', "asx"},
//   {'w', "edsaq"},
//   {'s', "wedcxzaq"},
//   {'x', "sdcza"},
//   {'e', "rfdsw"},
//   {'d', "erfvcxsw"},
//   {'c', "dfvxs"},
//   {'r', "tgfde"},
//   {'f', "rtgbvcde"},
//   {'v', "fgbcd"},
//   {'t', ""},
//   {'g', ""},
//   {'b', ""},
//   {'', ""},
// };

// std::size_t levenshtein(std::string const& lhs, std::string const& rhs, std::size_t w, std::size_t s, std::size_t a, std::size_t d)
// {
//   std::vector<std::size_t> v0 (rhs.size() + 1, 0);
//   std::vector<std::size_t> v1 (rhs.size() + 1, 0);
//   std::vector<std::size_t> v2 (rhs.size() + 1, 0);

//   for (std::size_t i = 0; i <= rhs.size(); ++i)
//   {
//     v1.at(i) = i;
//   }

//   for (std::size_t i = 0; i < lhs.size(); ++i)
//   {
//     v2.at(0) = (i + 1) * d;
//     for (std::size_t j = 0; j < rhs.size(); j++)
//     {
//       v2.at(j + 1) = std::min(
//         // deletion
//         v1.at(j + 1) + d,
//         // insertion
//         std::min(
//           v2.at(j) + a,
//           // substitution
//           v1.at(j) + (s * (lhs.at(i) != rhs.at(j)))));

//       if (i && j &&
//         (lhs.at(i - 1) == rhs.at(j)) &&
//         (lhs.at(i) == rhs.at(j - 1)))
//       {
//         v2.at(j + 1) = std::min(
//           v0.at(j + 1),
//           // transposition
//           v0.at(j - 1) + w);
//       }

//       // substitution
//       // v2.at(j + 1) = v1.at(j) + s * (lhs.at(i) != rhs.at(j));

//       // swap
//       // if (i > 0 && j > 0 && (lhs.at(i - 1) == rhs.at(j)) &&
//       //   (lhs.at(i) == rhs.at(j - 1)) && (v2.at(j + 1) > (v0.at(j - 1) + w)))
//       // {
//       //   v2.at(j + 1) = v0.at(j - 1) + w;
//       // }

//       // deletion
//       // if (v2.at(j + 1) > (v1.at(j + 1) + d))
//       // {
//       //   v2.at(j + 1) = v1.at(j + 1) + d;
//       // }

//       // insertion
//       // if (v2.at(j + 1) > (v2.at(j) + a))
//       // {
//       //   v2.at(j + 1) = v2.at(j) + a;
//       // }
//     }

//     std::swap(v0, v1);
//     std::swap(v1, v2);
//   }

//   return v1.at(rhs.size());
// }

} // namespace String

} // namespace OB
