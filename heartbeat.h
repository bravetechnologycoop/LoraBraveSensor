#ifndef HEARTBEAT_H
#define HEARTBEAT_H

namespace heartbeat
{
    /**
     * Gets the remaining heartbeat sleep duration.
     * @return The remaining heartbeat sleep duration in ms
     */
    unsigned int getRemainingDuration();

    /**
     * Gets the heartbeat interval.
     * @return The heartbeat interval in ms
     */
    unsigned int getInterval();

    /**
     * Sets the heartbeat interval.
     * @param interval The heartbeat interval in ms
     * @return The heartbeat interval in ms, -1 if unsuccessful
     */
    int setInterval(unsigned int timer); 

    /**
     * Resets the heartbeat timers to their default values.
     */
    void resetTimers();

    /**
     * Fetches heartbeat interval from eeprom.
     */
    void setupHeartbeat();
}

#endif // HEARTBEAT_H