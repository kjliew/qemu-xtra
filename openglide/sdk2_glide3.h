#ifndef __GLIDE3_H__
#define __GLIDE3_H__

#include "sdk2_3dfx.h"

typedef FxU32 GrContext_t;
typedef FxU32 GrEnableMode_t;
#define GR_MODE_DISABLE     0x0
#define GR_MODE_ENABLE      0x1

#define GR_AA_ORDERED            0x01
#define GR_ALLOW_MIPMAP_DITHER   0x02
#define GR_PASSTHRU              0x03
#define GR_SHAMELESS_PLUG        0x04
#define GR_VIDEO_SMOOTHING       0x05

/*
** grDrawVertexArray/grDrawVertexArrayContiguous primitive type
*/
#define GR_POINTS                        0
#define GR_LINE_STRIP                    1
#define GR_LINES                         2
#define GR_POLYGON                       3
#define GR_TRIANGLE_STRIP                4
#define GR_TRIANGLE_FAN                  5
#define GR_TRIANGLES                     6
#define GR_TRIANGLE_STRIP_CONTINUE       7
#define GR_TRIANGLE_FAN_CONTINUE         8

/*
** grGet/grReset types
*/
#define GR_BITS_DEPTH                   0x01
#define GR_BITS_RGBA                    0x02
#define GR_FIFO_FULLNESS                0x03
#define GR_FOG_TABLE_ENTRIES            0x04
#define GR_GAMMA_TABLE_ENTRIES          0x05
#define GR_GLIDE_STATE_SIZE             0x06
#define GR_GLIDE_VERTEXLAYOUT_SIZE      0x07
#define GR_IS_BUSY                      0x08
#define GR_LFB_PIXEL_PIPE               0x09
#define GR_MAX_TEXTURE_SIZE             0x0a
#define GR_MAX_TEXTURE_ASPECT_RATIO     0x0b
#define GR_MEMORY_FB                    0x0c
#define GR_MEMORY_TMU                   0x0d
#define GR_MEMORY_UMA                   0x0e
#define GR_NUM_BOARDS                   0x0f
#define GR_NON_POWER_OF_TWO_TEXTURES    0x10
#define GR_NUM_FB                       0x11
#define GR_NUM_SWAP_HISTORY_BUFFER      0x12
#define GR_NUM_TMU                      0x13
#define GR_PENDING_BUFFERSWAPS          0x14
#define GR_REVISION_FB                  0x15
#define GR_REVISION_TMU                 0x16
#define GR_STATS_LINES                  0x17  /* grGet/grReset */
#define GR_STATS_PIXELS_AFUNC_FAIL      0x18
#define GR_STATS_PIXELS_CHROMA_FAIL     0x19
#define GR_STATS_PIXELS_DEPTHFUNC_FAIL  0x1a
#define GR_STATS_PIXELS_IN              0x1b
#define GR_STATS_PIXELS_OUT             0x1c
#define GR_STATS_PIXELS                 0x1d  /* grReset */
#define GR_STATS_POINTS                 0x1e  /* grGet/grReset */
#define GR_STATS_TRIANGLES_IN           0x1f
#define GR_STATS_TRIANGLES_OUT          0x20
#define GR_STATS_TRIANGLES              0x21  /* grReset */
#define GR_SWAP_HISTORY                 0x22
#define GR_SUPPORTS_PASSTHRU            0x23
#define GR_TEXTURE_ALIGN                0x24
#define GR_VIDEO_POSITION               0x25
#define GR_VIEWPORT                     0x26
#define GR_WDEPTH_MIN_MAX               0x27
#define GR_ZDEPTH_MIN_MAX               0x28
#define GR_VERTEX_PARAMETER             0x29
#define GR_BITS_GAMMA                   0x2a

#define GR_MEMTYPE                      0x1000 /* Voodoo Banshee */

/*
** grGetString types
*/
#define GR_EXTENSION                    0xa0
#define GR_HARDWARE                     0xa1
#define GR_RENDERER                     0xa2
#define GR_VENDOR                       0xa3
#define GR_VERSION                      0xa4

typedef struct {
  GrScreenResolution_t resolution;
  GrScreenRefresh_t    refresh;
  int                  numColorBuffers;
  int                  numAuxBuffers;
} GrResolution;

typedef GrResolution GlideResolution;

#define GR_QUERY_ANY  ((FxU32)(~0))

typedef FxU32 GrCoordinateSpaceMode_t;
#define GR_WINDOW_COORDS    0x00
#define GR_CLIP_COORDS      0x01

/* Parameters for strips */
#define GR_PARAM_XY       0x01
#define GR_PARAM_Z        0x02
#define GR_PARAM_W        0x03
#define GR_PARAM_Q        0x04
#define GR_PARAM_FOG_EXT  0x05
#define GR_PARAM_A        0x10
#define GR_PARAM_RGB      0x20
#define GR_PARAM_PARGB    0x30
#define GR_PARAM_ST0      0x40
#define GR_PARAM_ST1      GR_PARAM_ST0+1
#define GR_PARAM_Q0       0x50
#define GR_PARAM_Q1       GR_PARAM_Q0+1

#define GR_PARAM_DISABLE  0x00
#define GR_PARAM_ENABLE   0x01


#ifdef __cplusplus
extern "C" {
#endif

FX_ENTRY void FX_CALL 
grEnable( GrEnableMode_t mode );

FX_ENTRY void FX_CALL 
grDisable( GrEnableMode_t mode );

FX_ENTRY FxU32 FX_CALL 
grGet( FxU32 pname, FxU32 plength, FxI32 *params );

FX_ENTRY const char * FX_CALL
grGetString( FxU32 pname );

FX_ENTRY void FX_CALL 
grCoordinateSpace( GrCoordinateSpaceMode_t mode );

FX_ENTRY void FX_CALL 
grDepthRange( FxFloat n, FxFloat f );

FX_ENTRY void FX_CALL 
grViewport( FxI32 x, FxI32 y, FxI32 width, FxI32 height );

FX_ENTRY void FX_CALL 
grFinish(void);

FX_ENTRY void FX_CALL 
grFlush(void);

FX_ENTRY GrProc FX_CALL
grGetProcAddress( char *procName );

FX_ENTRY FxBool FX_CALL 
grReset( FxU32 what );

FX_ENTRY void FX_CALL
grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode);

FX_ENTRY void FX_CALL
grGlideGetVertexLayout( void *layout );

FX_ENTRY void FX_CALL
grGlideSetVertexLayout( const void *layout );

FX_ENTRY void FX_CALL
grSstConfigPipeline( int arg0, int arg1, int arg2 );

FX_ENTRY FxBool FX_CALL
grSelectContext( GrContext_t context );

FX_ENTRY FxBool FX_CALL
wrap3x_grSstWinClose( GrContext_t context );

FX_ENTRY void FX_CALL
wrap3x_grGlideShutdown( void );

FX_ENTRY void FX_CALL 
wrap3x_grFogMode( GrFogMode_t mode );

FX_ENTRY void FX_CALL 
wrap3x_grFogTable( const GrFog_t ft[] );

FX_ENTRY void FX_CALL
wrap3x_grDrawPoint( const void *pt );

FX_ENTRY void FX_CALL
wrap3x_grDrawLine( const void *v1, const void *v2 );

FX_ENTRY void FX_CALL
wrap3x_grDrawTriangle( const void *a, const void *b, const void *c );

FX_ENTRY void FX_CALL
wrap3x_grAADrawTriangle(
                 const void *a, const void *b, const void *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 );

FX_ENTRY FxU32 FX_CALL 
wrap3x_grTexCalcMemRequired(
                     GrLOD_t lodmin, GrLOD_t lodmax,
                     GrAspectRatio_t aspect, GrTextureFormat_t fmt);

FX_ENTRY FxU32 FX_CALL 
wrap3x_grTexTextureMemRequired( FxU32     evenOdd,
                                 GrTexInfo *info   );

FX_ENTRY void FX_CALL 
wrap3x_grTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info );

FX_ENTRY void FX_CALL 
wrap3x_grTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info );

FX_ENTRY void FX_CALL 
wrap3x_grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data );

FX_ENTRY FxBool FX_CALL 
wrap3x_grTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end );

FX_ENTRY void FX_CALL
wrap3x_grTexDownloadTable( GrTexTable_t type,
                    void         *data );

FX_ENTRY void FX_CALL
wrap3x_grTexDownloadTablePartial( GrTexTable_t type,
                           void         *data,
                           int          start,
                           int          end );

FX_ENTRY void FX_CALL
wrap3x_grTexNCCTable( GrNCCTable_t table );

FX_ENTRY FxBool FX_CALL
wrap3x_gu3dfGetInfo( const char *filename, Gu3dfInfo *info );

FX_ENTRY FxBool FX_CALL
wrap3x_gu3dfLoad( const char *filename, Gu3dfInfo *data );

FX_ENTRY FxBool FX_CALL
wrap3x_grLfbWriteRegion( GrBuffer_t dst_buffer,
                  FxU32 dst_x, FxU32 dst_y,
                  GrLfbSrcFmt_t src_format,
                  FxU32 src_width, FxU32 src_height,
                  FxBool pixelPipeline,
                  FxI32 src_stride, void *src_data );

FX_ENTRY void FX_CALL
grLoadGammaTable( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue);

FX_ENTRY void FX_CALL
guGammaCorrectionRGB( FxFloat red, FxFloat green, FxFloat blue );

FX_ENTRY FxI32 FX_CALL
grQueryResolutions( const GrResolution *resTemplate, GrResolution *output );

FX_ENTRY void FX_CALL
grDrawVertexArray(FxU32 mode, FxU32 Count, void *pointers);

FX_ENTRY void FX_CALL
grDrawVertexArrayContiguous(FxU32 mode, FxU32 Count, void *pointers, FxU32 stride);


#ifdef __cplusplus
}
#endif

#endif /* __GLIDE3_H__ */

