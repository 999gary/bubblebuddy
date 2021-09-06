#include <assert.h>

// NOTE(jelly): yes, this breaks strict-alising rules
//              i don't care. turn strict-aliasing off.
//              (if you think using a union fixes this, you're wrong.)
static u32 float_as_u32(float x) { return *(u32 *)&x; }
static float u32_as_float(u32 n) { return *(float *)&n; }

typedef struct {
    s64 count;
    u8 *data;
} bit_buffer;

typedef struct {
    bit_buffer data;
    s64 at_bit;
} bit_reader;

u8 get_byte_safe(bit_buffer buf, s32 index) {
    if (index < buf.count) return buf.data[index];
    return 0;
}

int bit_reader_at_end(bit_reader *reader) {
    return reader->at_bit >= reader->data.count*8;
}

u64 bit_peek(bit_reader *reader, u32 count) {
    assert(count > 0 && count <= 64 - 7);
    if (reader->at_bit + count < reader->data.count*8) {
        s64 byte_index = reader->at_bit / 8;
        // TODO(jelly): better way to do this ????
        u64 bits = (((u64)get_byte_safe(reader->data, byte_index+0) << 0*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+1) << 1*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+2) << 2*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+3) << 3*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+4) << 4*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+5) << 5*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+6) << 6*8) |
                    ((u64)get_byte_safe(reader->data, byte_index+7) << 7*8));
        bits >>= reader->at_bit % 8;
        return bits & ((1ULL << count) - 1);
    }
    return 0;
}

u64 bit_eat(bit_reader *reader, u32 count) {
    u64 result = bit_peek(reader, count);
    reader->at_bit += count;
    return result;
}

s32 bit_eat_s32(bit_reader *reader) {
    return bit_eat(reader, 32);
}

float bit_eat_float(bit_reader *reader) {
    return u32_as_float(bit_eat(reader, 32));
}

typedef struct {
    int size;
    unsigned char *bytes;
    int at;
} bit_writer;

void bit_writer_safe_or(bit_writer *b, int index, unsigned char v) {
    if (index < b->size) {
        b->bytes[index] |= v;
    }
}

int bit_push(bit_writer *b, u64 bits, u32 count) {
    bits &= ((1ULL << count)-1);
    if (b->at + count <= b->size*8) {
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
    return 0;
}
int bit_push_s32(bit_writer *b, s32 n) { return bit_push(b, n, 32); }
int bit_push_float(bit_writer *b, float x) {
    return bit_push(b, float_as_u32(x), 32);
}
