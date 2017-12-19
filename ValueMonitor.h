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
    
    template<size_t T>
    uint8_t calculateScaledValuesToChart(std::array<uint8_t, T>& chart, const uint8_t chartBars, const MonitorWiew monitorView  = MW_RAW_DATA) const
    {
      // if monitorView
      LoopRecorder<float>* valueMonitor = mValues;
      uint8_t samplesAtBar = mMonitorSize / chartBars;
      for(uint8_t bar = 0; bar < chartBars; ++bar)
      {
        uint8_t leftSample = samplesAtBar * bar;
        uint8_t rightSample = leftSample + samplesAtBar;
        float sumOfAll = 0;
        uint8_t count = 0;
        bool foundMaxBar = false;
        bool foundMinBar = false;
    
        float averageForBar = getRangeAverage(*valueMonitor, leftSample, rightSample, foundMaxBar, foundMinBar);
    
        uint8_t inversIndex = chartBars - 1 - bar;
        if(count == samplesAtBar)
        {
          uint8_t level = 255 * ((averageForBar - valueMonitor->getMinExtrem()) / (valueMonitor->getMaxExtrem() - valueMonitor->getMinExtrem()));    //getDifferenceValue here use polymorphismus function
          if(foundMaxBar && foundMinBar)
            chart[bar] = level; //almost impossible
          else if(foundMaxBar)
            chart[bar] = 255;   //optimise fit to above level
          else if(foundMinBar)
            chart[bar] = 0;     //optimise fit to below level
          else
            chart[bar] = level; //standard scenario
        }
        else
          chart[bar] = 0;      //empty data storage
      }
    }

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
