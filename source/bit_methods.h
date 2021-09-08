#include <assert.h>

//#include <stdio.h>
//#include "types.h"

// NOTE(jelly): for now, our bit serializer switches between one that i wrote for xbox
//              and one similar in functionality to the one used by the game itself for gamecube
//              the gamecube's Big Endian byte ordering causes their serializer to order data very strangely

// NOTE(jelly): yes, this breaks strict-alising rules
//              i don't care. turn strict-aliasing off.
//              (if you think using a union fixes this, you're wrong.)
static u32 float_as_u32(float x) { return *(u32 *)&x; }
static float u32_as_float(u32 n) { return *(float *)&n; }

typedef struct {
    s64 count;
    u8 *data;
    s64 at; // NOTE(jelly): `bittally`
    
    // gamecube stuff
    s32 using_messed_up_gamecube_serializer;
    s32 bitidx; // NOTE(jelly): bit index into current 32bit word
    s32 curele; // NOTE(jelly): 32bit word index into data (byte index times 4)
} bit_buffer;

typedef bit_buffer bit_reader;
typedef bit_buffer bit_writer;

// ------ WEIRD GAMECUBE STUFF --------
u32 bit_gc_tab0[] = {
    1U<<0,
    1U<<1,
    1U<<2,
    1U<<3,
    1U<<4,
    1U<<5,
    1U<<6,
    1U<<7,
    1U<<8,
    1U<<9,
    1U<<10,
    1U<<11,
    1U<<12,
    1U<<13,
    1U<<14,
    1U<<15,
    1U<<16,
    1U<<17,
    1U<<18,
    1U<<19,
    1U<<20,
    1U<<21,
    1U<<22,
    1U<<23,
    1U<<24,
    1U<<25,
    1U<<26,
    1U<<27,
    1U<<28,
    1U<<29,
    1U<<30,
    1U<<31,
};

u32 bit_gc_tab1[] = {
    ~(1U<<0),
    ~(1U<<1),
    ~(1U<<2),
    ~(1U<<3),
    ~(1U<<4),
    ~(1U<<5),
    ~(1U<<6),
    ~(1U<<7),
    ~(1U<<8),
    ~(1U<<9),
    ~(1U<<10),
    ~(1U<<11),
    ~(1U<<12),
    ~(1U<<13),
    ~(1U<<14),
    ~(1U<<15),
    ~(1U<<16),
    ~(1U<<17),
    ~(1U<<18),
    ~(1U<<19),
    ~(1U<<20),
    ~(1U<<21),
    ~(1U<<22),
    ~(1U<<23),
    ~(1U<<24),
    ~(1U<<25),
    ~(1U<<26),
    ~(1U<<27),
    ~(1U<<28),
    ~(1U<<29),
    ~(1U<<30),
    ~(1U<<31),
};

u32 bit_reader_gc_read_bit(bit_writer *b) {
    if (b->count*8 < b->at + 1) {
        return 0;
    }
    
    u32 *p = (u32 *)b->data;
    u32 n = bit_gc_tab0[b->bitidx];
    u32 m = p[b->curele];
    b->bitidx++;
    m &= n;
    if (b->bitidx == 32) {
        b->curele++;
        b->bitidx = 0;
    }
    b->at++;
    return (-m | m) >> 0x1f;
}

void bit_reader_gc_read(bit_reader *b, u8 *buf, s32 elesize, s32 n) {
    s32 count;
    if (n < 1) {
        count = -n;
    }
    else {
        count = n * elesize * 8;
    }
    if (n < 0) {
        u32 *p = (u32 *)buf;
        s32 i = 0;
        s32 j = 0;
        while (j < count) {
            if (bit_reader_gc_read_bit(b) == 0) {
                *p &= bit_gc_tab1[i];
            }
            else {
                *p |= bit_gc_tab0[i];
            }
            i++;
            if (i == 32) {
                i = 0;
                p++;
            }
            j++;
        }
    }
    else {
        s32 j = 0;
        s32 i = 0;
        while (i < count) {
            if (bit_reader_gc_read_bit(b) == 0) {
                *buf &= (bit_gc_tab1[j]) & 0xff;
            }
            else {
                *buf |= (bit_gc_tab0[j]) & 0xff;
            }
            j++;
            if (j == 8) {
                j = 0;
                buf++;
            }
            i++;
        }
    }
}

void bit_writer_gc_write_bit(bit_writer *b, u32 bit) {
    if (b->count*8 < b->at + 1) {
        return;
    }
    
    u32 *p = (u32 *)b->data;
    p[b->curele] &= bit_gc_tab1[b->bitidx];
    if (bit) {
        p[b->curele] |= bit_gc_tab0[b->bitidx];
    }
    b->bitidx++;
    if (b->bitidx == 32) {
        b->curele++;
        b->bitidx = 0;
    }
    b->at++;
    return;
}

void bit_writer_gc_write(bit_writer *b, u8 *data, s32  elesize, s32 n) {
    u32 *p = (u32 *)data;
    s32 count;
    if (n == 0) {
        count = 0;
    }
    else {
        if (n < 1) {
            count = -n;
        }
        else {
            count = n * elesize * 8;
        }
        if (n < 0) {
            s32 i = 0;
            s32 j = 0;
            while (j < count) {
                bit_writer_gc_write_bit(b, *p & bit_gc_tab0[i]);
                i++;
                if (i == 32) {
                    i = 0;
                    p++;
                }
                j++;
            }
        }
        else {
            s32 j = 0;
            s32 i = 0;
            while (i < count) {
                bit_writer_gc_write_bit(b, data[0] & bit_gc_tab0[j]);
                j++;
                if (j == 8) {
                    j = 0;
                    data++;
                }
                i++;
            }
        }
    }
}

