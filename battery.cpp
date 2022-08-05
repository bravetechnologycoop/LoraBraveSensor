#include <Arduino.h>
#include "battery.h"

#define BATTERY_PIN PA5 // Battery analog input pin (labeled SPI_C5 on RAK3272)

unsigned int getBatteryLevel()
{
  return analogRead(BATTERY_PIN); 
}