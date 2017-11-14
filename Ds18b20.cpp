#include <Arduino.h>
#include "Ds18b20.h"
    
Ds18b20::Ds18b20(const uint8_t pin)
{
  mWire = new OneWire(pin);
  mWiredDevices = 0;
  mParasite = false;
  if(mWire)
  {
    DeviceAddress devAdd;
    mWire->reset_search();
    while (mWire->search(devAdd))
    {
      if (validAddress(devAdd))
      {  
        if (!mParasite && readPowerSupply(devAdd))   // is at least one parasite than all parasited
          mParasite = true;    
        ++mWiredDevices;
      }
    }
  }
}

Ds18b20::~Ds18b20()
{
  if(mWire)
    delete(mWire);
}

bool Ds18b20::validAddress(const DeviceAddress& devAdd)
{
  return (mWire->crc8(devAdd, 7) == devAdd[7]);
}

bool Ds18b20::readPowerSupply(const DeviceAddress& devAdd)
{
  mWire->reset();
  mWire->select(devAdd);
  mWire->write(READPOWERSUPPLY);
  if (mWire->read_bit() == 0) 
    return true;
  mWire->reset();
  return false; 
}

bool Ds18b20::isConnected(const DeviceAddress& devAdd, ScratchPad& scratchPad)
{
  bool b = readScratchPad(devAdd, scratchPad);
  return b && (mWire->crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}

bool Ds18b20::readScratchPad(const DeviceAddress& devAdd, ScratchPad& scratchPad)
{ 
  // Read all registers at scratchPad
  // byte 0: temperature LSB
  // byte 1: temperature MSB
  // byte 2: high alarm temp
  // byte 3: low alarm temp
  // byte 4: DS18S20: store for crc
  //         DS18B20 & DS1822: configuration register
  // byte 5: internal use & crc
  // byte 6: DS18S20: COUNT_REMAIN
  //         DS18B20 & DS1822: store for crc
  // byte 7: DS18S20: COUNT_PER_C
  //         DS18B20 & DS1822: store for crc
  // byte 8: SCRATCHPAD_CRC
  
  if (mWire->reset() == 0) 
    return false;

  mWire->select(devAdd);
  mWire->write(READSCRATCH);

  for(uint8_t i = 0; i < 9; ++i)
      scratchPad[i] = mWire->read();

  return (mWire->reset() == 1);
}

void Ds18b20::blockTillConversionComplete()
{
  while(!isConversionComplete());
  //TODO some timeout if connection corrupted.
}

bool Ds18b20::isConversionComplete()
{
  return (mWire->read_bit() == 1);
}

float Ds18b20::getCelsiusTemp(const uint8_t deviceIndex)
{
 if(mWiredDevices >= 1)
 {
    mWire->reset();
    mWire->skip();
    mWire->write(STARTCONVO, mParasite);
    blockTillConversionComplete();
    DeviceAddress devAdd;
    if(getAddress(devAdd, deviceIndex))
      return rawToCelsius(getTemp(devAdd));
  }
  return DEVICE_DISCONNECTED_C;
}

float Ds18b20::rawToCelsius(int16_t raw)
{
    if (raw <= DEVICE_DISCONNECTED_RAW)
      return DEVICE_DISCONNECTED_C;
    return static_cast<float>(raw) * 0.0078125;     // C = RAW/128
}

int16_t Ds18b20::getTemp(const DeviceAddress& devAdd)
{ 
  ScratchPad scratchPad;
  if(isConnected(devAdd, scratchPad)) 
    return calculateTemperature(devAdd, scratchPad);
  return DEVICE_DISCONNECTED_RAW; 
}

int16_t Ds18b20::calculateTemperature(const DeviceAddress& devAdd, ScratchPad& scratchPad)
{ 
   return (((int16_t) scratchPad[TEMP_MSB]) << 11) | (((int16_t) scratchPad[TEMP_LSB]) << 3);
}

bool Ds18b20::getAddress(DeviceAddress& devAdd, uint8_t index)
{
  uint8_t depth = 0;
  mWire->reset_search();

  while (depth <= index && mWire->search(devAdd)) 
  {
    if (depth == index && validAddress(devAdd)) 
      return true;
    depth++;
  }
  return false;
}

