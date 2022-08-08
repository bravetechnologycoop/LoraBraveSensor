#ifndef DOOR_H
#define DOOR_H

#define DOOR_PIN PA7 // Door sensor pin (labeled SPI_MOSI on RAK3272)

/**
 * setup the door pins
 */
void setupDoor();

/**
 * @returns true if the door is open
 */
bool isDoorOpen();

#endif // DOOR_H