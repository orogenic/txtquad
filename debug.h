#pragma once

/*
DebugInfo   define to enable debug level 3 (info) and lower
DebugWarn   define to enable debug level 2 (warn) and lower
DebugError  define to enable debug level 1 (error)

DebugRun    define to enable runtime debug level
            if defined, DebugInfo DebugWarn and DebugError set the initial debug level

DebugOn     debug code is live if any of the above are defined
            use it to conditionally compile debug-only code

Debug       the debug level (initial for runtime, static for compile-time)
            use it to reset runtime debug level
            or to write complex compile-time debug logic
*/

#if   defined(DebugInfo)
	#undef DebugInfo
	#define Debug 3
	#ifndef DebugRun
		#define info dbg
		#define warn dbg
		#define error dbg
	#endif
#elif defined(DebugWarn)
# undef DebugWarn
# define Debug 2
# ifndef DebugRun
#  define info(...)
#  define warn dbg
#  define error dbg
# endif
#elif defined(DebugError)
# undef DebugError
# define Debug 1
# ifndef DebugRun
#  define info(...)
#  define warn(...)
#  define error dbg
# endif
#else
# define Debug 0
# ifndef DebugRun
#  define info(...)
#  define warn(...)
#  define error(...)
# endif
#endif

#define DebugInfo  3
#define DebugWarn  2
#define DebugError 1
#define DebugOff   0

#if Debug || defined(DebugRun)
# define DebugOn 1
#else
# define DebugOn 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#define dbg(...) fprintf(stderr, __VA_ARGS__)

#if DebugOn
# include <stdarg.h>
# define fail(fmt, ...) _fail("Error: " fmt ": exiting\n", ##__VA_ARGS__)
static noreturn void _fail(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}
#else
# define fail(...) _fail()
static noreturn void _fail(void) {
	exit(EXIT_FAILURE);
}
#endif

#ifdef DebugRun
# define info(...)  do { if (debugLevel >= DebugInfo ) dbg(__VA_ARGS__); } while (0)
# define warn(...)  do { if (debugLevel >= DebugWarn ) dbg(__VA_ARGS__); } while (0)
# define error(...) do { if (debugLevel >= DebugError) dbg(__VA_ARGS__); } while (0)
# define DebugLevel(level) (debugLevel = (level))
static unsigned char debugLevel = Debug;
#else
# define DebugLevel(...)
#endif

