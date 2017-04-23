#include "BarChars.h"

/////////////////////////////////////////////////////////
/// BarChars
/// generate and display bar chars at the display hd44780
/////////////////////////////////////////////////////////

BarChars::BarChars(LiquidCrystal& lcd) : mLcd(lcd), mCHAR_COUNT(7), mCHAR_Y_DIMENSION(8)
{
  mCharLevels = new uint8_t[mCHAR_COUNT * mCHAR_Y_DIMENSION];  // used simple vector than two dimensional allocation
  createBarLevels();
}

BarChars::~BarChars()
{
  delete(mCharLevels);
}

/////////////////////////////////////////////////////////
/// createBarLevels
/// generate chars in the memory for custom chars
/// create chars through liquid crystal class
/////////////////////////////////////////////////////////

void BarChars::createBarLevels()
{
  for(uint8_t level = 0; level < mCHAR_COUNT; ++level)
  {
    for(uint8_t y = 0; y < mCHAR_Y_DIMENSION; ++y)
    {
      if(mCHAR_COUNT - level <= y)
        mCharLevels[mCHAR_Y_DIMENSION*level+y] = 0xFF;
      else
        mCharLevels[mCHAR_Y_DIMENSION*level+y] = 0x00;
    }
    mLcd.createChar(level, &mCharLevels[mCHAR_Y_DIMENSION * level]);
  }
}

/////////////////////////////////////////////////////////
/// writeBarLevel
/// write selected level custom char on the display
/////////////////////////////////////////////////////////

void BarChars::writeBarLevel(const uint8_t level)
{
  if(level == 0)       //save up the custom characters for empty and full worth
    mLcd.write(0x20);
  else if(level >= mCHAR_Y_DIMENSION)
    mLcd.write(0xFF);
  else
  {
    mLcd.write(static_cast<uint8_t>(level-1));
  }
}

uint8_t BarChars::getCharVerticalDimension()
{
  return mCHAR_Y_DIMENSION;
}
