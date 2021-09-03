#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#ifdef _MSC_VER
typedef int       int32;
typedef unsigned uint32;
#else
#include <stdint.h>
typedef uint32_t uint32;
typedef  int32_t  int32;
#endif

// NOTE(jelly): all code below assumes a little-endian architecture for now.

#include "bfbb_gci_header.h"
#include "hmac_sha1.c"

// NOTE(jelly): this code (and table) is based off of the crc32 code from the decomp
//              https://github.com/bfbbdecomp/bfbb/blob/2bc99a1efcb8fab4cbccfc416d226f4a54b851ab/src/Core/x/xutil.cpp
static const uint32 g_crc32_table[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75, 
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd, 
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95, 
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072, 
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba, 
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a, 
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53, 
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b, 
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b, 
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3, 
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec, 
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4, 
};
uint32 xUtil_crc_update(uint32 crc_accum, char* data, int datasize)
{
    int i, j;
    
    for (i = 0; i < datasize; i++)
    {
        j = ((crc_accum >> 24) ^ *data++) & 0xff;
        crc_accum = (crc_accum << 8) ^ g_crc32_table[j];
    }
    
    return crc_accum;
}

uint32 crc32_get_checksum(char *data, int size) {
    return xUtil_crc_update(-1, data, size);
}

#define BFBB_SAVE_FILE_LEDR_RANDOM_TEXT "--TakeMeToYourLeader--"

#pragma pack(push, 1)
typedef struct {
    uint32 crc32_checksum;
} bfbb_save_file_block_gdat;

typedef struct {
    char game_label[64];
    int32 progress;
    char unknown_bytes[12];
    int32 thumbnail_index;
    char random_text[sizeof(BFBB_SAVE_FILE_LEDR_RANDOM_TEXT)-1];
} bfbb_save_file_block_ledr;

typedef struct {
    uint32 sceneid; // NOTE(jelly): this is the FOURCC of the current level
} bfbb_save_file_block_room;

typedef struct {
    uint32 sound_mode;
    float music_volume;
    float sfx_volume;
    uint32 rumble;
} bfbb_save_file_block_pref;

typedef struct {
    int32 version;
} bfbb_save_file_block_svid;

typedef struct {
    union {
        uint32 id;
        char id_chars[4];
    };
    int32 block_size;
    int32 bytes_used;
} bfbb_save_file_block_header;

typedef struct {
    bfbb_save_file_block_header header;
    union {
        unsigned char raw_bytes[512 - sizeof(bfbb_save_file_block_header)]; // NOTE(jelly): padding to 1/2 KiB
        
        bfbb_save_file_block_gdat gdat;
        bfbb_save_file_block_ledr ledr;
        bfbb_save_file_block_room room;
        bfbb_save_file_block_pref pref;
        bfbb_save_file_block_svid svid;
    };
} bfbb_save_file_block;
#pragma pack(pop)

// NOTE(jelly): bfbb_save_file contains everything EXCEPT
//                -the 0x5880-sized GCI header
//                -the GDAT block (which goes at the start)
//                -the gigantic SFIL block (which goes at the end)
//                -any 0xBF padding
//                -any zero-padding at the end (like in .gci)
typedef struct {
    int original_file_size;
    uint32 original_crc32_checksum; // NOTE(jelly): these probably aren't needed
    int block_count;
    bfbb_save_file_block blocks[64];
} bfbb_save_file;

typedef struct {
    int size;
    unsigned char *bytes;
} buffer;

//#define FOURCC(s) ((uint32)(((s)[0]) | ((s)[1] << 8) | ((s)[2] << 16) | ((s)[3] << 24)))
#define FOURCC_CONST(a,b,c,d) ((uint32)(((d)) | ((c) << 8) | ((b) << 16) | ((a) << 24)))

// NOTE(jelly): i love programming languages
#define FOURCC_GDAT FOURCC_CONST('G', 'D', 'A', 'T')
#define FOURCC_LEDR FOURCC_CONST('L', 'E', 'D', 'R')
#define FOURCC_ROOM FOURCC_CONST('R', 'O', 'O', 'M')
#define FOURCC_PREF FOURCC_CONST('P', 'R', 'E', 'F')
#define FOURCC_SVID FOURCC_CONST('S', 'V', 'I', 'D')
#define FOURCC_SFIL FOURCC_CONST('S', 'F', 'I', 'L')

