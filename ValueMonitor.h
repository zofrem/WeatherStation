#ifndef VALUEMONITOR_H
#define VALUEMONITOR_H

#include <inttypes.h>
#include <limits>
#include "LoopRecorder.h"

/////////////////////////////////////////////////////////
/// ValueMonitor
/// handling monitoring analising physical values 
/////////////////////////////////////////////////////////

class ValueMonitor {
  public:
    ValueMonitor(const uint8_t size) : mMonitorSize(size), mBiggest(std::numeric_limits<float>::max()), mSmallest(std::numeric_limits<float>::min())
    {
      mValuesCount = 0;
      mValues = new LoopRecorder<float>(mMonitorSize);
      mMedianValues = new LoopRecorder<float>(mMonitorSize); 
      mLongTermMin = mBiggest;
      mLongTermMax = mSmallest;
      mCurrentValue = 0;
      mDifference = 0;
      mMinValue = 0;
      mMaxValue = 0; 
    };
    
    ~ValueMonitor()
    {
      if(mValues)
        delete(mValues);
      if(mMedianValues)
        delete(mMedianValues);
    };
    
    void setNewValue(const float value);
    float getCurrentValue() const;
    bool getValue(const uint8_t index, float& value) const;
    bool getLastValue(float& value) const;
    float getDifferenceValue() const;
    float getLongTermMinValue() const;
    float getLongTermMaxValue() const;
    float getMaxValue() const;
    float getMinValue() const;
    bool getMedianValue(float& value) const;
    uint32_t getValuesCount() const;
  
  private:
    ValueMonitor(const ValueMonitor& timer);
    void getlongTermMinMax();    //TODO const anc calculate longTERM split
    float calculateMaxValue(const LoopRecorder<float>& data) const;
    float calculateMinValue(const LoopRecorder<float>& data) const;
    void calculateMedian();
    //variables
    LoopRecorder<float>* mValues;
    LoopRecorder<float>* mMedianValues;
    uint8_t mMonitorSize;
    const float mBiggest;
    const float mSmallest;
    uint32_t mValuesCount; 
    //main values
    float mCurrentValue;
    float mDifference;
    float mMinValue;
    float mMaxValue;
    float mLongTermMin;
    float mLongTermMax;
    //statistic values
    float mMedianValue;
    

};

#endif
