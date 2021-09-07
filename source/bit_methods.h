#include <assert.h>

//#include <stdio.h>
//#include "types.h"

// NOTE(jelly): yes, this breaks strict-alising rules
//              i don't care. turn strict-aliasing off.
//              (if you think using a union fixes this, you're wrong.)
static u32 float_as_u32(float x) { return *(u32 *)&x; }
static float u32_as_float(u32 n) { return *(float *)&n; }

typedef struct {
    s64 count;
    u8 *data;
    s64 at;
    int using_messed_up_gamecube_serializer;
} bit_buffer;

typedef bit_buffer bit_reader;
typedef bit_buffer bit_writer;

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
    if (reader->at + count < reader->count*8) {
        if (reader->using_messed_up_gamecube_serializer && count == 1) {
            s64 byte_index = reader->at / 8;
            // TODO(jelly): better way to do this ????
            u64 bits = (((u64)get_byte_safe(reader, byte_index+3) << 0*8) |
                        ((u64)get_byte_safe(reader, byte_index+2) << 1*8) |
                        ((u64)get_byte_safe(reader, byte_index+1) << 2*8) |
                        ((u64)get_byte_safe(reader, byte_index+0) << 3*8) |
                        ((u64)get_byte_safe(reader, byte_index+7) << 4*8) |
                        ((u64)get_byte_safe(reader, byte_index+6) << 5*8) |
                        ((u64)get_byte_safe(reader, byte_index+5) << 6*8) |
                        ((u64)get_byte_safe(reader, byte_index+4) << 7*8));
            bits >>= reader->at % 8;
            bits &= ((1ULL << count) - 1);
            u8 *bytes = (u8 *)&bits;
            bit_buffer_safe_and(reader, byte_index+3, ~bytes[0]);
            bit_buffer_safe_and(reader, byte_index+2, ~bytes[1]);
            bit_buffer_safe_and(reader, byte_index+1, ~bytes[2]);
            bit_buffer_safe_and(reader, byte_index+0, ~bytes[3]);
            bit_buffer_safe_and(reader, byte_index+7, ~bytes[4]);
            bit_buffer_safe_and(reader, byte_index+6, ~bytes[5]);
            bit_buffer_safe_and(reader, byte_index+5, ~bytes[6]);
            bit_buffer_safe_and(reader, byte_index+4, ~bytes[7]);
            return bits;
        } else {
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
    }
    return 0;
}

u64 bit_eat(bit_reader *reader, u32 count) {
    u64 result = bit_peek(reader, count);
    reader->at += count;
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
    if (b->at + count <= b->count*8) {
        u32 byte_index = b->at / 8;
        u32 bit_index = b->at % 8;
        int i = 0;
        
        if (b->using_messed_up_gamecube_serializer && count == 1) {
            u32 *words = (u32 *)&bits;
            // TODO(jelly): replace w portable byteswap
            words[0] = _byteswap_ulong(words[0]);
            words[1] = _byteswap_ulong(words[1]);
        }
        bits <<= bit_index;
        while (bits) {
            bit_writer_safe_or(b, byte_index + i++, bits & 0xff);
            bits >>= 8;
        }
        b->at += count;
        return 1;
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