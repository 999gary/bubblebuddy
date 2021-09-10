#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "config.h"
#include "byteswap.h"
#include "bit_methods.h"

#define ArrayCount(arr) (sizeof(arr)/sizeof((arr)[0]))
#define Minimum(a, b) ((a) < (b) ? (a) : (b))
#define Maximum(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(v, a, b) ((v) < (a) ? (a) : (v) > (b) ? (b) : (v))

int is_uppercase_letter(u8);
int is_digit(u8);

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
    nk_context *nk_ctx; //nuklear context
    int running;
    int screen;
    int s1_adv;
    int s1_scene_id;
    char s1_fpath[4096];
    bfbb_save_file save_file;
#ifndef HIPHOP_SUCKS_AND_DOESNT_WORK_SAD_FACE
    hh* hiphop;
#endif
    int save_file_is_loaded;
} hit_main;

void hit_common_init(hit_main *cv);
void hit_update_and_render(hit_main *cv);

#endif //MAIN_H