void byteswap32(uint32 *p) {
    uint32 n = *p;
    *p = (((n &       0xff) << 24) | 
          ((n &     0xff00) <<  8) |
          ((n &   0xff0000) >>  8) |
          ((n & 0xff000000) >> 24));
}

unsigned char *peek_bytes(buffer *b, int count) {
    unsigned char *result = 0;
    if (b->size >= count) {
        result = b->bytes;
    }
    return result;
}

unsigned char *eat_bytes(buffer *b, int count) {
    unsigned char *result = 0;
    if (b->size >= count) {
        result = b->bytes;
        b->bytes += count;
        b->size -= count;
    }
    return result;
}

bfbb_save_file_block_header bfbb_save_file_block_header_parse(buffer *b, int is_gci) {
    bfbb_save_file_block_header result = {0};
    if (peek_bytes(b, sizeof(bfbb_save_file_block_header))) {
        result = *(bfbb_save_file_block_header *)eat_bytes(b, sizeof(bfbb_save_file_block_header));
    }
    if (is_gci) {
        byteswap32(&result.id);
        byteswap32((uint32 *)&result.block_size);
        byteswap32((uint32 *)&result.bytes_used);
    }
    return result;
}

#define BFBB_SAVE_FILE_MAGIC_STRING "SPONGEBOB:WHENROBOTSATTACK::RyanNeilDan"

// NOTE(jelly): this doesn't do the block header. maybe it should???
void bfbb_save_file_block_byteswap(bfbb_save_file_block *new_block) {
    switch (new_block->header.id) {
        case FOURCC_GDAT: {
            byteswap32(&new_block->gdat.crc32_checksum);
        } break;
        case FOURCC_LEDR: {
            byteswap32((uint32 *)&new_block->ledr.progress);
            byteswap32((uint32 *)&new_block->ledr.thumbnail_index);
        } break;
        case FOURCC_ROOM: {
            byteswap32(&new_block->room.sceneid);
        } break;
        case FOURCC_PREF: {
            byteswap32(&new_block->pref.sound_mode);
            byteswap32((uint32 *)&new_block->pref.music_volume);
            byteswap32((uint32 *)&new_block->pref.sfx_volume);
            byteswap32(&new_block->pref.rumble);
        } break;
        case FOURCC_SVID: {
            byteswap32((uint32 *)&new_block->svid.version);
        } break;
    }
}

int bfbb_save_file_looks_like_a_valid_header(bfbb_save_file_block_header *header) {
    int looks_like_a_valid_id = 1;
    int i;
    for (i = 0; i < 4; i++) {
        char c = header->id_chars[i];
        if (!(isupper(c) || isdigit(c))) {
            looks_like_a_valid_id = 0;
        }
    }
    return looks_like_a_valid_id && header->block_size > 0 && header->bytes_used > 0;
}

int bfbb_save_file_block_read(buffer *b, bfbb_save_file_block *new_block, int is_gci) {
    bfbb_save_file_block_header header = bfbb_save_file_block_header_parse(b, is_gci);
    if (bfbb_save_file_looks_like_a_valid_header(&header)) {
        int is_gdat = header.id == FOURCC_GDAT;
        int actual_block_size = is_gdat ? 4 : header.block_size;
        int actual_bytes_used = is_gdat ? 4 : header.bytes_used; 
        unsigned char *bytes;
        if (header.id == FOURCC_SFIL) return 0;
        bytes = eat_bytes(b, actual_block_size);
        if (!bytes) return -1;
        new_block->header = header;
        memcpy(new_block->raw_bytes, bytes, actual_bytes_used);
        if (is_gci) bfbb_save_file_block_byteswap(new_block);
        return 1;
    }
    return -1;
}

