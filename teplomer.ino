#include <SchedulerARMAVR.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <inttypes.h>
#include "BarChars.h"
#include "LoopRecorder.h"
#include "LiquidCrystalChart.h"


OneWire oneWire(6);
DallasTemperature outsideTemp(&oneWire);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const uint8_t DAY_SAMPLES = 240; // sample each six minute
const uint8_t DISPLAY_BARS = 16;
BarChars* barChar = new BarChars(lcd);  
LiquidCrystalChart* chartBar = new LiquidCrystalChart(lcd, *barChar, 0, 1, 1, DISPLAY_BARS);
LoopRecorder<float>* dailyTempRec = new LoopRecorder<float>(DAY_SAMPLES);

float currentTemp = 0;
float difference = 0;
uint8_t dailyGraph[DISPLAY_BARS];

void setup(void)
{
    for(uint8_t i = 0; i < DISPLAY_BARS; ++i)
      dailyGraph[i] = 0;
    Scheduler.startLoop(dataOutput);
    Scheduler.startLoop(hourUpdate);
    Serial.begin(9600);
    lcd.begin(16, 2);
    outsideTemp.begin();
}

float getAverageValue(const LoopRecorder<float>& data, const uint8_t samplesCount)
{
  float value = 0;
  uint8_t empty = 0;
  float sumOfValues = 0;
  for(uint8_t i = 0; i < samplesCount; ++i)
  {
    if(data.getLastSample(i, value))
      sumOfValues += value;
    else
      ++empty;
  }
  return sumOfValues / (samplesCount - empty);
}

float getMaxValue(const LoopRecorder<float>& data, const uint8_t samplesCount)
{
  float maxValue = -200;
  float value;
  for(uint8_t i = 0; i < samplesCount; ++i)
  {
    if(data.getLastSample(i, value))
      if(maxValue < value)
        maxValue = value;
  }
  return maxValue;
}

float getMinValue(const LoopRecorder<float>& data, const uint8_t samplesCount)
{
  float minValue = 200;
  float value; 
  for(uint8_t i = 0; i < samplesCount; ++i)
  {
    if(data.getLastSample(i, value))
      if(minValue > value)
        minValue = value;
  }  
  return minValue;
}

void loop(void)
{  
  currentTemp = outsideTemp.getTempCByIndex(0);
  chartBar->plotChart(dailyGraph);  
   
   outsideTemp.requestTemperatures();
   lcd.setCursor(0,0);
   lcd.print(currentTemp);
   lcd.setCursor(5,0);
   lcd.print((char)0xDF);
   lcd.setCursor(6,0);
   lcd.print((char)0x43);
   lcd.setCursor(10,0);
   lcd.print(difference);
   lcd.setCursor(14,0);
   lcd.print((char)0xDF);
   lcd.setCursor(15,0);
   lcd.print((char)0x43);
   Scheduler.delay(500);   
   yield();
}


void dataOutput()
{
  Serial.print("currentTemp:");
  Serial.println(currentTemp);
  Scheduler.delay(1000);
  yield(); 
}

void hourUpdate()
{
  dailyTempRec->pushBack(currentTemp);
  float minValue = getMinValue(*dailyTempRec, DAY_SAMPLES);
  float maxValue = getMaxValue(*dailyTempRec, DAY_SAMPLES);
  difference = maxValue - minValue;  
  uint8_t samplesAtBar = DAY_SAMPLES / DISPLAY_BARS;  
    for(uint8_t bar = 0; bar < DISPLAY_BARS; ++bar)
    {
      uint8_t leftSample = samplesAtBar * bar;
      uint8_t rightSample = leftSample + samplesAtBar;
      float averageForBar = 0;
      float sumOfAll = 0;
      uint8_t count = 0;
      bool foundMaxBar = false;
      bool foundMinBar = false;
      for(uint8_t sample = leftSample; sample < rightSample; ++sample)
      { // calculate average with more samples to one bar          
        float value = 0;
        if(dailyTempRec->getLastSample(sample, value))
        {
          sumOfAll += value;
          ++count;
          foundMaxBar = value == maxValue;
          foundMinBar = value == minValue;
        }   
      }
      if(count > 0 && sumOfAll > 0)
        averageForBar = sumOfAll / count;
      else
        averageForBar = 0;
      Serial.println(averageForBar, 2);
      Serial.print('\n');
      uint8_t inversIndex = DISPLAY_BARS - 1 - bar;
      if(count == samplesAtBar)
      {
        uint8_t level = 255 * ((averageForBar - minValue) / difference);
        if(foundMaxBar && foundMinBar)
          dailyGraph[inversIndex] = level; //almost impossible
        else if(foundMaxBar)
          dailyGraph[inversIndex] = 255;   //optimise fit to above level
        else if(foundMinBar)
          dailyGraph[inversIndex] = 0;     //optimise fit to below level
        else
          dailyGraph[inversIndex] = level; //standard scenario        
      }      
      else
         dailyGraph[inversIndex] = 0;      //empty data storage
  }

  Scheduler.delay(1000);
  yield();   
}






