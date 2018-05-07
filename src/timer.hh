#ifndef OB_TIMER_HH
#define OB_TIMER_HH

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>

namespace OB
{

class Timer
{
public:
  Timer()
  {
  }

  ~Timer()
  {
  }

  void start()
  {
    _start = std::chrono::high_resolution_clock::now();
  }

  void stop()
  {
    _stop = std::chrono::high_resolution_clock::now();
  }

  template<class T = std::chrono::nanoseconds>
  long int time()
  {
    return std::chrono::duration_cast<T>(_stop - _start).count();
  }

  long int time()
  {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(_stop - _start).count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  std::chrono::time_point<std::chrono::high_resolution_clock> _stop;

}; // class Timer

} // namespace OB

#endif // OB_TIMER_HH