int bfbb_save_file_read_(bfbb_save_file *result, unsigned char *bytes, int size, int is_gci) {
    buffer b = {size, bytes};
    char *magic_string = 0;
    bfbb_save_file_block gdat;
    int rc;
    int i = 0;
    
    if (is_gci) {
        eat_bytes(&b, 0x5880); // NOTE(jelly): gci file header
        
        eat_bytes(&b, 512); // NOTE(jelly): zeros
        magic_string = (char *)eat_bytes(&b, sizeof(BFBB_SAVE_FILE_MAGIC_STRING) - 1);
        eat_bytes(&b, 0x599); // NOTE(jelly): zeros - wiki says 599 (decimal); i assume this is a typo
        
        /*
        if (strcmp(magic_string, BFBB_SAVE_FILE_MAGIC_STRING)) {
            // assert(0);
            // TODO(jelly): ??? do we honestly care if it's not right?
        }
*/
    }
    
    rc = bfbb_save_file_block_read(&b, &gdat, is_gci);
    if (rc < 1) return 0;
    result->original_file_size = gdat.header.bytes_used;
    result->original_crc32_checksum = gdat.gdat.crc32_checksum;
    
    while (rc = bfbb_save_file_block_read(&b, &result->blocks[i++], is_gci)) {
        if (rc < 0) {
            result->block_count = 0;
            return 0;
        }
    }
    
    result->block_count = i -1;
    
    return 1;
}

void bfbb_save_file_make_dummy_gdat_block(bfbb_save_file_block *block) {
    block->header.id = FOURCC_GDAT;
    block->header.block_size = 1;
    block->header.bytes_used = 0;
    block->gdat.crc32_checksum = 0;
}

void bfbb_save_file_set_gdat_block(bfbb_save_file_block *block, int file_size, uint32 crc32_checksum, int is_gci) {
    block->header.id = FOURCC_GDAT;
    block->header.block_size = 1;
    block->header.bytes_used = file_size;
    block->gdat.crc32_checksum = crc32_checksum;
    
    if (is_gci) {
        byteswap32(&block->header.id);
        byteswap32((uint32 *)&block->header.block_size);
        byteswap32((uint32 *)&block->header.bytes_used);
        byteswap32(&block->gdat.crc32_checksum);
    }
}

typedef struct {
    int max_size;
    int size;
    unsigned char *bytes;
} write_buffer;

void write_bytes(write_buffer *b, unsigned char *src, int size) {
    if (b->size + size < b->max_size) {
        memcpy(b->bytes + b->size, src, size);
        b->size += size;
    }
}

void write_byte_n_times(write_buffer *b, unsigned char v, int n) {
    if (b->size + n < b->max_size) {
        memset(b->bytes + b->size, v, n);
        b->size += n;
    }
}

void bfbb_save_file_append_padding(write_buffer *b, int padding_size) {
    write_byte_n_times(b, 0xbf, padding_size);
}

bfbb_save_file_block *bfbb_save_file_append_block(write_buffer *b, bfbb_save_file_block *block, int is_gci) {
    int is_gdat = block->header.id == FOURCC_GDAT;
    //int is_cumd = block->header.id == FOURCC_CONST('C', 'U', 'M', 'D');
    int size_to_write =  is_gdat ? 4 : block->header.bytes_used;
    int padding_size = is_gdat ? 0 : block->header.block_size - size_to_write;
    /*
    if(block->header.id == FOURCC_CONST('L', 'E', 'D', 'R'))
    {
        for(int i = 0; i< block->header.bytes_used; i++)
        {
            if(block->raw_bytes[i] == '\0')
            {
                block->raw_bytes[i] = (char)0x01;
            }
        }
    }
    */
    bfbb_save_file_block *result = 0;
    
    if (is_gci) {
        bfbb_save_file_block_byteswap(block);
        // NOTE(jelly): we need to byteswap the id AFTER because
        //              bfbb_save_file_block_byteswap() needs to read the id in LE.
        byteswap32(&block->header.id);
        byteswap32((uint32 *)&block->header.block_size);
        byteswap32((uint32 *)&block->header.bytes_used);
    }
    result = (bfbb_save_file_block *)(b->bytes + b ->size);
    write_bytes(b, (unsigned char *)&block->header, sizeof(block->header));
    write_bytes(b, block->raw_bytes, size_to_write);
    bfbb_save_file_append_padding(b, padding_size);
    return result;
}

void bfbb_save_file_append_sfil(write_buffer *b, int is_gci) {
    if (is_gci) {
        unsigned char sfil_header[] = {
            0x53, 0x46, 0x49, 0x4C, 0x00, 0x00, 0x75,
            0x38, 0x00, 0x00, 0x00, 0x08, 0x52, 0x79, 
            0x61, 0x6E, 0x4E, 0x65, 0x69, 0x6C
        };
        write_bytes(b, sfil_header, sizeof(sfil_header));
        bfbb_save_file_append_padding(b, 30000);
    } else {
        unsigned char sfil_id[] = {
            0x4C, 0x49, 0x46, 0x53
        };
        write_bytes(b, sfil_id, sizeof(sfil_id));
        uint32 sfil_size = 51676-b->size-20-8;
        write_bytes(b, (unsigned char *)&sfil_size, sizeof(sfil_size));
        uint32 sfil_bytes_used = 8;
        write_bytes(b, (unsigned char *)&sfil_bytes_used, sizeof(sfil_bytes_used));
        write_bytes(b, (unsigned char *)"RyanNeil", 8);
        
        bfbb_save_file_append_padding(b, sfil_size - 8);
    }
}

