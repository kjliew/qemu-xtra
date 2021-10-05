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
static bool self_wnd;
static bool self_ctx;

bool InitialiseOpenGLWindow(FxU wnd, int x, int y, int width, int height)
{
    if (!wnd) {
        const char *title = "SDL2-OpenGLide";
        uint32_t flags = (UserConfig.InitFullScreen)? SDL_WINDOW_FULLSCREEN_DESKTOP:0;
        window = SDL_CreateWindow(title, x, y, width, height, flags);
        if (window) {
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
            render = SDL_CreateRenderer(window, -1, 0);
        }
        self_wnd = (window)? true:false;
    }
    else {
        uint32_t flags = SDL_GetWindowFlags((SDL_Window *)wnd);
        if (!flags)
            return false;
        self_wnd = false;
        window = (SDL_Window *)wnd;
    }
    context = SDL_GL_GetCurrentContext();
    if (!context) {
        context = SDL_GL_CreateContext(window);
        self_ctx = (context)? true:false;
    }
    if (context) {
        int cRedBits, cGreenBits, cBlueBits, cAlphaBits,cDepthBits, cStencilBits,
            cAuxBuffers, nSamples[2], has_sRGB = UserConfig.FramebufferSRGB;
        SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &cRedBits);
        SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &cGreenBits);
        SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &cBlueBits);
        SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &cAlphaBits);
        SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &cDepthBits);
        SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &cStencilBits);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &nSamples[0]);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &nSamples[1]);
        glGetIntegerv(GL_AUX_BUFFERS, &cAuxBuffers);

        fprintf(stderr, "Info: %s OpenGL %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
        fprintf(stderr, "Info: Pixel Format ABGR%d%d%d%d D%2dS%d nAux %d nSamples %d %d %s\n",
                cAlphaBits,cBlueBits, cGreenBits, cRedBits, cDepthBits, cStencilBits,
                cAuxBuffers, nSamples[0], nSamples[1], (has_sRGB)? "sRGB":"");

        if (UserConfig.InitFullScreen) {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            if (w > OpenGL.WindowWidth) {
                float r = (1.f * height) / width;
                OpenGL.WindowWidth = w * r;
                OpenGL.WindowHeight = h;
                OpenGL.WindowOffset = (w - OpenGL.WindowWidth) >> 1;
                UserConfig.Resolution = OpenGL.WindowWidth;
            }
        }

        if (UserConfig.QEmu && UserConfig.VsyncOff)
            SDL_GL_SetSwapInterval(0);

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

void FinaliseOpenGLWindow( void)
{
    if ( ramp_stored )
        SetGammaTable(&old_ramp);
    if ( self_ctx ) {
        self_ctx = false;
        SDL_GL_MakeCurrent(window, NULL);
        SDL_GL_DeleteContext(context);
    }
    if ( self_wnd ) {
        self_wnd = false;
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, 0);
    }
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
    if (window)
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

void SwapBuffers()
{
    if (self_wnd) {
        SDL_Event e;
        while(SDL_PollEvent(&e));
    }
    SDL_GL_SwapWindow(window);
}

#endif // C_USE_SDL
