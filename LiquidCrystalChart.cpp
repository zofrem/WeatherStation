#include "LiquidCrystalChart.h"

/////////////////////////////////////////////////////////
/// LiquidCrystalChart
/// display bar chart at the display hd44780
/// applicable for each standard size 20x04, 16x02, ...
/// posChartX & posChartY top left position of chart
/// chartHeight & chartBars dimensions of chart
/////////////////////////////////////////////////////////

LiquidCrystalChart::LiquidCrystalChart(LiquidCrystal& lcd, BarChars& barChars, const uint8_t posChartX, const uint8_t posChartY, const uint8_t chartHeight, const uint8_t chartBars)
: mLcd(lcd), mChars(barChars), mChartPosX(posChartX), mChartPosY(posChartY), mChartHeight(chartHeight), mChartBars(chartBars)
{
  mSegmentAccuracy = 0x100 / (mChartHeight * mChars.getCharVerticalDimension());
}

/////////////////////////////////////////////////////////
/// plotChart
/// display char bar chart based upon array data
/////////////////////////////////////////////////////////

void LiquidCrystalChart::plotChart(uint8_t* chartData)
{
  if(!chartData)
      return;
  for(uint8_t height = 0; height < mChartHeight; ++height)
  {
    uint8_t yDisplayPos = height + mChartPosY;        //char position from top left, display axis
    uint8_t yChartPos = mChartHeight - height;        //char position from down left, chart axis
    for(uint8_t bar = 0; bar < mChartBars; ++bar)
    {
      uint8_t xPos = bar + mChartPosX;
      uint8_t totalHeight = 0;                        //count of Y levels at whole chart
      if(chartData[bar] != 0)
        totalHeight = (chartData[bar] / mSegmentAccuracy) + 1; //calculate scale

      uint8_t actualRowHeight = yChartPos * mChars.getCharVerticalDimension();
      uint8_t belowRowHeight = 0;
      if(yChartPos != 0)
        belowRowHeight = (yChartPos - 1) * mChars.getCharVerticalDimension();

      uint8_t heightAtRow = 0;                        //empty char area
      if(actualRowHeight >= totalHeight && totalHeight > belowRowHeight)
      {
        heightAtRow =  totalHeight - belowRowHeight;  //threshold char area
      }
      else if(totalHeight > belowRowHeight)
      {
        heightAtRow = 8;                              //whole char height area
      }
      mLcd.setCursor(xPos, yDisplayPos);
      mChars.writeBarLevel(heightAtRow);
    }
  }
}
