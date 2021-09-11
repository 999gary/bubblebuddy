#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include "time.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define nk_sdl_render_macro(i, b, c) nk_sdl_render(i, b, c);


#define INT_MAX 2147483647
#define INT_MIN -2147483648


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GLES2_IMPLEMENTATION
#include "nuklear.h"
#include "style.c"
#include "nuklear_sdl_gles2.h"


int hit_file_select_write(char* path, int max_path_len, int *save_as_gci, int *extension_supplied)
{
    *path = "./saves/GameData.xsv";
    *save_as_gci = 1;
    *extension_supplied = 1;
    return 0;
}

int hit_file_select_read(char* path, int max_path_len)
{
    *path = "./saves/GameData.xsv";
}

void hit_message_box_ok(char *caption, char *message)
{
    printf("%s %s", caption, message);
}