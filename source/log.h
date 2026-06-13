#ifndef LOG_H
#define LOG_H

#ifdef __GNUG__
	#define LIKE_PRINTF __attribute__((format(printf, 2, 3)))
#else
	#define LIKE_PRINTF
#endif

enum LogLevel {
	DEBUG = -1,
	INFO = 0,
	WARNING = 1,
	ERROR = 2
};

void LOG_F(LogLevel lvl, const char* fmt, ...) LIKE_PRINTF;

#endif
