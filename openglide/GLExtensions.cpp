//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                    OpenGL Extensions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "GlOgl.h"
#include "GLRender.h"
#include "GLExtensions.h"

#include "platform/openglext.h"
#include "platform/error.h"

enum enExtensionType
{
    OGL_EXT_UNUSED = 0,
    OGL_EXT_REQUIRED,
    OGL_EXT_DESIRED,
};

struct stExtensionSupport
{
    const char *    name;
    enExtensionType type;
    bool *          userVar;
    bool *          internalVar;
};

// It is important that dummyExtVariable retains the value true, so
// we pass dummyExtVariable2 in places where the value may be altered.
bool dummyExtVariable = true;
bool dummyExtVariable2 = true;

stExtensionSupport glNecessaryExt[] =
{
#ifdef __darwin__
    { "GL_APPLE_packed_pixels",         OGL_EXT_REQUIRED,   &dummyExtVariable,                  &dummyExtVariable2 },
#else
    { "GL_EXT_packed_pixels",           OGL_EXT_REQUIRED,   &dummyExtVariable,                  &dummyExtVariable2 },
#endif
    { "GL_EXT_abgr",                    OGL_EXT_REQUIRED,   &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_bgra",                    OGL_EXT_REQUIRED,   &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_secondary_color",         OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_secondary_color },
    { "GL_ARB_multitexture",            OGL_EXT_DESIRED,    &UserConfig.ARB_multitexture,       &InternalConfig.ARB_multitexture },
    { "GL_EXT_fog_coord",               OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_fog_coord },
    { "GL_EXT_texture_env_add",         OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_texture_env_add },
#ifdef __darwin__
    { "GL_ARB_texture_env_combine",     OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_texture_env_combine },
#else
    { "GL_EXT_texture_env_combine",     OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_texture_env_combine },
#endif
    { "GL_EXT_texture_lod_bias",        OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_texture_lod_bias },
    { "GL_SGIS_generate_mipmap",        OGL_EXT_DESIRED,    &UserConfig.EnableMipMaps,          &InternalConfig.BuildMipMaps },
    { "GL_EXT_paletted_texture",        OGL_EXT_DESIRED,    &UserConfig.EXT_paletted_texture,   &InternalConfig.EXT_paletted_texture },
    { "GL_EXT_vertex_array",            OGL_EXT_DESIRED,    &UserConfig.EXT_vertex_array,       &InternalConfig.EXT_vertex_array },
    { "GL_EXT_blend_func_separate",     OGL_EXT_DESIRED,    &dummyExtVariable,                  &InternalConfig.EXT_blend_func_separate },
    { "GL_ARB_imaging",                 OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_ARB_texture_env_add",         OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_ARB_transpose_matrix",        OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_compiled_vertex_array",   OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_draw_range_elements",     OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_multi_draw_arrays",       OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_point_parameters",        OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_rescale_normal",          OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_separate_specular_color", OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_stencil_wrap",            OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_texture_edge_clamp",      OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_texture_object",          OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_EXT_vertex_weighting",        OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_IBM_texture_mirrored_repeat", OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_KTX_buffer_region",           OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_NV_blend_square",             OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_NV_evaluators",               OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_NV_fog_distance",             OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_NV_packed_depth_stencil",     OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_NV_texgen_reflection",        OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_NV_texture_env_combine4",     OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_SGIS_multitexture",           OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "GL_WIN_swap_hint",               OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "WGL_EXT_swap_control",           OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 },
    { "",                               OGL_EXT_UNUSED,     &dummyExtVariable,                  &dummyExtVariable2 }
}; 

//Functions
PFNGLCLIENTACTIVETEXTUREARBPROC         p_glClientActiveTexture = NULL;

PFNGLMULTITEXCOORD4FARBPROC             p_glMultiTexCoord4fARB = NULL;
PFNGLMULTITEXCOORD4FVARBPROC            p_glMultiTexCoord4fvARB = NULL;
PFNGLACTIVETEXTUREARBPROC               p_glActiveTextureARB = NULL;

PFNGLSECONDARYCOLOR3UBVEXTPROC          p_glSecondaryColor3ubvEXT = NULL;
PFNGLSECONDARYCOLOR3UBEXTPROC           p_glSecondaryColor3ubEXT = NULL;
PFNGLSECONDARYCOLOR3FVEXTPROC           p_glSecondaryColor3fvEXT = NULL;
PFNGLSECONDARYCOLOR3FEXTPROC            p_glSecondaryColor3fEXT = NULL;
PFNGLSECONDARYCOLORPOINTEREXTPROC       p_glSecondaryColorPointerEXT = NULL;

PFNGLFOGCOORDFEXTPROC                   p_glFogCoordfEXT = NULL;
PFNGLFOGCOORDPOINTEREXTPROC             p_glFogCoordPointerEXT = NULL;

PFNGLCOLORTABLEEXTPROC                  p_glColorTableEXT = NULL;
PFNGLCOLORSUBTABLEEXTPROC               p_glColorSubTableEXT = NULL;
PFNGLGETCOLORTABLEEXTPROC               p_glGetColorTableEXT = NULL;
PFNGLGETCOLORTABLEPARAMETERIVEXTPROC    p_glGetColorTableParameterivEXT = NULL;
PFNGLGETCOLORTABLEPARAMETERFVEXTPROC    p_glGetColorTableParameterfvEXT = NULL;

PFNGLBLENDFUNCSEPARATEEXTPROC           p_glBlendFuncSeparateEXT = NULL;

// Declarations
void GLExtensions( void );

void APIENTRY DummyV( const void *a )
{
}

void APIENTRY DummyF( GLfloat a )
{
}

void APIENTRY Dummy3ub( GLubyte a, GLubyte b, GLubyte c )
{
}


// check to see if Extension is Supported
// code by Mark J. Kilgard of NVidia modified by Fabio Barros
bool OGLIsExtensionSupported( const char * extension )
{
    const char  * extensions;
    const char  * start;
    char        * where, 
                * terminator;

    where = (char *) strchr( extension, ' ' );
    if ( where || ( *extension == '\0' ) )
    {
        return false;
    }

    extensions = (char*)glGetString( GL_EXTENSIONS );

    start = extensions;

    if ( *start == '\0' )
    {
        Error( "No OpenGL extension supported, using all emulated.\n" );
        return false;
    }

    while ( true )
    {
        where = (char *)strstr( start, extension );
        if ( !where )
        {
            break;
        }
        terminator = where + strlen( extension );
        if ( ( where == start ) || ( *( where - 1 ) == ' ' ) )
        {
            if ( ( *terminator == ' ' ) || ( *terminator == '\0' ) )
            {
                return true;
            }
        }
        start = terminator;
    }

    return false;
}

void ResetInternalConfig( void )
{
    InternalConfig.OGLVersion                   = 1;
    InternalConfig.Priority                     = 2;

    InternalConfig.FogEnable                    = false;
    InternalConfig.InitFullScreen               = false;
    InternalConfig.PrecisionFix                 = false;
    InternalConfig.EnableMipMaps                = false;
    InternalConfig.BuildMipMaps                 = false;
    InternalConfig.IgnorePaletteChange          = false;
    InternalConfig.Wrap565to5551                = false;
    InternalConfig.TextureEnv                   = false;

    InternalConfig.ARB_multitexture             = false;
    InternalConfig.EXT_paletted_texture         = false;
    InternalConfig.EXT_texture_env_add          = false;
    InternalConfig.EXT_texture_env_combine      = false;
    InternalConfig.EXT_vertex_array             = false;
    InternalConfig.EXT_fog_coord                = false;
    InternalConfig.EXT_blend_func_separate      = false;
    InternalConfig.EXT_texture_lod_bias         = false;
    InternalConfig.EXT_secondary_color          = false;

    InternalConfig.TextureMemorySize            = 16;
    InternalConfig.FrameBufferMemorySize        = 8;

    InternalConfig.CreateWindow                 = false;
}

void ValidateUserConfig( void )
{
    ResetInternalConfig( );

    InternalConfig.InitFullScreen               = UserConfig.InitFullScreen;
    InternalConfig.PrecisionFix                 = UserConfig.PrecisionFix;
    InternalConfig.EnableMipMaps                = UserConfig.EnableMipMaps;
    InternalConfig.IgnorePaletteChange          = UserConfig.IgnorePaletteChange;
    InternalConfig.Wrap565to5551                = UserConfig.Wrap565to5551;
    
    if ( ( UserConfig.TextureMemorySize >= OGL_MIN_TEXTURE_BUFFER ) && 
         ( UserConfig.TextureMemorySize <= OGL_MAX_TEXTURE_BUFFER ) )
    {
        InternalConfig.TextureMemorySize        = UserConfig.TextureMemorySize;
    }

    if ( ( UserConfig.FrameBufferMemorySize >= OGL_MIN_FRAME_BUFFER ) && 
         ( UserConfig.FrameBufferMemorySize <= OGL_MAX_FRAME_BUFFER ) )
    {
        InternalConfig.FrameBufferMemorySize    = UserConfig.FrameBufferMemorySize;
    }

    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "** OpenGL Information **\n" );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "Vendor:      %s\n", glGetString( GL_VENDOR ) );
    GlideMsg( "Renderer:    %s\n", glGetString( GL_RENDERER ) );
    GlideMsg( "Version:     %s\n", glGetString( GL_VERSION ) );
    GlideMsg( "Extensions:  %s\n", glGetString( GL_EXTENSIONS ) );

    GlideMsg( OGL_LOG_SEPARATE );

	int ver    = 0;
	int subver = 0;

        char *p = (char *)glGetString( GL_VERSION );
        ver = strtol(p, 0, 10);
        p = strchr(p, '.');
        subver = strtol(++p, 0, 10);
	//sscanf( (const char *)glGetString( GL_VERSION ), "%d.%d", &ver, &subver);

    InternalConfig.OGLVersion = ver * 100 + subver;

    GlideMsg( "OpenGL Extensions:\n" );
    GlideMsg( OGL_LOG_SEPARATE );

    int index = 0;
    while ( strlen( glNecessaryExt[ index ].name ) > 0 )
    {
        switch ( glNecessaryExt[ index ].type )
        {
        case OGL_EXT_REQUIRED:
            if ( ! OGLIsExtensionSupported( glNecessaryExt[ index ].name ) )
            {
                if (index || InternalConfig.OGLVersion < 102)
                {
                    char szError[ 256 ];
                    sprintf( szError, "Severe Problem: OpenGL %s extension is required for OpenGLide!",
                        glNecessaryExt[ index ].name );
                    Error( szError );
                    GlideMsg( szError );
                    ReportWarning( szError );
                }
            }
            break;

        case OGL_EXT_DESIRED:
            if ( !strcmp(glNecessaryExt[ index ].name, "GL_EXT_vertex_array") &&
                     InternalConfig.OGLVersion >= 101 )
            {
                if ( *glNecessaryExt[ index ].userVar )
                {
                    *glNecessaryExt[ index ].internalVar = true;
                    GlideMsg( "Extension %s is present and ENABLED\n", glNecessaryExt[ index ].name );
                }
                else
                {
                    char szError[ 256 ];
                    sprintf( szError, "Note: OpenGL %s extension is supported but disabled by user\n",
                        glNecessaryExt[ index ].name );
                    GlideMsg( szError );

                    *glNecessaryExt[ index ].internalVar = false;
                }
            }
            else if ( ! OGLIsExtensionSupported( glNecessaryExt[ index ].name ) )
            {
                char szError[ 256 ];
                sprintf( szError, "Note: OpenGL %s extension is not supported, emulating behavior.\n", 
                    glNecessaryExt[ index ].name );
                GlideMsg( szError );
            }
            else
            {
                if ( *glNecessaryExt[ index ].userVar )
                {
                    *glNecessaryExt[ index ].internalVar = true;
                    GlideMsg( "Extension %s is present and ENABLED\n", glNecessaryExt[ index ].name );
                }
                else
                {
                    char szError[ 256 ];
                    sprintf( szError, "Note: OpenGL %s extension is supported but disabled by user\n", 
                        glNecessaryExt[ index ].name );
                    GlideMsg( szError );

                    *glNecessaryExt[ index ].internalVar = false;
                }
            }
            break;

        case OGL_EXT_UNUSED:
            break;
        }
        ++index;
    }
    // The above loop leaves InternalConfig.BuildMipMaps set to the negation of its correct value.
    InternalConfig.BuildMipMaps = !InternalConfig.BuildMipMaps;

    GlideMsg( OGL_LOG_SEPARATE );

    if ( InternalConfig.EXT_texture_env_add  && 
         InternalConfig.EXT_texture_env_combine )
    {
        InternalConfig.TextureEnv   = true;
    }

    if ( InternalConfig.EXT_fog_coord )
    {
        InternalConfig.FogEnable    = (UserConfig.FogEnable)? true:false;
    }
    GLExtensions( );
}

