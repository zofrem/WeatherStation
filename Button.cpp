#include <Arduino.h>
#include "Button.h"


Button::Button(uint8_t buttonPin)
{
  mPin = buttonPin;
  pinMode(mPin,INPUT);
  mPreviousButtonStatus = false;
  mOnOffStatus = false;
}

/////////////////////////////////////////////////////////
/// isPressed 
/// detect button momentary activity known as "press"
/////////////////////////////////////////////////////////
bool Button::isPressed(void)
{
  bool statusButton = digitalRead(mPin);
  if((statusButton == true) && (mPreviousButtonStatus == false))
  {
    mPreviousButtonStatus = statusButton;
    return true;     
  }
  else
  {
    mPreviousButtonStatus = statusButton;
    return false;
  }
}

/////////////////////////////////////////////////////////
/// onOffState 
/// based on momentary switch emulate On/Off switch
/////////////////////////////////////////////////////////
bool Button::onOffState()
{
  if(isPressed() == true)
  {
    if(mOnOffStatus)
      mOnOffStatus = false;
    else
      mOnOffStatus = true;
  }
  return mOnOffStatus;
}

/////////////////////////////////////////////////////////
/// isDown 
/// berely detect finger force on button 
/////////////////////////////////////////////////////////
bool Button::isDown()
{
  bool mPreviousButtonStatus = digitalRead(mPin);
  return mPreviousButtonStatus = digitalRead(mPin); //return as current status
}
