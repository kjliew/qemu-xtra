//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*   Windows specific functions for handling display window
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined C_USE_SDL && defined WIN32

#include <windows.h>
#include <io.h>
#include <math.h>

#include "GlOgl.h"

#include "platform/window.h"

static HDC   hDC;
static HGLRC hRC;
static HWND  hWND;
static struct
{
    FxU16 red[ 256 ];
    FxU16 green[ 256 ];
    FxU16 blue[ 256 ];
} old_ramp;

static BOOL ramp_stored  = false;
static BOOL mode_changed = false;

#define DPRINTF(fmt, ...)
static LONG WINAPI GlideWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {
	case WM_MOUSEACTIVATE:
	    return MA_NOACTIVATEANDEAT;
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
	case WM_NCLBUTTONDOWN:
	    return 0;
	default:
	    break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static HWND CreateGlideWindow(const char *title, int w, int h)
{
    HWND 	hWnd;
    WNDCLASS 	wc;
    static HINSTANCE hInstance = 0;

    if (!hInstance) {
	memset(&wc, 0, sizeof(WNDCLASS));
	hInstance = GetModuleHandle(NULL);
	wc.style	= CS_OWNDC;
	wc.lpfnWndProc	= (WNDPROC)GlideWndProc;
	wc.lpszClassName = "GlideWnd";

	if (!RegisterClass(&wc)) {
	    DPRINTF("RegisterClass() faled, Error %08lx\n", GetLastError());
	    return NULL;
	}
    }
    
    RECT rect;
    rect.top = 0; rect.left = 0;
    rect.right = w; rect.bottom = h;
    AdjustWindowRectEx(&rect, WS_CAPTION, FALSE, 0);
    rect.right  -= rect.left;
    rect.bottom -= rect.top;
    hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE,
	    "GlideWnd", title, 
	    WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
	    CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom,
	    NULL, NULL, hInstance, NULL);
    GetClientRect(hWnd, &rect);
    DPRINTF("    window %lux%lu\n", rect.right, rect.bottom);
    ShowCursor(FALSE);
    ShowWindow(hWnd, SW_SHOW);

    return hWnd;
}

bool InitialiseOpenGLWindow(FxU wnd, int x, int y, int width, int height)
{
    PIXELFORMATDESCRIPTOR   pfd;
    int                     PixFormat;
    HWND                    hwnd = (HWND) wnd;

    if( hwnd == NULL )
    {
        hwnd = GetActiveWindow();
    }

    if ( hwnd == NULL )
    {
        hwnd = CreateGlideWindow("GlideWnd", width, height);
        if ( hwnd == NULL ) {
            MessageBox( NULL, "Failed to create window", "Error", MB_OK );
            exit( 1 );
        }
    }

    mode_changed = false;

    if ( UserConfig.InitFullScreen )
    {
        if (!UserConfig.QEmu) {
            SetWindowLong( hwnd, 
                           GWL_STYLE, 
                           WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
            MoveWindow( hwnd, 0, 0, width, height, false );
        }
        mode_changed = SetScreenMode( width, height );
    }
    else
    {
       if (!UserConfig.QEmu) {
           RECT rect, radj;
           int x_adj, y_adj;
           GetWindowRect(hwnd, &rect);
           GetClientRect(hwnd, &radj);
           x_adj = (rect.right - rect.left - radj.right) >> 1;
           y_adj = rect.bottom - rect.top - radj.bottom - x_adj;
           rect.right = rect.left + width;
           rect.bottom = rect.top + height;

           AdjustWindowRectEx( &rect, 
                               GetWindowLong( hwnd, GWL_STYLE ),
                               GetMenu( hwnd ) != NULL,
                               GetWindowLong( hwnd, GWL_EXSTYLE ) );
           MoveWindow( hwnd, 
                       x + rect.left + x_adj, y + rect.top + y_adj, 
                       x + ( rect.right - rect.left ),
                       y + ( rect.bottom - rect.top ),
                       true );
       }
    }

    hWND = hwnd;

    hDC = GetDC( hwnd );

    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize        = sizeof( pfd );
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cDepthBits   = 24;
    pfd.cAlphaBits   = 8;
    pfd.cStencilBits = 8;

    if ( !( PixFormat = GetPixelFormat(hDC))) {
        if ( !( PixFormat = ChoosePixelFormat( hDC, &pfd ) ) )
        {
            MessageBox( NULL, "ChoosePixelFormat() failed:  "
                        "Cannot find a suitable pixel format.", "Error", MB_OK );
            exit( 1 );
        } 

        // the window must have WS_CLIPCHILDREN and WS_CLIPSIBLINGS for this call to
        // work correctly, so we SHOULD set this attributes, not doing that yet
        if ( !SetPixelFormat( hDC, PixFormat, &pfd ) )
        {
            MessageBox( NULL, "SetPixelFormat() failed:  "
                        "Cannot set format specified.", "Error", MB_OK );
            exit( 1 );
        }
    }
    else
        SetPixelFormat( hDC, PixFormat, NULL );

    DescribePixelFormat( hDC, PixFormat, sizeof( PIXELFORMATDESCRIPTOR ), &pfd );
    GlideMsg( "ColorBits	= %d\n", pfd.cColorBits );
    GlideMsg( "DepthBits	= %d\n", pfd.cDepthBits );

    if ( pfd.cDepthBits > 16 )
    {
        UserConfig.PrecisionFix = false;
    }

    hRC = wglCreateContext( hDC );
    wglMakeCurrent( hDC, hRC );

    if (UserConfig.QEmu && UserConfig.VsyncOff) {
        const int swapInterval = 0;
        int (WINAPI *SwapIntervalEXT)(int) = (int (WINAPI *)(int))
            wglGetProcAddress("wglSwapIntervalEXT");
        if (SwapIntervalEXT)
            SwapIntervalEXT(swapInterval);
    }

    // ramp_stored = GetDeviceGammaRamp( pDC, &old_ramp );
    for (int i = 0; i < 0x100; i++) {
        old_ramp.red[i]   = (FxU16)(((i << 8) | i) & 0xFFFFU);
        old_ramp.green[i] = (FxU16)(((i << 8) | i) & 0xFFFFU);
        old_ramp.blue[i]  = (FxU16)(((i << 8) | i) & 0xFFFFU);
    }
    ramp_stored = true;

    return true;
}

void FinaliseOpenGLWindow( void)
{
    if ( ramp_stored )
        SetGammaTable(&old_ramp);

    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWND, hDC );

    if( mode_changed )
    {
        ResetScreenMode( );
    }
}

