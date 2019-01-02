#ifndef OB_SCOPED_MAP_HH
#define OB_SCOPED_MAP_HH

#include <cstddef>

#include <deque>
#include <vector>
#include <utility>
#include <unordered_map>
#include <initializer_list>

namespace OB
{

template<class K, class V>
class Scoped_Map
{
public:

  // map iterators
  using m_iterator = typename std::unordered_map<K, V>::iterator;
  using m_const_iterator = typename std::unordered_map<K, V>::const_iterator;

  // index iterators
  using i_iterator = typename std::deque<m_iterator>::iterator;
  using i_const_iterator = typename std::deque<m_const_iterator>::const_iterator;


  Scoped_Map()
  {
    add_scope();
  }

  Scoped_Map(std::initializer_list<std::pair<K, V>> const& lst)
  {
    add_scope();
    for (auto const& e : lst)
    {
      _it.back().emplace_back(_map.insert({e.first, e.second}).first);
    }
  }

  ~Scoped_Map()
  {
  }

  m_iterator begin()
  {
    return _map.begin();
  }

  m_const_iterator begin() const
  {
    return _map.begin();
  }

  m_const_iterator cbegin() const
  {
    return _map.cbegin();
  }

  m_iterator end()
  {
    return _map.end();
  }

  m_const_iterator end() const
  {
    return _map.end();
  }

  m_const_iterator cend() const
  {
    return _map.cend();
  }

  std::pair<m_iterator, bool> insert_or_assign(K const& k, V&& v)
  {
    auto it = _map.insert_or_assign(k, v);

    if (it.second)
    {
      _it.back().emplace_back(it.first);
    }

    return it;
  }

  Scoped_Map& operator()(K const& k, V const& v)
  {
    auto it = _map.insert_or_assign(k, v);

    if (it.second)
    {
      _it.back().emplace_back(it.first);
    }

    return *this;
  }

  V& at(K const& k)
  {
    return _map.at(k);
  }

  V const& at(K const& k) const
  {
    return _map.at(k);
  }

  m_iterator operator[](K const& k)
  {
    return _map.find(k);
  }

  m_const_iterator operator[](K const& k) const
  {
    return _map.find(k);
  }

  m_iterator find(K const& k)
  {
    return _map.find(k);
  }

  m_const_iterator find(K const& k) const
  {
    return _map.find(k);
  }

  Scoped_Map& clear()
  {
    _map.clear();
    _it = {};
    add_scope();
    return *this;
  }

  bool empty() const
  {
    return _map.empty();
  }

  std::size_t size() const
  {
    return _map.size();
  }

  std::size_t scope() const
  {
    return _it.size();
  }

  Scoped_Map& erase(K const& k)
  {
    auto it = _map.find(k);
    if (it != _map.end())
    {
      for (auto v = _it.rbegin(); v != _it.rend(); ++v)
      {
        for (auto e = v->begin(); e < v->end(); ++e)
        {
          if ((*e) == it)
          {
            v->erase(e);
            break;
          }
        }
      }
      _map.erase(it);
    }
    return *this;
  }

  Scoped_Map& add_scope()
  {
    _it.emplace_back(std::deque<m_iterator>());
    return *this;
  }

  Scoped_Map& rm_scope()
  {
    if (_it.size() > 1)
    {
      for (auto& e : _it.back())
      {
        _map.erase(e);
      }
      _it.erase(_it.end() - 1);
    }
    return *this;
  }

private:

  std::unordered_map<K, V> _map;
  std::vector<std::deque<m_iterator>> _it;
}; // class Scoped_Map

} // namespace OB

#endif // OB_SCOPED_MAP_HH
