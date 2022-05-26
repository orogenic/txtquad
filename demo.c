#include <inttypes.h>

#define Input
#define InputUtil
#define DebugFps
#define DebugInfo
#include "txtquad.h"

#define AlgAssert
#define AlgTest
#define AlgInline
// AlgIntrin noticeably slows compilation, not worth anything anyway
#include "alg.h"

#define DebugBlock
#include "block.h"

#define Mod(mod) mods.mod
static struct { bool Shift; } mods;

static const v4 XYBasis[4] = { Qrt, Qup, Qlft, Qdwn };
static v4 demo4mouser = Qrt;

static struct {
	struct { m4 model; v3 rgb; } demo6[Quads];
	struct { txtquad quad[0xFF]; } demo7;
	struct {
		m4 center, origin, demo2[4];
	} model;
	tqtr cam;
	m4 persp, ortho;
} cache;

static bool invertY = true;
static m4 view, *proj = &cache.persp, viewproj;

static void initcache(void) {
	cache.model.origin = M4i;
	cache.model.center = m4t(V3xy( -.5f + .5f * PointSize
	                             , -.5f ));

	{
		static f32 const z = 2.f
		,                d = .5f
		,                o = 1.f / 8.f
		,                s = .25f;
		static v4  const r = Qi;
		static v3  const t[4] =
			{ {  d - o,  d - o, z }
			, { -d - o,  d - o, z }
			, { -d - o, -d - o, z }
			, {  d - o, -d - o, z } };

		for (u8 i = 0; i < 4; ++i)
			cache.model.demo2[i] = m4trs(t[i], r, s);
	}

	{
		static f32 const s = .1f
		,                depth = 1.f + Quads / 60;
		f32 x, y, z;
		u32 a;
		for (u32 i = 0; i < Quads; ++i) {
			x = i % 10;
			y = i % 60 / 10;
			z = i / 60.f;
			a = i / 54;
			cache.demo6[i].model = m4trs
				( V3( -0.95f + 2.f * x * s
				    , -0.55f + 2.f * y * s
				    ,  1.00f + .5f * z * s )
				, XYBasis[i % 4]
				, s );
			cache.demo6[i].rgb = V3
				( x / 10.f
				, y / 6.f
				, 1.f - a / (a % 4 ? 1.f : depth) );
		}
	}

	{
		static f32 const s = 0.04f;
		for (u8 value = 0; value < 0xFF; ++value)
			cache.demo7.quad[value] = Txtquad
				( m4trs( V3(  s * (value % 16)
				           , -s * (value / 16)
				           , 1.f )
				       , Qi
				       , s )
				, V41
				, value );
	}
}

static bool demo1(tqtr *cam) {
	cam->t = V3bck;
	drawquad(Txtquad( cache.model.origin
	                , V41
	                , 'A' ));
	return false;
}

static bool demo2(tqtr *cam) {
	static txtquad quad = { .rgba = V41 };

	f64 const x = 1. - .5 * (1. + cos(frame.t * .5));
	char const c[] = { 'A' + 0 + 26 * x
	                 , 'Z' + 1 - 26 * x };

	for (u8 i = 0; i < 4; ++i) {
		quad.value = c[i % 2];
		quad.model = cache.model.demo2[i];
		drawquad(quad);
	}

	return false;
}

static bool demo3(tqtr *cam) {
	const bool semi = cos(frame.t) > 0.;
	cam->t = V3z(-1.5f);
	cam->r = qAxisAngle( semi ? V3fwd   :  V3bck
	                   , semi ? frame.t : -frame.t );
	drawquad(Txtquad(m4trs(V3ez(0, 2), cam->r, 1.f), V41, 'O'));
	drawquad(Txtquad( cache.model.center
	                , v4v( semi ? V3(1.f, .8f, .4f)
	                            : V3(1.f, .4f, .8f)
	                     , .4f + .6f * cos(frame.t * 2.) )
	                , semi ? 'A' : 'B' ));
	return false;
}

static char demo4str[1024] = { '\0' };
static size_t demo4strlen = 0;

static inline void demo4strappend(u32 u) {
	if (demo4strlen < ArraySize(demo4str) - 1) {
		demo4str[demo4strlen] = (char) (u & 0xFF);
		demo4str[++demo4strlen] = '\0';
	}
}

