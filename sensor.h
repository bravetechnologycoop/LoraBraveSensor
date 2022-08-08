#ifndef SENSOR_H
#define SENSOR_H

#define SENSOR_PIN PA6 // Sensor pin (labeled SPI_MISO on RAK3272)

/**
 * setup the sensor pins
 */
void setupSensor();

/**
 * @returns true if there is motion
 */
bool isThereMotion();

#endif // SENSOR_H