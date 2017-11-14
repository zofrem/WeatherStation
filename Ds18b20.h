
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// Whole library is based on standard arduino library DallasTemperature.h
// https://github.com/milesburton/Arduino-Temperature-Control-Library
// version what you currently keep is simplified version suitable for STM32duino
// used to DS18B20

#ifndef DS18B20_H
#define DS18B20_H

#include <inttypes.h>
#include <OneWireSTM.h>

/////////////////////////////////////////////////////////
/// Ds18b20
/// read data from Ds18xxx temperature sensors 
/////////////////////////////////////////////////////////

class Ds18b20 {
  public:
    Ds18b20(const uint8_t pin); 
    ~Ds18b20();    
    float getCelsiusTemp(const uint8_t deviceIndex);
  
  private:
    typedef uint8_t DeviceAddress[8];
    typedef uint8_t ScratchPad[9];
    Ds18b20(const Ds18b20& copy);
    OneWire* mWire;
    
    uint8_t mWiredDevices;
    bool mParasite;

    bool validAddress(const DeviceAddress& devAdd);
    bool readPowerSupply(const DeviceAddress& devAdd);
    uint8_t getResolution(const DeviceAddress& devAdd);
    bool isConnected(const DeviceAddress& devAdd, ScratchPad& scratchPad);
    bool readScratchPad(const DeviceAddress& devAdd, ScratchPad& scratchPad);    
    void blockTillConversionComplete();
    bool isConversionComplete();
    float rawToCelsius(int16_t raw);
    int16_t getTemp(const DeviceAddress& devAdd);
    int16_t calculateTemperature(const DeviceAddress& devAdd, ScratchPad& scratchPad);
    bool getAddress(DeviceAddress& devAdd, uint8_t index);


    // Model IDs
    #define DS18S20MODEL 0x10  // also DS1820
    #define DS18B20MODEL 0x28
    #define DS1822MODEL  0x22
    #define DS1825MODEL  0x3B
    #define DS28EA00MODEL 0x42
    
    // Error Codes
    #define DEVICE_DISCONNECTED_C -127
    #define DEVICE_DISCONNECTED_F -196.6
    #define DEVICE_DISCONNECTED_RAW -7040

    // OneWire commands
    #define STARTCONVO      0x44  // Tells device to take a temperature reading and put it on the scratchpad
    #define COPYSCRATCH     0x48  // Copy EEPROM
    #define READSCRATCH     0xBE  // Read EEPROM
    #define WRITESCRATCH    0x4E  // Write to EEPROM
    #define RECALLSCRATCH   0xB8  // Reload from last known
    #define READPOWERSUPPLY 0xB4  // Determine if device needs parasite power
    #define ALARMSEARCH     0xEC  // Query bus for devices with an alarm condition
    
    // Scratchpad locations
    #define TEMP_LSB        0
    #define TEMP_MSB        1
    #define HIGH_ALARM_TEMP 2
    #define LOW_ALARM_TEMP  3
    #define CONFIGURATION   4
    #define INTERNAL_BYTE   5
    #define COUNT_REMAIN    6
    #define COUNT_PER_C     7
    #define SCRATCHPAD_CRC  8
    
    // Device resolution
    #define TEMP_9_BIT  0x1F //  9 bit
    #define TEMP_10_BIT 0x3F // 10 bit
    #define TEMP_11_BIT 0x5F // 11 bit
    #define TEMP_12_BIT 0x7F // 12 bit

};

#endif
