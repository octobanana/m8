#ifndef M8_CACHE_HH
#define M8_CACHE_HH

#include <boost/serialization/map.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>

class Cache
{
  namespace fs = std::filesystem;

public:

  Cache(std::string file_name {}) :
  file_name_ {file_name}
  {
    fs::path path {file_name_};
    std::string filename {path.filename()};

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

  bool get(std::string const& key, std::string& val)
  {
    if (db_.find(key) == db_.end())
    {
      return false;
    }
    val = db_[key];
    return true;
  }

  void set(std::string const& key, std::string const& val)
  {
    db_[key] = val;
  }

private:

  void lock()
  {
    if (fs::exists(file_lock_))
    {
      throw std::runtime_error("cache lock file already in use");
    }
    std::ofstream file_lock {file_lock_};
    file_lock.close();
  }

  void unlock()
  {
    fs::remove(file_lock_);
  }

  void load()
  {
    if (! fs::exists(file_cache_)) return;
    std::ifstream icache {file_cache_, std::ios::binary};
    boost::archive::binary_iarchive iarch(icache);
    iarch >> db_;
  }

  void save()
  {
    std::ofstream ocache {file_cache_, std::ios::binary};
    boost::archive::binary_oarchive oarch(ocache);
    oarch << db_;
  }

  std::string const file_name_;
  std::string file_lock_;
  std::string file_cache_;
  std::map<std::string, std::string> db_;
}; // class Cache

#endif // M8_CACHE_HH
