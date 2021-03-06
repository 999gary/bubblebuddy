
#include "external/opensans_font.h"

typedef struct
{
    hit_main *cv;
    SDL_Window *win;
    SDL_GLContext glContext;
} args;

int hit_quit(args* argss)
{
    nk_sdl_shutdown();
    SDL_GL_DeleteContext(argss->glContext);
    SDL_DestroyWindow(argss->win);
    SDL_Quit();
}

int hit_loop(void* argss)
{
        args* arg = (args*)argss;
        hit_main* cv = arg->cv;
        SDL_Window* win = arg->win;
        SDL_Event evt;
        nk_input_begin(cv->nk_ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
            {
                hit_quit(arg);
                return 1;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(cv->nk_ctx);
        hit_update_and_render(cv);
        SDL_GetWindowSize(win, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);
        nk_sdl_render_macro(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        SDL_GL_SwapWindow(win);
}
char const * lFilterPatterns[2] = { "*.xsv", "*.gci" };
char* hit_file_select_read(hit_main *cv)
{
    
    return tinyfd_openFileDialog(
        "Select file", 
        "~", 
        2, 
        lFilterPatterns, 
        NULL,
        1 
        );
}

void hit_message_box_ok(char* title,char* message)
{

}

char* hit_file_select_write(hit_main *cv, int *save_as_gci, int *extension_supplied)
{
    return tinyfd_saveFileDialog(
        "Save File",
        "~",
        2,
        lFilterPatterns,
        NULL
        );
}

int main()
{
    #ifdef EMSCRIPTEN
    EM_ASM(
        FS.syncfs(true, function (err) {
            // Error
        });
    );
    #endif    
    hit_main cv = {0};
    cv.running = 1;
    SDL_Window *win;
    SDL_GLContext glContext;
    
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    win = SDL_CreateWindow("BFBB Save Editor",
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           WINDOW_WIDTH_INIT, WINDOW_HEIGHT_INIT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &window_width, &window_height);
    cv.nk_ctx = nk_sdl_init(win);
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *droid = nk_font_atlas_add_from_memory(atlas, (char *)OpenSansRegular, sizeof(OpenSansRegular), 15, 0);
    nk_sdl_font_stash_end();
    nk_style_set_font(cv.nk_ctx, &droid->handle);
    hit_common_init(&cv);
    args arg = {&cv, win, &glContext};
    #ifdef EMSCRIPTEN
    emscripten_set_main_loop_arg(hit_loop, (void*)&arg, 0, nk_true);
    #else
    while (cv.running)
    {
        if(hit_loop((void*)&arg))
        {
            cv.running = 0;
            return 0;
        }
    }
    #endif
    return 0;
}