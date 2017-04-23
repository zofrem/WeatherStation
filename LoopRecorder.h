#ifndef LOOPRECORDER_H
#define LOOPRECORDER_H

#include <inttypes.h>

/////////////////////////////////////////////////////////
/// LoopRecorder 
/// infinity loop memory like flight recorder(black box)
/////////////////////////////////////////////////////////

template <typename T>
class LoopRecorder
{  
public:
  LoopRecorder(const unsigned int size) : mMaxMemoryIndex(size - 1)        //size is compared as index - 1
  {
    mIndex = 0;
    mEndOfMemory = false;          
    if(size <= 256)                //support array up to 256 uint8_t + 1 = int require
      mLoopMemory = new T[size];    
  };

  ~LoopRecorder()
  {
    if(mLoopMemory)
      delete(mLoopMemory);
  }; 

  void pushBack(const T storedValue)
  {
    mLoopMemory[mIndex] = storedValue;
    if(mIndex >= mMaxMemoryIndex)
    {
      mIndex = 0;
      mEndOfMemory = true;
    }
    else
    {
      ++mIndex;   
    }
  };

  bool getLastSample(uint8_t pastTimeSample, T& returnValue) const
  {
    //pastTimeSample = 0 => present, pastTimeSample = 1 => sample from time - 1,... up to 255
    if(!mLoopMemory)                       
      return false;
    if(mIndex == 0 && !mEndOfMemory)                         
      return false;
    if(pastTimeSample > mMaxMemoryIndex)                     //more than mLoopMemory could store
      return false;
    if(!mEndOfMemory && pastTimeSample >= mIndex)            //time before recording to mLoopMemory
      return false;
    //calculate index of required sample  
    uint8_t indexAtPast = mIndex - pastTimeSample - 1;       //correct value except of below handled case
    if(mEndOfMemory && pastTimeSample >= mIndex)             //case when is require take from previous memory cycle
    {
      uint8_t previousCycleIndex = pastTimeSample - mIndex;  //how deep is index from the end of the memory
      indexAtPast = mMaxMemoryIndex - previousCycleIndex;    //for this case set correct indexAtPast respect to allocated memory size
    }
    returnValue = mLoopMemory[indexAtPast];   
    return true;    
  }
 
private:
  LoopRecorder(const LoopRecorder& paste);
  const uint8_t mMaxMemoryIndex;
  uint8_t mIndex;
  bool mEndOfMemory;
  T* mLoopMemory;
};

#endif

