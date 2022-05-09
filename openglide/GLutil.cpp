//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Utility File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "wrapper_config.h"
#include "GlOgl.h"
#include "Glextensions.h"
#include "OGLTables.h"

#include "platform.h"
#include "platform/window.h"
#include "platform/clock.h"
#include "fgfont.h"

// Configuration Variables
ConfigStruct    UserConfig;
ConfigStruct    InternalConfig;

// Extern prototypes
extern unsigned long    NumberOfErrors;

// Functions

VARARGDECL(void) GlideMsg( const char *szString, ... )
{
    va_list( Arg );
    va_start( Arg, szString );

    FILE *fHandle = fopen( GLIDEFILE, "at" );
    if ( !fHandle )
    {
        return;
    }
    vfprintf( fHandle, szString, Arg );
    fflush( fHandle );
    fclose( fHandle );

    va_end( Arg );
}

VARARGDECL(void) Error( const char *szString, ... )
{
    va_list( Arg );
    va_start( Arg, szString );

    if ( NumberOfErrors == 0 )
    {
        GenerateErrorFile( );
    }

    FILE *fHandle = fopen( ERRORFILE, "at" );
    if ( ! fHandle )
    {
        return;
    }
    vfprintf( fHandle, szString, Arg );
    fflush( fHandle );
    fclose( fHandle );

    va_end( Arg );
    NumberOfErrors++;
}

void GLErro( const char *Funcao )
{
    GLenum Erro = glGetError( );

    if ( Erro != GL_NO_ERROR )
    {
        Error( "%s: OpenGLError = %s\n", Funcao, gluErrorString( Erro ) );
    }
}

void ConvertColorB( GrColor_t GlideColor, FxU8 &R, FxU8 &G, FxU8 &B, FxU8 &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        R = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        B = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        B = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        R = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        B = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        R = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;
    }
}

void ConvertColor4B( GrColor_t GlideColor, FxU32 &C )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        C = GlideColor;
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        C = ( ( GlideColor & 0xFF00FF00 ) |
              ( ( GlideColor & 0x00FF0000 ) >> 16 ) |
              ( ( GlideColor & 0x000000FF ) <<  16 ) );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        C = ( ( ( GlideColor & 0x00FFFFFF ) << 8 ) |
              ( ( GlideColor & 0xFF000000 ) >> 24 ) );
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        C = ( ( ( GlideColor & 0xFF000000 ) >> 24 ) |
              ( ( GlideColor & 0x00FF0000 ) >>  8 ) |
              ( ( GlideColor & 0x0000FF00 ) <<  8 ) |
              ( ( GlideColor & 0x000000FF ) << 24 ) );
        break;
    }
}

GrColor_t ConvertConstantColor( float R, float G, float B, float A )
{
    GrColor_t r = (GrColor_t) R;
    GrColor_t g = (GrColor_t) G;
    GrColor_t b = (GrColor_t) B;
    GrColor_t a = (GrColor_t) A;

    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        return ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        return ( r << 24 ) | ( g << 16 ) | ( b << 8 ) | a;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        return ( b << 24 ) | ( g << 16 ) | ( r << 8 ) | a;
    }

    return 0;
}

void ConvertColorF( GrColor_t GlideColor, float &R, float &G, float &B, float &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        A = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        A = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;
    }
}

//*************************************************
FxU32 GetTexSize( const int Lod, const int aspectRatio, const int format )
{
    /*
    ** If the format is one of these:
    ** GR_TEXFMT_RGB_332
    ** GR_TEXFMT_YIQ_422
    ** GR_TEXFMT_ALPHA_8
    ** GR_TEXFMT_INTENSITY_8
    ** GR_TEXFMT_ALPHA_INTENSITY_44
    ** GR_TEXFMT_P_8
    ** Reduces the size by 2
    */
    return nSquareLod[ format > GR_TEXFMT_RSVD1 ][ aspectRatio ][ Lod ];
}

static char * FindConfig( const char *IniFile, const char *IniConfig )
{
    // Cannot return pointer to local buffer, unless
    // static.
    static char Buffer1[ 256 ];
    char    * EqLocation, 
            * Find;
    FILE    * file;

    Find = NULL;
    file = fopen( IniFile, "r" );

    while ( fgets( Buffer1, 255, file ) != NULL )
    {
        if ( ( EqLocation = strchr( Buffer1, '=' ) ) != NULL )
        {       
            if ( !strncmp( Buffer1, IniConfig, EqLocation - Buffer1 ) )
            {
                Find = EqLocation + 1;
                if ( Find[ strlen( Find ) - 1 ] == '\n' )
                {
                    Find[ strlen( Find ) - 1 ] = '\0';
                }
                break;
            }
        }
    }

    fclose( file );

    return Find;
}

