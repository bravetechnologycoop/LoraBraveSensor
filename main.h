#ifndef MAIN_H
#define MAIN_H

#define DEBUG_LOG_MORE false // Provides overly verbose debug/state logging
#define DEBUG_SERIAL_LOG_MORE if(DEBUG_LOG_MORE)Serial
#define DEBUG_LOG true // Provides general debug/state logging
#define DEBUG_SERIAL_LOG if(DEBUG_LOG || DEBUG_LOG_MORE)Serial

#endif // MAIN_H