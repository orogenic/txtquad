#include <assert.h>
#include "util.h"
static void algtest(void) {
	TypeInfo(v2);
	TypeInfo(v3);
	TypeInfo(v4);
	TypeInfo(m2);
	TypeInfo(m3);
	TypeInfo(m4);

	v2 v2a = V20, v2b = V21;
	m2 m2a = M20, m2b = M21;
	v3 v3a = V30, v3b = V31;
	m3 m3a = M30, m3b = M31;
	v4 v4a = V40, v4b = V41;
	m4 m4a = M40, m4b = M41;

	print(v2b);
	print(v3b);
	print(v4b);
	print(m2b);
	print(m3b);
	print(m4b);
	print(M2c0(V21));
	print(M3c0(V31));
	print(M4c0(V41));

	assert( eq(add(v2a, v2b), v2b));
	assert( eq(add(v3a, v3b), v3b));
	assert( eq(add(v4a, v4b), v4b));
	assert( eq(add(m2a, m2b), m2b));
	assert( eq(add(m3a, m3b), m3b));
	assert( eq(add(m4a, m4b), m4b));
	assert(feq(add(v2a, v2b), v2b));
	assert(feq(add(v3a, v3b), v3b));
	assert(feq(add(v4a, v4b), v4b));
	assert(feq(add(m2a, m2b), m2b));
	assert(feq(add(m3a, m3b), m3b));
	assert(feq(add(m4a, m4b), m4b));
	assert(!eq(add(v2a, v2b), v2a));
	assert(!eq(add(v3a, v3b), v3a));
	assert(!eq(add(v4a, v4b), v4a));
	assert(!eq(add(m2a, m2b), m2a));
	assert(!eq(add(m3a, m3b), m3a));
	assert(!eq(add(m4a, m4b), m4a));

	add(&v2a, v2b);
	add(&v3a, v3b);
	add(&v4a, v4b);
	add(&m2a, m2b);
	add(&m3a, m3b);
	add(&m4a, m4b);
	assert(eq(v2a, v2b));
	assert(eq(v3a, v3b));
	assert(eq(v4a, v4b));
	assert(eq(m2a, m2b));
	assert(eq(m3a, m3b));
	assert(eq(m4a, m4b));

	assert(feq(flerp(V2(0.f, 2.f), .5f), .5f * 2.f));
	assert(feq(flerp(0.f, 2.f, .5f), .5f * 2.f));

	v3 d = { 1.f, 2.f, 3.f };
	assert(eq(vzzz(d), V3e(d.z)));

	dbg("alg success\n");
}
