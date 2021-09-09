
// NOTE(jelly): all code below assumes a little-endian architecture for now.

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "bfbb_gci_header.h"
#include "hmac_sha1.c"


#define BASE_TYPE_TRIGGER      1
#define BASE_TYPE_PICKUP       4
#define BASE_TYPE_PLATFORM     6
#define BASE_TYPE_STATIC      11
#define BASE_TYPE_TIMER       14
#define BASE_TYPE_PORTAL      16
#define BASE_TYPE_GROUP       17
#define BASE_TYPE_SFX         19
#define BASE_TYPE_COUNTER     22
#define BASE_TYPE_BUTTON      24
#define BASE_TYPE_DISPATCHER  30
#define BASE_TYPE_COND        31
#define BASE_TYPE_CUTSCENEMGR 40
#define BASE_TYPE_TELEPORTBOX 49
#define BASE_TYPE_TASKBOX     53
#define BASE_TYPE_TAXI        57
#define BASE_TYPE_CAMERAFLY   62


typedef struct {
    u32 id;
    u8 type;
    char *name;
} scene_table_entry;


#include "scenedata/all.h"

// NOTE(jelly): this code (and table) is based off of the crc32 code from the decomp
//              https://github.com/bfbbdecomp/bfbb/blob/2bc99a1efcb8fab4cbccfc416d226f4a54b851ab/src/Core/x/xutil.cpp
static const u32 g_crc32_table[256] = {
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
u32 xUtil_crc_update(u32 crc_accum, char* data, int datasize)
{
    int i, j;
    
    for (i = 0; i < datasize; i++)
    {
        j = ((crc_accum >> 24) ^ *data++) & 0xff;
        crc_accum = (crc_accum << 8) ^ g_crc32_table[j];
    }
    
    return crc_accum;
}

u32 crc32_get_checksum(char *data, int size) {
    return xUtil_crc_update(-1, data, size);
}



#define BFBB_SAVE_FILE_LEDR_RANDOM_TEXT "--TakeMeToYourLeader--"

#pragma pack(push, 1)
typedef struct {
    u32 crc32_checksum;
} bfbb_save_file_block_gdat;


// TODO(jelly): figure out what the unknown bytes are (hopefully will can tell me someday)
typedef struct {
    char game_label[64];
    s32 progress;
    char unknown_bytes[12];
    u32 thumbnail_index;
    char unknown_bytes_[4];
    char random_text[sizeof(BFBB_SAVE_FILE_LEDR_RANDOM_TEXT)-1];
} bfbb_save_file_block_ledr;

typedef struct {
    char sceneid[5]; // NOTE(jelly): this is the FOURCC of the current level
} bfbb_save_file_block_room;

typedef struct {
    u32 sound_mode;
    float music_volume;
    float sfx_volume;
    u32 rumble;
} bfbb_save_file_block_pref;

typedef struct {
    s32 version;
} bfbb_save_file_block_svid;

typedef struct {
    s16 spats[15][8];
    s16 robot_data[15];
    s16 reminder_sock_cntr;
    s32 cheats;
} bfbb_save_file_block_cntr;

typedef struct {
    s32 socks;
    s32 pickups;
} bfbb_save_file_level_collectables;

typedef struct {
    s32 max_health;
    s32 character;
    s32 shinies;
    s32 spats;
    unsigned char has_bubble_bowl;
    unsigned char has_cruise_bubble;
    bfbb_save_file_level_collectables level_collectables[LEVEL_COUNT];
    s32 total_socks;
    char cutscene_played[14];
    //NOTE(Will): this stores 6 bits, i have no clue what they do :)
    u8 idiot_levels[6];
} bfbb_save_file_block_plyr;

typedef struct {
    u8 base_enable;
    u8 show_ent;
} base_type_trigger;

typedef struct {
    u8 base_enable;
    u8 show_ent;
    u8 state;
    u8 collected;
} base_type_pickup;

typedef struct {
    u8 base_enable;
    u8 show_ent;
} base_type_platform;

typedef struct {
    u8 base_enable;
    u8 show_ent;
} base_type_static;

typedef struct {
    u8 base_enable;
    u8 state;
    f32 seconds_left;
} base_type_timer;

typedef struct {
    u8 base_enable;
} base_type_group;

typedef struct {
    u8 base_enable;
} base_type_sfx;

typedef struct {
    u8 base_enable;
    s16 count;
    u8 state;
} base_type_counter;

typedef struct {
    u8 base_enable;
    u8 show_ent;
    u8 unknown_bit1;
    u8 unknown_bit2;
} base_type_button;

//wtf does this do
typedef struct {
    u8 base_enable;
} base_type_dispatcher;

typedef struct {
    u8 base_enable;
} base_type_cond;

typedef struct {
    u8 base_enable;
    u8 show_ent;
    u8 opened;
    u32 player_state;
} base_type_teleportbox;

typedef struct {
    u8 state;
} base_type_taskbox;

typedef struct {
    u8 base_enable;
} base_type_taxi;

typedef struct {
    u8 base_enable;
} base_type_camerafly;

typedef struct {
    u32 id, type;
    union {
        base_type_trigger     trigger;
        base_type_pickup      pickup;
        base_type_platform    platform;
        base_type_static      staticc;
        base_type_timer       timer;
        base_type_group       group;
        base_type_sfx         sfx;
        base_type_counter     counter;
        base_type_button      button;
        base_type_dispatcher  dispatcher;
        base_type_cond        cond;
        base_type_teleportbox tpbox;
        base_type_taskbox     taskbox;
        base_type_taxi        taxi;
        base_type_camerafly   camfly;
    };
} base_type;

typedef struct {
    u8 visited;
    f32 offsetx;
    f32 offsety;
    base_type *base; // NOTE(jelly): stb_ds dynamic array
} bfbb_save_file_block_scene;

typedef struct {
    union {
        u32 id;
        char id_chars[4];
    };
    s32 block_size;
    s32 bytes_used;
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
        bfbb_save_file_block_plyr plyr;
        bfbb_save_file_block_cntr cntr;
        bfbb_save_file_block_scene scene;
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
    u32 original_crc32_checksum; // NOTE(jelly): these probably aren't needed
    int is_big_endian;
    int block_count;
    bfbb_save_file_block blocks[64];
} bfbb_save_file;

typedef struct {
    int size;
    unsigned char *bytes;
} buffer;

//#define FOURCC(s) ((u32)(((s)[0]) | ((s)[1] << 8) | ((s)[2] << 16) | ((s)[3] << 24)))
#define FOURCC_CONST(a,b,c,d) ((u32)(((d)) | ((c) << 8) | ((b) << 16) | ((a) << 24)))

// NOTE(jelly): i love programming languages
#define FOURCC_GDAT FOURCC_CONST('G', 'D', 'A', 'T')
#define FOURCC_LEDR FOURCC_CONST('L', 'E', 'D', 'R')
#define FOURCC_ROOM FOURCC_CONST('R', 'O', 'O', 'M')
#define FOURCC_PLYR FOURCC_CONST('P', 'L', 'Y', 'R')

#define FOURCC_JF01 FOURCC_CONST('J', 'F', '0', '1')
#define FOURCC_JF02 FOURCC_CONST('J', 'F', '0', '2')
#define FOURCC_JF03 FOURCC_CONST('J', 'F', '0', '3')
#define FOURCC_JF04 FOURCC_CONST('J', 'F', '0', '4')
#define FOURCC_KF01 FOURCC_CONST('K', 'F', '0', '1')
#define FOURCC_KF02 FOURCC_CONST('K', 'F', '0', '2')
#define FOURCC_KF04 FOURCC_CONST('K', 'F', '0', '4')
#define FOURCC_KF05 FOURCC_CONST('K', 'F', '0', '5')
#define FOURCC_MNU3 FOURCC_CONST('M', 'N', 'U', '3')
#define FOURCC_RB01 FOURCC_CONST('R', 'B', '0', '1')
#define FOURCC_RB02 FOURCC_CONST('R', 'B', '0', '2')
#define FOURCC_RB03 FOURCC_CONST('R', 'B', '0', '3')
#define FOURCC_SM01 FOURCC_CONST('S', 'M', '0', '1')
#define FOURCC_SM02 FOURCC_CONST('S', 'M', '0', '2')
#define FOURCC_SM03 FOURCC_CONST('S', 'M', '0', '3')
#define FOURCC_SM04 FOURCC_CONST('S', 'M', '0', '4')
#define FOURCC_B101 FOURCC_CONST('B', '1', '0', '1')
#define FOURCC_B201 FOURCC_CONST('B', '2', '0', '1')
#define FOURCC_B302 FOURCC_CONST('B', '3', '0', '2')
#define FOURCC_B303 FOURCC_CONST('B', '3', '0', '3')
#define FOURCC_BB01 FOURCC_CONST('B', 'B', '0', '1')
#define FOURCC_BB02 FOURCC_CONST('B', 'B', '0', '2')
#define FOURCC_BB03 FOURCC_CONST('B', 'B', '0', '3')
#define FOURCC_BB04 FOURCC_CONST('B', 'B', '0', '4')
#define FOURCC_BC01 FOURCC_CONST('B', 'C', '0', '1')
#define FOURCC_BC02 FOURCC_CONST('B', 'C', '0', '2')
#define FOURCC_BC03 FOURCC_CONST('B', 'C', '0', '3')
#define FOURCC_BC04 FOURCC_CONST('B', 'C', '0', '4')
#define FOURCC_BC05 FOURCC_CONST('B', 'C', '0', '5')
#define FOURCC_DB01 FOURCC_CONST('D', 'B', '0', '1')
#define FOURCC_DB02 FOURCC_CONST('D', 'B', '0', '2')
#define FOURCC_DB03 FOURCC_CONST('D', 'B', '0', '3')
#define FOURCC_DB04 FOURCC_CONST('D', 'B', '0', '4')
#define FOURCC_DB06 FOURCC_CONST('D', 'B', '0', '6')
#define FOURCC_GL01 FOURCC_CONST('G', 'L', '0', '1')
#define FOURCC_GL02 FOURCC_CONST('G', 'L', '0', '2')
#define FOURCC_GL03 FOURCC_CONST('G', 'L', '0', '3')
#define FOURCC_GY01 FOURCC_CONST('G', 'Y', '0', '1')
#define FOURCC_GY02 FOURCC_CONST('G', 'Y', '0', '2')
#define FOURCC_GY03 FOURCC_CONST('G', 'Y', '0', '3')
#define FOURCC_GY04 FOURCC_CONST('G', 'Y', '0', '4')
#define FOURCC_HB00 FOURCC_CONST('H', 'B', '0', '0')
#define FOURCC_HB01 FOURCC_CONST('H', 'B', '0', '1')
#define FOURCC_HB02 FOURCC_CONST('H', 'B', '0', '2')
#define FOURCC_HB03 FOURCC_CONST('H', 'B', '0', '3')
#define FOURCC_HB04 FOURCC_CONST('H', 'B', '0', '4')
#define FOURCC_HB05 FOURCC_CONST('H', 'B', '0', '5')
#define FOURCC_HB06 FOURCC_CONST('H', 'B', '0', '6')
#define FOURCC_HB07 FOURCC_CONST('H', 'B', '0', '7')
#define FOURCC_HB08 FOURCC_CONST('H', 'B', '0', '8')
#define FOURCC_HB09 FOURCC_CONST('H', 'B', '0', '9')
#define FOURCC_HB10 FOURCC_CONST('H', 'B', '1', '0')
#define FOURCC_PG12 FOURCC_CONST('P', 'G', '1', '2')

#define FOURCC_CNTR FOURCC_CONST('C', 'N', 'T', 'R')
#define FOURCC_PREF FOURCC_CONST('P', 'R', 'E', 'F')
#define FOURCC_SVID FOURCC_CONST('S', 'V', 'I', 'D')
#define FOURCC_SFIL FOURCC_CONST('S', 'F', 'I', 'L')

int bfbb_save_file_fourcc_is_scene(u32 id) {
    static const u32 scene_fourccs[] = {
        FOURCC_JF01, FOURCC_JF02, FOURCC_JF03, FOURCC_JF04,
        FOURCC_KF01, FOURCC_KF02, FOURCC_KF04, FOURCC_KF05,
        FOURCC_MNU3,
        FOURCC_RB01, FOURCC_RB02, FOURCC_RB03,
        FOURCC_SM01, FOURCC_SM02, FOURCC_SM03, FOURCC_SM04,
        FOURCC_B101, FOURCC_B201, FOURCC_B302, FOURCC_B303,
        FOURCC_BB01, FOURCC_BB02, FOURCC_BB03, FOURCC_BB04,
        FOURCC_BC01, FOURCC_BC02, FOURCC_BC03, FOURCC_BC04, FOURCC_BC05,
        FOURCC_DB01, FOURCC_DB02, FOURCC_DB03,
        FOURCC_DB04, FOURCC_DB06,
        FOURCC_GL01, FOURCC_GL02, FOURCC_GL03,
        FOURCC_GY01, FOURCC_GY02, FOURCC_GY03, FOURCC_GY04,
        FOURCC_HB00, FOURCC_HB01, FOURCC_HB02, 
        FOURCC_HB03, FOURCC_HB04, FOURCC_HB05,
        FOURCC_HB06, FOURCC_HB07, FOURCC_HB08,
        FOURCC_HB09,
        FOURCC_PG12,
    };
    // TODO(jelly): is there REALLY no DB05??
    for (int i = 0; i < ArrayCount(scene_fourccs); i++) {
        if (id == scene_fourccs[i]) return 1;
    }
    return 0;
}

int bfbb_save_file_block_is_scene(bfbb_save_file_block *block) {
    return bfbb_save_file_fourcc_is_scene(block->header.id);
}

int bfbb_save_file_fourcc_is_bit_block(u32 id) {
    return id == FOURCC_PLYR || id == FOURCC_CNTR || bfbb_save_file_fourcc_is_scene(id);
}

void byteswap32_n(u8 *data, int count) {
    assert(count % 4 ==0);
    for (int i = 0; i < count; i += 4) {
        byteswap32((u32 *)&data[i]);
    }
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

u8 *eat_bytes_and_byteswap(buffer *b, int size) {
    u8 *result = eat_bytes(b, size);
    if (result)byteswap32_n(result, size);
    return result;
}

void eat_bf_padding(buffer *b) {
    while (b->size > 0 && *(u8 *)peek_bytes(b, 1) == 0xbf) 
        eat_bytes(b, 1);
}

// NOTE(jelly): in = 1 when reading in. in = 0 when writing out.
void bfbb_save_file_byteswap(u8 *data, int size, int in) {
    buffer b = {size, data};
    int header_size = 12;
    while (b.size >= 4) {
        u32 id = *(u32 *)peek_bytes(&b, 4);
        if (in) byteswap32(&id);
        switch (id) {
            case FOURCC_LEDR: {
                eat_bytes_and_byteswap(&b, header_size);
                eat_bytes(&b, 64);
                eat_bytes_and_byteswap(&b, 16);
                eat_bytes(&b, 4); // NOTE(jelly): apparently thumbnail index is NOT byteswapped!
                eat_bytes_and_byteswap(&b, 4); // TODO(jelly): should this be byteswapped?
                eat_bytes(&b, sizeof(BFBB_SAVE_FILE_LEDR_RANDOM_TEXT)-1); // TODO(jelly): check the string???
                eat_bf_padding(&b);
            } break;
            
            case FOURCC_ROOM: {
                eat_bytes_and_byteswap(&b, header_size);
                eat_bytes(&b, 4); // NOTE(jelly): sceneid is a string, so it is NOT byteswapped!!!
                eat_bf_padding(&b);
            } break;
            
            case FOURCC_SFIL: {
                eat_bytes_and_byteswap(&b, header_size);
                eat_bytes(&b, 8);
                eat_bytes_and_byteswap(&b, b.size/4*4);
            } break;
            
            default: {
                // NOTE(jelly): 'bit' blocks (plyr, cntr, scenes) SHOULD be byteswapped!
                //               the commented-out code below prevents them from being byteswapped 
                /*
                                if (!in && bfbb_save_file_fourcc_is_bit_block(id)) {
                                    
                                    eat_bytes_and_byteswap(&b, 4);
                                    u32 *block_size;
                                    if (in) block_size = (u32 *)eat_bytes_and_byteswap(&b, 4);
                                    else    block_size = (u32 *)eat_bytes(&b, 4);
                                    eat_bytes_and_byteswap(&b, 4);
                                    eat_bytes(&b, *block_size);
                                    if (!in) byteswap32(block_size);
                                }
                                else 
                    */            
                eat_bytes_and_byteswap(&b, 4);
            }
        }
    }
}

void bfbb_save_file_free_blocks(bfbb_save_file *save_file) {
    int block_count = save_file->block_count;
    for (int i = 0; i < block_count; i++) {
        bfbb_save_file_block *block = &save_file->blocks[i];
        if (bfbb_save_file_block_is_scene(block)) {
            arrfree(block->scene.base);
        }
    }
}

bfbb_save_file_block_header bfbb_save_file_block_header_parse(buffer *b, int is_gci) {
    bfbb_save_file_block_header result = {0};
    if (peek_bytes(b, sizeof(bfbb_save_file_block_header))) {
        result = *(bfbb_save_file_block_header *)eat_bytes(b, sizeof(bfbb_save_file_block_header));
    }
    return result;
}

#define BFBB_SAVE_FILE_MAGIC_STRING "SPONGEBOB:WHENROBOTSATTACK::RyanNeilDan"

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
        return 1;
    }
    return -1;
}

