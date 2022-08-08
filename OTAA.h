#ifndef OTAA_H
#define OTAA_H

/**
 * setup the OTAA protocol and establish a connection with the gateway
 */
void setupOTAA();

/**
 * sends uplink message to the server
 * @param payload the message to send, no spaces as it will be base64 encoded
 */
void uplink_routine(char *payload);

#endif // OTAA_H