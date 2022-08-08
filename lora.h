#ifndef OTAA_H
#define OTAA_H

#include <ArduinoJson.h>

/**
 * setup the OTAA protocol and establish a connection with the gateway
 */
void setupOTAA();

/**
 * sends uplink message to the server
 * @param payload the message to send, no spaces as it will be base64 encoded
 */
void uplink_routine(char *payload);

/**
 * sends uplink message to the server
 * @param payload the message to send, using ArduinoJson library
 */
void uplink_routine(DynamicJsonDocument payload);

#endif // OTAA_H