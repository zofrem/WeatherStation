#include <Arduino.h>
#include "LoopTimer.h"

bool LoopTimer::timer()
{
  if(mFirstRun)
  {
    resetTimer();
    mFirstRun = false;
    return true;
  }
  uint32_t now = millis();
  if(now >= (mLastProceedTime + mDelay))
  {
    resetTimer();
    return true; 
  } 
  return false;
}

bool LoopTimer::resetTimer()
{
  mLastProceedTime = millis();
  return true;
}