void GLExtensions( void )
{
    GLint NumberOfTMUs;

    p_glActiveTextureARB      = NULL;
    p_glMultiTexCoord4fARB    = NULL;

    p_glSecondaryColor3ubvEXT = (PFNGLSECONDARYCOLOR3UBVEXTPROC) DummyV;
    p_glSecondaryColor3fvEXT  = (PFNGLSECONDARYCOLOR3FVEXTPROC) DummyV;
    p_glFogCoordfEXT          = (PFNGLFOGCOORDFEXTPROC) DummyF;

    if ( InternalConfig.ARB_multitexture )
    {
        glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &NumberOfTMUs );
        GlideMsg( "MultiTexture Textures Units = %x\n", NumberOfTMUs );

        OpenGL.MultiTextureTMUs     = NumberOfTMUs;
        p_glClientActiveTexture       = (PFNGLCLIENTACTIVETEXTUREARBPROC) OGLGetProcAddress( "glClientActiveTextureARB" );
        p_glActiveTextureARB          = (PFNGLACTIVETEXTUREARBPROC) OGLGetProcAddress( "glActiveTextureARB" );
        p_glMultiTexCoord4fARB        = (PFNGLMULTITEXCOORD4FARBPROC) OGLGetProcAddress( "glMultiTexCoord4fARB" );
        p_glMultiTexCoord4fvARB       = (PFNGLMULTITEXCOORD4FVARBPROC) OGLGetProcAddress( "glMultiTexCoord4fvARB" );

        if ( ( p_glActiveTextureARB == NULL ) || 
             ( p_glMultiTexCoord4fARB == NULL ) || 
             ( p_glMultiTexCoord4fvARB == NULL ) )
        {
            Error( "Could not get the address of MultiTexture functions!\n" );
            InternalConfig.ARB_multitexture = false;
        }
    }

    if ( InternalConfig.EXT_secondary_color )
    {
        p_glSecondaryColor3ubvEXT     = (PFNGLSECONDARYCOLOR3UBVEXTPROC) OGLGetProcAddress( "glSecondaryColor3ubvEXT" );
        p_glSecondaryColor3ubEXT      = (PFNGLSECONDARYCOLOR3UBEXTPROC) OGLGetProcAddress( "glSecondaryColor3ubEXT" );
        p_glSecondaryColor3fvEXT      = (PFNGLSECONDARYCOLOR3FVEXTPROC) OGLGetProcAddress( "glSecondaryColor3fvEXT" );
        p_glSecondaryColorPointerEXT  = (PFNGLSECONDARYCOLORPOINTEREXTPROC) OGLGetProcAddress( "glSecondaryColorPointerEXT" );
        if ( ( p_glSecondaryColor3ubvEXT == NULL ) || 
             ( p_glSecondaryColor3ubEXT == NULL )  || 
             ( p_glSecondaryColorPointerEXT == NULL ) || 
             ( p_glSecondaryColor3fvEXT == NULL ) )
        {
            Error( "Could not get address of function glSecondaryColorEXT.\n" );
            InternalConfig.EXT_secondary_color = false;
        }
        else
        {
            glEnable( GL_COLOR_SUM_EXT );
        }
    }

    if ( InternalConfig.EXT_fog_coord )
    {
        p_glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC) OGLGetProcAddress( "glFogCoordfEXT" );
        p_glFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC) OGLGetProcAddress( "glFogCoordPointerEXT" );
        if ( ( p_glFogCoordfEXT == NULL ) || 
             ( p_glFogCoordPointerEXT == NULL ) )
        {
            Error( "Could not get address of function glFogCoordEXT.\n" );
            InternalConfig.EXT_fog_coord = false;
        }
        else
        {
            glFogi( GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT );
            glFogi( GL_FOG_MODE, GL_LINEAR );
            glFogf( GL_FOG_START, 0.0f );
            glFogf( GL_FOG_END, 1.0f );
        }
    }

    if ( InternalConfig.EXT_vertex_array )
    {
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        if ( InternalConfig.EXT_secondary_color )
        {
            glEnableClientState( GL_SECONDARY_COLOR_ARRAY_EXT );
        }
        if ( InternalConfig.EXT_fog_coord )
        {
            glEnableClientState( GL_FOG_COORDINATE_ARRAY_EXT );
        }

        RenderUpdateArrays( );
    }

    if ( InternalConfig.EXT_paletted_texture )
    {
        p_glColorTableEXT                 = (PFNGLCOLORTABLEEXTPROC) OGLGetProcAddress( "glColorTableEXT" );
        p_glColorSubTableEXT              = (PFNGLCOLORSUBTABLEEXTPROC) OGLGetProcAddress( "glColorSubTableEXT" );
        p_glGetColorTableEXT              = (PFNGLGETCOLORTABLEEXTPROC) OGLGetProcAddress( "glGetColorTableEXT" );
        p_glGetColorTableParameterivEXT   = (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) OGLGetProcAddress( "glGetColorTableParameterivEXT" );
        p_glGetColorTableParameterfvEXT   = (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) OGLGetProcAddress( "glGetColorTableParameterfvEXT" );

        if ( ( p_glColorTableEXT == NULL ) || 
             ( p_glColorSubTableEXT == NULL ) || 
             ( p_glGetColorTableEXT == NULL ) || 
             ( p_glGetColorTableParameterivEXT == NULL ) || 
             ( p_glGetColorTableParameterfvEXT == NULL ) )
        {
            Error( "Could not get address of function for PaletteEXT.\n" );
            InternalConfig.EXT_paletted_texture = false;
        }
        else
        {
            GlideMsg( "Using Palette Extension.\n" );
        }
    }

#ifdef OPENGL_DEBUG
    GLErro( "GLExtensions" );
#endif
}
