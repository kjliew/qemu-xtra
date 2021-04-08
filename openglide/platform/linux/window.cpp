//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*    Linux specific functions for handling display window
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined C_USE_SDL && !defined WIN32

#include <math.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>

#include "GlOgl.h"

#include "platform/openglext.h"
#include "platform/window.h"

#define _strtime(s) {time_t t = time(0); strftime(s, 99, "%H:%M:%S", localtime (&t));}
#define _strdate(s) {time_t t = time(0); strftime(s, 99, "%d %b %Y", localtime (&t));}
#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

typedef enum {bmCopy = 0, bmAux, bmExchange} BufferMethod;

static Display                  *dpy          = NULL;
static int                       scrnum;
static Colormap                  cmap;
static Window                    win;
static XVisualInfo              *xvi          = NULL;
static GLXContext                ctx          = NULL;
static bool                      vidmode_ext  = false;
static XF86VidModeModeInfo     **vidmodes;
static bool                      mode_changed = false;
static bool                      keep_win     = false;
static GLfloat                  *aux_buffer;
static BufferMethod              buffer_method;
static const char               *xstr;

static std::vector<unsigned short> gammaRamp;
static std::vector<XColor>         xcolors;

static int find_xstr(const char *xstr, const char *str)
{
    int ret = 0;
    char sbuf[1024], *stok;
    if (xstr)
        strncpy(sbuf, xstr, 1024);
    else
        memset(sbuf, 0, 1024);
    stok = strtok(sbuf, " ");
    while (stok) {
        if (!strncmp(stok, str, strnlen(str, 64))) {
            ret = 1;
            break;
        }
        stok = strtok(NULL, " ");
    }
    return ret;
}

bool OGLIsExtensionSupported( const char * extension );

bool InitialiseOpenGLWindow(FxU wnd, int x, int y, int width, int height)
{
    Window root;
    XVisualInfo *visinfo = 0;
    XSetWindowAttributes attr;
    unsigned long mask;

    if (!(dpy = XOpenDisplay(NULL)))
    {
        fprintf(stderr, "Error couldn't open the X display\n");
        return false;
    }

    scrnum = DefaultScreen(dpy);
    win = (Window)wnd;
    root = RootWindow(dpy, scrnum);

    buffer_method = bmCopy;
    xstr = glXQueryExtensionsString(dpy, scrnum);

#if defined(GLX_VERSION_1_3) && defined(GLX_OML_swap_method)
    // Experiment with GLX 1.3 and the GLX_OML_swap_method extension
    // Unable to verify operation as not supported by my video card
    // If supported glXSwapBuffer can be called with no copying required
    {
        typedef GLXFBConfig * (*GLXCHOOSEFBCONFIGPROC) (Display *dpy, int screen, const int *attrib_list, int *nelements);
        typedef XVisualInfo * (*GLXGETVISUALFROMFBCONFIGPROC) (Display *dpy, GLXFBConfig config);

        GLXCHOOSEFBCONFIGPROC glXChooseFBConfig =
            (GLXCHOOSEFBCONFIGPROC) OGLGetProcAddress ("glXChooseFBConfig");
        GLXGETVISUALFROMFBCONFIGPROC glXGetVisualFromFBConfig =
            (GLXGETVISUALFROMFBCONFIGPROC) OGLGetProcAddress ("glXGetVisualFromFBConfig");

        if (glXChooseFBConfig && glXGetVisualFromFBConfig)
        {
            static const int attrib[] =
            {
                  GLX_X_RENDERABLE    , True,
                  GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
                  GLX_RENDER_TYPE     , GLX_RGBA_BIT,
                  GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
                  GLX_RED_SIZE        , 8,
                  GLX_GREEN_SIZE      , 8,
                  GLX_BLUE_SIZE       , 8,
                  GLX_DEPTH_SIZE      , 24,
                  GLX_STENCIL_SIZE    , 8,
                  GLX_DOUBLEBUFFER    , True,
                  None
            };

            int oml_swap, fbid, elements;
            GLXFBConfig *fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), attrib, &elements);
            if (fbc && elements)
            {
                static const char *swapMethod[] = {
                    "swapNone", "swapXChg", "swapCpy", "swapUndef"
                };
                if (find_xstr(xstr, "GLX_OML_swap_method"))
                    glXGetFBConfigAttrib(dpy, *fbc, GLX_SWAP_METHOD_OML, &oml_swap);
                else
                    oml_swap = 0;
                glXGetFBConfigAttrib(dpy, *fbc, GLX_FBCONFIG_ID, &fbid);
                visinfo = glXGetVisualFromFBConfig(dpy, *fbc);
                if (visinfo) {
                    buffer_method = (oml_swap == GLX_SWAP_COPY_OML)? bmCopy:bmExchange;
                    fprintf(stderr, "    FBConfig id 0x%03x visual 0x%03lx %s\n", fbid,
                        visinfo->visualid, swapMethod[(oml_swap & 0x3)]);
                }
            }
            XFree(fbc);
        }
    }
#endif

    if (!visinfo)
    {
        int attrib[] =
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_DEPTH_SIZE, DefaultDepth(dpy, scrnum),
            None
        };
        visinfo = glXChooseVisual(dpy, scrnum, attrib);
//        static XVisualInfo v;
//        XMatchVisualInfo(dpy, scrnum, DefaultDepth(dpy, scrnum), DirectColor, &v);
//        visinfo = &v;
    }

    if (!visinfo)
    {
        fprintf(stderr, "Error couldn't get an RGB, Double-buffered, Depth visual\n");
        return false;
    }

    {   // Determine presence of video mode extension
        int major = 0, minor = 0;
        vidmode_ext = XF86VidModeQueryVersion (dpy, &major, &minor) != 0;
    }
        
    if (vidmode_ext && UserConfig.InitFullScreen)
        mode_changed = SetScreenMode( width, height );

    try
    {
        int size;
        if (!win && (visinfo->c_class == DirectColor))
        {   // If DirectColor we can use colormaps instead
            xcolors.resize (visinfo->colormap_size);
        }
        else if (XF86VidModeGetGammaRampSize(dpy, scrnum, &size))
        {
            gammaRamp.resize (size * 6);
            unsigned short *red   = &gammaRamp[0];
            unsigned short *green = red   + size;
            unsigned short *blue  = green + size;

            if (!XF86VidModeGetGammaRamp(dpy, scrnum, size, red, green, blue))
                gammaRamp.clear ();

            switch (size) {
                case 0x100:
                for (int i = 0; i < size; i++) {
                    *red++   = (unsigned short)(((i << 8) | i) & 0xFFFFU);
                    *green++ = (unsigned short)(((i << 8) | i) & 0xFFFFU);
                    *blue++  = (unsigned short)(((i << 8) | i) & 0xFFFFU);
                }
                break;
                case 0x400:
                for (int i = 0; i < size; i++) {
                    *red++   = (unsigned short)(((i << 6) | (((i << 6) & 0xFC00U) >> 10)) & 0xFFFFU);
                    *green++ = (unsigned short)(((i << 6) | (((i << 6) & 0xFC00U) >> 10)) & 0xFFFFU);
                    *blue++  = (unsigned short)(((i << 6) | (((i << 6) & 0xFC00U) >> 10)) & 0xFFFFU);
                }
                break;
                case 0x800:
                for (int i = 0; i < size; i++) {
                    *red++   = (unsigned short)(((i << 5) | (((i << 5) & 0xF800U) >> 11)) & 0xFFFFU);
                    *green++ = (unsigned short)(((i << 5) | (((i << 5) & 0xF800U) >> 11)) & 0xFFFFU);
                    *blue++  = (unsigned short)(((i << 5) | (((i << 5) & 0xF800U) >> 11)) & 0xFFFFU);
                }
                break;
            }
        }
    }
    catch (...)
    {
    }

    if (win) {
        keep_win = true;
    }
    else {
	if (!xcolors.size())
	    cmap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	else
	{
	    cmap = XCreateColormap(dpy, root, visinfo->visual, AllocAll);
	    SetGamma (1.0f);
	}

	// window attributes
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = cmap;
	attr.event_mask = X_MASK;
	if (mode_changed)
	{
	    mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore | 
		   CWEventMask | CWOverrideRedirect;
	    attr.override_redirect = True;
	    attr.backing_store = NotUseful;
	    attr.save_under = False;
	}
	else
	    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow(dpy, root, 0, 0, width, height,
			    0, visinfo->depth, InputOutput,
			    visinfo->visual, mask, &attr);
	XMapWindow(dpy, win);
    }

    if (mode_changed)
    {
        XMoveWindow(dpy, win, 0, 0);
        XRaiseWindow(dpy, win);
        XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
        XFlush(dpy);
        // Move the viewport to top left
        XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
    }

    xvi = visinfo;

    XFlush(dpy);

    ctx = glXCreateContext(dpy, visinfo, NULL, True);

    glXMakeCurrent(dpy, win, ctx);

    if (UserConfig.QEmu && UserConfig.VsyncOff) {
        union {
            void (*glXProc)(Display *, GLXDrawable, int);
            void (*MesaProc)(int);
        } SwapIntervalEXT;
        const int swapInterval = 0;
        if (find_xstr(xstr, "GLX_MESA_swap_control")) {
            SwapIntervalEXT.MesaProc = (void (*)(int))
                OGLGetProcAddress("glXSwapIntervalMESA");
            if (SwapIntervalEXT.MesaProc) {
                SwapIntervalEXT.MesaProc(swapInterval);
                fprintf(stderr, "    Use MESA SwapIntervalEXT(%d)\n", swapInterval);
            }
        }
        else if (find_xstr(xstr, "GLX_EXT_swap_control")) {
            SwapIntervalEXT.glXProc = (void (*)(Display *, GLXDrawable, int))
                OGLGetProcAddress("glXSwapIntervalEXT");
            if (SwapIntervalEXT.glXProc) {
                SwapIntervalEXT.glXProc(dpy, win, swapInterval);
                fprintf(stderr, "    Use GLX SwapIntervalEXT(%d)\n", swapInterval);
            }
        }
    }

    aux_buffer = 0;
    if (buffer_method == bmCopy)
    {
        GLint buffers = 0;
        glGetIntegerv(GL_AUX_BUFFERS, &buffers);

        if (buffers)
            buffer_method = bmAux;
        else
            aux_buffer = (GLfloat*) malloc (sizeof(*aux_buffer) * width * height * 3/*RGB*/);
    }

    UserConfig.PrecisionFix = false;
    return true;
}