int bfbb_save_file_write_out(bfbb_save_file *save_file, const char *path, int is_gci) {
    static unsigned char static_buffer[128*1024];
    write_buffer b = {sizeof(static_buffer), 0, static_buffer};
    int file_size, i;
    uint32 checksum;
    bfbb_save_file_block dummy_gdat, *gdat;
    int bytes_written_out = 0;
    
    if (is_gci) {
        const int GCI_EPOCH = 946702799;
        uint32 *date_address = (uint32*)(static_buffer + 0x28);
        uint32 date = (uint32)time(0) - GCI_EPOCH;
        
        byteswap32(&date);
        write_bytes(&b, (unsigned char *)bfbb_gci_header, sizeof(bfbb_gci_header));
        *date_address = date;
        write_byte_n_times(&b, 0, 512);
        write_bytes(&b, (unsigned char *)BFBB_SAVE_FILE_MAGIC_STRING, sizeof(BFBB_SAVE_FILE_MAGIC_STRING)-1);
        write_byte_n_times(&b, 0, 0x599);
    }
    
    bfbb_save_file_make_dummy_gdat_block(&dummy_gdat);
    gdat = bfbb_save_file_append_block(&b, &dummy_gdat, is_gci);
    for (i = 0; i < save_file->block_count; i++) {
        bfbb_save_file_append_block(&b, &save_file->blocks[i], is_gci);
    }
    
    bfbb_save_file_append_sfil(&b, is_gci);
    
    file_size = b.size - ((unsigned char *)gdat - static_buffer);
    checksum = crc32_get_checksum((char *)gdat + 16, file_size - 16);
    
    bfbb_save_file_set_gdat_block(gdat, file_size, checksum, is_gci);
    
    if (is_gci) {
        write_byte_n_times(&b, 0, 0x13f8);
    } else {
        sha1_hash160 xbox_sig = bfbb_hmac_sha1(b.bytes, b.size);
        write_bytes(&b, xbox_sig.hash, 20);
    }
    
    
    {
        FILE *out = fopen(path, "wb");
        if (out) {
            bytes_written_out = fwrite(b.bytes, 1, b.size, out);
            fclose(out);
        } else return 0;
    }
    
    return bytes_written_out == b.size;
}

// NOTE(jelly): maybe there can be more checks but who cares really
int bfbb_save_file_looks_like_gci_file(unsigned char *data, int len) {
    if (len > 0x6040 + 16) {
        int offset = 0x6040;
        return (data[offset+0] == 'G' &&
                data[offset+1] == 'D' &&
                data[offset+2] == 'A' &&
                data[offset+3] == 'T');
    }
    return 0;
}

int bfbb_save_file_looks_like_xsv_file(unsigned char *data, int len) {
    // TODO(jelly): pick a better number idiot
    if (len > 69) {
        return (data[0] == 'T' &&
                data[1] == 'A' &&
                data[2] == 'D' &&
                data[3] == 'G');
    }
    return 0;
}

int bfbb_save_file_read(bfbb_save_file *result, unsigned char *data, int len) {
    int file_type = -1;
    if (bfbb_save_file_looks_like_xsv_file(data, len)) file_type = 0;
    if (bfbb_save_file_looks_like_gci_file(data, len)) file_type = 1;
    if (file_type >= 0) return bfbb_save_file_read_(result, data, len, file_type);
    return 0;
}

// NOTE(jelly): API idea
bfbb_save_file_block *bfbb_save_file_find_block(bfbb_save_file *b, char *fourcc) {
    uint32 id = FOURCC_CONST(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
    int i;
    for (i = 0; i < b->block_count; i++) {
        bfbb_save_file_block *bl = &b->blocks[i];
        if (bl->header.id == id) {
            return bl;
        }
    }
    return 0;
}
