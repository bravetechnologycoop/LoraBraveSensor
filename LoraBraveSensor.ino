#include <Arduino.h>
#include "lora.h"
#include "sensors.h"
#include "fsm.h"
#include "flashAddresses.h"
#include "heartbeat.h"
#include "systemTimers.h"
#include "main.h"

#define BATTERY_PIN PA5
#define EEPROM_RESET_PIN PA4

AnalogSensor battery = AnalogSensor(BATTERY_PIN);

void setup()
{
  lora::setupOTAA();
  for (int i = 0; i < api.lorawan.rety.get() - 1; i++)
  {
    lora::sendUplink("Modify the comment if this gets sent :3"); // I beleive this flushes the retry buffer, 
  }                                                              // must happen before attempting uplinks

  pinMode(EEPROM_RESET_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(EEPROM_RESET_PIN), resetEeprom, RISING);

  attachInterrupt(
      digitalPinToInterrupt(DOOR_SENSOR_PIN), fsm::addToSensorDataQueue, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), fsm::addToSensorDataQueue, CHANGE);

  fsm::setupFSM();
  heartbeat::setupHeartbeat();
}

void loop()
{
  int heartBeatTimer = heartbeat::getRemainingDuration();
  int uplinkTimer = lora::getRemainingDuration();
  int stateSleepTimer = fsm::handleState();
  DEBUG_SERIAL_LOG_MORE.printf("Heartbeat timer: %is, ", heartBeatTimer / 1000);
  DEBUG_SERIAL_LOG_MORE.printf("Uplink timer: %is", uplinkTimer / 1000);
  DEBUG_SERIAL_LOG_MORE.printf("State sleep timer: %is\r\n", stateSleepTimer / 1000);
  int sleepDuration = min(heartBeatTimer, min(uplinkTimer, stateSleepTimer));

  if (sleepDuration > 0)
  {
    api.system.timer.create((RAK_TIMER_ID)FSM_TIMER, (RAK_TIMER_HANDLER)fsm::addToSensorDataQueue, RAK_TIMER_ONESHOT);
    api.system.timer.start((RAK_TIMER_ID)FSM_TIMER, sleepDuration, (void *)1);
  }
  if (fsm::isSensorDataQueueEmpty() && !lora::isUplinkInProgress())
  {
    DEBUG_SERIAL_LOG.printf("Attempting to sleep for %is...", sleepDuration / 1000);
    DEBUG_SERIAL_LOG.println("state queue empty, lora complete, sleeping");
    api.system.sleep.all();
    DEBUG_SERIAL_LOG.println("Woke up");
  }
  else
  {
    DEBUG_SERIAL_LOG_MORE.println("State queue not empty or lora not complete");
  }
}

void resetEeprom()
{
  DEBUG_SERIAL_LOG.printf("Resetting EEPROM\r\n");
  fsm::resetTimers();
  heartbeat::resetTimers();
}