#ifndef LIQUIDCRYSTALCHART_H
#define LIQUIDCRYSTALCHART_H

#include <inttypes.h>
#include <LiquidCrystal.h>
#include "BarChars.h"

/////////////////////////////////////////////////////////
/// LiquidCrystalChart
/// display bar chart at the display hd44780
/// applicable for each standard size 20x04, 16x02, ...
/// posChartX & posChartY top left positoon of chart
/// chartHeight & chartBars dimensions of chart
/////////////////////////////////////////////////////////

class LiquidCrystalChart {
  public:
    LiquidCrystalChart(LiquidCrystal& lcd, BarChars& barChars, uint8_t posX, uint8_t posY, uint8_t height, uint8_t bars);
    void plotChart(uint8_t* chartData);
    
  private:
    LiquidCrystalChart(const LiquidCrystalChart& paste);
    LiquidCrystal& mLcd;
    BarChars& mChars;
    const uint8_t mChartPosX;
    const uint8_t mChartPosY;
    const uint8_t mChartHeight;
    const uint8_t mChartBars;
    uint8_t mSegmentAccuracy;

};

#endif
