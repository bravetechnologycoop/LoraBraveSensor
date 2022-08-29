# Lora Based BraveSensor Firmware

# Project Overview

## Secrets File
Make a copy of `secretsExample.h` and rename it to `secrets.h`. It must go inside the root directory of the project. Replace the values for the DEVEUI, APPEUI, and APPKEY with values as follows: 
- DEVEUI: which can be found by scanning the QR code on the LoRaWan chip. 
- APPEUI: currently we use the same APPEUI as the one for BraveButtons
- APPKEY: APPEUI = hex(str(hex(DEVEUI)) + str(hex(APPEUI))), where '+' is string concatenation
An example one can be found in our private [living doc](https://app.clickup.com/2434616/v/dc/2a9hr-2261/2a9hr-8442). 

## Setup
Follow this [guide](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3272S-Breakout-Board/Quickstart/#rak3272s-breakout-board-as-a-stand-alone-device-using-rui3) on how to setup. The LoraBraveSensor directory can subsitute the Arduino Serial demo, as a guide on how to compile and upload our custom firmware. After going throught the entire process, I recommend using vscode's Arduino extension for easier file management. 

### Using Arduino Extension for VSCode
I recommend using VSCode's Arduino extension for easier file management, after going through the setup process with the Arduino IDE at least once. After opening the project with VSCode, if you notice red squiggly error messages for missing includes, follow [this](https://stackoverflow.com/questions/52234438/vs-code-giving-header-errors-for-arduino-missing-official-header) guide on how to fix them. On Windows 10, the board firmware files can be found at `C:/Users/{user}/AppData/Local/Arduino15/packages/rak_rui/hardware/stm32/3.4.2/cores/STM32WLE/component/rui_v3_api`, and can be added under `includePath` of the `.vscode/c_cpp_properties.json` file. 

## Serial Debugging
Can be done with any serial monitor (e.g. Arduino, Arduino's VSCode Extension, PlatformIO, etc.) with BAUD rate of 115200. Remember to compile the file with `DEBUG_LOG` set to `true`, or `DEBUG_LOG_MORE` set to `true` in `main.h` to be able to see serial logs. Setting the two values to `false` for compiliation of production code can potentially reduce power consumption, by skipping unusable serial signals. 

## Libraries
- [Arduino](https://www.arduino.cc/reference/en/) for general firmware
- [RAKwireless unified Interface V3 (RUI3)](https://docs.rakwireless.com/RUI3/) for hardware interfacing
- [ArduinoJson](https://arduinojson.org/) for LoRaWAN data

# Firmware

## State Machine
Based on the [state machine diagram](https://docs.google.com/drawings/d/14JmUKDO-Gs7YLV5bhE67ZYnGeZbBg-5sq0fQYwkhkI0/edit) from the original Particle BraveSensor

## PINS
[Diagram for pins](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3272S-Breakout-Board/Datasheet/#specifications)

SPI_MOSI (PA7) is the inverted door pin (low is door open, high is door closed). 

SPI_MISO (PA6) is the inverted motion pin (low is motion, high is no motion).

SPI_CLK (PA5) is the analog battery measurement pin (untested so far, usually returns 8### as the measurement). 

SPI_CS (PA4) is the eeprom reset pin. 

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
Downlink payload data use a base64 encoded json, e.g. , that is [converted](https://codebeautify.org/json-to-base64-converter), and sent via AWS IoT for LoRaWan -> Devices -> <i>Device ID</i> -> Device Traffic -> Queue downlink message -> Payload field (any FPort will do). 
For example: `# Lora Based BraveSensor Firmware

# Project Overview

## Setup
Follow this [guide](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3272S-Breakout-Board/Quickstart/#rak3272s-breakout-board-as-a-stand-alone-device-using-rui3) on how to setup. The LoraBraveSensor directory can subsitute the Arduino Serial demo, as a guide on how to compile and upload our custom firmware. After going throught the entire process, I recommend using vscode's Arduino extension for easier file management. 

## Secrets File
DEVEUI: which can be found by scanning the QR code on the LoRaWan chip. 
APPEUI: currently we use the same APPEUI as the one for BraveButtons
APPEUI: APPEUI = hex(str(hex(DEVEUI)) + str(hex(APPEUI)))
An example one can be found in our private [living doc](https://app.clickup.com/2434616/v/dc/2a9hr-2261/2a9hr-8442)

### Using Arduino Extension for VSCode
I recommend using VSCode's Arduino extension for easier file management, after going through the setup process with the Arduino IDE at least once. After opening the project with VSCode, if you notice red squiggly error messages for missing includes, follow [this](https://stackoverflow.com/questions/52234438/vs-code-giving-header-errors-for-arduino-missing-official-header) guide on how to fix them. On Windows 10, the board firmware files can be found at `C:/Users/{user}/AppData/Local/Arduino15/packages/rak_rui/hardware/stm32/3.4.2/cores/STM32WLE/component/rui_v3_api`, and can be added under `includePath` of the `.vscode/c_cpp_properties.json` file. 

## Serial Debugging
Can be done with any serial monitor (e.g. Arduino, Arduino's VSCode Extension, PlatformIO, etc.) with BAUD rate of 115200. 

## Libraries
- [Arduino](https://www.arduino.cc/reference/en/) for general firmware
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
Downlink payload data use a base64 encoded json, e.g. , that is [converted](https://codebeautify.org/json-to-base64-converter), and sent via AWS IoT for LoRaWan -> Devices -> <i>Device ID</i> -> Device Traffic -> Queue downlink message -> Payload field (any FPort will do). 
For example: `{"type":"heartbeat","battery":8192,"countdownTimer":10,"durationTimer":30,"stillnessTimer":30,"heartbeatInterval":60}` encoded to `eyJ0eXBlIjoiaGVhcnRiZWF0IiwiYmF0dGVyeSI6ODE5MiwiY291bnRkb3duVGltZXIiOjEwLCJkdXJhdGlvblRpbWVyIjozMCwic3RpbGxuZXNzVGltZXIiOjMwLCJoZWFydGJlYXRJbnRlcnZhbCI6NjB9`
### Uplinks
Uplinks can be viewed at AWS IoT for LoRaWan -> Test -> MQTT test client, and subscribing to `rak3272/pub`, with the payload being a base64 encoded json. 
For example:  `eyJ0eXBlIjoiaGVhcnRiZWF0IiwiYmF0dGVyeSI6ODE5MiwiY291bnRkb3duVGltZXIiOjEwLCJkdXJhdGlvblRpbWVyIjozMCwic3RpbGxuZXNzVGltZXIiOjMwLCJoZWFydGJlYXRJbnRlcnZhbCI6NjB9` 
decodes to `{"type":"heartbeat","battery":8192,"countdownTimer":10,"durationTimer":30,"stillnessTimer":30,"heartbeatInterval":60}`). 

## Power Saving Measures
The system is designed to go to sleep, unless there are changes to the digital input pins, or a timer interrupt. Limitation's of RUI3, results in a specific implementation -- more info can be found [here](https://forum.rakwireless.com/t/rui3-wake-on-interrupt/7460/8). 
RAK_TIMER_#, are one of five hardware timers provided by RUI3. 

## Flash Memory
Saving timers (e.g. countdown/duration/stillness) is done using the [RUI3 flash library](https://docs.rakwireless.com/RUI3/System/#flash). 
` encoded to `ewogICAgImNvdW50ZG93blRpbWVyIjogMTUsIAogICAgImR1cmF0aW9uVGltZXIiOiA2MCwgCiAgICAic3RpbGxuZXNzVGltZXIiOiAzMCwgCiAgICAiaGVhcnRiZWF0SW50ZXJ2YWwiOiAyNDAKfQ==`
### Uplinks
Uplinks can be viewed at AWS IoT for LoRaWan -> Test -> MQTT test client, and subscribing to `rak3272/pub`, with the payload being a base64 encoded json. 
For example:  `eyJhbGVydFR5cGUiOiJEdXJhdGlvbkFsZXJ0IiwiYmF0dGVyeSI6ODE5MiwiY291bnRkb3duVGltZXIiOjE1LCJkdXJhdGlvblRpbWVyIjozMCwic3RpbGxuZXNzVGltZXIiOjE1LCJoZWFydGJlYXRJbnRlcnZhbCI6MzAwfQ==` 
decodes to `{"alertType":"DurationAlert","battery":8192,"countdownTimer":15,"durationTimer":30,"stillnessTimer":15,"heartbeatInterval":300}`). 

## Power Saving Measures
The system is designed to go to sleep, unless there are changes to the digital input pins, or a timer interrupt. Limitation's of RUI3, results in a specific implementation -- more info can be found [here](https://forum.rakwireless.com/t/rui3-wake-on-interrupt/7460/8). 

## Flash Memory (EEPROM)
Saving timers (e.g. countdown/duration/stillness) is done using the [RUI3 flash library](https://docs.rakwireless.com/RUI3/System/#flash). 
The eeprom can be reset to the default values by rising the eeprom reset pin SPI_CS (PA4). 
