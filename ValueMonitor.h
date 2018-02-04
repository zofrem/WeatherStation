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
    bool getValue(const uint8_t index, float& value, const MonitorWiew monitorView = MW_RAW_DATA) const;
    bool getLastValue(float& value, const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getDifferenceValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getLongTermMinValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getLongTermMaxValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getMaxValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    float getMinValue(const MonitorWiew monitorView = MW_RAW_DATA) const;
    uint32_t getValuesCount() const;
    
    template<size_t T>
    uint8_t calculateScaledValuesToChart(std::array<uint8_t, T>& chart, const uint8_t chartBars, const MonitorWiew monitorView = MW_RAW_DATA) const
    {
      LoopRecorder<float>* valueMonitor = (monitorView == MW_RAW_DATA) ? mValues : mMedianValues;
      uint8_t samplesAtBar = mMonitorSize / chartBars;
      for(uint8_t bar = 0; bar < chartBars; ++bar)
      {
        uint8_t leftSample = samplesAtBar * bar;
        uint8_t rightSample = leftSample + samplesAtBar;
        float sumOfAll = 0;
        uint8_t count = 0;
        bool foundMaxBar = false;
        bool foundMinBar = false;
    
        float averageForBar = getRangeAverage(*valueMonitor, leftSample, rightSample, foundMaxBar, foundMinBar, count);
    
        uint8_t inversIndex = chartBars - 1 - bar;
        if(count == samplesAtBar)
        {
          float scopeDifference = valueMonitor->getMaxExtrem() - valueMonitor->getMinExtrem();
          float averageHight = averageForBar - valueMonitor->getMinExtrem();
          uint8_t level = 255 * (averageHight / scopeDifference);
          if(foundMaxBar && foundMinBar)
            chart[inversIndex] = level; //almost impossible
          else if(foundMaxBar)
            chart[inversIndex] = 255;   //optimise fit to above level
          else if(foundMinBar)
            chart[inversIndex] = 0;     //optimise fit to below level
          else
            chart[inversIndex] = level; //standard scenario
        }
        else
          chart[inversIndex] = 0;      //empty data storage
      }
    }

  private:
    ValueMonitor(const ValueMonitor& timer);
    void getlongTermMinMax();    //TODO const anc calculate longTERM split
    void calculateMedian();
    float getRangeAverage(const LoopRecorder<float>& data, const uint8_t leftValue, const uint8_t rightValue, bool& foundMaxBar, bool& foundMinBar, uint8_t& count) const;
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
