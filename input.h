#pragma once

#include <stdbool.h>
#include "types.h"

// you can implement these three functions
// for each function, return true to quit, false to continue
typedef bool tqunicode(u32 unicode); // unicode character callback
typedef bool tqcursor(f64 x, f64 y); // mouse cursor position callback
typedef bool tqscroll(f64 x, f64 y); // mouse scroll offset callback

#include "inputdefines.h"
#define   Key(K) TQ_KEY_##K
#define Mouse(B) TQ_BUTTON_##B

extern struct tqinput {
	// you must set the callbacks you want to enable *before* calling tqstart
	tqunicode * unicode;
	tqcursor  * cursor;
	tqscroll  * scroll;

	// 8 frames of memory per input, 1 bit per frame.
	// Lowest bit is most recent frame
	u8 key    [  Key(LAST) + 1],
	   button [Mouse(LAST) + 1];

	// these lists get polled every frame
	int * keys,
	    * buttons;
	u16   keyCount,
	      buttonCount;
} input;

// SetKeys(Key(), Key(), ...); SetButtons(Mouse(), Mouse(), ...);
#define    SetKeys(...)    setKeys( ArrayLength((int[]) { __VA_ARGS__ }) \
                                  ,             (int[]) { __VA_ARGS__ }  )
#define SetButtons(...) setButtons( ArrayLength((int[]) { __VA_ARGS__ }) \
                                  ,             (int[]) { __VA_ARGS__ }  )

//           Pressed = is down this frame
#define   KeyPressed(K) ((GetKey   (K) & 0b01) == 0b01)
#define MousePressed(B) ((GetMouse (B) & 0b01) == 0b01)

//              Down = is down this frame, was not down last frame
#define      KeyDown(K) ((GetKey   (K) & 0b11) == 0b01)
#define    MouseDown(B) ((GetMouse (B) & 0b11) == 0b01)

//                Up = is not down this frame, was down last frame
#define        KeyUp(K) ((GetKey   (K) & 0b11) == 0b10)
#define      MouseUp(B) ((GetMouse (B) & 0b11) == 0b10)

//              Held = is down this frame, was down last frame
#define      KeyHeld(K) ((GetKey   (K) & 0b11) == 0b11)
#define    MouseHeld(B) ((GetMouse (B) & 0b11) == 0b11)

// InputUtil makes Key* and Mouse* macros register inputs for polling, so you
// don't need to use SetKeys and SetButtons to specify which inputs to poll.
// If inputUtilFilename is set, registered inputs are written to that filename
// after tqstart returns.
#ifdef InputUtil
static ccstr inputUtilFilename = NULL;

typedef struct { bool found; ccstr name; } find;

static find _key     [  Key(LAST) + 1] = { 0 },
            _button  [Mouse(LAST) + 1] = { 0 };
static int  _keys    [  Key(LAST) + 1],
            _buttons [Mouse(LAST) + 1];

static u8 getKey (int key, ccstr name)
{
	if (_key[key].found) goto found;
	_key[key] = (find) { true, name };
	_keys[input.keyCount++] = key;
	info("InputUtil found key %s\n", name);
	found: return input.key[key];
}

static u8 getButton (int button, ccstr name)
{
	if (_button[button].found) goto found;
	_button[button] = (find) { true, name };
	_buttons[input.buttonCount++] = button;
	info("InputUtil found button %s\n", name);
	found: return input.button[button];
}

static void setKeys (u16 const keyCount, int keys[static const keyCount])
{
	for (u16 i = 0; i < keyCount; ++i)
		_key[_keys[i] = keys[i]].found = true;
	input.keyCount = keyCount;
}

static void setButtons (u8 const buttonCount, int buttons[static const buttonCount])
{
	for (u8 i = 0; i < buttonCount; ++i)
		_button[_buttons[i] = buttons[i]].found = true;
	input.buttonCount = buttonCount;
}

static void inputUtil (void)
{ input.keys = _keys; input.buttons = _buttons; }

static void writeKeys (FILE *o, ccstr out)
{
	if (!input.keyCount) return;
	fprintf(o, "SetKeys(\n");
	for (u16 i = 0; i < input.keyCount; ++i)
		_key[_keys[i]].name ? fprintf(o, "\tKey(%s),\n", _key[_keys[i]].name )
		                    : fprintf(o,      "\t%d,\n",      _keys[i]       ) ;
	fprintf(o, ");\n");
	info("InputUtil wrote keys to %s\n", out);
}

static void writeButtons (FILE *o, ccstr out)
{
	if (!input.buttonCount) return;
	fprintf(o, "SetButtons(\n");
	for (u8 i = 0; i < input.buttonCount; ++i)
		_button[_buttons[i]].name ? fprintf(o, "\tMouse(%s),\n", _button[_buttons[i]].name )
		                          : fprintf(o,        "\t%d,\n",         _buttons[i]       ) ;
	fprintf(o, ");\n");
	info("InputUtil wrote buttons to %s\n", out);
}

static void writeInputs (void)
{
	if (!inputUtilFilename) return;
	FILE *o = fopen(inputUtilFilename, "wb");
	writeKeys    (o, inputUtilFilename);
	writeButtons (o, inputUtilFilename);
	fclose(o);
}

	#define tqstart(update) Do( inputUtil();     \
	                            tqstart(update); \
	                            writeInputs();   )
	#define   GetKey(K) getKey    (  Key(K), #K)
	#define GetMouse(B) getButton (Mouse(B), #B)
#else
	#define   GetKey(K) input.key    [  Key(K)]
	#define GetMouse(B) input.button [Mouse(B)]

static void setKeys (u16 const keyCount, int keys[static const keyCount])
{ input.keys = keys; input.keyCount = keyCount; }

static void setButtons (u8 const buttonCount, int buttons[static const buttonCount])
{ input.buttons = buttons; input.buttonCount = buttonCount; }
#endif

