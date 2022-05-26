#pragma once

#define STR(x) #x
#define Str(x) STR(x)
#define ArrayLength(...) ( sizeof (__VA_ARGS__) / sizeof (__VA_ARGS__)[0] )
#define ArraySize ArrayLength
#define Min(a, b) ( (a) < (b) ? (a) : (b) )
#define Max(a, b) ( (a) > (b) ? (a) : (b) )
#define AlignSize(alignment, size) ( ((size) + (alignment) - 1) & ~((alignment) - 1) )
#define Do(...) do { __VA_ARGS__ } while (0)
#define PtrAddBytes(Ptr, Bytes) ((char *) (Ptr) + (Bytes))
#define PRIb8 "%c%c%c%c%c%c%c%c"
#define PRIb16 PRIb8  PRIb8
#define PRIb32 PRIb16 PRIb16
#define PRIb64 PRIb32 PRIb32
#define _PRIb8(i) (((i) & 0x80ll) ? '1' : '0') \
                , (((i) & 0x40ll) ? '1' : '0') \
                , (((i) & 0x20ll) ? '1' : '0') \
                , (((i) & 0x10ll) ? '1' : '0') \
                , (((i) & 0x08ll) ? '1' : '0') \
                , (((i) & 0x04ll) ? '1' : '0') \
                , (((i) & 0x02ll) ? '1' : '0') \
                , (((i) & 0x01ll) ? '1' : '0')
#define _PRIb16(i) _PRIb8  ((i) >>  8) , _PRIb8  (i)
#define _PRIb32(i) _PRIb16 ((i) >> 16) , _PRIb16 (i)
#define _PRIb64(i) _PRIb32 ((i) >> 32) , _PRIb32 (i)

#if DebugOn
# define TypeInfo(type) Do( info(" sizeof " #type " %3zuB (%4zu-bit)\n",   sizeof (type),   sizeof (type) * 8); \
                            info("alignof " #type " %3zuB (%4zu-bit)\n", _Alignof (type), _Alignof (type) * 8); )
#else
# define TypeInfo(...)
#endif
