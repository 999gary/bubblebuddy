#include "types.h"
#include "byteswap.h"
#include "config.h"
#include "bit_methods.h"

#define ArrayCount(arr) (sizeof(arr)/sizeof((arr)[0]))

#ifdef _WIN32
#include "win32_d3d9_include.h"
#elif __linux__ 
#include "sdlinclude.h"
#else
#error Platform not supported!
#endif

#define HIPHOP_IMPLEMENTATION
#include "hiphop.c"
#include "bfbb_save.c"

static int window_width, window_height;

typedef struct nk_context nk_context;

typedef struct {
    nk_context *nk_ctx; //nuklear context
    int running;
    int screen;
    int s1_adv;
    char s1_fpath[4096];
    bfbb_save_file save_file;
    hh* hiphop;
    int save_file_is_loaded;
} hit_main;

void hit_common_init(hit_main *cv);
void hit_update_and_render(hit_main *cv);

#ifdef _WIN32
#include "win32_d3d9_renderer.c"
#elif __linux__
#include "sdlrender.c"
#endif

/*-------------------------------------------

Util

-------------------------------------------*/

unsigned char *read_entire_file(char *path, int *size) {
    int n; 
    unsigned char *result = 0;
    FILE *in = fopen(path, "rb");
    assert(size);
    if (in) {
        fseek(in, 0, SEEK_END);
        n = ftell(in);
        rewind(in);
        result = malloc(n);
        fread(result, 1, n, in);
        fclose(in);
        *size = n;
    }
    return result;
}

/*-------------------------------------------

Global Components

-------------------------------------------*/

