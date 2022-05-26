#pragma once

#include "alg.h"

#define PointSize  (QuadSize / QuadPoints)
#define FontBounds V4z(PointSize)

typedef struct {
	v2    anchor; // [-1, 1] -> [0, 1]
	v4    bounds;
	tqtrs trs;
	v3    rgb;
	f32   subalpha;
	u32   value;
} tqsprite;

static inline txtquad quadsprite(tqsprite sprite) {
	mul(&sprite.anchor, .5f);
	add(&sprite.anchor, .5f);
	neg(&sprite.bounds.zw);
	add( &sprite.trs.t
	   , mul(mul(V3xy( -flerp( add(vxx(sprite.anchor), vxz(sprite.bounds))
	                         , sprite.anchor.x )
	                 , -flerp( add(vyy(sprite.anchor), vyw(sprite.bounds))
	                         , sprite.anchor.y ) ), sprite.trs.s), sprite.trs.r) );
	return Txtquad
		( m4trs(sprite.trs)
		, v4v(sprite.rgb, 1.f - sprite.subalpha)
		, sprite.value );
}

static inline void drawsprite(tqsprite const sprite) {
	drawquad(quadsprite(sprite));
}
