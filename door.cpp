#include <Arduino.h>
#include "door.h"

void setupDoor()
{
  pinMode(DOOR_PIN, INPUT);
}

bool isDoorOpen()
{
  return !digitalRead(DOOR_PIN);
}