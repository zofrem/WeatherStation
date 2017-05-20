#ifndef LOOPTIMER_H
#define LOOPTIMER_H

#include <inttypes.h>

/////////////////////////////////////////////////////////
/// LoopTimer
/// primitive cron timer
/////////////////////////////////////////////////////////

class LoopTimer {
  public:
    LoopTimer(const uint32_t delay) : mDelay(delay)
    {
      mFirstRun = true;
    }; 
    bool timer();
    bool resetTimer();
  
  private:
    LoopTimer(const LoopTimer& timer);
    uint32_t mLastProceedTime;
    uint32_t mDelay;
    bool mFirstRun;
};

#endif
