#include "Ds18b20.h"
#include "LiquidCrystal_I2C.h"
#include <inttypes.h>
#include "BarChars.h"
#include "LoopRecorder.h"
#include "LoopTimer.h"
#include "LiquidCrystalChart.h"
#include "Button.h"
#include "MemoryFree.h"

Ds18b20* outsideTemp = new Ds18b20(PB11);
LiquidCrystal_I2C* lcd = new LiquidCrystal_I2C(0x3F, 20, 4);
const uint8_t DAY_SAMPLES = 240; // sample each six minute
const uint8_t DISPLAY_BARS = 20;
const uint8_t DISPLAY_ROWS = 4;
BarChars* barChar =  new BarChars(*lcd);
LiquidCrystalChart* chartBar = new LiquidCrystalChart(*lcd, *barChar, 0, 1, 3, DISPLAY_BARS);
LiquidCrystalChart* bigChartBar = new LiquidCrystalChart(*lcd, *barChar, 0, 0, 4, DISPLAY_BARS);
LoopRecorder<float>* dailyTempRec = new LoopRecorder<float>(DAY_SAMPLES);
LoopTimer* measure = new LoopTimer(1000); //360000
LoopTimer* control = new LoopTimer(5000);  //10000
Button* button = new Button(PB10);
uint8_t menuPosition = 0;

bool firstRun = true;
float currentTemp = 0;
float difference = 0;
float minValue = 0;
float maxValue = 0;
float longTermMin = 200;
float longTermMax = -200; 
uint32_t countOfMeasures = 0;
uint8_t dailyGraph[DISPLAY_BARS];

void setup(void)
{
    lcd->begin();
    barChar->createBarLevels();
    for(uint8_t i = 0; i < DISPLAY_BARS; ++i)
      dailyGraph[i] = 0;
    Serial.begin(9600);
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

void getlongTermMinMax()
{
  countOfMeasures++;
  if(currentTemp > longTermMax)
    longTermMax = currentTemp;
  if(currentTemp < longTermMin)
    longTermMin = currentTemp;    
}

void loop(void)
{ 
  if(firstRun)
  {
    lcd->backlight();
    showWelcomeScreen();
    delay(2000); // mainly delay is requried DS18B20 get error 85 degrees at the power boot;
    lcd->noBacklight();
    firstRun = false;
  }
  bool updateScreen = false;
  if(measure->timer())
  {
    hourUpdate();
    updateScreen = true;
  } 
  
  if(button->isPressed())
  {
    menuPosition++;
    control->resetTimer();
    updateScreen = true;
  }
  
  if(control->timer())
  {
    menuPosition = 0;
    updateScreen = true;
  } 
  
  if(updateScreen)
  {
    if(menuPosition > 0)
      lcd->backlight();
    else
      lcd->noBacklight();
      
    if(menuPosition <= 1)
      showMainScreen();
    else if(menuPosition == 2)
      showDayComparationScreen();
    else if(menuPosition == 3)
     showMinMaxScreen();
    else if(menuPosition == 4)
     showLongTermMinMaxScreen();
    else if(menuPosition == 5)
      showGraphScreen(); 
    else if(menuPosition == 6)
      showInfoScreen();
    else
    {
      menuPosition = 1;
      showMainScreen();
    }
  }     
  Serial.println(freeMemory());
}

void hourUpdate()
{
  currentTemp = outsideTemp->getCelsiusTemp(0);
  getlongTermMinMax();
  dailyTempRec->pushBack(currentTemp);
  minValue = getMinValue(*dailyTempRec, DAY_SAMPLES);
  maxValue = getMaxValue(*dailyTempRec, DAY_SAMPLES);
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
        if(value == maxValue)
          foundMaxBar = true;
        if(value == minValue) 
          foundMinBar = true;
      }   
    }
    if(count > 0 && sumOfAll > 0)
      averageForBar = sumOfAll / count;
    else
      averageForBar = 0;
    uint8_t inversIndex = DISPLAY_BARS - 1 - bar;
    Serial.println(count);
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
}

void showMainScreen()
{   
   chartBar->plotChart(dailyGraph);
   showCelsiusTemperatureLeft(0, 0, currentTemp);
   showCelsiusTemperatureRight(8, 0, difference); 
}

void showMinMaxScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("Mini_24:");
  showCelsiusTemperatureRight(8, 0, minValue);
  
  lcd->setCursor(0,1);
  lcd->printstr("Maxi_24:");
  showCelsiusTemperatureRight(8, 1, maxValue);  
}

void showLongTermMinMaxScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("Minimal:");
  showCelsiusTemperatureRight(8, 0, longTermMin);
  
  lcd->setCursor(0,1);
  lcd->printstr("Maximal:");
  showCelsiusTemperatureRight(8, 1, longTermMax);   
}
void showGraphScreen()
{
  bigChartBar->plotChart(dailyGraph);  
}
void showInfoScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("Merania:");
  uint8_t digits = numDigits(countOfMeasures);
  clearChars(8, 16 - digits, 0);
  lcd->setCursor(DISPLAY_BARS - digits, 0);
  lcd->print(countOfMeasures);

  lcd->setCursor(0,1);
  lcd->printstr("MeraDni:");
  uint32_t days = countOfMeasures / DAY_SAMPLES;
  digits = numDigits(days);
  clearChars(8, 16 - digits, 1);
  lcd->setCursor(16 - digits, 1);
  lcd->print(days);
}

void showDayComparationScreen()
{
  lcd->setCursor(0,0);
  float dayAgoTemp = 0;
  float dayDiffTemp = 0;
  if(dailyTempRec->getLastSample(DAY_SAMPLES - 1 , dayAgoTemp)) 
  {
    if(dayAgoTemp < currentTemp)
    {
      lcd->printstr("Dnes teplejsie");
      dayDiffTemp = currentTemp - dayAgoTemp; 
    } 
    else
    {
      lcd->printstr("Dnes chladnejsie");
      dayDiffTemp = dayAgoTemp - currentTemp;
    }
    showCelsiusTemperatureLeft(0, 1, dayDiffTemp);    
  }
  else
  {
    lcd->printstr("Pockaj 24 hodin ");
  }
}

void showWelcomeScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("Jozef Lukac 2017");
  lcd->setCursor(0,1);
  lcd->printstr("Teplomer STM v1.1");
}

void showCelsiusTemperatureLeft(uint8_t xCursor, const uint8_t yCursor, const float& temperature)
{
   uint8_t space = 0;
   lcd->setCursor(xCursor,yCursor);
   lcd->print(temperature);
   xCursor += 4;
   if((10.00 <= temperature) || (-10.00 < temperature && temperature < 0))
     space++;
   else if(temperature <= -10.00)
     space += 2; 
   xCursor += space;
   lcd->setCursor(xCursor, yCursor);
   lcd->print((char)0xDF);
   xCursor++;
   lcd->setCursor(xCursor, yCursor);
   lcd->print((char)0x43);
   xCursor++;
   clearChars(xCursor, xCursor + (2 - space), yCursor);
}

void showCelsiusTemperatureRight(uint8_t xCursor, const uint8_t yCursor, const float& temperature)
{
   uint8_t whiteSpace = 0;
   if((10.00 <= temperature) || (-10.00 < temperature && temperature < 0))
     whiteSpace++;
   else if(0 <= temperature && temperature < 10.00)
     whiteSpace += 2;
   clearChars(xCursor, xCursor + whiteSpace, yCursor);      
   lcd->setCursor(xCursor + whiteSpace, yCursor);
   lcd->print(temperature);
   lcd->setCursor(xCursor + 6, yCursor);
   lcd->print((char)0xDF);
   lcd->setCursor(xCursor + 7, yCursor);
   lcd->print((char)0x43);
}

void clearChars(const uint8_t fromX, const uint8_t toX, const uint8_t line)
{
  for(uint8_t i = fromX; i < toX; ++i)
  {
    lcd->setCursor(i, line);
    lcd->print((char)0x20);
  }  
}

uint8_t numDigits(uint32_t x)  
{
    return (x < 10 ? 1 :   
        (x < 100 ? 2 :   
        (x < 1000 ? 3 :   
        (x < 10000 ? 4 :   
        (x < 100000 ? 5 :   
        (x < 1000000 ? 6 :   
        (x < 10000000 ? 7 :  
        (x < 100000000 ? 8 :  
        (x < 1000000000 ? 9 :  
        10)))))))));  
} 







