#include <Arduino.h>
#include "sensor.h"

void setupSensor()
{
    pinMode(SENSOR_PIN, INPUT);
}

bool isThereMotion()
{
    return !digitalRead(SENSOR_PIN);
}