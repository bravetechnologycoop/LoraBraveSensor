#include <Arduino.h>
#include "OTAA.h"
#include "door.h"
#include "sensor.h"
#include "fsm.h"
#include "flashAddresses.h"
#include "heartbeat.h"
#include "systemTimers.h"

void setup()
{
  setupOTAA();
  setupFSM();
  setupDoor();
  setupSensor();
  setupHeartbeat();
  attachInterrupt(
      digitalPinToInterrupt(DOOR_PIN), [] {}, CHANGE);
  attachInterrupt(
      digitalPinToInterrupt(SENSOR_PIN), [] {}, CHANGE);
  uplink_routine("Connected");
}

void loop()
{
  int sleepTimer = stateHandler();
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