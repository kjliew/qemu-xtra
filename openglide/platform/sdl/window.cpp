//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*     SDL specific functions for handling display window
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef C_USE_SDL

#include <stdlib.h>
#include <math.h>

#if defined(__linux__) || defined(__darwin__)
#include <dlfcn.h>
#define LOAD_SOLIB(x) \
    x = dlopen(dllname, RTLD_LAZY) ; \
    if (!x) { \
        const char *local_lib; \
        char local_path[] = "/usr/local/lib/libSDL2.sonamexyz"; \
        local_path[strlen("/usr/local/lib/")] = '\x0'; \
        local_lib = strcat(local_path, dllname); \
        x = dlopen(local_lib, RTLD_LAZY); \
    } \
    if (!x) { fprintf(stderr, "Error loading %s\n", dllname); return false; }
#define FREE_SOLIB(x) \
    dlclose(x); x = 0
#define INIT_SUBSS(x) \
    SDL20func.GLGetAttribute = (int (*)(SDL_GLattr, int *))dlsym(x, "SDL_GL_GetAttribute"); \
    SDL20func.GLSetAttribute = (int (*)(SDL_GLattr, int))dlsym(x, "SDL_GL_SetAttribute"); \
    InitSubSystem = (int (*)(const int))dlsym(x, "SDL_InitSubSystem"); \
    unsetenv("SDL_VIDEODRIVER")
#define QUIT_SUBSS(x) \
    QuitSubSystem = (void (*)(const int))dlsym(x, "SDL_QuitSubSystem")
#else /* WIN32 */
#define LOAD_SOLIB(x) \
    x = LoadLibrary(dllname); \
    if (!x) { fprintf(stderr, "Error loading %s\n", dllname); return false; }
#define FREE_SOLIB(x) \
    FreeLibrary((HMODULE)x); x = 0
#define INIT_SUBSS(x) \
    SDL20func.GLGetAttribute = (int (*)(SDL_GLattr, int *))GetProcAddress((HMODULE)x, "SDL_GL_GetAttribute"); \
    SDL20func.GLSetAttribute = (int (*)(SDL_GLattr, int))GetProcAddress((HMODULE)x, "SDL_GL_SetAttribute"); \
    InitSubSystem = (int (*)(const int))GetProcAddress((HMODULE)x, "SDL_InitSubSystem"); \
    SetEnvironmentVariable("SDL_VIDEODRIVER", NULL)
#define QUIT_SUBSS(x) \
    QuitSubSystem = (void (*)(const int))GetProcAddress((HMODULE)x, "SDL_QuitSubSystem")
#endif

#include "SDL.h"
#include "SDL_opengl.h"

#include "GlOgl.h"

#include "platform/window.h"

static struct gamma_ramp {
    uint16_t red[256];
    uint16_t blue[256];
    uint16_t green[256];
} old_ramp;
static bool ramp_stored;

static SDL_Window *window;
static SDL_Renderer *render;
static SDL_GLContext context;
static bool self_ctx, self_wnd, wnd_from;
static void *hlib;

bool InitialiseOpenGLWindow(FxU wnd, int x, int y, int width, int height)
{
    const char dllname[] =
#if defined(__darwin__)
        "libSDL2.dylib"
#elif defined(__linux__)
        "libSDL2-2.0.so.0"
#else /* WIN32 */
        "SDL2.dll"
#endif
    ;
    struct {
        int (*GLGetAttribute)(SDL_GLattr, int *);
        int (*GLSetAttribute)(SDL_GLattr, int);
    } SDL20func = {
        .GLGetAttribute = &SDL_GL_GetAttribute,
        .GLSetAttribute = &SDL_GL_SetAttribute,
    };

    self_wnd = false;
    if (!wnd) {
        const char *title = "SDL2-OpenGLide";
        uint32_t flags = (UserConfig.InitFullScreen)? SDL_WINDOW_FULLSCREEN_DESKTOP:0;
        window = SDL_CreateWindow(title, x, y, width, height, flags);
        if (window) {
            if (UserConfig.SamplesMSAA) {
                SDL20func.GLSetAttribute(SDL_GL_MULTISAMPLEBUFFERS, SDL_TRUE);
                SDL20func.GLSetAttribute(SDL_GL_MULTISAMPLESAMPLES, UserConfig.SamplesMSAA);
            }
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
            render = SDL_CreateRenderer(window, -1, 0);
        }
        self_wnd = (window)? true:false;
    }
    else {
#if (SIZEOF_INT_P == 8)
        /* SDL_Window is a native pointer. On 64-bit system, native pointers
         * should have more than 32-bit values. Windows HWND and X11 Window handles
         * are always in 32-bit values.
         *
         * Apple macOS NSWindow pointer has bit[32] set.
         */
        int (*InitSubSystem)(const int);
        LOAD_SOLIB(hlib);
        INIT_SUBSS(hlib);
        wnd_from = false;
        if (!(wnd & ((uintptr_t)0xFFFE << 32))) {
            if (InitSubSystem && !InitSubSystem(SDL_INIT_VIDEO))
                window = SDL_CreateWindowFrom((const void *)wnd);
            if (window) {
                if (UserConfig.SamplesMSAA) {
                    SDL20func.GLSetAttribute(SDL_GL_MULTISAMPLEBUFFERS, SDL_TRUE);
                    SDL20func.GLSetAttribute(SDL_GL_MULTISAMPLESAMPLES, UserConfig.SamplesMSAA);
                }
                SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
                render = SDL_CreateRenderer(window, -1, 0);
            }
            wnd_from = (window)? true:false;
        }
#else
        /* Never perform foreign window conversion for 32-bit system,
         * OpenGLide for 32-bit system should be configured for
         * native window with `--disable-sdl`.
         */
        if (0) { }
#endif
        else {
            uint32_t flags = SDL_GetWindowFlags((SDL_Window *)wnd);
            window = (SDL_Window *)wnd;
            if (flags & SDL_WINDOW_OPENGL)
                render = nullptr;
            else {
                render = SDL_GetRenderer(window);
                if (render)
                    SDL_DestroyRenderer(render);
                SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
                render = SDL_CreateRenderer(window, -1, 0);
            }
        }
    }
    if (render) {
        SDL_DestroyRenderer(render);
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
    }
    self_ctx = false;
    context = SDL_GL_GetCurrentContext();
    if (!context) {
        context = SDL_GL_CreateContext(window);
        self_ctx = (context)? true:false;
    }
    if (!context)
        fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, SDL_GetError());
    else {
        int cRedBits, cGreenBits, cBlueBits, cAlphaBits,cDepthBits, cStencilBits,
            cAuxBuffers, nSamples[2], has_sRGB = UserConfig.FramebufferSRGB;
        if (SDL_GL_MakeCurrent(window, context))
            fprintf(stderr, "%s\n", SDL_GetError());
        SDL20func.GLGetAttribute(SDL_GL_RED_SIZE, &cRedBits);
        SDL20func.GLGetAttribute(SDL_GL_GREEN_SIZE, &cGreenBits);
        SDL20func.GLGetAttribute(SDL_GL_BLUE_SIZE, &cBlueBits);
        SDL20func.GLGetAttribute(SDL_GL_ALPHA_SIZE, &cAlphaBits);
        SDL20func.GLGetAttribute(SDL_GL_DEPTH_SIZE, &cDepthBits);
        SDL20func.GLGetAttribute(SDL_GL_STENCIL_SIZE, &cStencilBits);
        SDL20func.GLGetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &nSamples[0]);
        SDL20func.GLGetAttribute(SDL_GL_MULTISAMPLESAMPLES, &nSamples[1]);
        glGetIntegerv(GL_AUX_BUFFERS, &cAuxBuffers);

        fprintf(stderr, "Info: %s OpenGL %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
        fprintf(stderr, "Info: Pixel Format ABGR%d%d%d%d D%2dS%d nAux %d nSamples %d %d %s\n",
                cAlphaBits,cBlueBits, cGreenBits, cRedBits, cDepthBits, cStencilBits,
                cAuxBuffers, nSamples[0], nSamples[1], (has_sRGB)? "sRGB":"");

        do {
            int w, h;
            SDL_GL_GetDrawableSize(window, &w, &h);
            if (h > OpenGL.WindowHeight) {
                float r = (1.f * height) / width,
                      win_r = (1.f * h) / w;
                if (r == win_r) {
                    OpenGL.WindowWidth = w;
                    OpenGL.WindowHeight = h;
                    OpenGL.WindowOffset = 0;
                }
                else {
                    OpenGL.WindowWidth = h / r;
                    OpenGL.WindowHeight = h;
                    OpenGL.WindowOffset = (w - OpenGL.WindowWidth) >> 1;
                }
                UserConfig.Resolution = OpenGL.WindowWidth;
            }
        } while(0);

        if (has_sRGB)
            glEnable(GL_FRAMEBUFFER_SRGB);

        if (cDepthBits > 16)
            UserConfig.PrecisionFix = false;

        for (int i = 0; i < 0x100; i++) {
            old_ramp.red[i]   = (uint16_t)(((i << 8) | i) & 0xFFFFU);
            old_ramp.green[i] = (uint16_t)(((i << 8) | i) & 0xFFFFU);
            old_ramp.blue[i]  = (uint16_t)(((i << 8) | i) & 0xFFFFU);
        }
        ramp_stored = true;

    }
    return (context)? true:false;
}

