# Lora Based BraveSensor Firmware


# Project Setup
Follow this [guide](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3272S-Breakout-Board/Quickstart/#rak3272s-breakout-board-as-a-stand-alone-device-using-rui3) on how to setup. I recommend using vscode's Arduino extension for easier file management. 
## Libraries
- [Arduino](https://docs.arduino.cc/) for general firmware
- [RAKwireless unified Interface V3 (RUI3)](https://docs.rakwireless.com/RUI3/) for hardware interfacing
- [ArduinoJson](https://arduinojson.org/) for LoRaWAN data

# Firmware

## State Machine
Based on the [state machine diagram](https://docs.google.com/drawings/d/14JmUKDO-Gs7YLV5bhE67ZYnGeZbBg-5sq0fQYwkhkI0/edit) from the original Particle BraveSensor


## Sensors

### Door Sensor
Assumes the usage of a binary door sensor, with a low being door open, and a high being door closed. The `SP1_MOSI` is the pin being used. 

### Motion Sensor
Assumes the usage of a binary motion sensor (such as the PIR), with a low meaning there is motion, and a high meaning there is no motion. The `SP1_MISO` is the pin being used. 

## Heartbeats
The heartbeat system is ran on a timer provided by RAK's RUI3, and will send a message to AWS in `HEARTBEAT_INTERVAL`ms intervals. 

## LoRaWan
OTAA is used to connect to the LoRaWan gateway, following [this](https://news.rakwireless.com/get-started-with-rui3-api/) guide. Changes have been made, such as using a [sub-band 2](https://forum.rakwireless.com/t/connecting-rak3172s-breakout-board-to-aws-iot-for-lorawan/7366) (8 channels). Upon device start/restart, it will attempt to connect to a gateway thrice, over 30 seconds, and will go to sleep should that fail. LoRa send requests are also done up to three times, at two sends per request. 
### Downlinks
Downlink payload data use a base64 encoded json, e.g. , that is [converted](https://codebeautify.org/json-to-base64-converter), and sent via AWS IoT for LoRaWan -> Devices -> <i>Device ID</i> -> Device Traffic -> Qeuue downlink message -> Payload field (any FPort will do). 
For example: `{
    "countdownTimer": 15, 
    "durationTimer": 60, 
    "stillnessTimer": 30, 
    "heartbeatInterval": 240
}` encoded to `ewogICAgImNvdW50ZG93blRpbWVyIjogMTUsIAogICAgImR1cmF0aW9uVGltZXIiOiA2MCwgCiAgICAic3RpbGxuZXNzVGltZXIiOiAzMCwgCiAgICAiaGVhcnRiZWF0SW50ZXJ2YWwiOiAyNDAKfQ==`
### Uplinks
Uplinks can be viewed at AWS IoT for LoRaWan -> Test -> MQTT test client, and subscribing to `rak3272/pub`, with the payload being a base64 encoded json. 
For example:  `eyJhbGVydFR5cGUiOiJEdXJhdGlvbkFsZXJ0IiwiYmF0dGVyeSI6ODE5MiwiY291bnRkb3duVGltZXIiOjE1MDAwLCJkdXJhdGlvblRpbWVyIjo2MCwic3RpbGxuZXNzVGltZXIiOjMwMDAwLCJoZWFydGJlYXRJbnRlcnZhbCI6MTIwMDAwfQ==` 
decodes to `{"alertType":"DurationAlert","battery":8192,"countdownTimer":15000,"durationTimer":60,"stillnessTimer":30000,"heartbeatInterval":120000}`). 

## Power Saving Measures
The system is designed to go to sleep, unless there are changes to the digital input pins, or a timer interrupt. Limitation's of RUI3, results in a specific implementation -- more info can be found [here](https://forum.rakwireless.com/t/rui3-wake-on-interrupt/7460/8). 

## Flash Memory
Saving timers (e.g. countdown/duration/stillness) is done using the [RUI3 flash library](https://docs.rakwireless.com/RUI3/System/#flash). 
