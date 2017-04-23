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
BarChars* barChar = new BarChars(lcd);  
LiquidCrystalChart* chartBar = new LiquidCrystalChart(lcd, *barChar, 0, 1, 1, 16);
LoopRecorder<float>* currentTempRec = new LoopRecorder<float>(16);
LoopRecorder<float>* dailyTempRec = new LoopRecorder<float>(16);

float cuttentAverageTemp = 0;
float difference = 0;
uint8_t hourSamples[16];

void setup(void)
{
    for(uint8_t i = 0; i <=16; ++i)
      hourSamples[i] = 0;
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
  currentTempRec->pushBack(outsideTemp.getTempCByIndex(0));
  cuttentAverageTemp = getAverageValue(*currentTempRec, 16);
  chartBar->plotChart(hourSamples);  
   
   outsideTemp.requestTemperatures();
   lcd.setCursor(0,0);
   lcd.print(cuttentAverageTemp);
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
   yield();
}


void dataOutput()
{
  Serial.print("cuttentAverageTemp:");
  Serial.println(cuttentAverageTemp);
  Scheduler.delay(1000);
  yield(); 
}

void hourUpdate()
{
  dailyTempRec->pushBack(cuttentAverageTemp);
  float minValue = getMinValue(*dailyTempRec, 16);
  difference = getMaxValue(*dailyTempRec, 16) - minValue; 
  for(uint8_t i = 0; i < 16; ++i)
  {
    float value = 0;
    uint8_t inversIndex = 15 - i;
    if(dailyTempRec->getLastSample(i, value))
      hourSamples[inversIndex] = 255 * ((value - minValue)/difference);
    else
     hourSamples[inversIndex] = 0;
  }

  Scheduler.delay(1000);
  yield();   
}






