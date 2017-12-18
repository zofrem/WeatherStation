#include "ValueMonitor.h"
#include <vector>
#include <algorithm>

void ValueMonitor::setNewValue(const float value)
{
  ++mValuesCount;
  mValues->pushBack(value);
  getlongTermMinMax();
  calculateMedian();
}

float ValueMonitor::getCurrentValue(const MonitorWiew monitorView) const
{
  float value = 0;
    getValue(0, value, monitorView);
  return value;
}

bool ValueMonitor::getValue(const uint8_t index, float& value, const MonitorWiew monitorView = MW_RAW_DATA) const
{
  if(monitorView == MW_RAW_DATA)
    return mValues->getLastSample(index, value);
  else
  {
    if(mValuesCount >= mMonitorSize)
    {
      mMedianValues->getLastSample(index, value);
      return true;
    }
    return false;
  }
}

bool ValueMonitor::getLastValue(float& value, const MonitorWiew monitorView) const //TODO move this logic below to LoopRecorder
{
  return getValue(mMonitorSize - 1, value, monitorView); //TODO LAST VALUE IS FLOATING
}

float ValueMonitor::getDifferenceValue(const MonitorWiew monitorView) const
{
  if(monitorView == MW_RAW_DATA)
    return mValues->getMaxExtrem() - mValues->getMinExtrem();
  else
    return mMedianValues->getMaxExtrem() - mMedianValues->getMinExtrem();
}

void ValueMonitor::getlongTermMinMax()
{
  float currentValue = getCurrentValue(); //TODO whole method is a garbage
  float currentMedianValue = getCurrentValue(MW_MEDIAN_DATA);
  if(currentValue > mLongTermMax)
    mLongTermMax = currentValue;
  if(currentValue < mLongTermMin)
    mLongTermMin = currentValue;
  if(currentMedianValue > mLongTermMaxMedian)
    mLongTermMaxMedian = currentMedianValue;
  if(currentMedianValue < mLongTermMinMedian)
    mLongTermMinMedian = currentMedianValue;
}

float ValueMonitor::getLongTermMinValue(const MonitorWiew monitorView) const
{
  return (monitorView == MW_RAW_DATA) ? mLongTermMin : mLongTermMinMedian;
}

float ValueMonitor::getLongTermMaxValue(const MonitorWiew monitorView) const
{
  return (monitorView == MW_RAW_DATA) ? mLongTermMax : mLongTermMaxMedian;
}

float ValueMonitor::getMaxValue(const MonitorWiew monitorView) const
{
  return (monitorView == MW_RAW_DATA) ? mValues->getMaxExtrem() : mMedianValues->getMaxExtrem();
}

float ValueMonitor::getMinValue(const MonitorWiew monitorView) const
{
  return (monitorView == MW_RAW_DATA) ? mValues->getMinExtrem() : mMedianValues->getMinExtrem();
}

void ValueMonitor::calculateMedian()
{
  if(mValuesCount >= mMonitorSize)
  {
    std::vector<float> histogram;
    for(uint8_t i = 0; i < mMonitorSize; ++i)
    {
      float value = 0;
      if(mValues->getLastSample(i, value))
        histogram.push_back(value);
    }
    if(histogram.size() == mMonitorSize)
    {
      float median;
      size_t size = histogram.size();
      sort(histogram.begin(), histogram.end());

      if (size % 2 == 0)
        median = (histogram[size / 2 - 1] + histogram[size / 2]) / 2;
      else
        median = histogram[size / 2];
      mMedianValues->pushBack(median);
    }
  }
}

uint32_t ValueMonitor::getValuesCount() const
{
  return mValuesCount;
}

/////////////////////////////////////////////////////
// calculate average from range part of monitor data
// range from index left to index right
// return average, if exist whole monitor extrems min/max
/////////////////////////////////////////////////////
float ValueMonitor::getRangeAverage(const LoopRecorder<float>& data, const uint8_t leftValue, const uint8_t rightValue, bool& foundMaxBar, bool& foundMinBar) const
{
  float averageForBar = 0;
  float sumOfAll = 0;
  uint8_t count = 0;
  for(uint8_t sample = leftValue; sample < rightValue; ++sample)
  { // calculate average from range of values
    float value = 0;
    if(data.getLastSample(sample, value))
    {
      sumOfAll += value;
      ++count;
      if(value == data.getMaxExtrem())  //extrems are calculated from all values
        foundMaxBar = true;
      if(value == data.getMinExtrem())
        foundMinBar = true;
    }
  }
  if(count > 0 && sumOfAll > 0)
    averageForBar = sumOfAll / count;
  else
    averageForBar = 0;
  return averageForBar;
}

uint8_t ValueMonitor::calculateScaledValuesToChart(std::array<uint8_t, 20>& chart, const uint8_t chartBars, const MonitorWiew monitorView) const
{
  // if monitorView
  LoopRecorder<float>* valueMonitor = mValues;
  uint8_t samplesAtBar = mMonitorSize / chartBars;  //TODO how to median graph
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
