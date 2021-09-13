
// NOTE(jelly): all code below assumes a little-endian architecture for now.

/*
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
*/

#include "bfbb_gci_header.h"
#include "bfbb_save.h"
#include "saves/100save.h"
#ifdef BFBBMIX
#include "scenedata/bfbbmix.h"
#else
#include "scenedata/bfbb.h"
#endif
#include "scene_tabs.h"
#include "hmac_sha1.c"

// NOTE(jelly): this code is based off of the crc32 code from the decomp
// https://github.com/bfbbdecomp/bfbb/blob/2bc99a1efcb8fab4cbccfc416d226f4a54b851ab/src/Core/x/xutil.cpp
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
    return xUtil_crc_update(0xFFFFFFFF, data, size);
}

// TODO(jelly): finish this
/*
void base_type_stack_alloc(base_type_stack *stack, s32 new_capacity) {
    
}

void base_type_stack_maybe_grow(base_type_stack *stack, s32 ) {
    if (stack->)
}

base_type *new_base_type(base_type_stack *stack) {
    base_type_stack_maybe_grow(stack, 1);
    return &stack->stack[stack->in_use++];
}
*/

int bfbb_save_file_fourcc_looks_like_scene(u32 id) {
    u32 a = (id >> 24) & 0xff; 
    u32 b = (id >> 16) & 0xff; 
    u32 c = (id >> 8) & 0xff; 
    u32 d = id & 0xff; 
    return (is_uppercase_letter(a) &&
            (is_uppercase_letter(b) || is_digit(b)) &&
            (is_uppercase_letter(c) || is_digit(c)) &&
            is_digit(d));
}

const scene_table_meta *get_scene_table_meta(u32 id) {
    for (int i = 0; i < ArrayCount(scene_tabs); i++) {
        const scene_table_meta *m = &scene_tabs[i];
        if (m->id == id) return m;
    }
    return 0;
}

