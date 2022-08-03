#include <Arduino.h>
#include "battery.h"

#define BATTERY_PIN PA5

unsigned int getBatteryLevel()
{
  return analogRead(BATTERY_PIN); 
}