#ifndef OB_STRING_HH
#define OB_STRING_HH

#include <string>
#include <vector>
#include <unordered_map>

#define var auto
#define let auto const
#define fn auto

namespace OB
{

namespace String
{

fn repeat(std::string const& str, size_t num)
-> std::string;

fn count(std::string const& str, std::string const& val)
-> size_t;

fn escape(std::string str)
-> std::string;

fn unescape(std::string str)
-> std::string;

fn replace_first(std::string str, std::string const& key, std::string const& val)
-> std::string;

fn replace_last(std::string str, std::string const& key, std::string const& val)
-> std::string;

fn replace_all(std::string str, std::string const& key, std::string const& val)
-> std::string;

fn delimit(std::string const& str, std::string const& delim)
-> std::vector<std::string>;

fn delimit_first(std::string const& str, std::string const& delim)
-> std::vector<std::string>;

fn format(std::string str, std::unordered_map<std::string, std::string> args)
-> std::string;

fn xformat(std::string str, std::unordered_map<std::string, std::string> args)
-> std::string;

fn correct(std::string const& str, std::vector<std::string> const& lst)
-> std::vector<std::string>;

fn starts_with(std::string const& str, std::string const& val)
-> bool;

fn ends_with(std::string const& str, std::string const& val)
-> bool;

fn uppercase(std::string const& str)
-> std::string;

fn lowercase(std::string const& str)
-> std::string;

} // namespace String

} // namespace OB

#undef var
#undef let
#undef fn

#endif // OB_STRING_HH
