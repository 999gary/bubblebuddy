#ifndef MAIN_H
#define MAIN_H

#include "types.h"

#ifdef WIN32_NO_CRT
#define assert(exp) do { if (!(exp)) { __debugbreak(); } } while(0)
void *virtual_alloc(u32);
void virtual_free(void*);
u32 win32_time(void *);
#define time win32_time
#else
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define virtual_alloc malloc
#define virtual_free  free
#endif

#define STB_SPRINTF_IMPLEMENTATION
#include "external/stb_sprintf.h"

#include "config.h"
#include "byteswap.h"
#include "bit_methods.h"

#define ArrayCount(arr) (sizeof(arr)/sizeof((arr)[0]))
#define Minimum(a, b) ((a) < (b) ? (a) : (b))
#define Maximum(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(v, a, b) ((v) < (a) ? (a) : (v) > (b) ? (b) : (v))

#include "memory_arena.h"
#include "bfbb_save.h"

#ifdef _WIN32
#include "win32_d3d9_include.h"
#elif __linux__ 
#include "sdlinclude.h"
#elif EMSCRIPTEN
#include "glesinclude.h"
#else
#error Platform not supported!
#endif

typedef struct nk_context nk_context;
typedef struct {
    nk_context *nk_ctx;
    memory_arena memory;
    int running;
    int screen;
    int s1_adv;
	int s1_tab_count;
    int s1_scene_id;
    bfbb_save_file *save_file;
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
    hh* hiphop;
#endif
    int save_file_is_loaded;
} hit_main;

void hit_common_init(hit_main *cv);
void hit_update_and_render(hit_main *cv);
void hit_try_load_save(hit_main *cv, char *path);

#endif //MAIN_H
