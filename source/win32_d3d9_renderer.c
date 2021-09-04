
#include "opensans_font.h"


static inline unsigned long long  win32_get_performance_counter() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

static inline unsigned long long  win32_get_performance_frequency() {
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return result.QuadPart;
}

static inline double win32_get_elapsed_ms(unsigned long long  start, unsigned long long  end, unsigned long long  perf_freq) {
    return 1000.0*(end - start)/perf_freq;
}

static IDirect3DDevice9 *device;
static IDirect3DDevice9Ex *deviceEx;
static D3DPRESENT_PARAMETERS present;

static float framerate;

// TODO: this doesn't work :((((((((
static void set_vsync(int on) {
#if 0
    nk_d3d9_create_state();
    
    if (on) present.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    else    present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    
    IDirect3DStateBlock9_Apply(d3d9.state);
    IDirect3DStateBlock9_Release(d3d9.state);
#endif
}

static void win32_d3d9_present(void) {
    HRESULT hr;
    hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
                                D3DCOLOR_COLORVALUE(255, 0, 255, 255), 0.0f, 0);
    NK_ASSERT(SUCCEEDED(hr));
    
    hr = IDirect3DDevice9_BeginScene(device);
    NK_ASSERT(SUCCEEDED(hr));
    nk_d3d9_render(NK_ANTI_ALIASING_ON);
    hr = IDirect3DDevice9_EndScene(device);
    NK_ASSERT(SUCCEEDED(hr));
    if (deviceEx) {
        hr = IDirect3DDevice9Ex_PresentEx(deviceEx, NULL, NULL, NULL, NULL, 0);
    } else {
        hr = IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);
    }
    if (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICEHUNG || hr == D3DERR_DEVICEREMOVED) {
        /* to recover from this, you'll need to recreate device and all the resources */
        MessageBoxW(NULL, L"D3D9 device is lost or removed!", L"Error", 0);
        exit(EXIT_FAILURE);
    } else if (hr == S_PRESENT_OCCLUDED) {
        /* window is not visible, so vsync won't work. Let's sleep a bit to reduce CPU usage */
        Sleep(10);
    }
    NK_ASSERT(SUCCEEDED(hr));
}

static LRESULT CALLBACK
WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    hit_main *cv = (hit_main *)GetWindowLongPtr(wnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_PAINT: {
            hit_update_and_render(cv);
            win32_d3d9_present();
        } break;
        case WM_SIZE: {
            if (device)
            {
                UINT width = LOWORD(lparam);
                UINT height = HIWORD(lparam);
                if (width != 0 && height != 0 &&
                    (width != present.BackBufferWidth || height != present.BackBufferHeight))
                {
                    nk_d3d9_release();
                    present.BackBufferWidth = width;
                    present.BackBufferHeight = height;
                    HRESULT hr = IDirect3DDevice9_Reset(device, &present);
                    NK_ASSERT(SUCCEEDED(hr));
                    nk_d3d9_resize(width, height);
                }
                
                window_width = width;
                window_height = height;
            }
        } break;
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    if (nk_d3d9_handle_event(wnd, msg, wparam, lparam))
        return 0;
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

static void create_d3d9_device(HWND wnd)
{
    HRESULT hr;
    
    present.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    present.BackBufferWidth = window_width;
    present.BackBufferHeight = window_height;
    present.BackBufferFormat = D3DFMT_X8R8G8B8;
    present.BackBufferCount = 1;
    present.MultiSampleType = D3DMULTISAMPLE_NONE;
    present.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present.hDeviceWindow = wnd;
    present.EnableAutoDepthStencil = TRUE;
    present.AutoDepthStencilFormat = D3DFMT_D24S8;
    present.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    present.Windowed = TRUE;
    
    {/* first try to create Direct3D9Ex device if possible (on Windows 7+) */
        typedef HRESULT WINAPI Direct3DCreate9ExPtr(UINT, IDirect3D9Ex**);
        Direct3DCreate9ExPtr *Direct3DCreate9Ex = (void *)GetProcAddress(GetModuleHandleA("d3d9.dll"), "Direct3DCreate9Ex");
        if (Direct3DCreate9Ex) {
            IDirect3D9Ex *d3d9ex;
            if (SUCCEEDED(Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9ex))) {
                hr = IDirect3D9Ex_CreateDeviceEx(d3d9ex, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
                                                 D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
                                                 &present, NULL, &deviceEx);
                if (SUCCEEDED(hr)) {
                    device = (IDirect3DDevice9 *)deviceEx;
                } else {
                    /* hardware vertex processing not supported, no big deal
                    retry with software vertex processing */
                    hr = IDirect3D9Ex_CreateDeviceEx(d3d9ex, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
                                                     D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
                                                     &present, NULL, &deviceEx);
                    if (SUCCEEDED(hr)) {
                        device = (IDirect3DDevice9 *)deviceEx;
                    }
                }
                IDirect3D9Ex_Release(d3d9ex);
            }
        }
    }
    
    if (!device) {
        /* otherwise do regular D3D9 setup */
        IDirect3D9 *d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
        
        hr = IDirect3D9_CreateDevice(d3d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
                                     D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
                                     &present, &device);
        if (FAILED(hr)) {
            /* hardware vertex processing not supported, no big deal
            retry with software vertex processing */
            hr = IDirect3D9_CreateDevice(d3d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
                                         D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
                                         &present, &device);
            NK_ASSERT(SUCCEEDED(hr));
        }
        IDirect3D9_Release(d3d9);
    }
}

