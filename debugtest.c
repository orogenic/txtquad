#include "debug.h"

int main(void) {
#ifdef DebugRun
for (;;) {
#endif
	info ("info  %d\n", DebugInfo);
	warn ("warn  %d\n", DebugWarn);
	error("error %d\n", DebugError);
	  dbg("dbg   %d\n", Debug);
#ifdef DebugRun
	if (dbgrun == 0) break;
	dbg("changing to debug level %u\n", --dbgrun);
}
#endif
	return 0;
}
