
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "types.h"
#include "log.h"

#if _WIN32
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0601 // Windows 7
	#endif
	#define WIN32_LEAN_AND_MEAN
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
#else
	#include <unistd.h>
#endif

namespace {

// Colors
#ifdef _WIN32
	#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else
	#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

// black      VTSEQ(30)
// red        VTSEQ(31)
// green      VTSEQ(32)
// yellow     VTSEQ(33)
// blue       VTSEQ(34)
// purple     VTSEQ(35)
// cyan       VTSEQ(36)
// light_gray VTSEQ(37)
// white      VTSEQ(37)
// light_red  VTSEQ(91)
// dim        VTSEQ(2)
// bold       VTSEQ(1)
// underline  VTSEQ(4)
// reset      VTSEQ(0)

const bool terminal_has_color = []() {
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hOut != INVALID_HANDLE_VALUE) {
		DWORD dwMode = 0;
		GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		return SetConsoleMode(hOut, dwMode) != 0;
	}
	return false;
#else
	if(!isatty(STDOUT_FILENO)) return false;

	if(const char* term = getenv("TERM")) {
		std::string str(term);
		if(str.find("color") != std::string::npos) return true;
		if(str.find("xterm") != std::string::npos) return true;
		if(str.find("cygwin") != std::string::npos) return true;
		if(str.find("linux") != std::string::npos) return true;
		if(str.find("screen") != std::string::npos) return true;
	}
	return false;
#endif
}();

const char* prefixes[] = { "DEBUG", "INFO", "WARNING", "ERROR" };
const char* colors[] = {VTSEQ(37; 2), VTSEQ(34), VTSEQ(33), VTSEQ(31) };

}

void LOG_F(LogLevel lvl, const char* fmt, ...) {
#if NDEBUG
	if(lvl < INFO) return;
#endif

	printf("%s[%s] ", (terminal_has_color ? colors[lvl + 1] : ""),
	       prefixes[lvl + 1]);

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("%s\n", (terminal_has_color ? VTSEQ(0) : ""));
}