static bool demo4(tqtr *cam) {
	static tqblock block =
		{ .str        = demo4str
		, .trs.t      = { 0.f, -.9f, 2.f }
		, .trs.s      = .25f
		, .anchor     = { 0.f, -1.f }
		, .justify    = JustifyRight
		, .spacing    = 1.f
		, .lineheight = 1.f };

	block.trs.r = demo4mouser;

	info( "Enter " PRIb8 " Backspace " PRIb8 " Escape " PRIb8 "\r"
	    , _PRIb8(input.key[Key(Enter)])
	    , _PRIb8(input.key[Key(Backspace)])
	    , _PRIb8(input.key[Key(Escape)]) );

	if (KeyUp(Backspace) || MouseUp(Right))
		if (demo4strlen) demo4str[--demo4strlen] = '\0';

	if (!Mod(Shift)) {
	if (KeyDown(Enter) || MouseDown(Left))
		demo4strappend('\n');
	if (KeyHeld(Escape) || MouseHeld(Middle))
		if (demo4strlen) demo4str[demo4strlen = 0] = '\0';
	} else {
	if (KeyDown(L)) block.justify = JustifyLeft;
	if (KeyDown(C)) block.justify = JustifyCenter;
	if (KeyDown(R)) block.justify = JustifyRight;
	}

	tqsprite sprite;
	initblock(&block);
	while (drawblock(&block, &sprite)) drawsprite(sprite);

	// blinking cursor
	sprite.value = fmod(frame.t, 1.) > .5 ? '_' : ' ';
	drawsprite(sprite);

	return false;
}

static bool demo5(tqtr *cam) {
	static tqsprite a =
		{ .anchor = { 1.f, -1.f }
		, .bounds = FontBounds
		, .trs.t  = Vz(1.f)
		, .trs.s  = .5f
		, .rgb    = { .8f, .3f, .3f }
		, .subalpha = .5
		, .value  = '1' };
	static tqsprite b =
		{ .anchor = { -1.f, 1.f }
		, .bounds = FontBounds
		, .trs.t  = Vz(1.f)
		, .trs.s  = .25f
		, .rgb    = V3e(.8f)
		, .subalpha = .5
		, .value  = '4' };
	static tqsprite c =
		{ .bounds = V4smul(PointSize, 2.f, 3.f, 4.f, 0.f)
		, .trs.t  = Vz(1.f)
		, .trs.s  = 1.f
		, .rgb    = { .1f, .1f, .2f }
		, .value  = '!' };

	f32 theta = .5f * sin(frame.t);

	a.trs.r = qAxisAngle(V3fwd,  theta);
	drawsprite(a);

	b.trs.r = qAxisAngle(V3fwd, -theta);
	drawsprite(b);

	for (u8 i = 0; i < 4; ++i) {
		c.trs.r = XYBasis[i];
		drawsprite(c);
	}

	return false;
}

static void nextword(tqblock *block) {
	block->trs.t.x += block->trs.s * block->ctx.extent.x;
}

static void fpsoverlay(const tqtr cam) {
	static tqblock block =
		{ .trs.r      = Qi
		, .trs.s      = .1f
		, .anchor     = { -1.f, 1.f }
		, .spacing    = 1.f
		, .lineheight = 1.f };
	block.trs.t = V3(-1.f, 1.f, 2.f);
	blockstr(&block, fps.str);
	nextword(&block);
	blockstr(&block, "FPS");
}

static void quadsoverlay(const tqtr cam) {
	static tqblock block =
		{ .trs.s      = .001f
		, .anchor     = { -1.f, 1.f }
		, .spacing    = 1.f
		, .lineheight = 1.f };

	static char str[7];
	if (frame.i % 10 == 0) snprintf(str, sizeof(str), "%6u", txt.quads);

	block.trs.t = add(cam.t, V3( -.013f
	                           , .007f - block.trs.s * LineHeight
	                           , 1.f / 60.f ));
	block.trs.r = cam.r;
	blockstr(&block, str);
	nextword(&block);
	blockstr(&block, "Quads");
}

