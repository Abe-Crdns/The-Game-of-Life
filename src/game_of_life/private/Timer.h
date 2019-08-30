#ifndef _TIMER_H
#define _TIMER_H

#include <chrono>

typedef std::chrono::steady_clock::time_point time_pt_t;

class Timer {
  private:
    time_pt_t m_tpStart;

  public:
    void Start();
    void Reset();
    bool WasStarted();
    double GetDuration();
};

#endif

