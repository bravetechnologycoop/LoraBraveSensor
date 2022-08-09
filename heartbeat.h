#ifndef HEARTBEAT_H
#define HEARTBEAT_H

/**
 * Gets the remaining heartbeat sleep duration.
 * @return The remaining heartbeat sleep duration in ms
 */
unsigned int getHeartbeatRemainingDuration(); 

/**
 * Gets the heartbeat interval.
 * @return The heartbeat interval in ms
 */
unsigned int getHeartbeatInterval(); 

#endif // HEARTBEAT_H