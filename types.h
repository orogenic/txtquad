#pragma once

#include <stdalign.h>
#include <stdint.h>

typedef char const * ccstr;
typedef char       * cstr;
typedef union v2     v2;
typedef union v3     v3;
typedef union v4     v4;
typedef union m2     m2;
typedef union m3     m3;
typedef union m4     m4;
typedef   int8_t     s8;
typedef  uint8_t     u8;
typedef  int16_t     s16;
typedef uint16_t     u16;
typedef  int32_t     s32;
typedef uint32_t     u32;
typedef float        f32;
typedef  int64_t     s64;
typedef uint64_t     u64;
typedef double       f64;

union v2 {                  f32 e[2], r[2];
           struct { union { f32 x, u, r0; };
                    union { f32 y, v, r1; }; }; };

union v3 {                                   f32 e[3], r[3];
           struct {                          f32       r0;
                                              v2 yz;                };
           struct { union {                   v2 xy;
                            struct { union { f32 x, i;     };
                                     union { f32 y, j, r1; }; }; };
                    union {                  f32 z, k, r2;       }; }; };

#ifdef AlgIntrin
# define algalignas alignas
#else
# define algalignas(...)
#endif
// if we call _mm_loadu_ps maybe only add alignas to v4s that are hit often? idk...
union v4 {                                     algalignas(16) f32 e[4],          r[4];
           struct {                                           f32                r0;
                                                               v3 yzw;                              };
           struct {                                            v2 xy;
                                                               v2 zw;                               };
           struct { union {                                    v3 xyz, ijk, rgb;
                            struct { union {                  f32 x,   i;                  };
                                     union {                   v2 yz;
                                             struct { union { f32 y,   j,        r1; };
                                                      union { f32 z,   k,        r2; }; }; }; }; };
                    union {                                   f32 w,   s,   a,   r3;             }; }; };

// m2 same layout as v4, potentially keep separate for "type safety" lol
union m2 { f32 e[2 * 2];
            v2 c[2];
   struct { v2 c0, c1; }; };

union m3 { f32 e[3 * 3];
            v3 c[3];
   struct { v3 c0, c1, c2; }; };

// alignas(64) would increase sizeof(txtquad) to 128, so __m512 is currently unacceptable
// could do __m256 alignas(32) since alignof(txtquad) is already 16 and is padded to 96
union m4 { f32 e[4 * 4];
            v4 c[4];
   struct { v4 c0, c1, c2, c3; }; };
