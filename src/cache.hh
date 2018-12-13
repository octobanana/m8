#ifndef OB_CACHE_HH
#define OB_CACHE_HH

#define var auto
#define let auto const
#define fn auto

#include <boost/serialization/map.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>

#include <filesystem>
namespace fs = std::filesystem;

namespace OB
{
class Cache
{
public:
  Cache(std::string file_name = {}):
  file_name_ {file_name}
  {
    var path = fs::path(file_name_);
    var filename = std::string(path.filename());
    if (filename.empty())
    {
      filename = "readline";
    }
    if (filename.at(0) == '.')
    {
      file_lock_ = "./.m8/" + filename + ".lock.m8";
      file_cache_ = "./.m8/" + filename + ".cache.m8";
    }
    else
    {
      file_lock_ = "./.m8/." + filename + ".lock.m8";
      file_cache_ = "./.m8/." + filename + ".cache.m8";
    }
    lock();
    load();
  }

  ~Cache()
  {
    save();
    unlock();
  }

  fn lock()
  -> void
  {
    if (fs::exists(file_lock_))
    {
      throw std::runtime_error("cache lock file already in use");
    }
    var file_lock = std::ofstream(file_lock_);
    file_lock.close();
  }

  fn unlock()
  -> void
  {
    fs::remove(file_lock_);
  }

  fn load()
  -> void
  {
    if (! fs::exists(file_cache_)) return;
    var icache = std::ifstream(file_cache_, std::ios::binary);
    boost::archive::binary_iarchive iarch(icache);
    iarch >> db_;
  }

  fn save()
  -> void
  {
    var ocache = std::ofstream(file_cache_, std::ios::binary);
    boost::archive::binary_oarchive oarch(ocache);
    oarch << db_;
  }

  fn get(std::string const& key, std::string& val)
  -> bool
  {
    if (db_.find(key) == db_.end())
    {
      // std::cout << "Cache: not available\n";
      return false;
    }
    val = db_[key];
    // std::cout << "Cache: available\n";
    return true;
  }

  fn set(std::string const& key, std::string const& val)
  -> void
  {
    db_[key] = val;
  }

private:
  std::string const file_name_;
  std::string file_lock_;
  std::string file_cache_;
  std::map<std::string, std::string> db_;

}; // class Cache
} // namespace OB

#undef var
#undef let
#undef fn
#endif // OB_CACHE_HH
