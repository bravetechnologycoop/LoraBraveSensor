#include <Arduino.h>
#include <ArduinoJson.h>
#include "fsm.h"
#include "heartbeat.h"
#include "secrets.h"
#include "main.h"
#include "lora.h"

#define OTAA_PERIOD (20000)

bool sending = false;
bool receiving = false;

static void lora::recvCallback(SERVICE_LORA_RECEIVE_T *data)
{
  receiving = true;
  if (data->BufferSize > 0)
  {
    DEBUG_SERIAL_LOG.println("Something received!");
    for (int i = 0; i < data->BufferSize; i++)
    {
      DEBUG_SERIAL_LOG.printf("%x", data->Buffer[i]);
    }
    DEBUG_SERIAL_LOG.print("\r\n");

    // testing
    DEBUG_SERIAL_LOG.println((char *)data->Buffer);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (char *)data->Buffer);
    if (doc.containsKey("countdownTimer"))
    {
      DEBUG_SERIAL_LOG_MORE.printf("countdownTimer set as %u\r\n", doc["countdownTimer"].as<unsigned int>() * 1000);
      fsm::setCountdownTimer(doc["countdownTimer"].as<unsigned int>() * 1000);
    }
    if (doc.containsKey("durationTimer"))
    {
      DEBUG_SERIAL_LOG_MORE.printf("durationTimer set as %i\r\n", doc["durationTimer"].as<unsigned int>() * 1000);
      fsm::setDurationTimer(doc["durationTimer"].as<unsigned int>() * 1000);
    }
    if (doc.containsKey("stillnessTimer"))
    {
      DEBUG_SERIAL_LOG_MORE.printf("stillnessTimer set as %i\r\n", doc["stillnessTimer"].as<unsigned int>() * 1000);
      fsm::setStillnessTimer(doc["stillnessTimer"].as<unsigned int>() * 1000);
    }
    if (doc.containsKey("heartbeatInterval")) {
      DEBUG_SERIAL_LOG_MORE.printf("heartbeatInterval set as %i\r\n", doc["heartbeatInterval"].as<unsigned int>() * 1000);
      heartbeat::setInterval(doc["heartbeatInterval"].as<unsigned int>() * 1000);
    }
  }
  receiving = false;
}

static void lora::joinCallback(int32_t status)
{
  DEBUG_SERIAL_LOG.printf("Join status: %d\r\n", status);
}

static void lora::sendCallback(int32_t status)
{
  if (status == 0)
  {
    DEBUG_SERIAL_LOG.println("Successfully sent (callback)");
  }
  else
  {
    DEBUG_SERIAL_LOG.println("Sending failed (callback)");
  }
  sending = false;
}

void lora::setupOTAA()
{
  DEBUG_SERIAL_LOG.begin(115200, RAK_AT_MODE);

  DEBUG_SERIAL_LOG.println("BraveSensor");
  DEBUG_SERIAL_LOG.println("------------------------------------------------------");

  // OTAA Device EUI MSB first
  uint8_t node_device_eui[8] = OTAA_DEVEUI;
  // OTAA Application EUI MSB first
  uint8_t node_app_eui[8] = OTAA_APPEUI;
  // OTAA Application Key MSB first
  uint8_t node_app_key[16] = OTAA_APPKEY;

  if (!api.lorawan.appeui.set(node_app_eui, 8))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set application EUI is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.appkey.set(node_app_key, 16))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set application key is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deui.set(node_device_eui, 8))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set device EUI is incorrect! \r\n");
    return;
  }

  if (!api.lorawan.band.set(OTAA_BAND))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set band is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set device class is incorrect! \r\n");
    return;
  }
  uint16_t maskBuff = 0x0002;
  if (!api.lorawan.mask.set(&maskBuff))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set mask is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.njm.set(RAK_LORA_OTAA)) // Set the network join mode to OTAA
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set network join mode is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.join()) // Join to Gateway
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - join fail! \r\n");
    return;
  }

  /** Wait for Join success */
  while (api.lorawan.njs.get() == 0)
  {
    DEBUG_SERIAL_LOG.print("Wait for LoRaWAN join 1...");
    api.lorawan.join();
    delay(10000);
    if (api.lorawan.njs.get() != 0)
      break;
    DEBUG_SERIAL_LOG.print("Wait for LoRaWAN join 2...");
    api.lorawan.join();
    delay(10000);
    if (api.lorawan.njs.get() != 0)
      break;
    DEBUG_SERIAL_LOG.print("Wait for LoRaWAN join 3...");
    api.lorawan.join();
    delay(10000);
    if (api.lorawan.njs.get() != 0)
      break;
    DEBUG_SERIAL_LOG.println("Connection failed! Sleeping...");
    while (true)
      api.system.sleep.all();
  }

  if (!api.lorawan.adr.set(true))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set adaptive data rate is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.rety.set(1))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set retry times is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.cfm.set(1))
  {
    DEBUG_SERIAL_LOG.printf("LoRaWan OTAA - set confirm mode is incorrect! \r\n");
    return;
  }

  /** Check LoRaWan Status*/
  DEBUG_SERIAL_LOG.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");            // Check Duty Cycle status
  DEBUG_SERIAL_LOG.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED"); // Check Confirm status
  uint8_t assigned_dev_addr[4] = {0};
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  DEBUG_SERIAL_LOG.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]); // Check Device Address
  DEBUG_SERIAL_LOG.printf("Uplink period is %ums\r\n", OTAA_PERIOD);
  api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerJoinCallback(joinCallback);
  api.lorawan.registerSendCallback(sendCallback);
}

void lora::sendUplink(char *payload)
{
  DEBUG_SERIAL_LOG_MORE.println("Starting uplink routine");
  /** Send the data package */
  sending = true;
  DEBUG_SERIAL_LOG.println("Sending request attempt 1");
  if (!api.lorawan.send(strlen(payload), (uint8_t *)&payload[0], 2, true, 2))
  {
    for (int i = 2; i < 4 && !api.lorawan.send(strlen(payload), (uint8_t *)&payload[0], 2, true, 2); i++)
    {
      delay(3000);
      DEBUG_SERIAL_LOG.printf("Sending request attempt %i\r\n", i);
    }
  }
  while (sending)
  {
    DEBUG_SERIAL_LOG.println("Waiting for sending...");
    delay(1500); // block until sending complete, to prevent sleeping during send, ceiling for usual send time
  }
  delay(3000); // for downlinks to be received
  while (receiving)
  {
    DEBUG_SERIAL_LOG.println("Waiting for receiving...");
    delay(1500); // block until receiving complete, to prevent sleeping during receive, ceiling for usual receive time
  }
  delay(1500); // for acknowledgement to be sent
  DEBUG_SERIAL_LOG_MORE.println("End of uplink routine");
}

void lora::sendUplink(lora::uplinkMessage msg)
{
  DynamicJsonDocument doc(1024); 
  doc["type"] = msg.type; 
  doc["battery"] = battery.getValue();
  doc["countdownTimer"] = fsm::getCountdownTimer() / 1000;
  doc["durationTimer"] = fsm::getDurationTimer() / 1000;
  doc["stillnessTimer"] = fsm::getStillnessTimer() / 1000;
  doc["heartbeatInterval"] = heartbeat::getInterval() / 1000;

  char output[1024] = ""; // arbitrary size
  serializeJson(doc, output, sizeof(output));

  DEBUG_SERIAL_LOG.printf("Json Uplink: %s\r\n", output);
  sendUplink(output);
}