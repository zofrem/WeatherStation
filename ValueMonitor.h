#ifndef VALUEMONITOR_H
#define VALUEMONITOR_H

#include <inttypes.h>
#include <limits>
#include <array>
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
    };
    
    ~ValueMonitor()
    {
      if(mValues)
        delete(mValues);
      if(mMedianValues)
        delete(mMedianValues);
    };

    enum MonitorWiew
    {
      MW_RAW_DATA,
      MW_MEDIAN_DATA
    };

    void setNewValue(const float value);
    float getCurrentValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    bool getValue(const uint8_t index, float& value, const MonitorWiew monitorView) const;
    bool getLastValue(float& value, const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getDifferenceValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getLongTermMinValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getLongTermMaxValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getMaxValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getMinValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    uint32_t getValuesCount() const;
    uint8_t calculateScaledValuesToChart(std::array<uint8_t, 20>& chart, const uint8_t chartBars, const MonitorWiew monitorView  = MW_RAW_DATA) const;

  private:
    ValueMonitor(const ValueMonitor& timer);
    void getlongTermMinMax();    //TODO const anc calculate longTERM split
    void calculateMedian();
    float getRangeAverage(const LoopRecorder<float>& data, const uint8_t leftValue, const uint8_t rightValue, bool& foundMaxBar, bool& foundMinBar) const;
    //variables
    LoopRecorder<float>* mValues;
    LoopRecorder<float>* mMedianValues;
    uint8_t mMonitorSize;
    const float mBiggest;
    const float mSmallest;
    uint32_t mValuesCount; 
    //main values
    float mLongTermMin;
    float mLongTermMax;
    //statistic values
    float mLongTermMinMedian;
    float mLongTermMaxMedian;
    

};

#endif
