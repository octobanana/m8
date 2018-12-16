#ifndef OB_ORDERED_MAP_HH
#define OB_ORDERED_MAP_HH

#include <cstddef>

#include <deque>
#include <utility>
#include <unordered_map>
#include <initializer_list>

namespace OB
{

template<class K, class V>
class Ordered_Map
{
public:
  // map iterators
  using m_iterator = typename std::unordered_map<K, V>::iterator;
  using m_const_iterator = typename std::unordered_map<K, V>::const_iterator;

  // index iterators
  using i_iterator = typename std::deque<m_iterator>::iterator;
  using i_const_iterator = typename std::deque<m_const_iterator>::const_iterator;


  Ordered_Map()
  {
  }

  Ordered_Map(std::initializer_list<std::pair<K, V>> const& lst)
  {
    for (auto const& e : lst)
    {
      _it.emplace_back(_map.insert({e.first, e.second}).first);
    }
  }

  ~Ordered_Map()
  {
  }

  Ordered_Map& operator()(K const& k, V const& v)
  {
    auto it = _map.insert_or_assign(k, v);

    if (it.second)
    {
      _it.emplace_back(it.first);
    }

    return *this;
  }

  i_iterator begin()
  {
    return _it.begin();
  }

  i_const_iterator begin() const
  {
    return _it.begin();
  }

  i_const_iterator cbegin() const
  {
    return _it.cbegin();
  }

  i_iterator end()
  {
    return _it.end();
  }

  i_const_iterator end() const
  {
    return _it.end();
  }

  i_const_iterator cend() const
  {
    return _it.cend();
  }

  m_iterator map_begin()
  {
    return _map.begin();
  }

  m_const_iterator map_begin() const
  {
    return _map.begin();
  }

  m_const_iterator map_cbegin() const
  {
    return _map.begin();
  }

  m_iterator map_end()
  {
    return _map.end();
  }

  m_const_iterator map_end() const
  {
    return _map.end();
  }

  m_const_iterator map_cend() const
  {
    return _map.end();
  }

  i_iterator operator[](std::size_t index)
  {
    return _it[index];
  }

  i_const_iterator const operator[](std::size_t index) const
  {
    return _it[index];
  }

  m_iterator operator[](K const& k)
  {
    return _map.find(k);
  }

  m_const_iterator operator[](K const& k) const
  {
    return _map.find(k);
  }

  std::size_t size() const
  {
    return _it.size();
  }

  Ordered_Map& clear()
  {
    _it.clear();
    _map.clear();
    return *this;
  }

  Ordered_Map& rm(K const& k)
  {
    auto it = _map.find(k);
    if (it != _map.end())
    {
      _map.erase(it);
      for (auto e = _it.begin(); e < _it.end(); ++e)
      {
        if ((*e) == it)
        {
          _it.erase(e);
          break;
        }
      }
    }
    return *this;
  }

private:
  std::unordered_map<K, V> _map;
  std::deque<m_iterator> _it;

}; // class Ordered_Map

} // namespace OB

#endif // OB_ORDERED_MAP_HH
