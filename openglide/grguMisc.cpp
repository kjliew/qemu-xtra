//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Other Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include "GlOgl.h"
#include "GLRender.h"


//*************************************************
//* Sets the External Error Function to call if
//* Glides Generates and Error
//*************************************************
FX_ENTRY void FX_CALL
grErrorSetCallback( void (*function)(const char *string, FxBool fatal) )
{
#ifdef OGL_DONE
    GlideMsg( "grErrorSetCallback( --- )\n" );
#endif

    ExternErrorFunction = function;
}

//*************************************************
//* Sets the Cull Mode
//*************************************************
FX_ENTRY void FX_CALL
grCullMode( GrCullMode_t mode )
{
#ifdef OGL_DONE
    GlideMsg( "grCullMode( %d )\n", mode );
#endif
    if ( ! OpenGL.WinOpen )
    {
        return;
    }

    RenderDrawTriangles( );

    Glide.State.CullMode = mode;

    switch ( Glide.State.CullMode )
    {
    case GR_CULL_DISABLE:
        glDisable( GL_CULL_FACE );
        glCullFace( GL_BACK );  // This will be called in initialization
        break;

    case GR_CULL_NEGATIVE:
        glEnable( GL_CULL_FACE );
        if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
        {
            glFrontFace( GL_CCW );
        }
        else
        {
            glFrontFace( GL_CW );
        }
        break;

    case GR_CULL_POSITIVE:
        glEnable( GL_CULL_FACE );
        if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
        {
            glFrontFace( GL_CW );
        }
        else
        {
            glFrontFace( GL_CCW );
        }
        break;
    }

#ifdef OPENGL_DEBUG
    GLErro( "grCullMode" );
#endif
}

