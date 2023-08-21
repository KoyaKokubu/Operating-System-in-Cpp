#pragma once

enum LogLevel {
    kError = 3,
    kWarn  = 4,
    kInfo  = 6,
    kDebug = 7,
};

/** @brief change threshold of LogLevel.
*
* Print only records above the threshold which is set here.
*/
void SetLogLevel(enum LogLevel level);
int Log(enum LogLevel level, const char* format, ...);