void FinaliseOpenGLWindow( void)
{
    if (dpy)
    {
        RestoreGamma ();
        if (ctx)
            glXDestroyContext(dpy, ctx);
        if (!keep_win)
            XDestroyWindow(dpy, win);
	ResetScreenMode( );
        XFree(xvi);
	XCloseDisplay(dpy);
    }
    ctx = NULL;
    dpy = NULL;
    win = 0;
    gammaRamp.clear ();
    xcolors.clear ();
}

void SetGamma(float value)
{
    double gamma = value;

    // This affects all windows
    if (gammaRamp.size())
    {
        int    size = (int)(gammaRamp.size() / 6);
        double max  = (size - 1);
        unsigned short *red    = &gammaRamp[0];
        unsigned short *green  = red    + size;
        unsigned short *blue   = green  + size;
        unsigned short *ramp_r = blue   + size;
        unsigned short *ramp_g = ramp_r + size;
        unsigned short *ramp_b = ramp_g + size;

        // Calculate the appropriate palette for the given gamma ramp
        for ( int i = 0; i < size; i++ )
        {   // Better represantation of glides 8 bit gamma (init/initvg/gamma.c : sst1InitGammaRGB)
            int v = (int)((max * pow( (double)i / max, 1.0 / gamma )) + 0.5);
            ramp_r[i] = red[v];
            ramp_g[i] = green[v];
            ramp_b[i] = blue[v];
        }
        XF86VidModeSetGammaRamp(dpy, scrnum, size, ramp_r, ramp_g, ramp_b);
    }
    else if (xcolors.size())
    {
        int    size = (int)(xcolors.size());
        double max  = (size - 1);

        // Private colormap.  Calculate the appropriate palette for the given gamma ramp
        for ( int i = 0; i < size; i++ )
        {
            unsigned short v =
                ((unsigned short)( (max * pow( (double)i / max, 1.0 / gamma )) + 0.5) << 8) | (unsigned short)i;
            xcolors[i].pixel = i << 16 | i << 8 | i;
            xcolors[i].red   = v;
            xcolors[i].green = v;
            xcolors[i].blue  = v;
            xcolors[i].flags = (DoRed|DoGreen|DoBlue);
        }
        XStoreColors(dpy, cmap, &xcolors[0], size);
        XSync(dpy, False);
    }
}