//*************************************************
//* Set the size and location of the hardware clipping window
//*************************************************
FX_ENTRY void FX_CALL 
grClipWindow( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grClipWindow( %d, %d, %d, %d )\n", minx, miny, maxx, maxy );
#endif

    RenderDrawTriangles( );

    Glide.State.ClipMinX = minx;
    Glide.State.ClipMaxX = maxx;
    Glide.State.ClipMinY = miny;
    Glide.State.ClipMaxY = maxy;
    // calculate the corresponding OpenGL coords
    // (the multiplication has to come first because the values are integers)
    OpenGL.ClipMinX = OpenGL.WindowWidth * minx / Glide.WindowWidth;
    OpenGL.ClipMaxX = OpenGL.WindowWidth * maxx / Glide.WindowWidth;
    OpenGL.ClipMinY = OpenGL.WindowHeight * miny / Glide.WindowHeight;
    OpenGL.ClipMaxY = OpenGL.WindowHeight * maxy / Glide.WindowHeight;
    OpenGL.ClipMinX += OpenGL.WindowOffset;
    OpenGL.ClipMaxX += OpenGL.WindowOffset;

    if ( ( Glide.State.ClipMinX != 0 ) || 
         ( Glide.State.ClipMinY != 0 ) ||
         ( Glide.State.ClipMaxX != (FxU32) Glide.WindowWidth ) ||
         ( Glide.State.ClipMaxY != (FxU32) Glide.WindowHeight ) )
    {
        OpenGL.Clipping = true;
    }
    else
    {
        OpenGL.Clipping = false;
    }

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
    {
        glOrtho( Glide.State.ClipMinX, Glide.State.ClipMaxX, 
                 Glide.State.ClipMinY, Glide.State.ClipMaxY, 
                 OpenGL.ZNear, OpenGL.ZFar );
        glViewport( OpenGL.ClipMinX, OpenGL.ClipMinY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
        // Used for the buffer clearing
        glScissor( OpenGL.ClipMinX, OpenGL.ClipMinY,
                   OpenGL.ClipMaxX - OpenGL.ClipMinX,
                   OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }
    else
    {
        glOrtho( Glide.State.ClipMinX, Glide.State.ClipMaxX, 
                 Glide.State.ClipMaxY, Glide.State.ClipMinY, 
                 OpenGL.ZNear, OpenGL.ZFar );
        glViewport( OpenGL.ClipMinX, OpenGL.WindowHeight - OpenGL.ClipMaxY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
        // Used for the buffer clearing
        glScissor( OpenGL.ClipMinX, OpenGL.WindowHeight - OpenGL.ClipMaxY,
                   OpenGL.ClipMaxX - OpenGL.ClipMinX,
                   OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }

    glMatrixMode( GL_MODELVIEW );
}

//*************************************************
FX_ENTRY void FX_CALL 
grDisableAllEffects( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grDisableAllEffects( )\n" );
#endif

    grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    grAlphaTestFunction( GR_CMP_ALWAYS );
    grChromakeyMode( GR_CHROMAKEY_DISABLE );
    grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
    grFogMode( GR_FOG_DISABLE );
}

//*************************************************
FX_ENTRY void FX_CALL
grResetTriStats( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grResetTriStats( )\n" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grTriStats( FxU32 *trisProcessed, FxU32 *trisDrawn )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTriStats( )\n" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grHints( GrHint_t hintType, FxU32 hintMask )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grHints( %d, %d )\n", hintType, hintMask );
#endif

    switch( hintType )
    {
    case GR_HINT_STWHINT:
        Glide.State.STWHint = hintMask;
        break;
    }
}

//*************************************************
/*
FX_ENTRY void FX_CALL
grSplash( float x, float y, float width, float height, FxU32 frame )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSplash( %-4.2f, %-4.2f, %-4.2f, %-4.2f, %lu )\n",
        x, y, width, height, frame );
#endif
}
*/

//*************************************************
FX_ENTRY void FX_CALL 
ConvertAndDownloadRle( GrChipID_t        tmu,
                       FxU32             startAddress,
                       GrLOD_t           thisLod,
                       GrLOD_t           largeLod,
                       GrAspectRatio_t   aspectRatio,
                       GrTextureFormat_t format,
                       FxU32             evenOdd,
                       FxU8              *bm_data,
                       long              bm_h,
                       FxU32             u0,
                       FxU32             v0,
                       FxU32             width,
                       FxU32             height,
                       FxU32             dest_width,
                       FxU32             dest_height,
                       FxU16             *tlut )
{
#ifdef OGL_NOTDONE
    GlideMsg( "ConvertAndDownloadRle( %d, %lu, %d, %d, %d, %d, %d, ---, %l, %lu, %lu, %lu, %lu, %lu, %lu, --- )\n",
        tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, bm_h, u0, v0, width, height,
        dest_width, dest_height );
#endif
#if (SIZEOF_INT_P == 4)
    FxU8 c, *texmem = new FxU8 [256 * 1024];
    FxU32 scount = 0, dcount = 0, offset = 4 + bm_h;
    FxU16 *tex = (FxU16 *)texmem, *src = tex + (dest_width * dest_height);
    int j, k;

    if (!texmem)
        return;
    // Line offset (v0)
    for (j = 0; j < v0; j++)
        offset += bm_data[4 + j];
    // Write height lines
    for (k = 0; k < height; k++) {
        // Decode one RLE line
        scount = offset;
        while((c = bm_data[scount]) != 0xE0U) {
            if (c > 0xE0U) {
                for (int count = 0; count < (c & 0x1FU); count++) {
                    // tlut is FxU16*
                    src[dcount] = tlut[bm_data[scount + 1]];
                    dcount++;
                }
                scount += 2;
            }
            else {
                src[dcount] = tlut[c];
                dcount++; scount++;
            }
        }
        // Copy line into destination texture, offset u0
        memcpy(tex + (k * dest_width), src + u0, dest_width * sizeof(FxU16));
        offset += bm_data[4 + j++];
        dcount = 0;
    }
    // One additional line
    if (height < dest_height)
        memcpy(tex + (k * dest_width), src + u0, dest_width * sizeof(FxU16));
    // Download decoded texture
    GrTexInfo info;
    info.smallLod = thisLod;
    info.largeLod = largeLod;
    info.aspectRatio = aspectRatio;
    info.format = format;
    info.data = tex;
    grTexDownloadMipMap(tmu, startAddress, evenOdd, &info);
    delete[] texmem;
#endif
}

//*************************************************
FX_ENTRY void FX_CALL 
grCheckForRoom( FxI32 n )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grCheckForRoom( %l )\n", n );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grParameterData( FxU32 param, FxU32 components, FxU32 type, FxI32 offset )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grParameterData( %lu, %lu, %lu, %l )\n",
        param, components, type, offset );
#endif
}

//*************************************************
FX_ENTRY int FX_CALL
guEncodeRLE16( void *dst, 
               void *src, 
               FxU32 width, 
               FxU32 height )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guEncodeRLE16( ---, ---, %lu, %lu ) = 1\n", width, height ); 
#endif

    return 1; 
}

//*************************************************
FX_ENTRY FxU32 FX_CALL
guEndianSwapWords( FxU32 value )
{
#ifdef OGL_DONE
    GlideMsg( "guEndianSwapWords( %lu )\n", value );
#endif

    return ( value << 16 ) | ( value >> 16 );
}

//*************************************************
FX_ENTRY FxU16 FX_CALL
guEndianSwapBytes( FxU16 value )
{
#ifdef OGL_DONE
    GlideMsg( "guEndianSwapBytes( %u )\n", value );
#endif

    return ( value << 8 ) | ( value >> 8 );
}

FX_ENTRY void FX_CALL
guMovieStart( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guMovieStart( ) - Not Supported\n" );
#endif
}

FX_ENTRY void FX_CALL
guMovieStop( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guMovieStop( ) - Not Supported\n" );
#endif
}

FX_ENTRY void FX_CALL
guMovieSetName( const char *name )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guMovieSetName( ) - Not Supported\n" );
#endif
}

