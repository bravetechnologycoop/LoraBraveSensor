#include <Arduino.h>
#include <ArduinoJson.h>
#include "fsm.h"
#include "heartbeat.h"
#include "secrets.h"
#include "battery.h"

#define OTAA_PERIOD (20000)

bool sending = false;
bool receiving = false;

void recvCallback(SERVICE_LORA_RECEIVE_T *data)
{
  receiving = true;
  if (data->BufferSize > 0)
  {
    Serial.println("Something received!");
    for (int i = 0; i < data->BufferSize; i++)
    {
      Serial.printf("%x", data->Buffer[i]);
    }
    Serial.print("\r\n");

    // testing
    Serial.println((char *)data->Buffer);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (char *)data->Buffer);
    if (doc.containsKey("countdownTimer"))
    {
      Serial.printf("countdownTimer set as %u\r\n", doc["countdownTimer"].as<unsigned int>() * 1000);
      setCountdownTimer(doc["countdownTimer"].as<unsigned int>() * 1000);
    }
    if (doc.containsKey("durationTimer"))
    {
      Serial.printf("durationTimer set as %i\r\n", doc["durationTimer"].as<unsigned int>() * 1000);
      setDurationTimer(doc["durationTimer"].as<unsigned int>());
    }
    if (doc.containsKey("stillnessTimer"))
    {
      Serial.printf("stillnessTimer set as %i\r\n", doc["stillnessTimer"].as<unsigned int>() * 1000);
      setStillnessTimer(doc["stillnessTimer"].as<unsigned int>() * 1000);
    }
    if (doc.containsKey("heartbeatInterval"))
    {
      Serial.printf("heartbeatInterval set as %i\r\n", doc["heartbeatInterval"].as<unsigned int>() * 1000);
      setHeartbeatInterval(doc["heartbeatInterval"].as<unsigned int>() * 1000);
    }
  }
  receiving = false;
}

void joinCallback(int32_t status)
{
  Serial.printf("Join status: %d\r\n", status);
}

void sendCallback(int32_t status)
{
  if (status == 0)
  {
    Serial.println("Successfully sent (callback)");
  }
  else
  {
    Serial.println("Sending failed (callback)");
  }
  sending = false;
}

void setupOTAA()
{
  Serial.begin(115200, RAK_AT_MODE);

  Serial.println("BraveSensor");
  Serial.println("------------------------------------------------------");

  // OTAA Device EUI MSB first
  uint8_t node_device_eui[8] = OTAA_DEVEUI;
  // OTAA Application EUI MSB first
  uint8_t node_app_eui[8] = OTAA_APPEUI;
  // OTAA Application Key MSB first
  uint8_t node_app_key[16] = OTAA_APPKEY;

  if (!api.lorawan.appeui.set(node_app_eui, 8))
  {
    Serial.printf("LoRaWan OTAA - set application EUI is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.appkey.set(node_app_key, 16))
  {
    Serial.printf("LoRaWan OTAA - set application key is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deui.set(node_device_eui, 8))
  {
    Serial.printf("LoRaWan OTAA - set device EUI is incorrect! \r\n");
    return;
  }

  if (!api.lorawan.band.set(OTAA_BAND))
  {
    Serial.printf("LoRaWan OTAA - set band is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A))
  {
    Serial.printf("LoRaWan OTAA - set device class is incorrect! \r\n");
    return;
  }
  uint16_t maskBuff = 0x0002;
  if (!api.lorawan.mask.set(&maskBuff))
  {
    Serial.printf("LoRaWan OTTA - set mask is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.njm.set(RAK_LORA_OTAA)) // Set the network join mode to OTAA
  {
    Serial.printf("LoRaWan OTAA - set network join mode is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.join()) // Join to Gateway
  {
    Serial.printf("LoRaWan OTAA - join fail! \r\n");
    return;
  }

  /** Wait for Join success */
  while (api.lorawan.njs.get() == 0)
  {
    Serial.print("Wait for LoRaWAN join 1...");
    api.lorawan.join();
    delay(10000);
    if (api.lorawan.njs.get() != 0) break; 
    Serial.print("Wait for LoRaWAN join 2...");
    api.lorawan.join();
    delay(10000);
    if (api.lorawan.njs.get() != 0) break; 
    Serial.print("Wait for LoRaWAN join 3...");
    api.lorawan.join();
    delay(10000);
    if (api.lorawan.njs.get() != 0) break; 
    Serial.println("Connection failed! Sleeping...");
    while (true) api.system.sleep.all(); 
  }

  if (!api.lorawan.adr.set(true))
  {
    Serial.printf("LoRaWan OTAA - set adaptive data rate is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.rety.set(1))
  {
    Serial.printf("LoRaWan OTAA - set retry times is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.cfm.set(1))
  {
    Serial.printf("LoRaWan OTAA - set confirm mode is incorrect! \r\n");
    return;
  }

  /** Check LoRaWan Status*/
  Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");            // Check Duty Cycle status
  Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED"); // Check Confirm status
  uint8_t assigned_dev_addr[4] = {0};
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]); // Check Device Address
  Serial.printf("Uplink period is %ums\r\n", OTAA_PERIOD);
  api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerJoinCallback(joinCallback);
  api.lorawan.registerSendCallback(sendCallback);
}

void uplink_routine(char *payload)
{
  Serial.println("Starting uplink routine");
  /** Send the data package */
  sending = true;
  Serial.println("Sending request attempt 1");
  if (!api.lorawan.send(strlen(payload), (uint8_t *)&payload[0], 2, true, 2))
  {
    for (int i = 2; i < 4 && !api.lorawan.send(strlen(payload), (uint8_t *)&payload[0], 2, true, 2); i++)
    {
      delay(3000);
      Serial.printf("Sending request attempt %i\r\n", i);
    }
  }
  while (sending)
  {
    Serial.println("Waiting for sending...");
    delay(1500); // block until sending complete, to prevent sleeping during send, ceiling for usual send time
  }
  delay(1500); // for downlinks to be received
  while (receiving)
  {
    Serial.println("Waiting for receiving...");
    delay(1500); // block until receiving complete, to prevent sleeping during receive, ceiling for usual receive time
  }
  delay(1500); // for acknowledgement to be sent
  Serial.println("End of uplink routine");
}

void uplink_routine(DynamicJsonDocument doc) {
    doc["battery"] = getBatteryLevel();
    doc["countdownTimer"] = getCountdownTimer();
    doc["durationTimer"] = getDurationTimer();
    doc["stillnessTimer"] = getStillnessTimer();
    doc["heartbeatInterval"] = getHeartbeatInterval();

    char output[1024] = ""; // arbitrary size
    serializeJson(doc, output, sizeof(output)); 

    Serial.printf("Json Uplink: %s\r\n", output);
    uplink_routine(output);
}