void RestoreGamma()
{
    if (gammaRamp.size())
    {
        int size = (int)(gammaRamp.size() / 6);
        unsigned short *red   = &gammaRamp[0];
        unsigned short *green = red   + size;
        unsigned short *blue  = green + size;
        XF86VidModeSetGammaRamp(dpy, scrnum, size, red, green, blue);
    }
}

void SetGammaTable(void *ptbl)
{
    int size = gammaRamp.size() / 6;
    struct s_ramp {
        unsigned short r[256];
        unsigned short g[256];
        unsigned short b[256];
    } *ramp_ptr = (struct s_ramp *) ptbl;
    unsigned short *red = &gammaRamp[0];
    unsigned short *green = red + size;
    unsigned short *blue = green + size;

    switch (size) {
        case 0x100:
            memcpy(red,   ramp_ptr->r, size);
            memcpy(green, ramp_ptr->g, size);
            memcpy(blue,  ramp_ptr->b, size);
            break;
        case 0x400:
            for (int i = 0; (i + 1) < 0x100; i++) {
                for (int j = 0; j < 4; j++) {
                      red[(i << 2) + j] = ramp_ptr->r[i] + (j * ((ramp_ptr->r[i + 1] - ramp_ptr->r[i]) >> 2));
                      red[(i << 2) + j] |=   (red[(i << 2) + j] & 0xFF00U) >> 8;
                    green[(i << 2) + j] = ramp_ptr->g[i] + (j * ((ramp_ptr->g[i + 1] - ramp_ptr->g[i]) >> 2));
                    green[(i << 2) + j] |= (green[(i << 2) + j] & 0xFF00U) >> 8;
                     blue[(i << 2) + j] = ramp_ptr->b[i] + (j * ((ramp_ptr->b[i + 1] - ramp_ptr->b[i]) >> 2));
                     blue[(i << 2) + j] |=  (blue[(i << 2) + j] & 0xFF00U) >> 8;
                }
            }
            for (int k = (size - 4); k < size; k++) {
                red[k]   = 0xFFFFU;
                green[k] = 0xFFFFU;
                blue[k]  = 0xFFFFU;
            }
            break;
        case 0x800:
            for (int i = 0; (i + 1) < 0x100; i++) {
                for (int j = 0; j < 8; j++) {
                      red[(i << 3) + j] = ramp_ptr->r[i] + (j * ((ramp_ptr->r[i + 1] - ramp_ptr->r[i]) >> 3));
                      red[(i << 3) + j] |=   (red[(i << 3) + j] & 0xFF00U) >> 8;
                    green[(i << 3) + j] = ramp_ptr->g[i] + (j * ((ramp_ptr->g[i + 1] - ramp_ptr->g[i]) >> 3));
                    green[(i << 3) + j] |= (green[(i << 3) + j] & 0xFF00U) >> 8;
                     blue[(i << 3) + j] = ramp_ptr->b[i] + (j * ((ramp_ptr->b[i + 1] - ramp_ptr->b[i]) >> 3));
                     blue[(i << 3) + j] |=  (blue[(i << 3) + j] & 0xFF00U) >> 8;
                }
            }
            for (int k = (size - 8); k < size; k++) {
                red[k]   = 0xFFFFU;
                green[k] = 0xFFFFU;
                blue[k]  = 0xFFFFU;
            }
            break;
        default:
            fprintf(stderr, "Unsupported gammaRamp.size() == %d\n", size);
            return;
    }
    XF86VidModeSetGammaRamp(dpy, scrnum, size, red, green, blue);
}

