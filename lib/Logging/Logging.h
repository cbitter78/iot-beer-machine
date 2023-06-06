#ifndef LOGGING_H
#define LOGGING_H

#ifdef PRINT_DEBUG
#define DEBUG_PRINT(...)    { Serial.print(__VA_ARGS__);   }
#define DEBUG_PRINTLN(...)  { Serial.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...)    {}
#define DEBUG_PRINTLN(...)  {}
#endif

#ifdef PRINT_INFO
#define INFO_PRINT(...)     { Serial.print(__VA_ARGS__);   }
#define INFO_PRINTLN(...)   { Serial.println(__VA_ARGS__); }
#else
#define INFO_PRINT(...)     {}
#define INFO_PRINTLN(...)   {}
#endif

#ifdef PRINT_WARN
#define WARN_PRINT(...)     { Serial.print(__VA_ARGS__);   }
#define WARN_PRINTLN(...)   { Serial.println(__VA_ARGS__); }
#else
#define WARN_PRINT(...)     {}
#define WARN_PRINTLN(...)   {}
#endif

#ifdef PRINT_ERROR
#define ERROR_PRINT(...)    { Serial.print(__VA_ARGS__);   }
#define ERROR_PRINTLN(...)  { Serial.println(__VA_ARGS__); }
#else
#define ERROR_PRINT(...)    {}
#define ERROR_PRINTLN(...)  {}
#endif

#endif