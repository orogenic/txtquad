#pragma once

#include "sprite.h"

#define LineHeight (QuadSize + PointSize)

#define JustifyLeft   0.f
#define JustifyCenter .5f
#define JustifyRight  1.f

typedef struct {
	v2    anchor;
	tqtrs trs;
	f32   justify;
	f32   spacing;    // vertical and horizontal spacing
	f32   lineheight; // vertical spacing multiplier for newlines
	ccstr str;
	struct tqblockctx {
		v2    offset;
		v2    extent;
		ccstr ptr;
		ccstr eol;
		f32   lineheight;
		f32   lines;
		f32   height;
	} ctx;
} tqblock;

#define initblock(block) _initblock(block, &(block)->ctx)
static void _initblock
	( tqblock     const * const block
	, struct tqblockctx * const ctx )
{
	f32 const lineheight = block->spacing
	                     * block->lineheight
	                     * LineHeight ;

	v2 extent = Vy(-lineheight);
	f32 x = 0.f;

	for (ccstr ptr = block->str; *ptr; ++ptr)
		if (*ptr == '\n') {
			extent.y -= lineheight;
			x = 0.f;
		} else {
			x += block->spacing * QuadSize;
			extent.x = fmax(extent.x, x);
		}

	v2 offset = block->anchor;
	schur(&offset, V2(-.5f, .5f));
	  sub(&offset, .5f);
	schur(&offset, extent);

	*ctx = (struct tqblockctx)
		{ .offset     = offset
		, .extent     = extent
		, .ptr        = block->str
		, .eol        = block->str
		, .lineheight = lineheight };
}

#ifdef DebugBlock
#define debugblock(trs, rgb) _debugblock(&(trs), &(rgb))
static inline void _debugblock
	( tqtrs const * const trs
	, v3    const * const rgb )
{
	drawsprite( (tqsprite)
	{ .trs =
		{ .t = add(trs->t, mul(mul(V3fwd, -1e-3f), trs->r)) // is this z priority or something?
		, .r = trs->r
		, .s = trs->s * .15f }
	, .rgb   = *rgb
	, .value = 1 } );
}
#endif

static inline char LatinLowerToUpper(char c) {
	return 'a' <= c && c <= 'z' ? c - 32 : c;
}

#define drawblock(block, sprite) _drawblock(block, &(block)->ctx, sprite)
static bool _drawblock
	( tqblock     const * const block
	, struct tqblockctx * const ctx
	, tqsprite          * const sprite )
{
	while (*ctx->ptr == '\n') {
		ctx->eol = ++ctx->ptr;
		ctx->height += ctx->lineheight;
		++ctx->lines;
	}

	u32 const range = ctx->ptr - ctx->eol; // TODO this range/eol stuff all feels refactorable...

	u32 justifyRange = 0;
	if (block->justify > JustifyLeft)
		for ( ccstr ptr = ctx->ptr
		    ; *ptr && *ptr != '\n'
		    ; ++ptr ) ++justifyRange;
	else justifyRange = 1;

	f32 width = flerp( range * block->spacing * QuadSize
	                 , ctx->extent.x - justifyRange * block->spacing * QuadSize
	                 , block->justify );

	v3 offset = { .xy = add(ctx->offset, V2(width, -ctx->height)) };

	// Correct for center of rotation (x,y) and packing (z)
	add(&offset, V3( -.5f * PointSize + .5f
	               , -.5f
	               , (ctx->lines + range % 2) * 1e-4f ));
	mul(&offset, block->trs.s);

	*sprite = (tqsprite)
		{ .anchor   = { 0 }
		, .bounds   = FontBounds
		, .trs      =
			{ .t = add(block->trs.t, mul(offset, block->trs.r))
			, .r = block->trs.r
			, .s = block->trs.s }
		, .rgb      = V31 // TODO give block color control
		, .subalpha = 0.f
		, .value    = LatinLowerToUpper(*ctx->ptr++) };

	if (sprite->value) return true;

#ifdef DebugBlock
{
	static v3 offset = Vz(-2e-4f)
	,         extent = Vz(-1e-4f) ;
	static v3 const lightr = V3ex(.2f, 1.f)
	,               lightg = V3ey(.2f, 1.f)
	,               lightb = V3ez(.2f, 1.f) ;
	offset.xy = mul(vxy(ctx->offset), block->trs.s);
	extent.xy = mul(vxy(ctx->extent), block->trs.s);
	tqtrs trs = { .r = block->trs.r
	            , .s = block->trs.s };
	trs.t = add(block->trs.t, mul(offset, trs.r));
	debugblock(trs, lightr);
	trs.t = add(block->trs.t, mul(add(offset, extent), trs.r));
	debugblock(trs, lightb);
	trs.t = block->trs.t;
	debugblock(trs, lightg);
}
#endif

	return false;
}

static inline void blockstr(tqblock *block, ccstr const str) {
	tqsprite sprite;
	block->str = str;
	initblock(block);
	while (drawblock(block, &sprite)) drawsprite(sprite);
}
