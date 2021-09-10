
/*

TODO(jelly): STATE OF THE PROGRAM
--------------------------------------
-THE GUI FUCKING SUCKS, FUCKING FIX IT - not fixed
-PLYR block is still broken!
-SFIL block is the wrong size
-Add a load file button - fixed (almost)
-Stop clamping things please lemme finish the game at like 100000% - fixed (mostly outside of things that will break)
-Port to linux
-Port to emscripten
-just generally clean up the code it's pretty garbage

*/

#include "main.h"
#define HIPHOP_IMPLEMENTATION
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
#include "hiphop.c"
#endif
#include "bfbb_save.c"

static u32 window_width, window_height;

#ifdef _WIN32
#include "win32_d3d9_renderer.c"
#elif __linux__
#include "sdlrender.c"
#elif EMSCRIPTEN
#include "sdlrender.c"
#endif

/*-------------------------------------------

Util

-------------------------------------------*/

int is_uppercase_letter(u8 c) {
    return c >= 'A' && c <= 'Z';
}

int is_digit(u8 c) {
    return c >= '0' && c <= '9';
}

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

void hit_message_box_ok_fmt(char *caption, char *message_fmt, ...) {
    va_list args;
    static char buffer[4096]; 
    va_start(args, message_fmt);
    vsnprintf(buffer, sizeof(buffer), message_fmt, args);
    va_end(args);
    hit_message_box_ok(caption, buffer);
}

void hit_load_save(hit_main *cv)
{
    bfbb_save_file_free_blocks(&cv->save_file);
    char buffer[4096] = {0};
    if(!hit_file_select_read(buffer, sizeof(buffer)))
    {
        FILE *in = fopen(buffer, "rb");
        if (in) {
            int size = 0;
            unsigned char *data = read_entire_file(buffer, &size);
            bfbb_save_file save_file;
            if (bfbb_save_file_read(&cv->save_file, data, size)) {
                cv->save_file_is_loaded = 1;
                memcpy(cv->s1_fpath, buffer, sizeof(buffer));
            } else {
                hit_message_box_ok_fmt("Failed to Load Save File", 
                                       "'%s' doesn't appear to be a valid"
                                       " BFBB save file in a format that we support. Sorry. :(", buffer);
            }
        } else {
            hit_message_box_ok_fmt("Failed to Load Save File", "Couldn't open file '%s'. Sorry :(", buffer);
        }
    }
}

/*-------------------------------------------

Global Components

-------------------------------------------*/