// ------------------------------------

u8 get_byte_safe(bit_buffer *buf, s32 index) {
    if (index < buf->count) return buf->data[index];
    return 0;
}

int bit_reader_at_end(bit_reader *reader) {
    return reader->at >= reader->count*8;
}

void bit_buffer_safe_and(bit_buffer *b, int index, unsigned char v) {
    if (index < b->count) {
        b->data[index] &= v;
    }
}

u64 bit_peek(bit_reader *reader, u32 count) {
    assert(count > 0 && count <= 64 - 7);
    s64 byte_index = reader->at / 8;
    // TODO(jelly): better way to do this ????
    u64 bits = (((u64)get_byte_safe(reader, byte_index+0) << 0*8) |
                ((u64)get_byte_safe(reader, byte_index+1) << 1*8) |
                ((u64)get_byte_safe(reader, byte_index+2) << 2*8) |
                ((u64)get_byte_safe(reader, byte_index+3) << 3*8) |
                ((u64)get_byte_safe(reader, byte_index+4) << 4*8) |
                ((u64)get_byte_safe(reader, byte_index+5) << 5*8) |
                ((u64)get_byte_safe(reader, byte_index+6) << 6*8) |
                ((u64)get_byte_safe(reader, byte_index+7) << 7*8));
    bits >>= reader->at % 8;
    return bits & ((1ULL << count) - 1);
}

u64 bit_eat(bit_reader *reader, u32 count) {
    u64 result = 0;
    if (reader->using_messed_up_gamecube_serializer) {
        u32 n[2];
        switch (count) {
            case 1:  bit_reader_gc_read(reader, (u8 *)&n, 4, -1);                        break;
            case 8:  bit_reader_gc_read(reader, (u8 *)&n, 1, 1);                         break;
            case 16: bit_reader_gc_read(reader, (u8 *)&n, 2, 1);  byteswap16((u16 *)&n); break;
            case 32: bit_reader_gc_read(reader, (u8 *)&n, 4, 1);  byteswap32(&n);        break;
            
            // TODO(jelly): verify that this is right??
            case 6: bit_reader_gc_read(reader, (u8 *)&n, 4, -6); break;
            case 7: bit_reader_gc_read(reader, (u8 *)&n, 4, -7); break;
            
            default: assert(!"INVALID `COUNT` PASSED TO BIT READER IN GAMECUBE MODE");
        }
        result = n[0];
    } else {
        result = bit_peek(reader, count);
        reader->at += count;
    }
    return result;
}

s32 bit_eat_s32(bit_reader *reader) {
    return bit_eat(reader, 32);
}

float bit_eat_float(bit_reader *reader) {
    return u32_as_float(bit_eat(reader, 32));
}

void bit_writer_safe_or(bit_writer *b, int index, unsigned char v) {
    if (index < b->count) {
        b->data[index] |= v;
    }
}

int bit_push(bit_writer *b, u64 bits, u32 count) {
    bits &= ((1ULL << count)-1);
    if (b->using_messed_up_gamecube_serializer) {
        u32 n[2];
        n[0] = bits;
        switch (count) {
            case 1:                         bit_writer_gc_write(b, (u8 *)&n, 4, -1); break;
            case 8:                         bit_writer_gc_write(b, (u8 *)&n, 1,  1); break;
            case 16: byteswap16((u16 *)&n); bit_writer_gc_write(b, (u8 *)&n, 2,  1); break;
            case 32: byteswap32(&n);        bit_writer_gc_write(b, (u8 *)&n, 4,  1); break;
            
            // TODO(jelly): verify that this is right??
            case 6: bit_writer_gc_write(b, (u8 *)&n, 4, -6); break;
            case 7: bit_writer_gc_write(b, (u8 *)&n, 4, -7); break;
            
            default: assert(!"INVALID `COUNT` PASSED TO BIT WRITER IN GAMECUBE MODE");
        }
        return 1;
    } else {
        if (b->at + count <= b->count*8) {
            u32 byte_index = b->at / 8;
            u32 bit_index = b->at % 8;
            int i = 0;
            bits <<= bit_index;
            while (bits) {
                bit_writer_safe_or(b, byte_index + i++, bits & 0xff);
                bits >>= 8;
            }
            b->at += count;
            return 1;
        }
    }
    return 0;
}
int bit_push_s32(bit_writer *b, s32 n) { return bit_push(b, n, 32); }
int bit_push_float(bit_writer *b, float x) {
    return bit_push(b, float_as_u32(x), 32);
}

// NOTE(jelly): testing
#if 0
int main(void) {
    unsigned char buffer[8] = {0};
    bit_writer b = {sizeof(buffer), buffer, 0, 1};
    bit_push(&b, 1, 1);
    bit_push(&b, 4, 32);
    for (int i = 0; i < 4; i++) {
        printf("0x%02x ", buffer[i]);
    }
    b.at = 0;
    printf("\n%llu\n", bit_eat(&b, 1));
    printf("%llu\n", bit_eat(&b, 32));
}
#endif