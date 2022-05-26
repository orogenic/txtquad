#pragma once

#include <stdbool.h>

// you implement this. the engine calls it once per frame.
// return true to quit, false to continue
typedef bool tqupdate(void);

// call this with your update function to start the engine
void tqstart(tqupdate *);

#include "types.h"

// NOTE matches struct txtquad in txt.vert
// NOTE alignof(txtquad) depends on AlgIntrin
typedef struct {
	m4  model;           // (align 4 or 16)  64     +
	v4  rgba;            // (align 4 or 16)  16     +
	union { u32 uint[3]; // (align 4      ) (4 * 3
	        v3  vec3; }; // (align 4      )  OR 12) +
	u32 value;           // (align 4      )  4      = 96
	} txtquad;           // (align 4 or 16)  96 % 16 == 0

#define Txtquad( Model \
               , Rgba  \
               , Value ) ((txtquad) { .model = Model \
                                    , .rgba  = Rgba  \
                                    , .value = Value })

extern struct tqtxt {
	m4      * viewproj;
	u32     * invertY;
	txtquad * quad;
	u32       quads;
} txt;

#include "config.h"

static inline void drawquad(txtquad const quad) {
	if (txt.quads < Quads) txt.quad[txt.quads++] = quad;
}

extern struct tqframe {
	u64  i;          // frame counter
	u64  it;         // integral *logical* current time
	f64  t, dt, ts,  // fractional *logical* current time, delta time, time scale
	     dtmax,      // upper bound of logical delta time *before* scaling (0. is no max)
	     rt, rdt;    // *real* current time, delta time
	s32  w, h,       // window size in screen coordinates
	     fbw, fbh;   // framebuffer size in pixels
	bool resized;    // true if window size changed since last frame (always true first frame)
	u32  imageIndex; // current swapchain image index
} frame;

extern struct tqconfig {
	s32   width, height; // window width and height in screen coordinates
	bool  windowed,      // default is fullscreen
	      decorated,     // default is undecorated
	      resizable,     // default is not resizable
	      maximized,     // default is not maximized
	      cursor;        // default is cursor hidden
	ccstr title;         // app and window title
	u32   version;       // app version
} config;

void tqDebugLevel(u8 level); // set engine's debug level

#ifdef DebugFps
# define RdtSize 60
# define AvgFps  (RdtSize / fps.rdtsum)
# define AvgRdt  (fps.rdtsum / RdtSize)
# define Sec     (fps.sec <= frame.rdt) // true once per second
extern struct tqfps {
	f64  rdt[RdtSize], // real delta time circular buffer
	     rdtsum,       // real delta time moving sum
	     sec;          // second accumulator
	char str[4];       // string of average FPS updated every 10 frames
} fps;
#endif

#include "debug.h"
#include "util.h"

#ifdef Input
# include "input.h"
#endif
