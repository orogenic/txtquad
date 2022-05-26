#include <stdlib.h>
#include <stdio.h>

#include "util.h"

const char debugtest[] = "./debugtest.exe"
,          rmdebugtest[] = "rm debugtest.exe"
,         *debugtests[] =
{ "clang            -DDebugInfo  debugtest.c -o debugtest.exe"
, "clang            -DDebugWarn  debugtest.c -o debugtest.exe"
, "clang            -DDebugError debugtest.c -o debugtest.exe"
, "clang                         debugtest.c -o debugtest.exe"
, "clang -DDebugRun -DDebugInfo  debugtest.c -o debugtest.exe"
, "clang -DDebugRun -DDebugWarn  debugtest.c -o debugtest.exe"
, "clang -DDebugRun -DDebugError debugtest.c -o debugtest.exe"
, "clang -DDebugRun              debugtest.c -o debugtest.exe" };

int main(void) {
	for (int i = 0; i < ArraySize(debugtests); ++i) {
		puts(debugtests[i]);
		system(debugtests[i]);
		puts(debugtest);
		system(debugtest);
	}
	system(rmdebugtest);
	return 0;
}