int bfbb_save_file_fourcc_is_scene(u32 id) {
    for (int i = 0; i < ArrayCount(scene_tabs); i++) {
        if (id == scene_tabs[i].id) return 1;
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

bfbb_save_file_block_header bfbb_save_file_block_header_parse(buffer *b, int is_gci) {
    bfbb_save_file_block_header result = {0};
    if (peek_bytes(b, sizeof(bfbb_save_file_block_header))) {
        result = *(bfbb_save_file_block_header *)eat_bytes(b, sizeof(bfbb_save_file_block_header));
    }
    return result;
}

int bfbb_save_file_looks_like_a_valid_header(bfbb_save_file_block_header *header) {
    int looks_like_a_valid_id = 1;
    int i;
    for (i = 0; i < 4; i++) {
        char c = header->id_chars[i];
        if (!(is_uppercase_letter(c) || is_digit(c))) {
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
        case BASE_TYPE_UIFONT:
        {
            b.uifont.base_enable = (u8)bit_eat(br, 1);
            b.uifont.show_ent = (u8)bit_eat(br, 1);
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
    // NOTE(jelly): declaring a scene like this before writing to block->scene
    //              MUST be done because block->scene is a union and 
    //              reading from block->scene while writing to it will probably break
    bfbb_save_file_block_scene scene = {0};
    
    // TODO(jelly): linked lists of these arrays
    u32 max_base_count = Maximum(table_count, 512);
    scene.bases = MemoryArenaAllocTypeCount(save_file->memory, base_type, max_base_count);
    
    scene.visited = bit_eat(br, 1);
    scene.offsetx = bit_eat_float(br);
    scene.offsety = bit_eat_float(br);
    for (int i = 0; i < table_count; i++) {
        scene.bases[i] = bfbb_save_file_read_scene_block_base_type(save_file, i, br, &table[i]);
    }
    scene.base_count = table_count;
    block->scene = scene;
    return 1; // TODO(jelly): more checks??
}

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
            //TODO(Will): Implement these :)
#ifdef BFBBMIX
            case(FOURCC_EX01):
            case(FOURCC_EX02):
            case(FOURCC_EX03):
            case(FOURCC_EX04):
            case(FOURCC_EX05):
            case(FOURCC_EX06):
            case(FOURCC_EX07):
            case(FOURCC_EX08):
            case(FOURCC_EX09):
            case(FOURCC_PG02):
            case(FOURCC_PG13):
#endif
            case(FOURCC_LEDR):
            case(FOURCC_ROOM):
            case(FOURCC_PREF):
            case(FOURCC_SVID): break;
            default:
            {
                const scene_table_meta *m;
                if (bfbb_save_file_fourcc_looks_like_scene(block->header.id) &&
                    (m = get_scene_table_meta(block->header.id))) {
                    bfbb_save_file_read_scene(save_file, &br, block, m->table, m->count);
                } else {
                    char* chars = block->header.id_chars;
                    printf("Unknown block: %c%c%c%c", chars[3], chars[2], chars[1], chars[0]);
                    assert(0);
                }
            }
        }
    }
    // TODO(jelly): do more checks??
    return 1;
}

int bfbb_save_file_read_file_type(bfbb_save_file *result, unsigned char *bytes, int size, int is_gci) {
    buffer b = {size, bytes};
    bfbb_save_file_block gdat;
    int rc;
    int i = 0;
    
    result->is_big_endian = is_gci;
    
    if (is_gci) {
        eat_bytes(&b, 0x5880); // NOTE(jelly): gci file header
        
        eat_bytes(&b, 512); // NOTE(jelly): zeros
        eat_bytes(&b, sizeof(BFBB_SAVE_FILE_GCI_MAGIC_STRING) - 1);
        eat_bytes(&b, 0x599); // NOTE(jelly): zeros
        
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
    base_type bt = block->scene.bases[n];
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
        case BASE_TYPE_UIFONT:
        {
            bit_push(b, bt.uifont.base_enable, 1);
            bit_push(b, bt.uifont.show_ent, 1);
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

void bfbb_save_file_write_scene(write_buffer *b, bit_writer *bw, bfbb_save_file_block *block, scene_table_entry *table, s32 table_count) {
    bfbb_save_file_write_scene_block_stuff(bw, block); // TODO(jelly): name this better for godsake
    for (int i =0; i < block->scene.base_count; i++) {
        if (i < table_count) bfbb_save_file_write_scene_block(bw, &table[i], i, block);
        else {
            // TODO(jelly): diagnostic?
            assert(0);
        }
    }
    unsigned char *data = b->bytes + b->size;
    int n = block->header.bytes_used;
    b->size += n;
}

// NOTE(jelly): the `append` functions append to a `write_buffer` - they do NOT append a block to a save file.

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
#ifdef BFBBMIX
            case(FOURCC_EX01):
            case(FOURCC_EX02):
            case(FOURCC_EX03):
            case(FOURCC_EX04):
            case(FOURCC_EX05):
            case(FOURCC_EX06):
            case(FOURCC_EX07):
            case(FOURCC_EX08):
            case(FOURCC_EX09):
            case(FOURCC_PG02):
            case(FOURCC_PG13): write_bytes(b, block->raw_bytes, size_to_write); break;
#endif
            default: {
                if (bfbb_save_file_fourcc_looks_like_scene(block->header.id)) {
                    const scene_table_meta *m = get_scene_table_meta(block->header.id);
                    if (m) {
                        bfbb_save_file_write_scene(b, &bw, block, m->table, m->count);
                    } else {
                        char* chars = block->header.id_chars;
                        printf("Unknown block: %c%c%c%c", chars[3], chars[2], chars[1], chars[0]);
                        assert(0);
                    }
                } else {
                    // TODO(jelly): check to make sure it's a block that we know about?
                    write_bytes(b, block->raw_bytes, size_to_write);
                }
            }
        }
    }
    
    bfbb_save_file_append_padding(b, padding_size);
    return result;
}

void bfbb_save_file_append_sfil(bfbb_save_file *save_file, write_buffer *b, int is_gci) {
    int size_of_data = 0;
    for(int i = 0; i<save_file->block_count; i++)
    {
        size_of_data+=save_file->blocks[i].header.block_size;
    }
    //size_of_data = b->size;
    //TODO(Will): Figure out how to actually fucking do this :)
    u32 sfil_size = 0xc808 - size_of_data + 1036 + 8;
    assert(b->size + sfil_size < b->max_size);
    if (is_gci) sfil_size -= 0x6040; // NOTE(jelly): size of entire gci header
    u32 sfil_bytes_used = 8;
    
    unsigned char sfil_id[] = {
        0x4C, 0x49, 0x46, 0x53
    };
    write_bytes(b, sfil_id, sizeof(sfil_id));
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
        write_bytes(&b, (unsigned char *)BFBB_SAVE_FILE_GCI_MAGIC_STRING, sizeof(BFBB_SAVE_FILE_GCI_MAGIC_STRING)-1);
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
    if (file_type >= 0) return bfbb_save_file_read_file_type(result, data, len, file_type);
    return 0;
}

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

typedef struct {
    char chars[8];
} small_string;

small_string stringifiy_fourcc(u32 id) {
    small_string result = {0};
    result.chars[0] = (id >> 24) & 0xff;
    result.chars[1] = (id >> 16) & 0xff;
    result.chars[2] = (id >>  8) & 0xff;
    result.chars[3] = (id >>  0) & 0xff;
    return result;
}