// NOTE(jelly): the two functions below return zero on success because C# is really cool
int hit_file_select_read(char* path, int max_path_len)
{
    OPENFILENAMEA file = {0};
    file.lpstrFile = path;
    file.nMaxFile = max_path_len;
    file.lStructSize = sizeof file;
    file.lpstrFilter = "All Files\0*.*\0\0";
    return !GetOpenFileNameA(&file);
}

int hit_file_select_write(char* path, int max_path_len, int *save_as_gci, int *extension_supplied)
{
    OPENFILENAMEA file = {0};
    file.lpstrFile = path;
    file.nMaxFile = max_path_len;
    file.lStructSize = sizeof file;
    file.lpstrFilter = "Xbox Save File\0.xsv\0Gamecube Save File\0.gci\0\0";
    if (GetSaveFileNameA(&file)) {
        *save_as_gci = (file.nFilterIndex - 1) != 0; // NOTE(jelly): clamping to 1 or 0
        *extension_supplied = file.nFileExtension != 0;
        return 0;
    }
    return 1;
}

void hit_message_box_ok(char *caption, char *message) {
    MessageBoxA(0, message, caption, MB_OK);
}

int hit_init(hit_main *cv) {
    window_height = WINDOW_HEIGHT_INIT;
    window_width = WINDOW_WIDTH_INIT;
    WNDCLASSW wc;
    RECT rect = { 0, 0, window_width, window_height };
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_APPWINDOW;
    HWND wnd;
    int running = 1;
    int needs_refresh = 1;
    
    /* Win32 */
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(0);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"Jelly is stupid >:(";
    RegisterClassW(&wc);
    
    AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    
    wnd = CreateWindowExW(exstyle, wc.lpszClassName, L"BFBB Save Editor",
                          style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                          rect.right - rect.left, rect.bottom - rect.top,
                          NULL, NULL, wc.hInstance, NULL);
    
    SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)cv);
    
    create_d3d9_device(wnd);
    /* GUI */
    cv->nk_ctx = nk_d3d9_init(device, window_width, window_height);
    
    struct nk_context *ctx = cv->nk_ctx;
    
    struct nk_font_atlas *atlas;
    
    nk_d3d9_font_stash_begin(&atlas);
    //struct nk_font *droid = nk_font_atlas_add_from_memory(atlas, (char *)LeroyLetteringLightBeta01, sizeof(LeroyLetteringLightBeta01), 15, 0);
    struct nk_font *droid = nk_font_atlas_add_from_memory(atlas, (char *)OpenSansRegular, sizeof(OpenSansRegular), 20, 0);
    nk_d3d9_font_stash_end();
    nk_style_set_font(ctx, &droid->handle);
    unsigned long long  perf_freq = win32_get_performance_frequency();
    unsigned long long  start_tick = win32_get_performance_counter();
    double elapsed_ms = 0.0;
    hit_common_init(cv);
    while (running)
    {
        MSG msg;
        nk_input_begin(ctx);
        if (needs_refresh == 0) {
            if (GetMessageW(&msg, NULL, 0, 0) <= 0)
                running = 0;
            else {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            needs_refresh = 1;
        } else needs_refresh = 0;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                running = 0;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        needs_refresh = 1;
        nk_input_end(ctx);
        
        // TODO(jelly): clear the screen magenta; it's not happening in present for some reason??
        
        hit_update_and_render(cv);
        framerate = 1000.0f / elapsed_ms;
        
        win32_d3d9_present();
        
        unsigned long long  end_tick = win32_get_performance_counter();
        elapsed_ms = win32_get_elapsed_ms(start_tick, end_tick, perf_freq);
        start_tick = end_tick;
    }
    
    // TODO: fun fact if we know we're on windows, we know that windows is going to clean up for us so we actually don't have to
    //nk_d3d9_shutdown();
    //UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

/*int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) { 
    hit_main hit_main = {0};
    hit_main.running = 1;
    return hit_init(&hit_main);
}*/
int main() {    
    hit_main hit_main = {0};
    hit_main.running = 1;
    return hit_init(&hit_main);
}
