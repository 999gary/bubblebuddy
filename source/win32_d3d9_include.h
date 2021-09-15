#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <commdlg.h>

#define NK_INCLUDE_FIXED_TYPES

#ifndef WIN32_NO_CRT
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#endif
#define NK_ZERO_COMMAND_MEMORY // NOTE(jelly): testing this
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_D3D9_IMPLEMENTATION
#include "external/nuklear.h"
#include "style.c"
#include <d3d9.h>

// TODO(jelly): look in to best allocation scheme for nuklear

#if 0

void *nk_alloc_wrapper(nk_handle h, void *old, nk_size size) {
    return virtual_alloc(size);
    memory_arena *m = (memory_arena *)h.ptr;
    return memory_arena_alloc(m, size);
    
}
void nk_free_wrapper(nk_handle h, void *old) {
    virtual_free(old);
}

void *nk_alloc_wrapper2(nk_handle h, void *old, nk_size size) {
    memory_arena *m = (memory_arena *)h.ptr;
    return memory_arena_alloc(m, size);
    
}
void nk_free_wrapper2(nk_handle h, void *old) {
    //virtual_free(old);
}

#else

void *nk_alloc_wrapper(nk_handle h, void *old, nk_size size) {
    return HeapAlloc(GetProcessHeap(), 0, size);
    
}
void nk_free_wrapper(nk_handle h, void *old) {
    HeapFree(GetProcessHeap(), 0, old);
}

void *nk_alloc_wrapper2(nk_handle h, void *old, nk_size size) {
    return HeapAlloc(GetProcessHeap(), 0, size);
    
}
void nk_free_wrapper2(nk_handle h, void *old) {
    HeapFree(GetProcessHeap(), 0, old);
}

#endif

#include "external/nuklear_d3d9.h"