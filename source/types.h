
typedef float   f32;
typedef double  f64;

#ifdef _MSC_VER
typedef char      s8;
typedef short    s16;
typedef int      s32;
typedef __int64  s64;
typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
typedef unsigned __int64 u64;
#else
#include <stdint.h>
//TODO(Will): Jelly fix this :)
typedef int8_t    s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif