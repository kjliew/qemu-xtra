#include <windows.h>
#include "wr211dll.h"

//#define DEBUG_WRAP2X
#ifdef DEBUG_WRAP2X
#define DPRINTF(fmt, ...) \
    do { fprintf(stderr, "  " fmt , ## __VA_ARGS__); } while(0)
#else
#define DPRINTF(fmt, ...)
#endif

struct tblGlideResolution {
    int w;
    int	h;
};

static struct tblGlideResolution tblRes[] = {
  { .w = 320, .h = 200   }, //0x0
  { .w = 320, .h = 240   }, //0x1
  { .w = 400, .h = 256   }, //0x2
  { .w = 512, .h = 384   }, //0x3
  { .w = 640, .h = 200   }, //0x4
  { .w = 640, .h = 350   }, //0x5
  { .w = 640, .h = 400   }, //0x6
  { .w = 640, .h = 480   }, //0x7
  { .w = 800, .h = 600   }, //0x8
  { .w = 960, .h = 720   }, //0x9
  { .w = 856, .h = 480   }, //0xa
  { .w = 512, .h = 256   }, //0xb
  { .w = 1024, .h = 768  }, //0xC
  { .w = 1280, .h = 1024 }, //0xD
  { .w = 1600, .h = 1200 }, //0xE
  { .w = 400, .h = 300   }, //0xF
  { .w = 0, .h = 0},
};

static struct emuLfb {
    GrLfbBypassMode_t bypass;
    GrOriginLocation_t origin;
    GrLfbWriteMode_t writeMode;
    GrBuffer_t grBuffer;
    GrLfbInfo_t info;
    unsigned long vLfb[(1024*600*2) >> 2];
    int w, h, lock[2];
    void convert(const void *dst, const void *src) {
        typedef unsigned long v4ul __attribute__((vector_size (16)));
        v4ul *d = (v4ul *)dst;
        v4ul *s = (v4ul *)src;
        switch(writeMode) {
            case GR_LFBWRITEMODE_555:
                for (int i = 0; i < (w >> 3); i++) {
                    static const v4ul alpha = {0x80008000, 0x80008000, 0x80008000, 0x80008000};
                    static const v4ul mask1 = {0xFFC0FFC0, 0xFFC0FFC0, 0xFFC0FFC0, 0xFFC0FFC0};
                    static const v4ul mask2 = {0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F};
                    d[i] = alpha | ((s[i] & mask1) >> 1) | (s[i] & mask2);
                }
                break;
            default:
                memcpy(d, s, (w << 1));
                break;
        }
    }
    void fill(void) {
        unsigned char *dst, *src;
        grLfbLock(GR_LFB_READ_ONLY, grBuffer, GR_LFBWRITEMODE_565, origin, (bypass)? 0:1, &info);
        dst = (unsigned char *)vLfb;
        src = (unsigned char *)info.lfbPtr;
        for (int i = 0; i < h; i++) {
            convert(dst, src);
            dst += 2048;
            src += info.strideInBytes;
        }
        grLfbUnlock(GR_LFB_READ_ONLY, grBuffer);
    }
    void flush(void) {
        grLfbWriteRegion(grBuffer, 0, 0, writeMode, w, h, 2048, vLfb);
    }
} Lfb2x;

FX_ENTRY void FX_CALL
wrap_grBufferSwap( int swap_interval )
{
    if (Lfb2x.lock[1])
        Lfb2x.flush();
    grBufferSwap(swap_interval);
}

FX_ENTRY void FX_CALL 
wrap_grLfbBegin( void )
{
    Lfb2x.lock[Lfb2x.grBuffer] = 1;
    if (Lfb2x.lock[1])
        Lfb2x.fill();
}

FX_ENTRY void FX_CALL 
wrap_grLfbBypassMode( GrLfbBypassMode_t mode )
{
    Lfb2x.bypass = mode;
}

FX_ENTRY void FX_CALL 
wrap_grLfbEnd( void )
{
    Lfb2x.flush();
    Lfb2x.lock[Lfb2x.grBuffer] = 0;
}

FX_ENTRY const FxU32 * FX_CALL 
wrap_grLfbGetReadPtr( GrBuffer_t buffer )
{
    Lfb2x.grBuffer = buffer & 0x01U;
    Lfb2x.lock[Lfb2x.grBuffer] = 1;
    return (const FxU32 *)Lfb2x.vLfb;
}

FX_ENTRY void * FX_CALL 
wrap_grLfbGetWritePtr( GrBuffer_t buffer )
{
    Lfb2x.grBuffer = buffer & 0x01U;
    Lfb2x.lock[Lfb2x.grBuffer] = 1;
    return Lfb2x.vLfb;
}

FX_ENTRY void FX_CALL 
wrap_grLfbOrigin(GrOriginLocation_t origin)
{
    Lfb2x.origin = origin;
}

FX_ENTRY void FX_CALL 
wrap_grLfbWriteMode( GrLfbWriteMode_t mode )
{
    Lfb2x.writeMode = mode;
}

FX_ENTRY FxBool FX_CALL 
wrap_grSstOpen(
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          GrSmoothingMode_t    smoothing_filter,
          int                  num_buffers )
{
    FxU hwnd = (FxU)GetActiveWindow();
    Lfb2x.w = tblRes[screen_resolution].w;
    Lfb2x.h = tblRes[screen_resolution].h;
    DPRINTF("SstWinOpen cformat %d", color_format);
    return grSstWinOpen(hwnd, screen_resolution, refresh_rate,
            color_format, origin_location, num_buffers, 1);
            
}

FX_ENTRY void FX_CALL
wrap_grSstPassthruMode( GrPassthruMode_t mode)
{
    GrControl_t code;
    switch(mode) {
        case GR_PASSTHRU_SHOW_VGA:
            code = GR_CONTROL_DEACTIVATE;
                break;
        case GR_PASSTHRU_SHOW_SST1:
            code = GR_CONTROL_ACTIVATE;
                break;
        default:
                return;
    }
    grSstControl(code);
}

FX_ENTRY void FX_CALL
wrap_guFbReadRegion(
               const int src_x, const int src_y,
               const int w, const int h, const void *dst,
               const int strideInBytes
               )
{
    grLfbReadRegion(Lfb2x.grBuffer, src_x, src_y, w, h, strideInBytes, (void *)dst);
}

FX_ENTRY void FX_CALL
wrap_guFbWriteRegion(
                const int dst_x, const int dst_y,
                const int w, const int h, const void *src,
                const int strideInBytes
                )
{
    grLfbWriteRegion(Lfb2x.grBuffer, dst_x, dst_y, Lfb2x.writeMode, w, h, strideInBytes, (void *)src);
    Lfb2x.fill();
}

FX_ENTRY void FX_CALL
wrap_grSplash(void)
{
    grSplash(0, 0, Lfb2x.w, Lfb2x.h, 0);
}

BOOL APIENTRY DllMain( HINSTANCE hModule,
        DWORD dwReason,
        LPVOID lpReserved
        )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_ATTACH:
            DPRINTF("Attached - glide.dll " __DATE__ " " __TIME__);
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

