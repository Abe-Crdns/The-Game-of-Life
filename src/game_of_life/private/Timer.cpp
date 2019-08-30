#ifndef _TIMER_CPP
#define _TIMER_CPP

#include "Timer.h"

void Timer::Start(){ 
  m_tpStart = std::chrono::high_resolution_clock::now(); 
}

bool Timer::WasStarted(){ 
  return m_tpStart.time_since_epoch().count(); 
}

void Timer::Reset(){
  time_pt_t tmp;
  m_tpStart = tmp;
}

double Timer::GetDuration(){
  std::chrono::duration<double> duration = 
    std::chrono::high_resolution_clock::now() - m_tpStart;

  return duration.count();
}

#endif