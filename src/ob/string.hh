#ifndef OB_STRING_HH
#define OB_STRING_HH

#include <cstddef>

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <regex>
#include <limits>
#include <utility>
#include <map>

namespace OB
{

namespace String
{

std::vector<std::string> split(std::string const& str, std::string const& delim, std::size_t size = std::numeric_limits<std::size_t>::max());

std::string plural(std::string const& str, std::size_t num);

std::string plural(std::string const& str, std::string const& end, std::size_t num);

bool assert_rx(std::string str, std::regex rx);

std::optional<std::vector<std::string>> match(std::string const& str, std::regex rx);

std::string repeat(std::string const& str, std::size_t num);

std::size_t count(std::string const& str, std::string const& val);

std::string escape(std::string str);

std::string unescape(std::string str);

std::string replace_first(std::string str, std::string const& key, std::string const& val);

std::string replace_last(std::string str, std::string const& key, std::string const& val);

std::string replace_all(std::string str, std::string const& key, std::string const& val);

std::vector<std::string> delimit(std::string const& str, std::string const& delim);

std::vector<std::string> delimit_first(std::string const& str, std::string const& delim);

std::pair<std::string, std::string> delimit_pair(std::string const& str, std::string const& delim);

std::string format(std::string str, std::unordered_map<std::string, std::string> args);

std::string xformat(std::string str, std::unordered_map<std::string, std::string> args);

std::string trim(std::string str);

std::string sanitize_html(std::string str);

std::string sanitize_sql(std::string str);

std::string sanitize_query(std::string str);

std::string file(std::string const& str);

std::string uppercase(std::string const& str);

std::string lowercase(std::string const& str);

bool starts_with(std::string const& str, std::string const& val);

bool ends_with(std::string const& str, std::string const& val);

std::vector<std::string> correct(std::string const& str, std::vector<std::string> const& lst);

std::vector<std::string> bayes(std::vector<std::string> const& list, std::string str);

std::size_t levenshtein(std::string const& lhs, std::string const& rhs);

std::size_t damerau_levenshtein(std::string const& lhs, std::string const& rhs);

} // namespace String

} // namespace OB

#endif // OB_STRING_HH
