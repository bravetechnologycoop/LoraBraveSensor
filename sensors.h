#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

/**
 * Sensor class
 *
 * Used for any sensor that uses a binary digital input
 */
class BinarySensor
{
private:
    uint8_t pin;

public:
    /**
     * Constructor
     *
     * @param pin The pin the sensor is connected to
     */
    BinarySensor(uint8_t pin);
    /**
     * Returns the value of the sensor
     *
     * @return true if the sensor inputs high, false otherwise
     */
    bool isSignalHigh();
};

/**
 * Door Sensor class DoorSensor
 * 
 * Uses an binary input, where a low signal indicates the door is open
 */
class DoorSensor : public BinarySensor
{
public:
    /**
     * Constructor
     *
     * @param pin The pin the sensor is connected to
     */
    DoorSensor(uint8_t pin);
    /**
     * Returns true if the door is open, false otherwise
     *
     * @return true if the door is open, false otherwise
     */
    bool isDoorOpen();
};

/**
 * Motion Sensor class
 * 
 * Uses a binary input, where a low signal indicates motion
 */
class MotionSensor : public BinarySensor
{
public:
    /**
     * Constructor
     *
     * @param pin The pin the sensor is connected to
     */
    MotionSensor(uint8_t pin);
    /**
     * Returns true if the motion sensor is detecting motion, false otherwise
     *
     * @return true if the motion sensor is detecting motion, false otherwise
     */
    bool isThereMotion();
};

class AnalogSensor
{
private:
    uint8_t pin;
    public: 
    /**
     * Constructor
     *
     * @param pin The pin the sensor is connected to
     */
    AnalogSensor(uint8_t pin);
    /**
     * Returns the value of the sensor
     *
     * @return the value of the sensor
     */
    int getValue();
}; 

#endif // SENSORS_H