void hit_top_panel(hit_main *cv)
{
    float win_height = window_height/20.0f;
    if(nk_begin(cv->nk_ctx, "Tab Panel", nk_rect(0, 0, window_width, win_height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ))
    {
        nk_layout_row_dynamic(cv->nk_ctx, win_height*.7f, 6);
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
        if(nk_button_label(cv->nk_ctx, "HIPHOPTOOL"))
        {
            cv->screen = 1;
        }
#endif
        if(nk_menu_begin_text(cv->nk_ctx, "Save Edit Menu", 14, NK_TEXT_ALIGN_CENTERED, nk_vec2(window_width/3.0f, (window_height/20.0f)*(cv->save_file.block_count))))
        {
            nk_layout_row_dynamic(cv->nk_ctx, window_height/40.0f, 3);
            if(nk_menu_item_label(cv->nk_ctx, "General", NK_TEXT_ALIGN_CENTERED))
            {
                cv->s1_scene_id = 0;
            }
            int kj = 1;
            for(int i = 0; i<cv->save_file.block_count; i++)
            {
                bfbb_save_file_block b = cv->save_file.blocks[i];
                if(bfbb_save_file_fourcc_is_scene(b.header.id))
                {
                    if(!(kj%3))
                    {    
                        nk_layout_row_dynamic(cv->nk_ctx, window_height/40.0f, 3);
                    }
                    small_string label = stringifiy_fourcc(b.header.id);
                    if(nk_menu_item_label(cv->nk_ctx, label.chars, NK_TEXT_ALIGN_CENTERED))
                    {
                        cv->s1_scene_id = b.header.id;
                    }
                }
            }
            nk_menu_end(cv->nk_ctx);
        }
    }
    nk_end(cv->nk_ctx);
}

/*-------------------------------------------

Screen 0 Components

-------------------------------------------*/

#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE

void hit_s0_data(hit_main *cv)
{
    float win_height_offset = window_height/20.0f;
    float win_height = window_height - window_height/20.0f;
    if(nk_begin(cv->nk_ctx, "Data Panel", nk_rect(0, win_height_offset, window_width, win_height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(cv->nk_ctx, win_height/20.0f, 3);
        nk_label(cv->nk_ctx, cv->hiphop->hipa.block.block_id, NK_TEXT_ALIGN_LEFT);
    }
    nk_end(cv->nk_ctx);
}

#endif

/*-------------------------------------------

Screen 1 Components

-------------------------------------------*/

void nk_menu_begin_labelf(struct nk_context *ctx, nk_flags align, struct nk_vec2 size, const char *fmt, ...) {
    static char buffer[2048];
    
    va_list args;
    va_start(args, fmt);
    int result = vsnprintf(buffer, 2048, fmt, args);
    va_end(args);
    
    nk_menu_begin_label(ctx, buffer, align, size);
}

void nk_checkbox_label_u8(nk_context* ctx, const char * label, u8* value)
{
    nk_bool a = !*value;
    nk_checkbox_label(ctx, label, &a);
    *value = !a;
}

void nk_base_type_begin(nk_context *ctx, float row_height, u32 id, u32 type) {
    nk_layout_row_dynamic(ctx, row_height, 2);
    nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "ID: %x", id);
    nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Type: %s", get_base_type_name(type));
}

void nk_base_enabled_and_shown(nk_context *ctx, float row_height, u8 *enabled, u8 *shown) {
    nk_layout_row_dynamic(ctx, row_height, 2);
    nk_checkbox_label_u8(ctx, "Enabled", enabled);
    nk_checkbox_label_u8(ctx, "Shown", shown);
}

void hit_s1_scene_switch(hit_main *cv, bfbb_save_file_block* block, int j, float win_height)
{
    //float row_height = win_height/3/8;
    float row_height = win_height*(1.0f/24.0f);
    
    base_type* b = &block->scene.base[j];
    nk_base_type_begin(cv->nk_ctx, row_height, b->id, b->type);
    switch(b->type)
    {
        case(BASE_TYPE_TRIGGER):
        {
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->trigger.base_enable, &b->trigger.show_ent);
            break;         
        }
        case(BASE_TYPE_PICKUP):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
            u8 flag[7];
            //printf("%x\n", b->pickup.state);
            u8 state = b->pickup.state;
            for(int i = 0; i<7; i++)
            {
                flag[i] = state & 1;
                state >>= 1;
            }
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->pickup.base_enable, &b->pickup.show_ent);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 4);
            char buffer[128];
            for(int i = 0; i<7; i++)
            {
                sprintf(buffer, "Flag #%d", i+1);
                nk_checkbox_label_u8(cv->nk_ctx, buffer, &flag[i]);
                memset(buffer, 0, 128);
            }
            state = 0;
            for (int i = 7; i>=0; i--)
            {
                state |= flag[i] << i;
            }
            b->pickup.state = state;
            nk_checkbox_label_u8(cv->nk_ctx, "Collected", &b->pickup.collected);
            break;
        }
        case(BASE_TYPE_PLATFORM):
        {
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->platform.base_enable, &b->platform.show_ent);
            break;
        }
        case(BASE_TYPE_STATIC):
        {
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->staticc.base_enable, &b->staticc.show_ent);
            break;         
        }
        case(BASE_TYPE_TIMER):
        {
            s32 state = b->timer.state;
            f32 sl = b->timer.seconds_left;
            
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->timer.base_enable);
            nk_property_int(cv->nk_ctx, "#State", 0, &state, 255, 1, 1);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_property_float(cv->nk_ctx, "#Seconds Left", 0.0f, &sl, 500.0f, .5f, .5f);
            
            b->timer.seconds_left = sl;
            b->timer.state = state;
            break;
        }
        case(BASE_TYPE_GROUP):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->group.base_enable);
            break;
        }
        case(BASE_TYPE_SFX):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->sfx.base_enable);
            break;
        }
        case(BASE_TYPE_COUNTER):
        {
            s32 counter = b->counter.count;
            //printf("%x\n", b->pickup.state);
            s32 state = (s32)b->counter.state;
            
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->counter.base_enable);
            nk_property_int(cv->nk_ctx, "#State", 0, &state, 6, 1, 1);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_property_int(cv->nk_ctx, "#Counter", INT16_MIN, &counter, INT16_MAX, 1, 1);
            
            
            b->counter.count = counter;
            b->counter.state = (u8)state;
            break;
        }
        case(BASE_TYPE_BUTTON):
        {
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->button.base_enable, &b->button.show_ent);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
            nk_checkbox_label_u8(cv->nk_ctx, "Unknown bit #1", &b->button.unknown_bit1);
            nk_checkbox_label_u8(cv->nk_ctx, "Unknown bit #2", &b->button.unknown_bit2);
            break;
        }
        case(BASE_TYPE_DISPATCHER):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->dispatcher.base_enable);
            break;
        }
        case(BASE_TYPE_COND):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->cond.base_enable);
            break;
        }
        case(BASE_TYPE_TASKBOX):
        {
            s32 state = b->taskbox.state;
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_property_int(cv->nk_ctx, "#State", 0, &state, 6, 1, 1);
            b->taskbox.state = state;
            break;
        }
        case(BASE_TYPE_CUTSCENEMGR):
        {
            break;
        }
        case(BASE_TYPE_TELEPORTBOX):
        {
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->tpbox.base_enable, &b->tpbox.show_ent);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_checkbox_label_u8(cv->nk_ctx, "Opened", &b->tpbox.opened);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_property_int(cv->nk_ctx, "#Player State", INT_MIN, &b->tpbox.player_state, INT_MAX, 1, 1);
            break;
        }
        case(BASE_TYPE_TAXI):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->taxi.base_enable);
            break;
        }
        case(BASE_TYPE_CAMERAFLY):
        {
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
            nk_checkbox_label_u8(cv->nk_ctx, "Enabled", &b->camfly.base_enable);
            break;
        }
        default:
        {
            printf("Unknown base type %d\n", b->type);
            assert(0);
        }
    }
}