void hit_top_panel(hit_main *cv)
{
    float win_height = window_height/20;
    if(nk_begin(cv->nk_ctx, "Tpab Panel", nk_rect(0, 0, window_width, win_height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ))
    {
        nk_layout_row_dynamic(cv->nk_ctx, win_height*.7, 6);
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
        if(nk_button_label(cv->nk_ctx, "HIPHOPTOOL"))
        {
            cv->screen = 1;
        }
#endif
        if(nk_button_label(cv->nk_ctx, "SAVEEDIT"))
        {
            cv->screen = 0;
        }
    }
    nk_end(cv->nk_ctx);
}

/*-------------------------------------------

Screen 0 Components

-------------------------------------------*/

void hit_s0_data(hit_main *cv)
{
    float win_height_offset = window_height/20;
    float win_height = window_height - window_height/20;
    if(nk_begin(cv->nk_ctx, "Data Panel", nk_rect(0, win_height_offset, window_width, win_height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(cv->nk_ctx, win_height/20, 3);
        nk_label(cv->nk_ctx, cv->hiphop->hipa.block.block_id, NK_TEXT_ALIGN_LEFT);
    }
    nk_end(cv->nk_ctx);
}

/*-------------------------------------------

Screen 1 Components

-------------------------------------------*/

char nibble_to_hex_char(unsigned char nibble) {
    if (nibble < 10) return nibble + '0';
    else if (nibble < 16) return nibble + 'A' - 10;
    return 0;
}

unsigned char hex_char_to_nibble(char hex_char) {
    if (hex_char >= '0' && hex_char <= '9') return hex_char - '0';
    else if (hex_char >= 'A' && hex_char <= 'F') return hex_char - 'A' + 10;
    else if (hex_char >= 'a' && hex_char <= 'f') return hex_char - 'a' + 10;
    return 0;
}

// NOTE(jelly): rip hex editor
#ifndef HEX_EDITORS_SUCK_DONT_USE_THEM

void add_byte_to_hex_string(unsigned char byte, int block_index, int* counter) {
    bytes_in_hex[block_index][*counter] = nibble_to_hex_char(byte >> 4);
    bytes_in_hex[block_index][*counter+1] = nibble_to_hex_char(byte & 0xf);
    *counter+=3;
}

#endif

void nk_menu_begin_labelf(struct nk_context *ctx, nk_flags align, struct nk_vec2 size, const char *fmt, ...) {
    static char buffer[2048];
    
    va_list args;
    va_start(args, fmt);
    int result = vsnprintf(buffer, 2048, fmt, args);
    va_end(args);
    
    nk_menu_begin_label(ctx, buffer, align, size);
}

char* thumbnail_label_from_id(int32_t id)
{
    char* lookup[14] = {
        "Bikini Bottom",
        "Jellyfish Fields",
        "Downtown Bikini Bottom",
        "Goo Lagoon",
        "Poseidome",
        "Rock Bottom",
        "Mermalair",
        "Sand Mountain",
        "Industrial Park",
        "Kelp Forest",
        "Flying Dutchman's Graveyard",
        "SpongeBob's Dream",
        "Chum Bucket Lab",
        "Bikini Bottom"
    };
    if (id < 0) id = 0;
    if (id > 13) id = 13;
    return lookup[id];
}

u32 drawable_block_ids[] = {
    FOURCC_PLYR, FOURCC_LEDR, FOURCC_ROOM, FOURCC_HB02, FOURCC_CNTR
};

int is_drawable_block(bfbb_save_file_block *block) {
    u32 id = block->header.id;
    for (int i = 0; i < ArrayCount(drawable_block_ids); i++) {
        if (id == drawable_block_ids[i]) return 1;
    }
    return 0;
}
void hit_s1_data(hit_main *cv)
{
    if(!cv->save_file_is_loaded)
        return;
    float win_height_offset = window_height/20;
    float win_height = window_height - (window_height/20*2);
    
    bfbb_save_file *save_file = &cv->save_file;
    bfbb_save_file_block *blocks = save_file->blocks;
    int block_count = save_file->block_count;
    
    static int is_hex_string_initialized = 0;
    
#ifndef HEX_EDITORS_SUCK_DONT_USE_THEM
    if (!is_hex_string_initialized) {
        for (int block_index = 0; block_index < block_count; block_index++) {
            int at = 0;
            bfbb_save_file_block *block = &blocks[block_index];
            for (int i = 0; i < block->header.bytes_used; i++) {
                add_byte_to_hex_string(block->raw_bytes[i], block_index, &at);
            }
        }
        is_hex_string_initialized = 1;
    }
#endif
    
#if 0
    {
        //NOTE(jelly): testing code
        int scene_count = 0;
        int max_base_type_count = 0;
        for (int i = 0; i < save_file->block_count; i++) {
            bfbb_save_file_block *block = &save_file->blocks[i];
            if (bfbb_save_file_block_is_scene(block)) {
                int n = arrlen(block->scene.base);
                if (n > max_base_type_count) max_base_type_count = n;
                scene_count++;
            }
        }
        printf("max base type count = %d\n", max_base_type_count);
        printf("scene count = %d\n", scene_count);
        //-------------------------
    }
#endif
    
    if(nk_begin(cv->nk_ctx, "Data Panel", nk_rect(0, win_height_offset, window_width, win_height), NK_WINDOW_BORDER))
    {
        int kj = 0;
        for(int i = 0; i < block_count; i++)
        {
            if(!is_drawable_block(&save_file->blocks[i]))
                continue;
            
            if(!(kj%3))
                nk_layout_row_dynamic(cv->nk_ctx, win_height/3, 3);
            kj++;
            char title[16] = {0};
            for (int j = 0; j < 4; j++) {
                title[j] = blocks[i].header.id_chars[3-j];
            }
            
            if(nk_group_begin_titled(cv->nk_ctx, title, title, NK_WINDOW_BORDER | NK_WINDOW_TITLE))
            {
                switch(blocks[i].header.id)
                {
                    case(FOURCC_LEDR):
                    { 
                        if(cv->s1_adv)
                        {
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 2);
                            nk_label(cv->nk_ctx, "Game Label:", NK_TEXT_ALIGN_CENTERED);
                            nk_edit_string_zero_terminated(cv->nk_ctx, NK_EDIT_FIELD, blocks[i].ledr.game_label, 64, (nk_plugin_filter)NK_FILTER_INT);
                            /*
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 2);
                            nk_label(cv->nk_ctx, "Thumbnail ID:", NK_TEXT_ALIGN_CENTERED);
                            nk_property_int(cv->nk_ctx, thumbnail_label_from_id(blocks[i].ledr.thumbnail_index), 0, &blocks[i].ledr.thumbnail_index, 13, 1, 1);
                            */
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_property_int(cv->nk_ctx, "Progress", 0, &blocks[i].ledr.progress, 100, 1, .5);
                            /*
                            if(nk_menu_begin_label(cv->nk_ctx, thumbnail_label_from_id(blocks[i].ledr.thumbnail_index), NK_TEXT_ALIGN_CENTERED, nk_vec2(window_width/3-20, win_height/3/2)))
                            {
                                for(int i = 0; i<14; i++)
                                {
                                    nk_layout_row_dynamic(cv->nk_ctx, win_height/20, 1);
                                    if(nk_menu_item_label(cv->nk_ctx, thumbnail_label_from_id(i), NK_TEXT_ALIGN_LEFT))
                                    {
                                        blocks[i].ledr.thumbnail_index = i;
                                    }
                                }
                                nk_menu_end(cv->nk_ctx);
                            }
                            */
                            break;  
                        }
                    }
                    case(FOURCC_ROOM):
                    {
                        if(cv->s1_adv)
                        {
                            //TODO(Will): Make this a selection box
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 2);
                            nk_label(cv->nk_ctx, "Room:", NK_TEXT_ALIGN_CENTERED);
                            int len = 5;
                            nk_edit_string(cv->nk_ctx, NK_EDIT_FIELD, (char*)&blocks[i].room.sceneid, &len, 5, (nk_plugin_filter)NK_FILTER_INT);
                            break;
                        }
                    }
                    case(FOURCC_PLYR):
                    {
                        if(cv->s1_adv)
                        {
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_property_int(cv->nk_ctx, "Max Health", 0, &blocks[i].plyr.max_health, 6, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_property_int(cv->nk_ctx, "Character", 0, &blocks[i].plyr.character, 2, 1, 5);
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_property_int(cv->nk_ctx, "Shinies", 0, &blocks[i].plyr.shinies, 99999, 100, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_property_int(cv->nk_ctx, "Spats", 0, &blocks[i].plyr.spats, 100, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_bool a, b;
                            a = !blocks[i].plyr.has_bubble_bowl;
                            b = !blocks[i].plyr.has_cruise_bubble;
                            nk_checkbox_label(cv->nk_ctx, "BB Unlocked", &a);
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                            nk_checkbox_label(cv->nk_ctx, "CB Unlocked", &b);
                            
                            blocks[i].plyr.has_bubble_bowl = !a;
                            blocks[i].plyr.has_cruise_bubble = !b;
                            break;
                        }
                    }
                    case(FOURCC_CNTR):
                    {
                        nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                        nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_CENTERED, "0 = Spat not found");
                        nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                        nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_CENTERED, "1 = Spat found");
                        nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                        nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_CENTERED, "2 = Spat Collected");
                        for(int k = 0; k<15; k++)
                        {
                            nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 8);
                            for(int j = 0; j<spat_count_per_world[k]; j++)
                            {
                                s16* spat = &blocks[i].cntr.spats[k][j];
                                char yeah[2];
                                yeah[0] = nibble_to_hex_char(*spat & 0xf);
                                yeah[1] = '\0';
                                if(nk_button_label(cv->nk_ctx, yeah))
                                {
                                    if(*spat == 2)
                                    {
                                        *spat = 0;
                                    }
                                    else
                                    {
                                        *spat += 1;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case(FOURCC_HB02):
                    {
                        for(int j = 0; j<arrlen(blocks[i].scene.base); j++)
                        {
                            base_type* b = &blocks[i].scene.base[j];
                            switch(b->type)
                            {
                                case(BASE_TYPE_TRIGGER):
                                {
                                    nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 2);
                                    nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_CENTERED, "ID: %x", b->id);
                                    nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_CENTERED, "Type: %x", b->type);   
                                    nk_bool a, c;
                                    a = b->trigger.base_enable;
                                    c = b->trigger.show_ent;
                                    nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                                    nk_checkbox_label(cv->nk_ctx, "Enabled", &a);
                                    nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 1);
                                    nk_checkbox_label(cv->nk_ctx, "Shown", &c);
                                    
                                    b->trigger.base_enable = a;
                                    b->trigger.show_ent = c;
                                    
                                }
                            }
                        }
                        break;
                    }
#ifndef HEX_EDITORS_SUCK_DONT_USE_THEM
                    default:
                    {
                        for(int b = 0; b < blocks[i].header.bytes_used; b++)
                        {
                            int len = 2;
                            if(!(b%8))
                                nk_layout_row_dynamic(cv->nk_ctx, win_height/3/8, 8);
                            
                            // TODO(jelly): nk_plugin_filter restricts what the user can type - we only want hex, 
                            //              but that doesn't seem to be offered by nuklear? only float and int?
                            nk_edit_string_zero_terminated(cv->nk_ctx, NK_EDIT_FIELD, &bytes_in_hex[i][b*3], 3, (nk_plugin_filter)0);
                            
                            //cv->save_file->blocks[i].raw_bytes[b] = hex_string_to_byte(&bytes_in_hex + counter); // ?????
                        }
                        break;
                    }
#endif
                    
                }
                nk_group_end(cv->nk_ctx);
            }
            
        }
    }
    nk_end(cv->nk_ctx);
}

void hit_s1_bottom_panel(hit_main *cv)
{
    float win_height = window_height/20;
    if(nk_begin(cv->nk_ctx, "Bottom Panel", nk_rect(0, window_height-win_height, window_width, win_height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ))
    {
        nk_layout_row_dynamic(cv->nk_ctx, win_height*.7, 3);
        if(nk_button_label(cv->nk_ctx, "Save File"))
        {
#ifndef HEX_EDITORS_SUCK_DONT_USE_THEM
            for(int i = 0; i<cv->save_file.block_count; i++)
            {
                if(cv->s1_adv && cv->save_file.blocks[i].header.id == FOURCC_LEDR)
                    continue;
                for(int y = 0; y<cv->save_file.blocks[i].header.bytes_used; y++)
                {
                    cv->save_file.blocks[i].raw_bytes[y] = hex_string_to_byte(&bytes_in_hex[i][y*3]);
                }
            }
#endif
            int save_as_gci = -1;
            int extension_supplied = -1;
            static char path_buffer[4096];
            int save_success = 0;
            if (!hit_file_select_write(path_buffer, sizeof(path_buffer), &save_as_gci, &extension_supplied)) {
                assert(save_as_gci == 0 || save_as_gci == 1);
                if (!extension_supplied) {
                    char *exts[] = { ".xsv", ".gci" };
                    char *ext = exts[save_as_gci];
                    // NOTE(jelly): i hate C
                    strncat(path_buffer, ext, sizeof(path_buffer) - strlen(path_buffer) - 1);
                }
                if (bfbb_save_file_write_out(&cv->save_file, path_buffer, save_as_gci)) {
                    save_success = 1;
                }
            }
            if (!save_success) {
                hit_message_box_ok("Save Failed", "Failed to Save the File :(. Sorry.");
            }
        }
        cv->s1_adv = 1;
        nk_label(cv->nk_ctx, cv->s1_fpath, NK_TEXT_ALIGN_CENTERED);
    }
    nk_end(cv->nk_ctx);
}


/*-------------------------------------------

Screens

Home = 0;

-------------------------------------------*/

void hit_screen_hiphop(hit_main *cv)
{
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
    hit_s0_data(cv);
    hit_top_panel(cv);
#endif
}

void hit_screen_save(hit_main *cv)
{
    hit_s1_data(cv);
    hit_s1_bottom_panel(cv);
    hit_top_panel(cv);
}


void hit_update_and_render(hit_main *cv)
{
    switch(cv->screen)
    {
        case(1):
        {
            hit_screen_hiphop(cv);
            break;
        }
        default:
        {
            hit_screen_save(cv);
            break;
        }
    }
}

void hit_common_init(hit_main *cv)
{
    char buffer[PATH_BUFFER_SIZE];
    if(!hit_file_select_read(buffer, PATH_BUFFER_SIZE))
    {
        FILE *in = fopen(buffer, "rb");
        if (in) {
            int size = 0;
            unsigned char *data = read_entire_file(buffer, &size);
            bfbb_save_file save_file;
            if (!bfbb_save_file_read(&cv->save_file, data, size)) {
                // TODO(jelly): the file couldn't be parsed properly: TELL THE USER OR SOMETHING !!!
            }
            cv->save_file_is_loaded = 1;
        }
        memcpy(cv->s1_fpath, buffer, 4096);
    }
    
    //TODO(Will): Make this all one function
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
    hh h = {0};
    cv->hiphop = &h;
    hh_read_file_from_disk(cv->hiphop, "gl01.HIP");
#endif
}
