#ifndef OTAA_H
#define OTAA_H

#include <ArduinoJson.h>

namespace lora
{
    const std::string DURATION = "duration"; 
    const std::string STILLNESS = "stillness"; 
    const std::string HEARTBEAT = "heartbeat";
    const std::string EEPROM_ERROR = "eepromError"; 
    struct uplinkMessage
    {
        std::string type; 
    };

    /**
     * setup the OTAA protocol and establish a connection with the gateway
     */
    void setupOTAA();

    /**
     * sends uplink message to the server
     * @param payload the message to send, no spaces as it will be base64 encoded
     */
    void sendUplink(char *payload);

    /**
     * sends uplink message to the server
     * @param payload the message to send, using ArduinoJson library
     */
    void sendUplink(lora::uplinkMessage msg); 

    /**
     * the callback function for the LoRa module to call when a message is received
     */
    static void recvCallback(SERVICE_LORA_RECEIVE_T *data);

    /**
     * the callback function for the LoRa module to call when a join request is received
     */
    static void joinCallback(int32_t status);

    /**
     * the callback function for the LoRa module to call when a message is sent
     */
    static void sendCallback(int32_t status);
}
#endif // OTAA_H