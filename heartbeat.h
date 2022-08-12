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
}

#endif // HEARTBEAT_H