void GetOptions( void )
{
    FILE        * IniFile;
    char        * Pointer;
    char        Path[ 256 ];

    UserConfig.FogEnable                    = true;
    UserConfig.InitFullScreen               = false;
    UserConfig.PrecisionFix                 = true;
    UserConfig.CreateWindow                 = false;
    UserConfig.EnableMipMaps                = false;
    UserConfig.BuildMipMaps                 = false;
    UserConfig.IgnorePaletteChange          = false;
    UserConfig.ARB_multitexture             = true;
    UserConfig.EXT_paletted_texture         = true;
    UserConfig.EXT_texture_env_add          = false;
    UserConfig.EXT_texture_env_combine      = false;
    UserConfig.EXT_vertex_array             = false;
    UserConfig.EXT_fog_coord                = true;
    UserConfig.EXT_blend_func_separate      = false;
    UserConfig.Wrap565to5551                = true;
    UserConfig.FramebufferSRGB              = false;
    UserConfig.Annotate                     = false;

    UserConfig.Resolution                   = 0;

    UserConfig.TextureMemorySize            = 16;
    UserConfig.FrameBufferMemorySize        = 8;
    UserConfig.SamplesMSAA                  = 0;

    UserConfig.Priority                     = 2;

    // Maintain existing behaviour
    UserConfig.NoSplash                     = true;
    UserConfig.ShamelessPlug                = false;

    strcpy( Path, INIFILE );

    GlideMsg( "Configuration file is %s\n", Path );
    
    if ( access( Path, 0 ) == -1 )
    {
        IniFile = fopen( Path, "w" );
        fprintf( IniFile, "Configuration File for OpenGLide\n\n" );
        fprintf( IniFile, "Info:\n" );
        fprintf( IniFile, "Priority goes from 0(HIGH) to 5(IDLE)\n" );
        fprintf( IniFile, "Output resolution: 0 = original, 1.0-16.0 = scale factor, >16 = fixed width (height calculated automatically)\n" );
        fprintf( IniFile, "Texture Memory goes from %d to %d\n", OGL_MIN_TEXTURE_BUFFER, OGL_MAX_TEXTURE_BUFFER );
        fprintf( IniFile, "Frame Buffer Memory goes from %d to %d\n", OGL_MIN_FRAME_BUFFER, OGL_MAX_FRAME_BUFFER );
        fprintf( IniFile, "All other fields are boolean with 1(TRUE) and 0(FALSE)\n\n" );
        fprintf( IniFile, "Version=%s\n\n", OpenGLideVersion );
        fprintf( IniFile, "[Options]\n" );
        fprintf( IniFile, "WrapperPriority=%d\n", UserConfig.Priority );
        fprintf( IniFile, "CreateWindow=%d\n", UserConfig.CreateWindow );
        fprintf( IniFile, "InitFullScreen=%d\n", UserConfig.InitFullScreen );
        fprintf( IniFile, "Resolution=%.1f\n", UserConfig.Resolution );
        fprintf( IniFile, "FogEnable=%d\n", UserConfig.FogEnable );
        fprintf( IniFile, "EnableMipMaps=%d\n", UserConfig.EnableMipMaps );
        fprintf( IniFile, "IgnorePaletteChange=%d\n", UserConfig.IgnorePaletteChange );
        fprintf( IniFile, "Wrap565to5551=%d\n", UserConfig.Wrap565to5551 );
        fprintf( IniFile, "FramebufferSRGB=%d\n", UserConfig.FramebufferSRGB );
        fprintf( IniFile, "EnablePrecisionFix=%d\n", UserConfig.PrecisionFix );
        fprintf( IniFile, "EnableMultiTextureEXT=%d\n", UserConfig.ARB_multitexture );
        fprintf( IniFile, "EnablePaletteEXT=%d\n", UserConfig.EXT_paletted_texture );
        fprintf( IniFile, "EnableVertexArrayEXT=%d\n", UserConfig.EXT_vertex_array );
        fprintf( IniFile, "TextureMemorySize=%d\n", UserConfig.TextureMemorySize );
        fprintf( IniFile, "FrameBufferMemorySize=%d\n", UserConfig.FrameBufferMemorySize );
        fprintf( IniFile, "SamplesMSAA=%d\n", UserConfig.SamplesMSAA );
        fprintf( IniFile, "Annotate=%d\n", UserConfig.Annotate );
        fprintf( IniFile, "NoSplash=%d\n", UserConfig.NoSplash );
        fclose( IniFile );
    }
    else
    {
        Pointer = FindConfig( Path, "Version" );
        if ( Pointer && !strcmp( Pointer, OpenGLideVersion ) )
        {
            if ( (Pointer = FindConfig(Path, "CreateWindow")) )
                UserConfig.CreateWindow = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "InitFullScreen")) )
                UserConfig.InitFullScreen = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "Resolution")) )
                UserConfig.Resolution = atof( Pointer );
            if ( (Pointer = FindConfig(Path, "FogEnable")) )
                UserConfig.FogEnable = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "EnableMipMaps")) )
                UserConfig.EnableMipMaps = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "IgnorePaletteChange")) )
                UserConfig.IgnorePaletteChange = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "EnablePrecisionFix")) )
                UserConfig.PrecisionFix = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "EnableMultiTextureEXT")) )
                UserConfig.ARB_multitexture = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "EnablePaletteEXT")) )
                UserConfig.EXT_paletted_texture = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "EnableVertexArrayEXT")) )
                UserConfig.EXT_vertex_array = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "TextureMemorySize")) )
                UserConfig.TextureMemorySize = atoi( Pointer );
            if ( (Pointer = FindConfig(Path, "WrapperPriority")) )
                UserConfig.Priority = atoi( Pointer );
            if ( (Pointer = FindConfig(Path, "Wrap565to5551")) )
                UserConfig.Wrap565to5551 = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "FramebufferSRGB")) )
                UserConfig.FramebufferSRGB = atoi( Pointer ) ? true : false;
            if ( (Pointer = FindConfig(Path, "FrameBufferMemorySize")) )
                UserConfig.FrameBufferMemorySize = atoi( Pointer );
            if ( (Pointer = FindConfig(Path, "SamplesMSAA")) )
                UserConfig.SamplesMSAA = atoi( Pointer );
            if ( (Pointer = FindConfig(Path, "Annotate")) )
                UserConfig.Annotate = atoi( Pointer ) ? true : false;;
            if ( (Pointer = FindConfig(Path, "NoSplash")) )
                UserConfig.NoSplash = atoi( Pointer ) ? true : false;;
            if ( (Pointer = FindConfig(Path, "ShamelessPlug")) )
                UserConfig.ShamelessPlug = atoi( Pointer ) ? true : false;;
        }
        else
        {
            remove( Path );
            GetOptions( );
        }
    }
}

