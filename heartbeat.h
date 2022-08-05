#ifndef HEARTBEAT_H
#define HEARTBEAT_H

/**
 * setup timer for heartbeat interval
 */
void setupHeartbeat();

/**
 * set the heartbeat interval in the flash address
 * @param interval the new heartbeat interval in ms
 * @returns true if successful, false otherwise
 */
bool setHeartbeatInterval(unsigned int interval);

/**
 * get the heartbeat interval from the flash address
 * @returns true if successful, false otherwise
 */
bool updateHeartbeatInterval();

/**
 * @returns the heartbeat interval in ms
 */
unsigned int getHeartbeatInterval()

/**
 * sends heartbeat message
 */
void heartbeatHandler(void);

#endif // HEARTBEAT_H