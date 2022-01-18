//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Fog Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <math.h>

#include "GlOgl.h"
#include "GLRender.h"
#include "OGLTables.h"


//*************************************************
//* download a fog table
//* Fog is applied after color combining and before alpha blending.
//*************************************************
FX_ENTRY void FX_CALL
grFogTable( const GrFog_t *ft )
{
#ifdef OGL_DONE
    GlideMsg( "grFogTable( --- )\n" );
#endif

    if ( InternalConfig.FogEnable )
    {
        memcpy( Glide.FogTable, (GrFog_t *)ft, GR_FOG_TABLE_SIZE * sizeof( FxU8 ) );
    }
}

//*************************************************
FX_ENTRY void FX_CALL
grFogColorValue( GrColor_t fogcolor )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grFogColorValue( %x )\n", fogcolor );
#endif

    RenderDrawTriangles( );

    Glide.State.FogColorValue = fogcolor;
    ConvertColorF( fogcolor, 
                   OpenGL.FogColor[ 0 ], 
                   OpenGL.FogColor[ 1 ], 
                   OpenGL.FogColor[ 2 ], 
                   OpenGL.FogColor[ 3 ] );
    glFogfv( GL_FOG_COLOR, &OpenGL.FogColor[0] );
}

//*************************************************
FX_ENTRY void FX_CALL
grFogMode( GrFogMode_t mode )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grFogMode( %d )\n", mode );
#endif

    static GrFogMode_t  modeSource, 
                        modeAdd;
    static GLfloat      ZeroColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };

    RenderDrawTriangles( );

    modeSource = mode & ( GR_FOG_WITH_TABLE | GR_FOG_WITH_ITERATED_ALPHA );
    modeAdd = mode & ( GR_FOG_MULT2 | GR_FOG_ADD2 );

    if ( modeSource )
    {
        OpenGL.Fog = true;
        if ( InternalConfig.EXT_fog_coord )
        {
            glEnable( GL_FOG );
        }
    }
    else
    {
        OpenGL.Fog = false;
        glDisable( GL_FOG );
    }

    switch ( modeAdd )
    {
    case GR_FOG_MULT2:
    case GR_FOG_ADD2:
        glFogfv( GL_FOG_COLOR, &ZeroColor[ 0 ] );
        break;

    default:
        glFogfv( GL_FOG_COLOR, &OpenGL.FogColor[ 0 ] );
        break;
    }
    
    Glide.State.FogMode = modeSource;
//    Glide.State.FogMode = mode;
}

//*************************************************
FX_ENTRY void FX_CALL
guFogGenerateExp( GrFog_t *fogtable, float density )
{
  int   i;
  float f;
  float scale;
  float dp;

  dp = density * guFogTableIndexToW( GR_FOG_TABLE_SIZE - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -dp ) );

  for ( i = 0; i < GR_FOG_TABLE_SIZE; i++ ) {
     dp = density * guFogTableIndexToW( i );
     f = ( 1.0F - ( float ) exp( -dp ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;
     fogtable[i] = ( GrFog_t ) f;
  }
} /* guFogGenerateExp */

//*************************************************
FX_ENTRY void FX_CALL
guFogGenerateExp2( GrFog_t *fogtable, float density )
{
  int   i;
  float f;
  float scale;
  float dp;

  dp = density * guFogTableIndexToW( GR_FOG_TABLE_SIZE - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -( dp * dp ) ) );

  for ( i = 0; i < GR_FOG_TABLE_SIZE; i++ ) {
     dp = density * guFogTableIndexToW( i );
     f = ( 1.0F - ( float ) exp( -( dp * dp ) ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;
     fogtable[i] = ( GrFog_t ) f;
  }
} /* guFogGenerateExp2 */

//*************************************************
FX_ENTRY void FX_CALL
guFogGenerateLinear( GrFog_t *fogtable,
                     float nearZ, float farZ )
{
   int i;
   float world_w;
   float f;

  for ( i = 0; i < GR_FOG_TABLE_SIZE; i++ ) {
    world_w = guFogTableIndexToW( i );
    if ( world_w > 65535.0F )
      world_w = 65535.0F;

    f = ( world_w - nearZ ) / ( farZ - nearZ );
    if ( f > 1.0F )
      f = 1.0F;
    else if ( f < 0.0F )
      f = 0.0F;
    f *= 255.0F;
    fogtable[i] = ( GrFog_t ) f;
  }
} /* guFogGenerateLinear */

//*************************************************
//* convert a fog table index to a floating point eye-space w value
//*************************************************
FX_ENTRY float FX_CALL
guFogTableIndexToW( int i )
{
#ifdef OGL_DONE
    GlideMsg( "guFogTableIndexToW( %d )\n", i );
#endif
#ifdef OGL_DEBUG
    if ( ( i < 0 ) ||
         ( i >= GR_FOG_TABLE_SIZE ) )
    {
        Error( "Error on guFogTableIndexToW( %d )\n", i );
    }
#endif

    return tableIndexToW[ i ];
}