void SetGamma(float value)
{
    struct
    {
        WORD red[256];
        WORD green[256];
        WORD blue[256];
    } ramp;

    for ( int i = 0; i < 256; i++ )
    {
        WORD v = (WORD)( 0xffff * pow( i / 255.0, 1.0 / value ) );

        ramp.red[ i ] = ramp.green[ i ] = ramp.blue[ i ] = ( v & 0xff00 );
    }

    SetGammaTable(&ramp);
}

void RestoreGamma()
{
}

void SetGammaTable(void *ptbl)
{
    HDC pDC = GetDC(NULL);
    BOOL (WINAPI *SetGammaExt)(HDC, LPVOID) = (BOOL (WINAPI *)(HDC, LPVOID))
        wglGetProcAddress("wglSetDeviceGammaRamp3DFX");
    if (SetGammaExt)
        SetGammaExt( pDC, ptbl );
    else
    SetDeviceGammaRamp( pDC, ptbl );
    ReleaseDC( NULL, pDC );
}

void GetGammaTable(void *ptbl)
{
    HDC pDC = GetDC(NULL);
    BOOL (WINAPI *GetGammaExt)(HDC, LPVOID) = (BOOL (WINAPI *)(HDC, LPVOID))
        wglGetProcAddress("wglGetDeviceGammaRamp3DFX");
    if (GetGammaExt)
        GetGammaExt( pDC, ptbl );
    else
    GetDeviceGammaRamp( pDC, ptbl );
    ReleaseDC( NULL, pDC );
}

bool SetScreenMode(int &xsize, int &ysize)
{
    HDC     hdc;
    FxU32   bits_per_pixel;
    bool    found;
    DEVMODE DevMode;

    hdc = GetDC( hWND );
    bits_per_pixel = GetDeviceCaps( hdc, BITSPIXEL );
    ReleaseDC( hWND, hdc );
    
    found = false;
    DevMode.dmSize = sizeof( DEVMODE );
    
    for ( int i = 0; 
          !found && EnumDisplaySettings( NULL, i, &DevMode ) != false; 
          i++ )
    {
        if ( ( DevMode.dmPelsWidth == (FxU32)xsize ) && 
             ( DevMode.dmPelsHeight == (FxU32)ysize ) && 
             ( DevMode.dmBitsPerPel == bits_per_pixel ) )
        {
            found = true;
        }
    }
    
    return ( found && ChangeDisplaySettings( &DevMode, CDS_RESET|CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL );
}

void ResetScreenMode()
{
    ChangeDisplaySettings( NULL, 0 );
}

void SwapBuffers()
{
    SwapBuffers(hDC);
}

#endif // !C_USE_SDL && WIN32
