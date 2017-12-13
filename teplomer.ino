#include "Ds18b20.h"
#include "LiquidCrystal_I2C.h"
#include <inttypes.h>
#include "BarChars.h"
#include "ValueMonitor.h"
#include "LoopTimer.h"
#include "LiquidCrystalChart.h"
#include "Button.h"
#include "Adafruit_BMP280.h"

Adafruit_BMP280* bmp = new Adafruit_BMP280();
Ds18b20* outsideTemp = new Ds18b20(PB11);
LiquidCrystal_I2C* lcd = new LiquidCrystal_I2C(0x3F, 20, 4);
const uint8_t DAY_SAMPLES = 240; // sample each six minute for a whole day
const uint8_t DISPLAY_BARS = 20;
const uint8_t DISPLAY_ROWS = 4;
const uint8_t MENU_SCREENS = 4;
BarChars* barChar =  new BarChars(*lcd);
LiquidCrystalChart* chartBar = new LiquidCrystalChart(*lcd, *barChar, 0, 1, 3, DISPLAY_BARS);
ValueMonitor* outTempMonitor = new ValueMonitor(DAY_SAMPLES);
ValueMonitor* pressureMonitor = new ValueMonitor(DAY_SAMPLES);
ValueMonitor* inTempMonitor = new ValueMonitor(DAY_SAMPLES);
LoopTimer* measure = new LoopTimer(4000); //360000
LoopTimer* control = new LoopTimer(10000);  //10000
Button* buttTemp = new Button(PB12);
Button* buttPress = new Button(PB13);
Button* buttHumi = new Button(PB14);
uint8_t menuPosition = 0; // pages
uint8_t menuParameter = 0; // 0 = temp out, 1 = pressure, 2 = temp in

bool firstRun = true;
float currentValues[3];
float differences[3];
float minValues[3];
float maxValues[3];
float longTermMin[3];
float longTermMax[3]; 
uint32_t countOfMeasures = 0;
uint8_t dailyGraph[3][DISPLAY_BARS];

#define KEY(a,b)  ((a<<8) | b)

void setup(void)
{

    pinMode(LED_BUILTIN, OUTPUT);
    bmp->begin();
    lcd->begin();
    barChar->createBarLevels();
    for(uint8_t i = 0; i < DISPLAY_BARS; ++i)
      for(uint8_t j = 0; j < 3; ++j)
        dailyGraph[j][i] = 0;
    Serial.begin(9600);
}


void loop(void)
{  
  bool measureUpdate = false;
  showBeginScreenProcedure();
    
  if(measure->timer())
  {
    hourUpdate();
    measureUpdate = true;
  } 
    
  if(buttonMenuMovementLogic() || measureUpdate)
  {
    backlightLogic();
    if(menuPosition == 0)
      showTempOutMainScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(0,1))
      showTempOutMainScreen();    
    else if(KEY(menuParameter, menuPosition) == KEY(1,1))
      showPressureMainScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(2,1))
      showTempInMainScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(0,2))
      showTempOutDayComparationScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(1,2))
      showPressDayComparationScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(2,2))
      showTempInDayComparationScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(0,3))
      showMinMaxTempOutScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(1,3))
      showMinMaxPressScreen();
    else if(KEY(menuParameter, menuPosition) == KEY(2,3))
      showMinMaxTempInScreen();
    else
      showInfoScreen();
  }
}

void showBeginScreenProcedure()
{
  if(firstRun)
  {
    lcd->backlight();
    showWelcomeScreen();
    delay(2000); // mainly delay is requried DS18B20 get error 85 degrees at the power boot;
    lcd->noBacklight();
    firstRun = false;
  }
}

bool buttonMenuMovementLogic()
{
  bool updateScreen = false;
  bool pressedTemp = buttTemp->isPressed();
  bool pressedPress = buttPress->isPressed();
  bool pressedHumi= buttHumi->isPressed();
  
  if(menuPosition >= MENU_SCREENS)
    menuPosition = 0;

  if(control->timer())
  {
    menuPosition = 0;
    menuParameter = 0;
    updateScreen = true;
  } 
  
  if(pressedTemp || pressedPress || pressedHumi)
  {
    control->resetTimer();
    updateScreen = true;

    if(pressedTemp)
    {
      if(menuParameter == 0)
        menuPosition++;
      else
      {
        menuPosition = 1;
        menuParameter = 0;
      }
    }
    else if(pressedPress)
    {
      if(menuParameter == 1)
        menuPosition++;
      else
      {
        menuPosition = 1;
        menuParameter = 1;
      }
    }
    else if(pressedHumi)
    {
      if(menuParameter == 2)
        menuPosition++;
      else
      {
        menuPosition = 1;
        menuParameter = 2;
      }
    }
  }  
  return updateScreen;
}

