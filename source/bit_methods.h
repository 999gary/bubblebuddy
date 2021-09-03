#include <assert.h>
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
        // TODO: if i pad all the packets with 8 bytes of zeroes, then i don't have to do get_byte_safe()
        // TODO: better way to do this ????
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