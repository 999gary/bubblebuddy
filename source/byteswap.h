#ifndef BYTESWAP_H
#define BYTESWAP_H

// NOTE(jelly): i know there are intrinsics, but this is nice and portable :)

void byteswap16(u16 *p) {
    u16 n = *p;
    *p = (((n & 0xff) << 8) |
          ((n & 0xff00) >> 8));
}

void byteswap32(u32 *p) {
    u32 n = *p;
    *p = (((n &       0xff) << 24) |
          ((n &     0xff00) <<  8) |
          ((n &   0xff0000) >>  8) |
          ((n & 0xff000000) >> 24));
}

#endif //BYTESWAP_H