base_type bfbb_save_file_read_scene_block_base_type(bfbb_save_file *save_file, s32 block_index, bit_reader* br, scene_table_entry* p)
{
    base_type b = {0};
    b.id = p->id;
    b.type = p->type;
    switch(p->type)
    {
        case BASE_TYPE_TRIGGER:
        {
            b.trigger.base_enable = (u8)bit_eat(br, 1);
            b.trigger.show_ent = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_PICKUP:
        {
            b.pickup.base_enable = (u8)bit_eat(br, 1);
            b.pickup.show_ent = (u8)bit_eat(br, 1);
            b.pickup.state = bit_eat(br, 7);
            b.pickup.collected = bit_eat(br, 1);
            //u8 enthidden = state & 0x4;
            break;
        }
        case BASE_TYPE_PLATFORM:
        {
            b.platform.base_enable = (u8)bit_eat(br, 1);
            b.platform.show_ent = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_STATIC:
        {
            b.staticc.base_enable = (u8)bit_eat(br, 1);
            b.staticc.show_ent = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_TIMER:
        {
            b.timer.base_enable = (u8)bit_eat(br, 1);
            b.timer.state = (u8)bit_eat(br, 8);
            b.timer.seconds_left = bit_eat_float(br);
            break;
        }
        case BASE_TYPE_GROUP:
        {
            b.group.base_enable = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_SFX:
        {
            b.sfx.base_enable = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_COUNTER:
        {
            b.counter.base_enable = (u8)bit_eat(br, 1);
            b.counter.state = (u8)bit_eat(br, 8);
            b.counter.count = (s16)bit_eat(br, 16); 
            break;
        }
        case BASE_TYPE_BUTTON:
        {
            b.button.base_enable = (u8)bit_eat(br, 1);
            b.button.show_ent = (u8)bit_eat(br, 1);
            b.button.unknown_bit1 = (u8)bit_eat(br, 1);
            b.button.unknown_bit2 = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_DISPATCHER:
        {
            b.dispatcher.base_enable = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_COND:
        {
            b.cond.base_enable = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_TELEPORTBOX:
        {
            b.tpbox.base_enable = (u8)bit_eat(br, 1);
            b.tpbox.show_ent = (u8)bit_eat(br, 1);
            b.tpbox.opened = (u8)bit_eat(br, 1);
            b.tpbox.player_state = (u32)bit_eat(br, 32);
            break;
        }
        case BASE_TYPE_TASKBOX:
        {
            b.taskbox.state = (u8)bit_eat(br, 8);
            break;
        }
        case BASE_TYPE_TAXI:
        {
            b.taxi.base_enable = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_CAMERAFLY:
        {
            b.camfly.base_enable = (u8)bit_eat(br, 1);
            break;
        }
        case BASE_TYPE_CUTSCENEMGR:
        {
            //NOTE(Will): It does nothing?
            break;
        }
        default:
        {
            printf("Unknown base type %d\n", p->type);
            assert(0);
        }
    }
    return b;
}

int bfbb_save_file_read_scene(bfbb_save_file *save_file, bit_reader *br, bfbb_save_file_block *block, scene_table_entry *table, int table_count) {
    // NOTE(jelly): declaring a scene like this before writing to block->scene MUST be done because block->scene is a union
    //              and reading from scene while writing to it will be broken.
    bfbb_save_file_block_scene scene = {0};
    
    scene.visited = bit_eat(br, 1);
    scene.offsetx = bit_eat_float(br);
    scene.offsety = bit_eat_float(br);
    for (int i = 0; i < table_count; i++) {
        arrput(scene.base, bfbb_save_file_read_scene_block_base_type(save_file, i, br, &table[i]));
    }
    block->scene = scene;
    return 1; // TODO(jelly): more checks??
}

#define CaseSceneTableName(fourcc) fourcc##_table
#define CaseSceneRead(sf,br,bl,fourcc) \
case FOURCC_##fourcc: \
bfbb_save_file_read_scene(sf, br, bl, CaseSceneTableName(fourcc), ArrayCount(CaseSceneTableName(fourcc)));\
break;

const u8 spat_count_per_world[] = {
    8,8,8,8,1,8,8,8,1,8,8,8,2,8,8
};

int bfbb_save_file_read_bit_blocks(bfbb_save_file *save_file)
{
    for(int i = 0; i<save_file->block_count; i++)
    {
        bfbb_save_file_block* block = &save_file->blocks[i];
        bit_buffer b = {block->header.bytes_used, block->raw_bytes, 0, save_file->is_big_endian};
        bit_reader br = b;
        switch(block->header.id)
        {
            case(FOURCC_PLYR):
            {
                bfbb_save_file_block_plyr p = {0};
                bit_eat(&br, 1);
                p.max_health = bit_eat_s32(&br);
                p.character = bit_eat_s32(&br);
                p.shinies = bit_eat_s32(&br);
                p.spats = bit_eat_s32(&br);
                p.has_bubble_bowl = bit_eat(&br, 8);
                p.has_cruise_bubble = bit_eat(&br, 8);
                
                for(int i = 0; i<LEVEL_COUNT; i++)
                {
                    p.level_collectables[i].socks = bit_eat_s32(&br);
                    p.level_collectables[i].pickups = bit_eat_s32(&br);
                }
                
                p.total_socks = bit_eat_s32(&br);
                for(int i = 0; i<14; i++)
                {
                    p.cutscene_played[i] = (char)bit_eat_s32(&br);
                }
                for(int i = 0; i<6; i++)
                {
                    p.idiot_levels[i] = bit_eat(&br, 1);
                }
                block->plyr = p;
                break;
            }
            CaseSceneRead(save_file, &br, block, JF01);
            CaseSceneRead(save_file, &br, block, JF02);
            CaseSceneRead(save_file, &br, block, JF03);
            CaseSceneRead(save_file, &br, block, JF04);
            CaseSceneRead(save_file, &br, block, KF01);
            CaseSceneRead(save_file, &br, block, KF02);
            CaseSceneRead(save_file, &br, block, KF04);
            CaseSceneRead(save_file, &br, block, KF05);
            CaseSceneRead(save_file, &br, block, MNU3);
            CaseSceneRead(save_file, &br, block, RB01);
            CaseSceneRead(save_file, &br, block, RB02);
            CaseSceneRead(save_file, &br, block, RB03);
            CaseSceneRead(save_file, &br, block, SM01);
            CaseSceneRead(save_file, &br, block, SM02);
            CaseSceneRead(save_file, &br, block, SM03);
            CaseSceneRead(save_file, &br, block, SM04);
            CaseSceneRead(save_file, &br, block, B101);
            CaseSceneRead(save_file, &br, block, B201);
            CaseSceneRead(save_file, &br, block, B302);
            CaseSceneRead(save_file, &br, block, B303);
            CaseSceneRead(save_file, &br, block, BB01);
            CaseSceneRead(save_file, &br, block, BB02);
            CaseSceneRead(save_file, &br, block, BB03);
            CaseSceneRead(save_file, &br, block, BB04);
            CaseSceneRead(save_file, &br, block, BC01);
            CaseSceneRead(save_file, &br, block, BC02);
            CaseSceneRead(save_file, &br, block, BC03);
            CaseSceneRead(save_file, &br, block, BC04);
            CaseSceneRead(save_file, &br, block, BC05);
            CaseSceneRead(save_file, &br, block, DB01);
            CaseSceneRead(save_file, &br, block, DB02);
            CaseSceneRead(save_file, &br, block, DB03);
            CaseSceneRead(save_file, &br, block, DB04);
            CaseSceneRead(save_file, &br, block, DB06);
            CaseSceneRead(save_file, &br, block, GL01);
            CaseSceneRead(save_file, &br, block, GL02);
            CaseSceneRead(save_file, &br, block, GL03);
            CaseSceneRead(save_file, &br, block, GY01);
            CaseSceneRead(save_file, &br, block, GY02);
            CaseSceneRead(save_file, &br, block, GY03);
            CaseSceneRead(save_file, &br, block, GY04);
            CaseSceneRead(save_file, &br, block, HB00);
            CaseSceneRead(save_file, &br, block, HB01);
            CaseSceneRead(save_file, &br, block, HB02);
            CaseSceneRead(save_file, &br, block, HB03);
            CaseSceneRead(save_file, &br, block, HB04);
            CaseSceneRead(save_file, &br, block, HB05);
            CaseSceneRead(save_file, &br, block, HB06);
            CaseSceneRead(save_file, &br, block, HB07);
            CaseSceneRead(save_file, &br, block, HB08);
            CaseSceneRead(save_file, &br, block, HB09);
            CaseSceneRead(save_file, &br, block, HB10);
            CaseSceneRead(save_file, &br, block, PG12);
            case(FOURCC_CNTR):
            {
                bfbb_save_file_block_cntr c = {0};
                bit_eat(&br, 1);
                for(int i = 0; i<15; i++)
                {
                    for(int j = 0; j<spat_count_per_world[i]; j++)
                    {
                        c.spats[i][j] = bit_eat(&br, 16);
                    }
                }
                for(int i = 0; i<15; i++)
                {
                    c.robot_data[i] = bit_eat(&br, 16);
                }
                c.reminder_sock_cntr = bit_eat(&br, 16);
                c.cheats = bit_eat_s32(&br);
                block->cntr = c;
                break;
            }
            case(FOURCC_LEDR):
            {
                break;
            }
            case(FOURCC_ROOM):
            {
                break;
            }
            case(FOURCC_PREF):
            {
                break;
            }
            case(FOURCC_SVID):
            {
                break;
            }
            default:
            {
                char* chars = block->header.id_chars;
                printf("Unknown block: %c%c%c%c", chars[3], chars[2], chars[1], chars[0]);
                assert(0);
            }
        }
    }
    // TODO(jelly): do more checks??
    return 1;
}

int bfbb_save_file_read_(bfbb_save_file *result, unsigned char *bytes, int size, int is_gci) {
    buffer b = {size, bytes};
    char *magic_string = 0;
    bfbb_save_file_block gdat;
    int rc;
    int i = 0;
    
    result->is_big_endian = is_gci;
    
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
        
        bfbb_save_file_byteswap(b.bytes, b.size, 1);
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
    
    result->block_count = i-1;
    
    bfbb_save_file_read_bit_blocks(result);
    
    return 1;
}

void bfbb_save_file_make_dummy_gdat_block(bfbb_save_file_block *block) {
    block->header.id = FOURCC_GDAT;
    block->header.block_size = 1;
    block->header.bytes_used = 0;
    block->gdat.crc32_checksum = 0;
}

void bfbb_save_file_set_gdat_block(bfbb_save_file_block *block, int file_size, u32 crc32_checksum, int is_gci) {
    block->header.id = FOURCC_GDAT;
    block->header.block_size = 1;
    block->header.bytes_used = file_size;
    block->gdat.crc32_checksum = crc32_checksum;
    
    if (is_gci) {
        byteswap32(&block->header.id);
        byteswap32(&block->header.block_size);
        byteswap32(&block->header.bytes_used);
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


void bfbb_save_file_append_zero_padding(write_buffer *b, int padding_size) {
    write_byte_n_times(b, 0x00, padding_size);
}

void bfbb_save_file_append_padding(write_buffer *b, int padding_size) {
    write_byte_n_times(b, 0xbf, padding_size);
}

void bfbb_save_file_write_scene_block(bit_writer *b, scene_table_entry* p, s32 n, bfbb_save_file_block *block)
{
    base_type bt = block->scene.base[n];
    switch(p->type)
    {
        case BASE_TYPE_TRIGGER:
        {
            bit_push(b, bt.trigger.base_enable, 1);
            bit_push(b, bt.trigger.show_ent, 1);
            break;
        }
        case BASE_TYPE_PICKUP:
        {
            bit_push(b, bt.pickup.base_enable, 1);
            bit_push(b, bt.pickup.show_ent, 1);
            bit_push(b, bt.pickup.state, 7);
            bit_push(b, bt.pickup.collected, 1);
            break;
        }
        case BASE_TYPE_PLATFORM:
        {
            bit_push(b, bt.platform.base_enable, 1);
            bit_push(b, bt.platform.show_ent, 1);
            break;
        }
        case BASE_TYPE_STATIC:
        {
            bit_push(b, bt.staticc.base_enable, 1);
            bit_push(b, bt.staticc.show_ent, 1);
            break;
        }
        case BASE_TYPE_TIMER:
        {
            bit_push(b, bt.timer.base_enable, 1);
            bit_push(b, bt.timer.state, 8);
            bit_push_float(b, bt.timer.seconds_left);
            break;
        }
        case BASE_TYPE_GROUP:
        {
            bit_push(b, bt.group.base_enable, 1);
            break;
        }
        case BASE_TYPE_SFX:
        {
            bit_push(b, bt.sfx.base_enable, 1);
            break;
        }
        case BASE_TYPE_COUNTER:
        {
            bit_push(b, bt.counter.base_enable, 1);
            bit_push(b, bt.counter.state, 8);
            bit_push(b, bt.counter.count, 16);
            break;
        }
        case BASE_TYPE_BUTTON:
        {
            bit_push(b, bt.button.base_enable, 1);
            bit_push(b, bt.button.show_ent, 1);
            bit_push(b, bt.button.unknown_bit1, 1);
            bit_push(b, bt.button.unknown_bit2, 1);
            break;
        }
        case BASE_TYPE_DISPATCHER:
        {
            bit_push(b, bt.dispatcher.base_enable, 1);
            break;
        }
        case BASE_TYPE_COND:
        {
            bit_push(b, bt.cond.base_enable, 1);
            break;
        }
        case BASE_TYPE_TELEPORTBOX:
        {
            bit_push(b, bt.tpbox.base_enable, 1);
            bit_push(b, bt.tpbox.show_ent, 1);
            bit_push(b, bt.tpbox.opened, 1);
            bit_push(b, bt.tpbox.player_state, 32);
            break;
        }
        case BASE_TYPE_TASKBOX:
        {
            bit_push(b, bt.taskbox.state, 8);
            break;
        }
        case BASE_TYPE_TAXI:
        {
            bit_push(b, bt.taxi.base_enable, 1);
            break;
        }
        case BASE_TYPE_CAMERAFLY:
        {
            bit_push(b, bt.camfly.base_enable, 1);
            break;
        }
        case BASE_TYPE_CUTSCENEMGR:
        {
            //NOTE(Will): It does nothing?
            break;
        }
        default:
        {
            printf("Unknown base type %d", p->type);
            assert(0);
        }
    }
}

void bfbb_save_file_write_scene_block_stuff(bit_writer *bw, bfbb_save_file_block *block)
{
    bit_push(bw, block->scene.visited, 1);
    bit_push_float(bw, block->scene.offsetx);
    bit_push_float(bw, block->scene.offsety);
}

void bfbb_save_file_write_scene(bit_writer *bw, bfbb_save_file_block *block, scene_table_entry *table, write_buffer *b, int is_big_endian) {
    bfbb_save_file_write_scene_block_stuff(bw, block); // TODO(jelly): name this better for godsake
    for (int i =0; i < arrlen(block->scene.base); i++) {
        bfbb_save_file_write_scene_block(bw, &table[i], i, block);
    }
    unsigned char *data = b->bytes + b->size;
    int n = block->header.bytes_used;
    b->size += n;
}

#define CaseSceneWrite(bw, block, b, is_gci, fourcc) case FOURCC_##fourcc: bfbb_save_file_write_scene(bw, block, fourcc##_table, b, is_gci); break;

bfbb_save_file_block *bfbb_save_file_append_block(write_buffer *b, bfbb_save_file_block *block, int is_gci) {
    int is_gdat = block->header.id == FOURCC_GDAT;
    int size_to_write =  is_gdat ? 4 : block->header.bytes_used;
    int padding_size = is_gdat ? 0 : block->header.block_size - size_to_write;
    
    bfbb_save_file_block *result = 0;
    
    result = (bfbb_save_file_block *)(b->bytes + b ->size);
    write_bytes(b, (unsigned char *)&block->header, sizeof(block->header));
    
    {
        bit_writer bw = {b->max_size - b->size, b->bytes + b->size, 0, is_gci};
        switch (block->header.id) {
            case FOURCC_PLYR: {
                bit_push(&bw, 1, 1);
                bit_push_s32(&bw, block->plyr.max_health);
                bit_push_s32(&bw, block->plyr.character);
                bit_push_s32(&bw, block->plyr.shinies);
                bit_push_s32(&bw, block->plyr.spats);
                bit_push(&bw, block->plyr.has_bubble_bowl, 8);
                bit_push(&bw, block->plyr.has_cruise_bubble, 8);
                for(int i = 0; i<LEVEL_COUNT; i++)
                {
                    bit_push_s32(&bw, block->plyr.level_collectables[i].socks);
                    bit_push_s32(&bw, block->plyr.level_collectables[i].pickups);
                }
                bit_push_s32(&bw, block->plyr.total_socks);
                for(int i = 0; i<14; i++)
                {
                    bit_push_s32(&bw, block->plyr.cutscene_played[i]);
                }
                
                for(int i = 0; i<6; i++)
                {
                    bit_push(&bw, block->plyr.idiot_levels[i], 1);
                }
                b->size+=size_to_write;
            } break;
            CaseSceneWrite(&bw, block, b, is_gci, JF01);
            CaseSceneWrite(&bw, block, b, is_gci, JF02);
            CaseSceneWrite(&bw, block, b, is_gci, JF03);
            CaseSceneWrite(&bw, block, b, is_gci, JF04);
            CaseSceneWrite(&bw, block, b, is_gci, KF01);
            CaseSceneWrite(&bw, block, b, is_gci, KF02);
            CaseSceneWrite(&bw, block, b, is_gci, KF04);
            CaseSceneWrite(&bw, block, b, is_gci, KF05);
            CaseSceneWrite(&bw, block, b, is_gci, MNU3);
            CaseSceneWrite(&bw, block, b, is_gci, RB01);
            CaseSceneWrite(&bw, block, b, is_gci, RB02);
            CaseSceneWrite(&bw, block, b, is_gci, RB03);
            CaseSceneWrite(&bw, block, b, is_gci, SM01);
            CaseSceneWrite(&bw, block, b, is_gci, SM02);
            CaseSceneWrite(&bw, block, b, is_gci, SM03);
            CaseSceneWrite(&bw, block, b, is_gci, SM04);
            CaseSceneWrite(&bw, block, b, is_gci, B101);
            CaseSceneWrite(&bw, block, b, is_gci, B201);
            CaseSceneWrite(&bw, block, b, is_gci, B302);
            CaseSceneWrite(&bw, block, b, is_gci, B303);
            CaseSceneWrite(&bw, block, b, is_gci, BB01);
            CaseSceneWrite(&bw, block, b, is_gci, BB02);
            CaseSceneWrite(&bw, block, b, is_gci, BB03);
            CaseSceneWrite(&bw, block, b, is_gci, BB04);
            CaseSceneWrite(&bw, block, b, is_gci, BC01);
            CaseSceneWrite(&bw, block, b, is_gci, BC02);
            CaseSceneWrite(&bw, block, b, is_gci, BC03);
            CaseSceneWrite(&bw, block, b, is_gci, BC04);
            CaseSceneWrite(&bw, block, b, is_gci, BC05);
            CaseSceneWrite(&bw, block, b, is_gci, DB01);
            CaseSceneWrite(&bw, block, b, is_gci, DB02);
            CaseSceneWrite(&bw, block, b, is_gci, DB03);
            CaseSceneWrite(&bw, block, b, is_gci, DB04);
            CaseSceneWrite(&bw, block, b, is_gci, DB06);
            CaseSceneWrite(&bw, block, b, is_gci, GL01);
            CaseSceneWrite(&bw, block, b, is_gci, GL02);
            CaseSceneWrite(&bw, block, b, is_gci, GL03);
            CaseSceneWrite(&bw, block, b, is_gci, GY01);
            CaseSceneWrite(&bw, block, b, is_gci, GY02);
            CaseSceneWrite(&bw, block, b, is_gci, GY03);
            CaseSceneWrite(&bw, block, b, is_gci, GY04);
            CaseSceneWrite(&bw, block, b, is_gci, HB00);
            CaseSceneWrite(&bw, block, b, is_gci, HB01);
            CaseSceneWrite(&bw, block, b, is_gci, HB02);
            CaseSceneWrite(&bw, block, b, is_gci, HB03);
            CaseSceneWrite(&bw, block, b, is_gci, HB04);
            CaseSceneWrite(&bw, block, b, is_gci, HB05);
            CaseSceneWrite(&bw, block, b, is_gci, HB06);
            CaseSceneWrite(&bw, block, b, is_gci, HB07);
            CaseSceneWrite(&bw, block, b, is_gci, HB08);
            CaseSceneWrite(&bw, block, b, is_gci, HB09);
            CaseSceneWrite(&bw, block, b, is_gci, HB10);
            CaseSceneWrite(&bw, block, b, is_gci, PG12);
            case(FOURCC_CNTR):
            {
                bit_push(&bw, 1, 1);
                for(int i = 0; i<15; i++)
                {
                    for(int j = 0; j<spat_count_per_world[i]; j++)
                    {
                        bit_push(&bw, block->cntr.spats[i][j], 16);
                    }
                }
                for(int i = 0; i<15; i++)
                {
                    bit_push(&bw, block->cntr.robot_data[i], 16);
                }
                bit_push(&bw, block->cntr.reminder_sock_cntr, 16);
                bit_push_s32(&bw, block->cntr.cheats);
                b->size+=size_to_write;
                break;
            }
            default: write_bytes(b, block->raw_bytes, size_to_write);
        }
    }
    
    bfbb_save_file_append_padding(b, padding_size);
    return result;
}

// TODO(jelly): rename bfbb_save_file_append_block (and other functions) because ppl might get confused 
//              and think that they append a block to a bfbb_save_file struct when they're really
//              just for writing the blocks to a buffer.

void bfbb_save_file_append_sfil(bfbb_save_file *save_file, write_buffer *b, int is_gci) {
    int size_of_data = 0;
    for(int i = 0; i<save_file->block_count; i++)
    {
        size_of_data+=save_file->blocks[i].header.block_size;
    }
    //size_of_data = b->size;
    //TODO(Will): Figure out how to actually fucking do this :)
    u32 sfil_size = 0xBB88;
    //if (is_gci) sfil_size -= 0x6040; // NOTE(jelly): size of entire gci header
    u32 sfil_bytes_used = 8;
    
    // TODO(jelly): remove if statement to only be what's in the else now that byteswapping is all done in bulk
    is_gci = 0;
    if (is_gci) {
        unsigned char sfil_id[] = {
            0x53, 0x46, 0x49, 0x4C
        };
        write_bytes(b, sfil_id, sizeof(sfil_id));
    } else {
        unsigned char sfil_id[] = {
            0x4C, 0x49, 0x46, 0x53
        };
        write_bytes(b, sfil_id, sizeof(sfil_id));
    }
    write_bytes(b, (unsigned char *)&sfil_size, sizeof(sfil_size));
    write_bytes(b, (unsigned char *)&sfil_bytes_used, sizeof(sfil_bytes_used));
    write_bytes(b, (unsigned char *)"RyanNeil", 8);
    
    bfbb_save_file_append_padding(b, sfil_size - 8);
}

int bfbb_save_file_write_out(bfbb_save_file *save_file, const char *path, int is_gci) {
    static unsigned char static_buffer[128*1024];
    write_buffer b = {sizeof(static_buffer), 0, static_buffer};
    int file_size, i;
    u32 checksum;
    bfbb_save_file_block dummy_gdat, *gdat;
    int bytes_written_out = 0;
    
    save_file->is_big_endian = is_gci;
    
    memset(static_buffer, 0, sizeof(static_buffer)); // NOTE(jelly): in case we write out multiple files, i want this buffer clear.
    
    if (is_gci) {
        const int GCI_EPOCH = 946702799;
        u32 *date_address = (u32*)(static_buffer + 0x28);
        u32 date = (u32)time(0) - GCI_EPOCH;
        
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
    
    bfbb_save_file_append_sfil(save_file, &b, is_gci);
    
    file_size = b.size - ((unsigned char *)gdat - static_buffer);
    
    if (is_gci) bfbb_save_file_byteswap((u8 *)gdat, file_size, 0);
    
    checksum = crc32_get_checksum((char *)gdat + 16, file_size - 16);
    bfbb_save_file_set_gdat_block(gdat, file_size, checksum, is_gci);
    
    if (is_gci) {
        write_byte_n_times(&b, 0, 0x14040 - b.size);
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
    u32 id = FOURCC_CONST(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
    int i;
    for (i = 0; i < b->block_count; i++) {
        bfbb_save_file_block *bl = &b->blocks[i];
        if (bl->header.id == id) {
            return bl;
        }
    }
    return 0;
}
