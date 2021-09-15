

#include <intrin.h>

int _fltused = 1;

void _wassert(wchar_t const* message,  
              wchar_t const* filename,  
              unsigned line) {
    __debugbreak();
}

void qsort(void *ptr, size_t count, size_t size,
           int (*comp)(const void *, const void *)) {
    // NOTE(jelly): actually just insertion sort lol
    char *a = (char *)ptr;
    int i = 1;
    static unsigned char tmp[4096];
    assert(size < sizeof(tmp));
    while (i < count) {
        memcpy(tmp, a + i*size, size);
        int j = i - 1;
        while (j >= 0 && comp(a + j*size, tmp) > 0) {
            memcpy(a + (j+1)*size, a + j*size, size);
            j--;
        }
        memcpy(a + (j+1)*size, tmp, size);
        i++;
    }
}

// --------------------------- MATH --------------------------

double frac_part(double x) {
    return x - (s32)x;
}

#pragma function(sqrt)
double sqrt(double x) {
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss((float)x)));
}
/*
// Handbook of Mathematical Functions
// M. Abramowitz and I.A. Stegun, Ed.

// Absolute error <= 6.7e-5
#pragma function(acos)
double acos(double x) {
    float negate = (float)(x < 0);
    x = abs(x);
    float ret = -0.0187293;
    ret = ret * x;
    ret = ret + 0.0742610;
    ret = ret * x;
    ret = ret - 0.2121144;
    ret = ret * x;
    ret = ret + 1.5707288;
    ret = ret * sqrt(1.0-x);
    ret = ret - 2 * negate * ret;
    return negate * 3.14159265358979 + ret;
}
*/
// Approximation of f(x) = acos(x)
// on interval [ -1, 1 ]
// with a polynomial of degree 6.
#pragma function(acos)
double acos(double x)
{
    double u = -4.8574102973990186e-34;
    u = u * x + -1.4693875000176811;
    u = u * x + 7.7184590392230222e-34;
    u = u * x + 1.2487950400795607;
    u = u * x + -3.1725614658374574e-34;
    u = u * x + -1.2959947327532642;
    return u * x + 1.5707963267948966;
}

typedef struct {
    float x, y, z;
} v3;