static struct {
    uint64_t last;
    uint32_t fcount;
    float ftime;
    int base;
} fxstats;

static void fgFontGenList(int first, int count, uint32_t listBase)
{
    const SFG_Font *font = &fgFontFixed8x13;
    int org_alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &org_alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = first; i < (first + count); i++) {
        const unsigned char *face = font->Characters[i];
        glNewList(listBase++, GL_COMPILE);
        glBitmap(
            face[ 0 ], font->Height,
            font->xorig, font->yorig,
            ( float )( face [ 0 ] ), 0.0,
            ( face + 1 )
        );
        glEndList();
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, org_alignment);
}

static void drawstr(const char *str, const int colors)
{
    glPushMatrix();
    glPushAttrib(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT |
        GL_STENCIL_BUFFER_BIT |
        GL_LIGHTING_BIT |
        GL_SCISSOR_BIT |
        GL_TEXTURE_BIT |
        GL_TRANSFORM_BIT |
        GL_VIEWPORT_BIT |
        GL_CURRENT_BIT);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, OpenGL.WindowWidth, 0, OpenGL.WindowHeight);

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_SCISSOR_TEST);
    glScissor((OpenGL.WindowOffset+11), 6, 11 + (8 * strlen(str)), (6 + 9));
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glListBase(fxstats.base);
    glColor3ubv((const GLubyte *)&colors);
    glRasterPos2i(13, 8);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);

    glPopMatrix();
    glPopAttrib();
    glPopMatrix();
}

#ifndef NANOSECONDS_PER_SECOND
#define NANOSECONDS_PER_SECOND get_ticks_per_sec()
static uint64_t get_ticks_per_sec(void) { return 1000000000LL; }
#endif
static uint64_t get_ticks_monotonic(void)
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp))
        return -1;
    return (tp.tv_sec * NANOSECONDS_PER_SECOND) + tp.tv_nsec;

}
void annotate_last(void)
{
    if (fxstats.base)
        glDeleteLists(fxstats.base, 256);
    fxstats.base = 0;
    fxstats.fcount = 0;
    fxstats.last = 0;
}

