#include <stdio.h>

#define Input
#define InputUtil
#include "txtquad.h"

#include "alg.h"

#define FontBit(x, y) (GridOffset(x) + GridOffset(y) * 128)
#define Bitmask(b) (1 << (7 - b))
#define GridOffset(x) ((x) - (1 + (x) / 9))
#define GridZ 0.002f
#define CursorZ 0.001f

#define ModPressed(Mod) (KeyPressed(Left##Mod) || KeyPressed(Right##Mod))

static const u32 PointCount = 16 * 9 + 1; // including inner and outer edge grid lines
static u8 font[8 * 16 * 16]; // 16^2 glyphs, 8^2 bits/glyph
static f32 pointSize;
static f32 zoom = 1.f;
static struct {
	s16 c, r;
	f32 x, y;
} cursor;
static struct {
	u16 bits[1000], undo, redo;
} edits = { 0 };
static v2 centering = V20;
static v2 pan = V20;
static bool painted[SwapchainImageCount] = { 0 };
static ccstr fontFilename;

static inline void cell(s16 c, s16 r, f32 z, v4 rgba) {
	drawquad(Txtquad(m4trs(V3( centering.x + pan.x + c * pointSize
	                         , centering.y + pan.y + r * pointSize
	                         , z ), Qi, pointSize), rgba, 1));
}

static inline void gridcell(u8 c, u8 r) {
	cell(c, r, GridZ, V4ew(.1f, 1.f));
}

static inline void fontcell(u8 c, u8 r) {
	u16 b = FontBit(c, r);
	if (font[b / 8] & Bitmask(b % 8)) cell(c, r, GridZ, V41);
}

static void repaint(void) {
	txt.quads = 0;
	for (u8 c = 0; c < PointCount; ++c)
		for (u8 r = 0; r < PointCount; ++r)
			if (!(r % 9) || !(c % 9))
				gridcell(c, r);
			else
				fontcell(c, r);
	cell(cursor.c, cursor.r, CursorZ, V4yw(1.f, 1.f));
	*txt.viewproj = m4screen(frame.w, frame.h, false);
	painted[frame.imageIndex] = true;
}

static void invalidate(void) {
	for (u8 i = 0; i < SwapchainImageCount; ++i) painted[i] = false;
}

static void recalc(void) {
	cursor.c = (cursor.x - centering.x - pan.x) / pointSize;
	cursor.r = (cursor.y - centering.y - pan.y) / pointSize;
	invalidate();
}

static void resize(void) {
	if (frame.w < frame.h) {
		pointSize = (f32) frame.w / PointCount * zoom;
		centering = V2(0.f, (frame.h - pointSize * PointCount) * .5f);
	} else {
		pointSize = (f32) frame.h / PointCount * zoom;
		centering = V2((frame.w - pointSize * PointCount) * .5f, 0.f);
	}
	recalc();
}

static void edit(void) {
	if ( 0 <= cursor.c && cursor.c <= PointCount
	  && 0 <= cursor.r && cursor.r <= PointCount
	  && (cursor.c % 9)
	  && (cursor.r % 9) ) {
		u16 b = edits.bits[edits.undo++]
		      = FontBit(cursor.c, cursor.r);
		edits.redo = edits.undo;
		font[b / 8] ^= Bitmask(b % 8);
		invalidate();
	}
}

static void undo(void) {
	if (!edits.undo) return;
	u16 b = edits.bits[--edits.undo];
	font[b / 8] ^= Bitmask(b % 8);
	invalidate();
}

static void redo(void) {
	if (edits.undo == edits.redo) return;
	u16 b = edits.bits[edits.undo++];
	font[b / 8] ^= Bitmask(b % 8);
	invalidate();
}

static void readFont(ccstr in) {
	FILE *i = fopen(in, "rb");
	if (NULL == i)
		goto FileError;
	// TODO can initialize font memory to zero if font file does not exist or is empty (length 0)
	if (EOF == fseek(i, 11, SEEK_SET))
		goto FileError;
	if (sizeof(font) != fread(font, 1, sizeof(font), i))
		goto FileError;
	if (EOF == fclose(i))
		goto FileError;
	invalidate();
	return;
FileError:
	fail("Error reading font");
}

static void writeFont(ccstr out) {
	FILE *o = fopen(out, "wb");
	if (NULL == o)
		goto FileError;
	if (EOF == fputs("P4 128 128\n", o))
		goto FileError;
	if (sizeof(font) != fwrite(font, 1, sizeof(font), o))
		goto FileError;
	if (EOF == fclose(o))
		goto FileError;
	return;
FileError:
	fail("Error writing font");
}

static bool fontCursor(f64 x, f64 y) {
	cursor.x = x;
	cursor.y = y;
	recalc();
	return false;
}

static bool fontScroll(f64 x, f64 y) {
	if (ModPressed(Shift)) {
		     if (y > 0) zoom *= 2.f;
		else if (y < 0) zoom *= .5f;
		resize();
	} else {
		     if (x > 0) pan.x += pointSize * 9;
		else if (x < 0) pan.x -= pointSize * 9;
		     if (y > 0) pan.y += pointSize * 9;
		else if (y < 0) pan.y -= pointSize * 9;
		recalc();
	}
	return false;
}

static bool fontUpdate(void) {
	if (ModPressed(Shift)) {
		if (KeyDown(R)) readFont(fontFilename);
		if (KeyDown(W) || KeyDown(X)) writeFont(fontFilename);
		if (KeyDown(X) || KeyDown(Q)) return true;
		if (KeyDown(Enter)) { zoom  = 1.f; resize(); }
		if (KeyDown(Up   )) { zoom *= 2.f; resize(); }
		if (KeyDown(Down )) { zoom *= .5f; resize(); }
		if (MouseDown(Right)) redo();
	} else {
		if (KeyDown(Enter)) { pan    = V20;             recalc(); }
		if (KeyDown(Up   )) { pan.y -= pointSize * 9.f; recalc(); }
		if (KeyDown(Down )) { pan.y += pointSize * 9.f; recalc(); }
		if (KeyDown(Left )) { pan.x -= pointSize * 9.f; recalc(); }
		if (KeyDown(Right)) { pan.x += pointSize * 9.f; recalc(); }
		if (MouseDown(Right)) undo();
	}

	if (MouseDown(Left)) edit();
	if (MouseDown(Middle)) { zoom = 1.f; pan = V20; resize(); }
	if (frame.resized) resize();
	if (!painted[frame.imageIndex]) repaint();
	return false;
}

int main(int argc, char *argv[]) {
	if (argc < 2) fontFilename = "font.pbm";
	else if (argc == 2) fontFilename = argv[1];
	else return 1;
	readFont(fontFilename);
	config.title = fontFilename;
	input.cursor = fontCursor;
	input.scroll = fontScroll;
	tqDebugLevel(DebugOff);
	tqstart(fontUpdate);
	return 0;
}
