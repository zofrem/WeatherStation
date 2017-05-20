#ifndef Button_h
#define Button_h

#include <inttypes.h>


class Button;
typedef void (*buttonEventHandler)(Button&);

class Button {
  public:
    Button(uint8_t buttonPin);   
    
    bool isPressed();
    bool onOffState();
    bool isDown();
    
  private: 
    uint8_t mPin;
    bool mPreviousButtonStatus;
    bool mOnOffStatus;
};

#endif