void backlightLogic()
{
  if(menuPosition > 0)
    lcd->backlight();
  else
    lcd->noBacklight();
}

void hourUpdate()
{
  outTempMonitor->setNewValue(outsideTemp->getCelsiusTemp(0));
  pressureMonitor->setNewValue(bmp->readPressure());
  inTempMonitor->setNewValue(bmp->readTemperature()); //TODO how to median graph
  
  updateGraph(*outTempMonitor, 0);
  updateGraph(*pressureMonitor, 1);
  updateGraph(*inTempMonitor, 2);
}

void updateGraph(const ValueMonitor& valueMonitor, const uint8_t param)
{
  uint8_t samplesAtBar = DAY_SAMPLES / DISPLAY_BARS;  //TODO how to median graph
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
      if(valueMonitor.getValue(sample, value))     //TODO how to median graph
      {
        sumOfAll += value;
        ++count;
        if(value == valueMonitor.getMaxValue())    //TODO how to median graph
          foundMaxBar = true;
        if(value == valueMonitor.getMinValue())    //TODO how to median graph
          foundMinBar = true;
      }   
    }
    if(count > 0 && sumOfAll > 0)
      averageForBar = sumOfAll / count;
    else
      averageForBar = 0;
    uint8_t inversIndex = DISPLAY_BARS - 1 - bar;
    if(count == samplesAtBar)
    {
      uint8_t level = 255 * ((averageForBar - valueMonitor.getMinValue()) / valueMonitor.getDifferenceValue());    //TOTOK
      if(foundMaxBar && foundMinBar)
        dailyGraph[param][inversIndex] = level; //almost impossible
      else if(foundMaxBar)
        dailyGraph[param][inversIndex] = 255;   //optimise fit to above level
      else if(foundMinBar)
        dailyGraph[param][inversIndex] = 0;     //optimise fit to below level
      else
        dailyGraph[param][inversIndex] = level; //standard scenario        
    }      
    else
      dailyGraph[param][inversIndex] = 0;      //empty data storage
  }
}

void showTempOutMainScreen()
{
   chartBar->plotChart(dailyGraph[0]);
   showCelsiusTemperatureLeft(0, 0, outTempMonitor->getCurrentValue());
   showCelsiusTemperatureRight(12, 0, outTempMonitor->getDifferenceValue());
}

void showPressureMainScreen()
{   
   chartBar->plotChart(dailyGraph[1]);
   showPressureLeft(0, 0, pressureMonitor->getCurrentValue());
   showPressureRight(13, 0, pressureMonitor->getDifferenceValue());
}

void showTempInMainScreen()
{   
   chartBar->plotChart(dailyGraph[2]);
   showCelsiusTemperatureLeft(0, 0, inTempMonitor->getCurrentValue());
   showCelsiusTemperatureRight(12, 0, inTempMonitor->getDifferenceValue());
}

void showMinMaxTempOutScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("VonkuMini_24:");
  showCelsiusTemperatureRight(12, 0, outTempMonitor->getMinValue());
  
  lcd->setCursor(0,1);
  lcd->printstr("VonkuMaxi_24:");
  showCelsiusTemperatureRight(12, 1, outTempMonitor->getMaxValue());

  lcd->setCursor(0,2);
  lcd->printstr("VonkuMinimal:");
  showCelsiusTemperatureRight(12, 2, outTempMonitor->getLongTermMinValue());
  
  lcd->setCursor(0,3);
  lcd->printstr("VonkuMaximal:");
  showCelsiusTemperatureRight(12, 3, outTempMonitor->getLongTermMaxValue());
}

void showMinMaxPressScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("TlakMini_24:");
  showPressureRight(13, 0, pressureMonitor->getMinValue());
  
  lcd->setCursor(0,1);
  lcd->printstr("TlakMaxi_24:");
  showPressureRight(13, 1, pressureMonitor->getMaxValue());

  lcd->setCursor(0,2);
  lcd->printstr("TlakMinimal:");
  showPressureRight(13, 2, pressureMonitor->getLongTermMinValue());
  
  lcd->setCursor(0,3);
  lcd->printstr("TlakMaximal:");
  showPressureRight(13, 3, pressureMonitor->getLongTermMaxValue());
}

void showMinMaxTempInScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("InterMini_24:");
  showCelsiusTemperatureRight(12, 0, inTempMonitor->getMinValue());
  
  lcd->setCursor(0,1);
  lcd->printstr("InterMaxi_24:");
  showCelsiusTemperatureRight(12, 1, inTempMonitor->getMaxValue());

  lcd->setCursor(0,2);
  lcd->printstr("InterMinimal:");
  showCelsiusTemperatureRight(12, 2, inTempMonitor->getLongTermMinValue());
  
  lcd->setCursor(0,3);
  lcd->printstr("InterMaximal:");
  showCelsiusTemperatureRight(12, 3, inTempMonitor->getLongTermMaxValue());
}

void showInfoScreen()
{
  lcd->setCursor(0,0);
  lcd->printstr("Merania:");
  uint8_t digits = numDigits(outTempMonitor->getValuesCount());
  clearChars(8, DISPLAY_BARS - digits, 0);
  lcd->setCursor(DISPLAY_BARS - digits, 0);
  lcd->print(outTempMonitor->getValuesCount());

  lcd->setCursor(0,1);
  lcd->printstr("MeraDni:");
  uint32_t days = outTempMonitor->getValuesCount() / DAY_SAMPLES;
  digits = numDigits(days);
  clearChars(8, DISPLAY_BARS - digits, 1);
  lcd->setCursor(DISPLAY_BARS - digits, 1);
  lcd->print(days);
  clearChars(0, 20, 2);
  clearChars(0, 20, 3);
}

void showTempOutDayComparationScreen()
{
  lcd->setCursor(0,0);
  float dayDiffTemp = 0;
  if(dayComparationScreen(*outTempMonitor, dayDiffTemp))
  {
    if(dayDiffTemp > 0)
      lcd->printstr("Dnes teplejsie:");
    else if(dayDiffTemp == 0)
      lcd->printstr("Dnes ako vcera:");
    else
    {
      lcd->printstr("Dnes chladnejsie o");
      dayDiffTemp *= -1;
    }
    showCelsiusTemperatureLeft(0, 1, dayDiffTemp);
  }
  else
    lcd->printstr("Pockaj 24 hodin ");
}

void showPressDayComparationScreen()
{
  lcd->setCursor(0,0);
  float dayDiffTemp = 0;
  if(dayComparationScreen(*pressureMonitor, dayDiffTemp))
  {
    if(dayDiffTemp > 0)
      lcd->printstr("Dnes vyssi:");
    else if(dayDiffTemp == 0)
      lcd->printstr("Dnes ako vcera:");
    else
    {
      lcd->printstr("Dnes nizsi:");
      dayDiffTemp *= -1;
    }
    showCelsiusTemperatureLeft(0, 1, dayDiffTemp);
  }
  else
    lcd->printstr("Pockaj 24 hodin ");
}

void showTempInDayComparationScreen()
{
  lcd->setCursor(0,0);
  float dayDiffTemp = 0;
  if(dayComparationScreen(*inTempMonitor, dayDiffTemp))
  {
    if(dayDiffTemp > 0)
      lcd->printstr("Dnes teplejsie:");
    else if(dayDiffTemp == 0)
      lcd->printstr("Dnes ako vcera:");
    else
    {
      lcd->printstr("Dnes chladnejsie o");
      dayDiffTemp *= -1;
    }
    showCelsiusTemperatureLeft(0, 1, dayDiffTemp);
  }
  else
    lcd->printstr("Pockaj 24 hodin ");
}

bool dayComparationScreen(const ValueMonitor& valueMonitor, float& dayDiff)
{
  lcd->setCursor(0,0);
  float dayAgo = 0;
  if(valueMonitor.getLastValue(dayAgo))
  {
    dayDiff = valueMonitor.getCurrentValue() - dayAgo;
    return true;  
  }
  else
  {
    return false; // wait 24 hours
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

void showPressureLeft(uint8_t xCursor, const uint8_t yCursor, const float& pressure)
{
  uint8_t digits = numDigits(pressure);
  lcd->setCursor(xCursor, yCursor);
  lcd->print(static_cast<int>(pressure));
  lcd->setCursor(xCursor + digits, yCursor);
  lcd->printstr("Pa");
  //clearChars(xCursor + digits + 2, xCursor + 8, yCursor); 
}

void showPressureRight(uint8_t xCursor, const uint8_t yCursor, const float& pressure)
{

  uint8_t digits = numDigits(pressure);
  lcd->setCursor(xCursor + 5 - digits, yCursor);
  lcd->print(static_cast<int>(pressure));
  clearChars(xCursor, xCursor + 5 - digits, yCursor);
  lcd->setCursor(xCursor + 5, yCursor);
  lcd->printstr("Pa");
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







