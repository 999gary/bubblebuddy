#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "external/tinyfiledialogs.h"
#include "external/tinyfiledialogs.c"

#define nk_sdl_render_macro(i, b, c) nk_sdl_render(i);

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
#define NK_SDL_GL2_IMPLEMENTATION
#include "external/nuklear.h"
#include "style.c"
#include "external/nuklear_sdl_gl2.h"