void GetGammaTable(void *ptbl)
{
    int size = gammaRamp.size() / 6;
    struct s_ramp {
        unsigned short r[256];
        unsigned short g[256];
        unsigned short b[256];
    } *ramp_ptr = (struct s_ramp *) ptbl;
    unsigned short *red = &gammaRamp[0];
    unsigned short *green = red + size;
    unsigned short *blue = green + size;
    if (size)
        XF86VidModeGetGammaRamp(dpy, scrnum, size, red, green, blue);

    switch (size) {
        case 0x100:
            memcpy(ramp_ptr->r,   red, size);
            memcpy(ramp_ptr->g, green, size);
            memcpy(ramp_ptr->b,  blue, size);
            break;
        case 0x400:
            for (int i = 0; i < 0x100; i ++) {
                ramp_ptr->r[i] = (  red[i << 2] & 0xFF00U) | i;
                ramp_ptr->g[i] = (green[i << 2] & 0xFF00U) | i;
                ramp_ptr->b[i] = ( blue[i << 2] & 0xFF00U) | i;
            }
            break;
        case 0x800:
            for (int i = 0; i < 0x100; i ++) {
                ramp_ptr->r[i] = (  red[i << 3] & 0xFF00U) | i;
                ramp_ptr->g[i] = (green[i << 3] & 0xFF00U) | i;
                ramp_ptr->b[i] = ( blue[i << 3] & 0xFF00U) | i;
            }
            break;
        default:
            fprintf(stderr, "Unsupported gammaRamp.size() == %d\n", size);
            break;
    }
}

bool SetScreenMode(int &width, int &height)
{
    int best_fit, best_dist, dist, x, y, i;
    int num_vidmodes;
    best_dist = 9999999;
    best_fit = -1;

    XF86VidModeGetAllModeLines(dpy, scrnum, &num_vidmodes, &vidmodes);

    for (i = 0; i < num_vidmodes; i++)
    {
        if (width > vidmodes[i]->hdisplay ||
            height > vidmodes[i]->vdisplay)
            continue;

        x = width - vidmodes[i]->hdisplay;
        y = height - vidmodes[i]->vdisplay;
        dist = (x * x) + (y * y);
        if (dist < best_dist)
        {
            best_dist = dist;
            best_fit = i;
        }
    }

    if (best_fit != -1)
    {
        width  = vidmodes[best_fit]->hdisplay;
        height = vidmodes[best_fit]->vdisplay;

        // change to the mode
        XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
        // Move the viewport to top left
        XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
        return true;
    }

    if (vidmodes)
    {
        XFree(vidmodes);
        vidmodes=0;
    }

    return false;
}

void ResetScreenMode()
{
    if (mode_changed)
    {
        XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[0]);
        if (vidmodes)
        {
            XFree(vidmodes);
            vidmodes=0;
        }
    }
    if (aux_buffer)
        free (aux_buffer);
}

void SwapBuffers()
{
    if (buffer_method == bmExchange)
    {
        glXSwapBuffers( dpy, win );
        return;
    }

    // What a pain.  Under Glide front/back buffers are swapped.
    // Under Linux GL copies the back to front buffer and the
    // back buffer becomes underfined.  So we have to copy the
    // front buffer manually to the back (probably noticable
    // performance hit).  NOTE the restored image looks quantised.
    // Verify the screen mode used...
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    
    if (buffer_method == bmAux)
    {
        glReadBuffer(GL_FRONT); 
        glDrawBuffer(GL_AUX0);
        glRasterPos2i(0, OpenGL.WindowHeight - 1);
        glCopyPixels(0, 0, OpenGL.WindowWidth, OpenGL.WindowHeight, GL_COLOR);
        glXSwapBuffers(dpy, win);
        glReadBuffer(GL_AUX0);
        glDrawBuffer(GL_BACK);
        glRasterPos2i(0, OpenGL.WindowHeight - 1);
        glCopyPixels(0, 0, OpenGL.WindowWidth, OpenGL.WindowHeight, GL_COLOR);
    }
    else if (buffer_method == bmCopy)
    {
        if (!aux_buffer) // For testing
            glXSwapBuffers( dpy, win );
        else
        {
            GLenum type = GL_FLOAT;

            glReadBuffer( GL_FRONT );
            glReadPixels( 0, 0, 
                          OpenGL.WindowWidth, OpenGL.WindowHeight,
                          GL_RGB, type, (void *)aux_buffer );

            glXSwapBuffers( dpy, win );

            glDrawBuffer( GL_BACK );
            glRasterPos2i(0, OpenGL.WindowHeight - 1);
            glDrawPixels( OpenGL.WindowWidth, OpenGL.WindowHeight, GL_RGB,
                          type, (void *)aux_buffer );
        }
    }

    if ( OpenGL.Blend )
        glEnable( GL_BLEND );
    glDrawBuffer( OpenGL.RenderBuffer );
}

#endif // !C_USE_SDL && !WIN32