void annotate_stat(void)
{
    static char stats_line[] = "xxxx frames in xxx.x seconds xxx.x FPS";
    uint64_t curr;
    int i;

    if (UserConfig.Annotate) {
        if (fxstats.last == 0) {
            if (fxstats.fcount == 0) {
                fxstats.base = glGenLists(256);
                fgFontGenList(0, 255, fxstats.base);
                snprintf(stats_line, sizeof(stats_line), "%s", "Init ...");
            }
            fxstats.fcount = 0;
            fxstats.ftime = 0;
            fxstats.last = get_ticks_monotonic();
            return;
        }
        curr = get_ticks_monotonic();
        fxstats.fcount++;
        fxstats.ftime += (curr - fxstats.last) * (1.f / NANOSECONDS_PER_SECOND);
        fxstats.last = curr;
        i = (int)fxstats.ftime;
        if (i && ((i % 5) == 0)) {
            fxstats.last = 0;
            snprintf(stats_line, sizeof(stats_line), "%-4u frames in %-4.1f seconds %-5.1f FPS",
                fxstats.fcount, fxstats.ftime, fxstats.fcount / fxstats.ftime);
        }
        drawstr(stats_line, 0x00FFFFFF);
    }
}

FX_ENTRY void FX_CALL setConfig(FxU32 flags, void *magic)
{
    if (magic) {
#ifdef C_USE_SDL
        uint32_t *SignSDL = (uint32_t *)magic;
        *SignSDL = (*SignSDL == 0x58326724 /*'$g2X'*/)?
            0x324c4453 /*'SDL2'*/:0;
#endif
    }
    UserConfig.EnableMipMaps = (UserConfig.EnableMipMaps == 0)?
        ((flags & WRAPPER_FLAG_MIPMAPS) != 0):UserConfig.EnableMipMaps;
    UserConfig.FramebufferSRGB = (UserConfig.FramebufferSRGB == 0)?
        ((flags & WRAPPER_FLAG_FRAMEBUFFER_SRGB) != 0):UserConfig.FramebufferSRGB;
    UserConfig.SamplesMSAA = (UserConfig.SamplesMSAA == 0)?
        ((flags & WRAPPER_FLAG_MSAA_MASK)? (1 << ((flags & WRAPPER_FLAG_MSAA_MASK) >> 2)):0):UserConfig.SamplesMSAA;
    UserConfig.Annotate = (UserConfig.Annotate == 0)?
        ((flags & WRAPPER_FLAG_ANNOTATE) != 0):UserConfig.Annotate;
    UserConfig.VsyncOff = ((flags & WRAPPER_FLAG_VSYNCOFF) != 0);
    UserConfig.QEmu = ((flags & WRAPPER_FLAG_QEMU) != 0);
    UserConfig.InitFullScreen = (flags & WRAPPER_FLAG_WINDOWED)? false:true;
}

FX_ENTRY void FX_CALL setConfigRes(int res)
{
    UserConfig.Resolution = (float)res;
}

bool ClearAndGenerateLogFile( void )
{
    FILE    * GlideFile;
    char    tmpbuf[ 128 ];

    remove( ERRORFILE );
    GlideFile = fopen( GLIDEFILE, "w" );
    if ( ! GlideFile )
    {
        return false;
    }
    fclose( GlideFile );

    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "OpenGLide Log File\n" );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "***** OpenGLide %s *****\n", OpenGLideVersion );
    GlideMsg( OGL_LOG_SEPARATE );
    _strdate( tmpbuf );
    GlideMsg( "Date: %s\n", tmpbuf );
    _strtime( tmpbuf );
    GlideMsg( "Time: %s\n", tmpbuf );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( OGL_LOG_SEPARATE );
    ClockFreq = ClockFrequency( );
    GlideMsg( "Clock Frequency: %-4.2f Mhz\n", ClockFreq / 1000000.0f );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( OGL_LOG_SEPARATE );

    return true;
}

void CloseLogFile( void )
{
    char tmpbuf[ 128 ];
    GlideMsg( OGL_LOG_SEPARATE );
    _strtime( tmpbuf );
    GlideMsg( "Time: %s\n", tmpbuf );
    GlideMsg( OGL_LOG_SEPARATE );

#ifdef OGL_DEBUG
    Fps = (float) Frame * ClockFreq / FpsAux;
    GlideMsg( "FPS = %f\n", Fps );
    GlideMsg( OGL_LOG_SEPARATE );
#endif
}

bool GenerateErrorFile( void )
{
    char    tmpbuf[ 128 ];
    FILE    * ErrorFile;

    ErrorFile = fopen( ERRORFILE, "w");
    if( !ErrorFile )
    {
        return false;
    }
    fclose( ErrorFile );

    NumberOfErrors++;
    Error(  OGL_LOG_SEPARATE );
    Error(  "OpenGLide Error File\n");
    Error(  OGL_LOG_SEPARATE );
    _strdate( tmpbuf );
    Error(  "Date: %s\n", tmpbuf );
    _strtime( tmpbuf );
    Error(  "Time: %s\n", tmpbuf );
    Error(  OGL_LOG_SEPARATE );
    Error(  OGL_LOG_SEPARATE );

    return true;
}

