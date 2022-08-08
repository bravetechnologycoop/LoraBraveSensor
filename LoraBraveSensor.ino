#include <Arduino.h>
#include "lora.h"
#include "sensors.h"
#include "fsm.h"
#include "flashAddresses.h"
#include "heartbeat.h"
#include "systemTimers.h"
#include "main.h"

#define DOOR_SENSOR_PIN PA7
#define MOTION_SENSOR_PIN PA6
#define BATTERY_PIN PA5

DoorSensor doorSensor = DoorSensor(DOOR_SENSOR_PIN);
MotionSensor motionSensor = MotionSensor(MOTION_SENSOR_PIN);
AnalogSensor battery = AnalogSensor(BATTERY_PIN);

void setup()
{
  setupOTAA();
  setupFSM();
  setupHeartbeat();
  attachInterrupt(
      digitalPinToInterrupt(DOOR_SENSOR_PIN), [] {}, CHANGE);
  attachInterrupt(
      digitalPinToInterrupt(MOTION_SENSOR_PIN), [] {}, CHANGE);
  uplink_routine("Online");
}

void loop()
{
  int sleepTimer = stateHandler(doorSensor, motionSensor);
  Serial.print("Sleeping for ");
  Serial.println(sleepTimer);
  if (sleepTimer > 0)
  {
    api.system.timer.create((RAK_TIMER_ID)FSM_TIMER, (RAK_TIMER_HANDLER)[](void *){}, RAK_TIMER_ONESHOT);
    api.system.timer.start((RAK_TIMER_ID)FSM_TIMER, sleepTimer, (void *)1);
  }
  api.system.sleep.all();
  Serial.println("Woke up");
}