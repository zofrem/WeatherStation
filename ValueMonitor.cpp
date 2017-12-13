#include "ValueMonitor.h"
#include <vector>
#include <algorithm>

void ValueMonitor::setNewValue(const float value)
{
  ++mValuesCount;
  mCurrentValue = value;
  mValues->pushBack(value);
  getlongTermMinMax();
  mMinValue = calculateMinValue(*mValues);
  mMaxValue = calculateMaxValue(*mValues);
  mDifference = mMaxValue - mMinValue;
  calculateMedian();
}

float ValueMonitor::getCurrentValue() const
{
  return mCurrentValue;
}

bool ValueMonitor::getValue(const uint8_t index, float& value) const
{
  return(mValues->getLastSample(index, value));
}

bool ValueMonitor::getLastValue(float& value) const
{
  return(mValues->getLastSample(mMonitorSize - 1, value));
}

float ValueMonitor::getDifferenceValue() const
{
  return mDifference;
}

void ValueMonitor::getlongTermMinMax()
{
  if(mCurrentValue > mLongTermMax)
    mLongTermMax = mCurrentValue;
  if(mCurrentValue < mLongTermMin)   //TODO require else if?
    mLongTermMin = mCurrentValue;
}

float ValueMonitor::getLongTermMinValue() const
{
  return mLongTermMin;
}

float ValueMonitor::getLongTermMaxValue() const
{
  return mLongTermMax;
}

float ValueMonitor::getMaxValue() const
{
  return mMaxValue; 
}

float ValueMonitor::calculateMaxValue(const LoopRecorder<float>& data) const
{
  float maxValue = mSmallest;
  float value;
  for(uint8_t i = 0; i < mMonitorSize; ++i)
  {
    if(data.getLastSample(i, value))
    {
      if(maxValue < value)
        maxValue = value;
    }
    else
      break;
  }
  return maxValue;
}

float ValueMonitor::getMinValue() const
{
  return mMinValue;
}

float ValueMonitor::calculateMinValue(const LoopRecorder<float>& data) const
{
  float minValue = mBiggest;
  float value;
  for(uint8_t i = 0; i < mMonitorSize; ++i)
  {
    if(data.getLastSample(i, value))
    {
      if(minValue > value)
        minValue = value;
    }
    else
      break;
  }
  return minValue;
}

bool ValueMonitor::getMedianValue(float& value) const
{
  if(mValuesCount >= mMonitorSize)
  {
    value = mMedianValue;
    return true;
  }
  return false;
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
