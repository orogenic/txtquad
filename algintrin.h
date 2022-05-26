#pragma once

#include <stdalign.h>

#include <immintrin.h>
#include <xmmintrin.h>

#include "types.h"

static inline f32 dotv3(const v3 a, const v3 b) {
	alignas(16) f32 v[4];
	__m128 ae = _mm_loadu_ps(a.e);
	__m128 be = _mm_loadu_ps(b.e);
	__m128 ab = _mm_mul_ps(ae, be);
	_mm_store_ps(v, ab);
	return v[0] + v[1] + v[2];
}

static inline f32 dotv4(const v4 a, const v4 b) {
	alignas(16) f32 v[4];
	__m128 ae = _mm_load_ps(a.e);
	__m128 be = _mm_load_ps(b.e);
	__m128 ab = _mm_mul_ps(ae, be);
	_mm_store_ps(v, ab);
	return v[0] + v[1] + v[2] + v[3];
}

static inline f32 magsqv3(const v3 a) {
	alignas(16) f32 v[4];
	__m128 a1 = _mm_loadu_ps(a.e);
	__m128 a2 = _mm_loadu_ps(a.e);
	__m128 aa = _mm_mul_ps(a1, a2);
	_mm_store_ps(v, aa);
	return v[0] + v[1] + v[2];
}

static inline f32 magsqv4(const v4 a) {
	alignas(16) f32 v[4];
	__m128 a1 = _mm_load_ps(a.e);
	__m128 a2 = _mm_load_ps(a.e);
	__m128 aa = _mm_mul_ps(a1, a2);
	_mm_store_ps(v, aa);
	return v[0] + v[1] + v[2] + v[3];
}
