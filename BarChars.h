#ifndef BARCHARS_H
#define BARCHARS_H

#include <inttypes.h>
#include "LiquidCrystal_I2C.h"

/////////////////////////////////////////////////////////
/// BarChars
/// generate and display bar chars at the display hd44780
/////////////////////////////////////////////////////////

class BarChars {
  public:
    BarChars(LiquidCrystal_I2C& lcd);
    ~BarChars();
    void createBarLevels();
    void writeBarLevel(const uint8_t level);
    uint8_t getCharVerticalDimension();

  private:
    BarChars(const BarChars& paste);
    LiquidCrystal_I2C& mLcd;
    uint8_t* mCharLevels;
    const uint8_t mCHAR_COUNT;
    const uint8_t mCHAR_Y_DIMENSION;
};

#endif