static bool demo6(tqtr *cam) {
	static u32 quads = 0x100;
	static f64 targetfps = 59.;
	static txtquad quad = { .value = 'A' };

	for (u32 i = 0; i < quads; ++i) {
		quad.model = cache.demo6[i].model;
		quad.rgba  = v4v( cache.demo6[i].rgb
		                , .5f + .5f * fmod(frame.t * .4, 1.f) );
		txt.quad[txt.quads + i] = quad;
	}
	txt.quads += quads;

	quadsoverlay(*cam);

	u32 budget = Quads + quads - txt.quads;

	if (Sec) info( "%u quads %s\n"
	             , quads
	             , quads == budget ? "(max)" : "" );

#define QuadsStep 0x50
#define FpsStep 10.

	if (!Mod(Shift)) {
	if (KeyDown(Down) && targetfps >= 10.)
		targetfps -= FpsStep;
	if (KeyDown(Up))
		targetfps += FpsStep;
	}

	if (AvgFps < targetfps) {
		if (quads >= QuadsStep)
			quads -= QuadsStep;
		else if (quads > 0)
			quads = 0;
	} else {
		if (quads <= budget - QuadsStep)
			quads += QuadsStep;
		else if (quads < budget)
			quads = budget;
	}

	return false;
}

static bool demo7(tqtr *cam) {
	for (u8 value = 0; value < 0xFF; ++value)
		drawquad(cache.demo7.quad[value]);
	return false;
}

void updateviewproj(tqtr cam, bool update) {

	if (Mod(Shift)) {
	       if (KeyDown(P)) {
		proj = &cache.persp;
		update = true;
	} else if (KeyDown(O)) {
		proj = &cache.ortho;
		update = true;
	}
	}

	if (frame.resized || update) {
		cache.persp = m4persp(60.f, (f32) frame.w / frame.h, .001f, 1024.f, invertY);
		cache.ortho = m4ortho(2.f, (f32) frame.w / frame.h, .001f, 1024.f, invertY);
		update = true;
	}

	if (!eq(cam, cache.cam)) {
		view = m4view(cache.cam = cam);
		update = true;
	}

	if (update) viewproj = mul(*proj, view);
}

typedef bool (*tqdemo)(tqtr *cam);

static tqdemo demo = demo6;

bool update(void) {
	static double pts;
	static bool pause = false;

	Mod(Shift) = KeyPressed(LeftShift) || KeyPressed(RightShift); // TODO gonna move this to the backend

	if (Mod(Shift)) {
	     if (KeyDown(Q)) return true;
	     if (KeyDown(1)) demo = demo1;
	else if (KeyDown(2)) demo = demo2;
	else if (KeyDown(3)) demo = demo3;
	else if (KeyDown(4)) demo = demo4;
	else if (KeyDown(5)) demo = demo5;
	else if (KeyDown(6)) demo = demo6;
	else if (KeyDown(7)) demo = demo7;
	}

	tqtr cam = { V30, Qi };
	txt.quads = 0;

	if (demo(&cam)) return true;

	fpsoverlay(cam); // TODO actual screen space

	if (Sec) info( "FPS=%.3f\tRDT=%.3fms\tRT=%.3fs\tDT=%.3fms\tT=%.3fs\tIT=%" PRIu64 "s\n"
	             , AvgFps, AvgRdt * 1e3, frame.rt, frame.dt * 1e3, frame.t, frame.it );

	if (Mod(Shift)) {
	if (KeyDown(Y     )) { invertY = !invertY; updateviewproj(cam, true); }
	if (KeyDown(Down  )) frame.ts *= .5;
	if (KeyDown(Up    )) frame.ts *= 2.;
	if (KeyDown(Enter )) frame.ts  = 1.;
	if (KeyDown(Escape)) {
		pause = !pause;
		if (pause) {
			pts = frame.ts;
			frame.ts = 0.;
		} else frame.ts = pts;
	}
	}

	updateviewproj(cam, false);
	*txt.viewproj = viewproj;
	*txt.invertY = invertY;

	return false;
}

bool unicode(u32 u) {
	if (!Mod(Shift) && demo == demo4) demo4strappend(u);
	return false;
}

bool cursor(f64 x, f64 y) {
	info("x %6.1lf y %6.1lf w %4d h %4d\r", x, y, frame.w, frame.h);
	if (demo == demo4)
		demo4mouser = mul( qAxisAngle(V3rt, (-.5 + y / frame.h) * .5 * AlgPi)
		                 , qAxisAngle(V3up, (-.5 + x / frame.w) * .5 * AlgPi) );
	return false;
}

int main(void) {
	algtest();
	initcache();
	input.unicode = unicode;
	input.cursor = cursor;
	tqDebugLevel(DebugOff);
	tqstart(update);
	return 0;
}
