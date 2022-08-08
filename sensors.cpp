#include <Arduino.h>
#include "sensors.h"

BinarySensor::BinarySensor(uint8_t pin) : pin(pin)
{
    pinMode(pin, INPUT);
}

bool BinarySensor::isSignalHigh()
{
    return digitalRead(pin);
}

DoorSensor::DoorSensor(uint8_t pin) : BinarySensor(pin)
{
}

bool DoorSensor::isDoorOpen()
{
    return !isSignalHigh();
}

MotionSensor::MotionSensor(uint8_t pin) : BinarySensor(pin)
{
}

bool MotionSensor::isThereMotion()
{
    return !isSignalHigh();
}

AnalogSensor::AnalogSensor(uint8_t pin) : pin(pin)
{
    pinMode(pin, INPUT);
}

int AnalogSensor::getValue()
{
    return analogRead(pin);
}