v3 V3(float x, float y, float z) {
    v3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

float v3_dot(v3 a, v3 b) {
    return (a.x * b.x +
            a.y * b.y +
            a.z * b.z);
}

v3 v3_add(v3 a, v3 b) {
    return V3(a.x + b.x,
              a.y + b.y,
              a.z + b.z);
}

v3 v3_sub(v3 a, v3 b) {
    return V3(a.x - b.x,
              a.y - b.y,
              a.z - b.z);
}

v3 v3_hadamard(v3 a, v3 b) {
    return V3(a.x * b.x,
              a.y * b.y,
              a.z * b.z);
}

#define V4V3Swizzle(v, a, b, c) V3(v[a], v[b], v[c])
#define V3Swizzle(v, a, b, c) V3(v.a, v.b, v.c)

// TODO(jelly): try a lolremez version?
// NOTE(jelly): https://developer.download.nvidia.com/cg/cos.html
#pragma function(cos)
double cos(double nn) {
    float a = (float)nn;
    const float c0[] = {0, 0.5f, 1, 0};
    const float c1[] = {0.25f, -9, 0.75f, 0.159154943091f };
    const float c2[] = {24.9808039603f, -24.9808039603f, -60.1458091736f,  60.1458091736f};
    const float c3[] = { 85.4537887573f, -85.4537887573f,-64.9393539429f,  64.9393539429f };
    const float c4[] = {19.7392082214f, -19.7392082214f, -1, 1};
    
    int x = 0;
    int y = 1;
    int z = 2;
    int w = 3;
    
    v3 r0;
    v3 r1;
    v3 r2;
    
    r1.x = c1[w] * a;
    r1.y = frac_part(r1.x);
    r2.x = (float)(r1.y < c1[x]);
    r2.y = (float)(r1.y >= c1[y]);
    r2.z = (float)(r1.y >= c1[z]);
    r2.y = v3_dot(r2, V4V3Swizzle(c4, z, w, z));
    r0 = v3_sub(V4V3Swizzle(c0, x, y, z), V3Swizzle(r1, y, y, y));
    r0 = v3_hadamard(r0, r0);
    r1 = v3_add(v3_hadamard(V4V3Swizzle(c2, x, y, x), r0), V4V3Swizzle(c2, z, w, z));
    r1 = v3_add(v3_hadamard(r1, r0), V4V3Swizzle(c3, x, y, x));
    r1 = v3_add(v3_hadamard(r1, r0), V4V3Swizzle(c3, z, w, z));
    r1 = v3_add(v3_hadamard(r1, r0), V4V3Swizzle(c4, x, y, x));
    r1 = v3_add(v3_hadamard(r1, r0), V4V3Swizzle(c4, z, w, z));
    return v3_dot(r1, V3(-r2.x, -r2.y, -r2.z));
}

// NOTE(jelly): https://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/
#pragma function(pow)
double pow(double a, double b) {
    // calculate approximation with fraction of the exponent
    int e = (int) b;
    union {
        double d;
        int x[2];
    } u = { a };
    u.x[1] = (int)((b - e) * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    
    // exponentiation by squaring with the exponent's integer part
    // double r = u.d makes everything much slower, not sure why
    double r = 1.0;
    while (e) {
        if (e & 1) {
            r *= a;
        }
        a *= a;
        e >>= 1;
    }
    
    return r * u.d;
}

int absolute(int n) {
    if (n < 0) return -n;
    return n;
}

// NOTE(jelly): i know these are technically wrong wrt negatives
#pragma function(floor)
double floor(double x) {
    return x - frac_part(x) - (x < 0);
}

#pragma function(ceil)
double ceil(double x) {
    return -floor(-x);
}

#pragma function(fmod)
double fmod(double a, double b) {
    double c = frac_part(absolute(a/b))*absolute(b);
    return (a < 0) ? -c : c;
}

// ----------------------------

// -------------------------------- MEMORY -----------------------------

void *virtual_alloc(u32 size) {
    return VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void virtual_free(void *ptr) {
    VirtualFree(ptr, 0, MEM_DECOMMIT|MEM_RELEASE);
}

// -------------------------------- TIME --------------------------------

// NOTE(jelly): helpful stack overflow person: https://stackoverflow.com/questions/20370920/convert-current-time-from-windows-to-unix-timestamp-in-c-or-c
u32 win32_time(void *unused) {
    //Get the number of seconds since January 1, 1970 12:00am UTC
    //Code released into public domain; no attribution required.
    
    const u64  UNIX_TIME_START = 0x019DB1DED53E8000; //January 1, 1970 (start of Unix epoch) in "ticks"
    const u64  TICKS_PER_SECOND = 10000000; //a tick is 100ns
    
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft); //returns ticks in UTC
    
    //Copy the low and high parts of FILETIME into a LARGE_INTEGER
    //This is so we can access the full 64-bits as an Int64 without causing an alignment fault
    LARGE_INTEGER li;
    li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    
    //Convert ticks since 1/1/1970 into seconds
    return ((li.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND);
}

// ------------------------------- FILE IO -------------------------------

unsigned char *read_entire_file(memory_arena *memory, char *path, int *size_ptr) {
    u32 MAX_FILE_SIZE = 128*1024; // TODO(jelly): pull this out, maybe pass it in???
    unsigned char *result = 0;
    HANDLE handle = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        DWORD size = GetFileSize(handle, NULL);
        if (size < MAX_FILE_SIZE) {
            unsigned char *mem = (unsigned char *)(memory_arena_alloc(memory, size));
            DWORD bytes_read;
            BOOL success = ReadFile(handle, mem, size, &bytes_read, NULL);
            if (success && bytes_read == size) { 
                result = mem;
                if (size_ptr) *size_ptr = size;
            }
        }
        CloseHandle(handle);
    }
    return result;
}

int write_out_file(char *path, unsigned char *data, int len) {
    HANDLE handle = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        BOOL success = WriteFile(handle, data, len, &bytes_written, NULL);
        CloseHandle(handle);
        return success;
    }
    return 0;
}

// ------------------------------- MEMCPY/MEMSET -------------------------------

#pragma function(memcpy)
void *memcpy(void *dest, const void *src, size_t size) {
    __movsb(dest, src, size);
    return dest;
}

#pragma function(memset)
void *memset(void *dest, int data, size_t size) {
    __stosb(dest, data, size);
    return dest;
}

// ------------------------------- MISC ----------------------------------

// NOTE(jelly): doesn't call all the atexit functions...who cares? we don't use them
void exit(int code) { ExitProcess(code); }

// ----------------------------------------------------------------------

int mainCRTStartup(void) {
    ExitProcess(main());
}

int WinMainCRTStartup(void) {
    ExitProcess(main());
}