void hit_s1_scene_screen(hit_main* cv, scene_table_entry* table, char* id, float win_height)
{
    int kj = 0;
    bfbb_save_file_block *b = bfbb_save_file_find_block(&cv->save_file, id);
    for(int i = 0; i<arrlen(b->scene.base); i++)
    {
        base_type *bt = &b->scene.base[i];
        if(!(kj%3))
            nk_layout_row_dynamic(cv->nk_ctx, win_height*(1.0f/3.0f), 3);
        kj++;
        if(nk_group_begin_titled(cv->nk_ctx, table[i].name, table[i].name, NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            hit_s1_scene_switch(cv, b, i, win_height);
            nk_group_end(cv->nk_ctx);
        }
    }
}

void hit_s1_spats_set_all_to(nk_context *ctx, float row_height, bfbb_save_file_block_cntr *cntr, s32 v, char *label) {
    nk_layout_row_dynamic(ctx, row_height, 2);
    if(nk_button_label(ctx, "Set All To"))
    {
        for(int k = 0; k<15; k++)
        {
            for(int j = 0; j<spat_count_per_world[k]; j++)
            {
                cntr->spats[k][j] = v;
            }
        }
    }
    nk_labelf(ctx, NK_TEXT_ALIGN_CENTERED, label);
}

void hit_s1_data(hit_main *cv)
{
    if(!cv->save_file_is_loaded)
        return;
    float win_height_offset = window_height/20.0f;
    float win_height = window_height - (window_height/20.0f*2.0f);
    
    bfbb_save_file *save_file = &cv->save_file;
    bfbb_save_file_block *blocks = save_file->blocks;
    int block_count = save_file->block_count;
    
    static int is_hex_string_initialized = 0;
    
    if(nk_begin(cv->nk_ctx, "Data Panel", nk_rect(0, win_height_offset, window_width, win_height), NK_WINDOW_BORDER))
    {
        
        if (bfbb_save_file_fourcc_looks_like_scene(cv->s1_scene_id)) {
            const scene_table_meta *m = get_scene_table_meta(cv->s1_scene_id);
            if (m) {
                small_string name = stringifiy_fourcc(cv->s1_scene_id);
                hit_s1_scene_screen(cv, m->table, name.chars, win_height);
            } else {
                // TODO(jelly): diagnostic?
                assert(0);
            }
        } else {
            int kj = 0;
            for(int i = 0; i < block_count; i++)
            {
                bfbb_save_file_block *block = &blocks[i];
                if(!is_drawable_block(block))
                    continue;
                if(kj == 0)
                    nk_layout_row_dynamic(cv->nk_ctx, win_height/4.0f, 3);
                else if(!(kj%3))
                    nk_layout_row_dynamic(cv->nk_ctx, win_height/1.5f, 2);
                kj++;
                char title[16] = {0};
                for (int j = 0; j < 4; j++) {
                    title[j] = block->header.id_chars[3-j];
                }
                
                float row_height = win_height/3.0f/8.0f;
                
                if(nk_group_begin_titled(cv->nk_ctx, title, title, NK_WINDOW_BORDER | NK_WINDOW_TITLE))
                {
                    switch(blocks[i].header.id)
                    {
                        case(FOURCC_LEDR):
                        { 
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                            nk_label(cv->nk_ctx, "Game Label:", NK_TEXT_ALIGN_CENTERED);
                            nk_edit_string_zero_terminated(cv->nk_ctx, NK_EDIT_FIELD, blocks[i].ledr.game_label, 64, (nk_plugin_filter)NK_FILTER_INT);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                            nk_label(cv->nk_ctx, thumbnail_label_from_id(block->ledr.thumbnail_index), NK_TEXT_ALIGN_CENTERED);
                            nk_property_int(cv->nk_ctx, "ID:", 0, &block->ledr.thumbnail_index, 13, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "Progress", 0, &block->ledr.progress, 100, 1, .5);
                            break;  
                        }
                        case(FOURCC_ROOM):
                        {
                            //TODO(Will): Make this a selection box
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                            nk_label(cv->nk_ctx, "Room:", NK_TEXT_ALIGN_CENTERED);
                            nk_edit_string_zero_terminated(cv->nk_ctx, NK_EDIT_FIELD, block->room.sceneid, 5, (nk_plugin_filter)NK_FILTER_INT);
                            break;
                        }
                        case(FOURCC_PREF):
                        {
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "Sound Mode", 0, (s32*)&block->pref.sound_mode, INT_MAX, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_float(cv->nk_ctx, "Music Volume", 0.0f, &block->pref.music_volume, 5000.0f, .5f, .5f);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_float(cv->nk_ctx, "SFX Volume", 0.0f, &block->pref.sfx_volume, 5000.0f, .5f, .5f);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_bool a = !block->pref.rumble;
                            nk_checkbox_label(cv->nk_ctx, "Rumble", &a);
                            block->pref.rumble = !a;
                            break;
                        }
                        case(FOURCC_PLYR):
                        {
                            
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "#Max Health", 0, &block->plyr.max_health, 6, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "#Character", 0, &block->plyr.character, 2, 1, 5);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "#Shinies", INT_MIN, &block->plyr.shinies, INT_MAX, 100, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "#Spats", 0, &block->plyr.spats, 100, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_checkbox_label_u8(cv->nk_ctx, "BB Unlocked", &block->plyr.has_bubble_bowl);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_checkbox_label_u8(cv->nk_ctx, "CB Unlocked", &block->plyr.has_cruise_bubble);
                            for(int k = 0; k<LEVEL_COUNT - 2; k++)
                            {
                                nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                                nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_LEFT, "%s:", thumbnail_label_from_id(k));
                                nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                                nk_property_int(cv->nk_ctx, "#Socks", INT_MIN, &block->plyr.level_collectables[k].socks, INT_MAX, 1, 1);
                                nk_property_int(cv->nk_ctx, "#Pickups", INT_MIN, &block->plyr.level_collectables[k].pickups, INT_MAX, 1, 1);
                            }
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "#Total Socks", INT_MIN, &block->plyr.total_socks, INT_MAX, 1, 1);nk_layout_row_dynamic(cv->nk_ctx, row_height, 4);
                            char buffer[12];
                            for(int k = 0; k<14; k++)
                            {
                                memset(buffer, 0, 12);
                                sprintf(buffer, "Cutscene %d", k+1);
                                nk_checkbox_label_u8(cv->nk_ctx, buffer, &block->plyr.cutscene_played[k]);
                            }
                            break;
                        }
                        case(FOURCC_CNTR):
                        {
                            hit_s1_spats_set_all_to(cv->nk_ctx, row_height, &block->cntr, 0, "0 = Spat not found");
                            hit_s1_spats_set_all_to(cv->nk_ctx, row_height, &block->cntr, 1, "1 = Spat found");
                            hit_s1_spats_set_all_to(cv->nk_ctx, row_height, &block->cntr, 2, "2 = Spat Collected");
                            for(int k = 0; k<15; k++)
                            {
                                nk_layout_row_begin(cv->nk_ctx, NK_DYNAMIC, row_height, 20);
                                nk_layout_row_push(cv->nk_ctx, .41f);
                                nk_label(cv->nk_ctx, thumbnail_label_from_id(k), NK_TEXT_ALIGN_RIGHT);
                                nk_layout_row_push(cv->nk_ctx, .080f);
                                for(int j = 0; j<spat_count_per_world[k]; j++)
                                {
                                    s16* spat = &block->cntr.spats[k][j];
                                    s16 s = *spat;
                                    Clamp(s, 0, 9);
                                    char spat_str[8] = {0};
                                    spat_str[0] = s + '0';
                                    if(nk_button_label(cv->nk_ctx, spat_str))
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
                            /*
                            for(int k = 0; k<15; k++)
                            {
                                // TODO(jelly): make this editable
                                nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                                nk_labelf(cv->nk_ctx, NK_TEXT_ALIGN_LEFT, "%x", block->cntr.robot_data[k]);
                            }
                        */
                            break;
                        }
                    }
                    nk_group_end(cv->nk_ctx);
                }
            }
            
        }
        nk_end(cv->nk_ctx);
    }
}

void hit_s1_bottom_panel(hit_main *cv)
{
    float win_height = window_height/20.0f;
    if(nk_begin(cv->nk_ctx, "Bottom Panel", nk_rect(0, window_height-win_height, window_width, win_height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR ))
    {
        nk_layout_row_dynamic(cv->nk_ctx, win_height*.7f, 3);
        if(nk_button_label(cv->nk_ctx, "Save File"))
        {
            int save_as_gci = -1;
            int extension_supplied = 0;
            char path_buffer[4096] = {0}; // NOTE(jelly): 4 kilos on the stack
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
        if(nk_button_label(cv->nk_ctx, "Load File"))
        {
            //TODO(Will): Stop this from breaking after a few clicks
            hit_load_save(cv);
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
    //hit_load_save(cv);
    
    //TODO(Will): Make this all one function
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
    hh h = {0};
    cv->hiphop = &h;
    hh_read_file_from_disk(cv->hiphop, "gl01.HIP");
#endif
}
