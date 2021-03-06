
/*

TODO(jelly): STATE OF THE PROGRAM
--------------------------------------
-THE GUI FUCKING SUCKS, FUCKING FIX IT - not fixed
-PLYR block is still broken!
-SFIL block is the wrong size
-Stop clamping things please lemme finish the game at like 100000% - fixed (mostly outside of things that will break)
-Port to linux
-Port to emscripten - in progress
-just generally clean up the code it's pretty garbage - in progress
-Make a good theme
*/

#include "main.h"
#define HIPHOP_IMPLEMENTATION
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
#include "hiphop.c"
#endif

int is_uppercase_letter(char c) {
    return c >= 'A' && c <= 'Z';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

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

#ifdef WIN32_NO_CRT
#include "win32_no_crt.c"
#else
unsigned char *read_entire_file(memory_arena *memory, char *path, int *size) {
    const int MAX_READ_FILE_SIZE = 128*1024; // NOTE(jelly): i think this is a good upper bound
    int n; 
    unsigned char *result = 0;
    FILE *in = fopen(path, "rb");
    assert(size);
    if (in) {
        fseek(in, 0, SEEK_END);
        n = ftell(in);
        rewind(in);
        if (n > 0 && n < MAX_READ_FILE_SIZE) {
            result = memory_arena_alloc(memory, n);
            fread(result, 1, n, in);
            *size = n;
        }
        fclose(in);
    }
    return result;
}
int write_out_file(char *path, unsigned char *data, int len) {
    FILE *out = fopen(path, "wb");
    if (out) {
        int bytes_written_out = fwrite(data, 1, len, out);
        fclose(out);
        return bytes_written_out == len;
    }
    return 0;
}
#endif

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

void hit_message_box_ok_fmt(char *caption, char *message_fmt, ...) {
    va_list args;
    static char buffer[4096]; 
    va_start(args, message_fmt);
    stbsp_vsnprintf(buffer, sizeof(buffer), message_fmt, args);
    va_end(args);
    hit_message_box_ok(caption, buffer);
}

void hit_try_load_save(hit_main *cv, char *path) {
    memory_arena_state state = memory_arena_begin_scratch_memory(&cv->memory);
    int size = 0;
    unsigned char *data = read_entire_file(&cv->memory, path, &size);
    if (data) {
        memory_arena_reset(&cv->save_file->memory);
        if (bfbb_save_file_read(cv->save_file, data, size)) {
            cv->save_file_is_loaded = 1;
            cv->save_file->path = path;
        } else {
            hit_message_box_ok_fmt("Failed to Load Save File", 
                                   "'%s' doesn't appear to be a valid"
                                   " BFBB save file in a format that we support. Sorry. :(", path);
            
            cv->save_file_is_loaded = 0;
        }
    } else {
        cv->save_file_is_loaded = 0;
        hit_message_box_ok_fmt("Failed to Load Save File", "'%s' is either too big to be BFBB save file or couldn't be opened. Sorry :(", path);
    }
    memory_arena_end_scratch_memory(&cv->memory, state);
}

void hit_load_save(hit_main *cv)
{
    char *path = hit_file_select_read(cv);
    if (path) {
        hit_try_load_save(cv, path);
    } else {
        // TODO(jelly): error message????
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
        if(cv->save_file_is_loaded &&
           nk_menu_begin_symbol_text(cv->nk_ctx, "Save Edit Menu", 14, NK_TEXT_CENTERED, NK_SYMBOL_TRIANGLE_DOWN, 
                                     nk_vec2(window_width/3.0f, (window_height/20.0f)*(cv->save_file->block_count))))
        {
            nk_layout_row_dynamic(cv->nk_ctx, window_height/40.0f, 3);
            if(nk_menu_item_label(cv->nk_ctx, "General", NK_TEXT_CENTERED))
            {
                cv->s1_scene_id = 0;
            }
            int kj = 1;
            for(int i = 0; i<cv->save_file->block_count; i++)
            {
                bfbb_save_file_block b = cv->save_file->blocks[i];
                if(bfbb_save_file_fourcc_is_scene(b.header.id))
                {
                    if(!(kj%3))
                    {    
                        nk_layout_row_dynamic(cv->nk_ctx, window_height/40.0f, 3);
                    }
                    small_string label = stringifiy_fourcc(b.header.id);
                    if(nk_menu_item_label(cv->nk_ctx, label.chars, NK_TEXT_CENTERED))
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
        nk_label(cv->nk_ctx, cv->hiphop->hipa.block.block_id, NK_TEXT_LEFT);
    }
    nk_end(cv->nk_ctx);
}

#endif

/*-------------------------------------------

Screen 1 Components

-------------------------------------------*/

void nk_labelf(struct nk_context *ctx, nk_flags align, const char *fmt, ...) {
    static char buffer[4096];
    
    va_list args;
    va_start(args, fmt);
    int result = stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    nk_label(ctx, buffer, align);
}

void nk_menu_begin_labelf(struct nk_context *ctx, nk_flags align, struct nk_vec2 size, const char *fmt, ...) {
    static char buffer[4096];
    
    va_list args;
    va_start(args, fmt);
    int result = stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    nk_menu_begin_label(ctx, buffer, align, size);
}

void nk_checkbox_label_u8(nk_context* ctx, const char * label, u8* value)
{
    nk_bool a = *value;
    nk_checkbox_label(ctx, label, &a);
    *value = a;
}

void nk_base_type_begin(nk_context *ctx, float row_height, u32 id, u32 type) {
    nk_layout_row_dynamic(ctx, row_height, 2);
    nk_labelf(ctx, NK_TEXT_LEFT, "ID: %x", id);
    nk_labelf(ctx, NK_TEXT_LEFT, "Type: %s", get_base_type_name(type));
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
    
    base_type* b = &block->scene.bases[j];
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
            for(int i = 0; i<ArrayCount(flag); i++)
            {
                flag[i] = state & 1;
                state >>= 1;
            }
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->pickup.base_enable, &b->pickup.show_ent);
            nk_layout_row_dynamic(cv->nk_ctx, row_height, 4);
            char buffer[256];
            for(int i = 0; i<ArrayCount(flag); i++)
            {
                stbsp_snprintf(buffer, sizeof(buffer), "Flag #%d", i+1);
                nk_checkbox_label_u8(cv->nk_ctx, buffer, &flag[i]);
                memset(buffer, 0, sizeof(buffer));
            }
            state = 0;
            for (int i = 0; i < ArrayCount(flag); i++)
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
        case(BASE_TYPE_UIFONT):
        {
            nk_base_enabled_and_shown(cv->nk_ctx, row_height, &b->uifont.base_enable, &b->uifont.show_ent);
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
            //printf("Unknown base type %d\n", b->type);
            assert(0);
        }
    }
}

void hit_s1_scene_screen(hit_main* cv, scene_table_entry* table, char* id, float win_height)
{
    int kj = 0;
    bfbb_save_file_block *b = bfbb_save_file_find_block(cv->save_file, id);
    for(int i = 0; i<b->scene.base_count; i++)
    {
        base_type *bt = &b->scene.bases[i];
        if(!(kj%3))
            nk_layout_row_dynamic(cv->nk_ctx, win_height*(1.0f/3.0f), 3);
        kj++;
        if(nk_group_begin_titled(cv->nk_ctx, table[i].name, table[i].name, NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
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
    nk_labelf(ctx, NK_TEXT_CENTERED, label);
}
/*
char* get_cheat_label_by_id(u8 id)
{
    char* table[] = {
        "Art Theatre",
        "Add Spatulas",
        "Add Shinies",
        "",

    }
}
*/
void hit_s1_data(hit_main *cv)
{
    if(!cv->save_file_is_loaded)
        return;
    float win_height_offset = window_height/20.0f;
	float tab_size = win_height_offset / 1.5;
    float win_height = window_height - (window_height/20.0f*2.0f);
    
    bfbb_save_file *save_file = cv->save_file;
    bfbb_save_file_block *blocks = save_file->blocks;
    int block_count = save_file->block_count;
    
    static int is_hex_string_initialized = 0;
    
    // NOTE(jelly): for testing base_count upper-bound
#if 0
    u32 max_count = 0;
    for (int i = 0; i < block_count; i++) {
        bfbb_save_file_block *block = &blocks[i];
        if (bfbb_save_file_block_is_scene(block)) {
            u32 count = block->scene.base_count;
            max_count = Maximum(count, max_count);
        }
    }
#endif
    
    if(nk_begin(cv->nk_ctx, "Data Panel", nk_rect(0, win_height_offset, window_width, win_height), NK_WINDOW_BORDER))
    {
		s32 tab_count = 2;
		nk_menubar_begin(cv->nk_ctx);
		nk_layout_row_dynamic(cv->nk_ctx, tab_size, tab_count);
		for (s32 i = 0; i < tab_count; i++)
		{
            struct nk_style_button button_style = {
                {NK_STYLE_ITEM_COLOR, {nk_rgba(185, 185, 185, 255)}},
                {NK_STYLE_ITEM_COLOR, {nk_rgba(185, 185, 185, 255)}},

            };
			if (nk_button_label_styled(cv->nk_ctx, &button_style, "tab")) {
				
			}
		}
		nk_menubar_end(cv->nk_ctx);

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
                u32 id = block->header.id;
                if(!is_drawable_block(block))
                    continue;
                if(kj == 0)
                    nk_layout_row_dynamic(cv->nk_ctx, win_height/4.0f, 3);
                else if(!(kj%3))
                    nk_layout_row_dynamic(cv->nk_ctx, win_height/1.5f, 2);
                kj++;
                
                char *title = stringifiy_fourcc(id).chars;
                
                float row_height = win_height/3.0f/8.0f;
                
                nk_flags flags = NK_WINDOW_BORDER | NK_WINDOW_TITLE;
                if (id == FOURCC_PREF) flags |= NK_WINDOW_NO_SCROLLBAR;
                if(nk_group_begin_titled(cv->nk_ctx, title, title, flags))
                {
                    switch(id)
                    {
                        case(FOURCC_LEDR):
                        { 
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                            nk_label(cv->nk_ctx, "Game Label:", NK_TEXT_CENTERED);
                            nk_edit_string_zero_terminated(cv->nk_ctx, NK_EDIT_FIELD, blocks[i].ledr.game_label, 64, (nk_plugin_filter)NK_FILTER_INT);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                            nk_label(cv->nk_ctx, thumbnail_label_from_id(block->ledr.thumbnail_index), NK_TEXT_CENTERED);
                            nk_property_int(cv->nk_ctx, "ID:", 0, &block->ledr.thumbnail_index, 13, 1, 1);
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "Progress", 0, &block->ledr.progress, 100, 1, .5);
                            break;
                        }
                        case(FOURCC_ROOM):
                        {
                            //TODO(Will): Make this a selection box
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                            nk_label(cv->nk_ctx, "Room:", NK_TEXT_CENTERED);
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
                                nk_labelf(cv->nk_ctx, NK_TEXT_LEFT, "%s:", thumbnail_label_from_id(k));
                                nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                                nk_property_int(cv->nk_ctx, "#Socks", INT_MIN, &block->plyr.level_collectables[k].socks, INT_MAX, 1, 1);
                                nk_property_int(cv->nk_ctx, "#Pickups", INT_MIN, &block->plyr.level_collectables[k].pickups, INT_MAX, 1, 1);
                            }
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            nk_property_int(cv->nk_ctx, "#Total Socks", INT_MIN, &block->plyr.total_socks, INT_MAX, 1, 1);nk_layout_row_dynamic(cv->nk_ctx, row_height, 4);
                            char buffer[256];
                            for(int k = 0; k<14; k++)
                            {
                                memset(buffer, 0, sizeof(buffer));
                                stbsp_snprintf(buffer, sizeof(buffer), "Cutscene %d", k+1);
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
                                nk_label(cv->nk_ctx, thumbnail_label_from_id(k), NK_TEXT_RIGHT);
                                nk_layout_row_push(cv->nk_ctx, .080f);
                                for(int j = 0; j<spat_count_per_world[k]; j++)
                                {
                                    s16* spat = &block->cntr.spats[k][j];
                                    s16 s = *spat;
                                    s = Clamp(s, 0, 9);
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
                            for(int k = 0; k<15; k++)
                            {
                                nk_layout_row_dynamic(cv->nk_ctx, row_height, 2);
                                s16* rbdata = &block->cntr.robot_data[k];
                                s16 r = *rbdata;
                                Clamp(r, 0, 9);
                                char rbdata_str[8] = {0};
                                rbdata_str[0] = r + '0';
                                //TODO(Will): Add robot names.
                                nk_labelf(cv->nk_ctx, NK_TEXT_CENTERED, "Robot #%d", k+1);
                                if(nk_button_label(cv->nk_ctx, rbdata_str))
                                {
                                    if(*rbdata == 2)
                                    {
                                        *rbdata = 0;
                                    }
                                    else
                                    {
                                        *rbdata += 1;
                                    }
                                }
                            }
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 1);
                            s32 sock_cntr = block->cntr.reminder_sock_cntr;
                            nk_property_int(cv->nk_ctx, "#Reminder Sock CNTR", 0, &sock_cntr, INT16_MAX, 1, 1);
                            block->cntr.reminder_sock_cntr = sock_cntr;
                            nk_layout_row_dynamic(cv->nk_ctx, row_height, 4);
                            u8 flags[16];
                            s32 state = block->cntr.cheats;
                            char buffer[256];
                            for(int k = 0; k < ArrayCount(flags); k++)
                            {
                                memset(buffer, 0, sizeof(buffer));
                                flags[k] = state & 1;
                                state >>= 1;
                                stbsp_snprintf(buffer, sizeof(buffer), "Cheat #%d", k+1);
                                nk_checkbox_label_u8(cv->nk_ctx, buffer, &flags[k]);
                            }
                            state = 0;
                            for(int k = 0; k < ArrayCount(flags); k++)
                            {
                                state |= (s32)flags[k] << k;
                            }
                            block->cntr.cheats = state;
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
            int save_success = 0;
            char *path = hit_file_select_write(cv, &save_as_gci, &extension_supplied);
            if (path) {
                assert(save_as_gci == 0 || save_as_gci == 1);
                if (!extension_supplied) {
                    char *exts[] = { ".xsv", ".gci" };
                    char *ext = exts[save_as_gci];
                    // TODO(jelly): this is pretty hacky pls fix
                    assert(memory_arena_get_size_unused(&cv->save_file->memory) > 4);
                    strcat(path, ext);
                    memory_arena_alloc(&cv->save_file->memory, 5);
                }
                if (bfbb_save_file_write_out(cv->save_file, path, save_as_gci)) {
                    save_success = 1;
                }
            }
            if (!save_success) {
                hit_message_box_ok("Save Failed", "Failed to Save the File :(. Sorry.");
            }
        }
        if(nk_button_label(cv->nk_ctx, "Load File"))
        {
            hit_load_save(cv);
        }
        cv->s1_adv = 1;
        if (cv->save_file_is_loaded && cv->save_file->path) {
            nk_label(cv->nk_ctx, cv->save_file->path, NK_TEXT_CENTERED);
        }
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
    set_style(cv->nk_ctx, CURRENT_THEME);
}

void hit_common_init(hit_main *cv)
{
    cv->save_file = MemoryArenaAllocType(&cv->memory, bfbb_save_file);
    
    //hit_load_save(cv);
    // TODO(jelly): actually set the save file data struct instead of doing this laziness
#ifndef DONT_USE_BAKED_IN_SAVE
    bfbb_save_file_read(cv->save_file, save_data_100, sizeof(save_data_100));
    cv->save_file_is_loaded = 1;
#endif
    
    //TODO(Will): Make this all one function
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
    hh h = {0};
    cv->hiphop = &h;
    hh_read_file_from_disk(cv->hiphop, "gl01.HIP");
#endif
}