void FinaliseOpenGLWindow(void)
{
    int has_sRGB = UserConfig.FramebufferSRGB;

    if ( ramp_stored )
        SetGammaTable(&old_ramp);
    if ( has_sRGB )
        glDisable(GL_FRAMEBUFFER_SRGB);

    SetSwapInterval(-1);

    if ( self_ctx ) {
        if (SDL_GL_MakeCurrent(window, NULL) == 0)
            SDL_GL_DeleteContext(context);
    }
    if ( self_wnd )
        SDL_DestroyWindow(window);
    if ( wnd_from ) {
        void (*QuitSubSystem)(const int);
        QUIT_SUBSS(hlib);
        QuitSubSystem(SDL_INIT_VIDEO);
    }
    if (hlib)
        FREE_SOLIB(hlib);
    context = 0;
    window = 0;
}

void SetGamma(float value)
{
    struct gamma_ramp ramp;

    for (int i = 0; i < 0x100; i++) {
        uint16_t v = (uint16_t)(0xFFFFU * pow(i / 255.f, 1.f / value));
        ramp.red[i]   = v & 0xFF00U;
        ramp.green[i] = v & 0xFF00U;
        ramp.blue[i]  = v & 0xFF00U;
    }
    SetGammaTable(&ramp);
}

void RestoreGamma()
{
}

void SetGammaTable(void *ptbl)
{
    struct gamma_ramp *ramp = (struct gamma_ramp *)ptbl;
    if (window && !UserConfig.FramebufferSRGB)
        SDL_SetWindowGammaRamp(window, ramp->red, ramp->green, ramp->blue);
}

void GetGammaTable(void *ptbl)
{
    struct gamma_ramp *ramp = (struct gamma_ramp *)ptbl;
    if (window)
        SDL_GetWindowGammaRamp(window, ramp->red, ramp->green, ramp->blue);
}

bool SetScreenMode(int &xsize, int &ysize)
{
    return true;
}

void ResetScreenMode()
{
}

void SetSwapInterval(const int i)
{
    static int last_i = -1;
    if (last_i != i) {
        last_i = i;
        if (i >= 0)
            SDL_GL_SetSwapInterval(i);
    }
}

void SwapBuffers()
{
    if (self_wnd) {
        SDL_Event e;
        while(SDL_PollEvent(&e));
    }
    if (UserConfig.swap12) {
        void (*glSwapFunc)(void) = (void (*)(void))UserConfig.swap12;
        glSwapFunc();
        return;
    }
    SDL_GL_SwapWindow(window);
}

#endif // C_USE_SDL
