#ifndef OTAA_H
#define OTAA_H

#include <ArduinoJson.h>

namespace lora
{
    struct uplinkMessage
    {
        enum alertType_t { DURATION = 0, STILLNESS = 1, HEARTBEAT = 2 };
        alertType_t alertType;
    };
    const char JSON_ALERT_TYPE_KEY[] = "alertType"; 
    const char JSON_ALERT_TYPE_VALUE_DURATION_ALERT[] = "DurationAlert";
    const char JSON_ALERT_TYPE_VALUE_STILLNESS_ALERT[] = "StillnessAlert";
    const char JSON_ALERT_TYPE_VALUE_HEARTBEAT[] = "Heartbeat";
    const char JSON_BATTERY_LEVEL_KEY[] = "batteryLevel"; 
    const char JSON_COUNTDOWN_TIMER_KEY[] = "countdownTimer";
    const char JSON_DURATION_TIMER_KEY[] = "durationTimer";
    const char JSON_STILLNESS_TIMER_KEY[] = "stillnessTimer";
    const char JSON_HEARTBEAT_INTERVAL_KEY[] = "heartbeatInterval";

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