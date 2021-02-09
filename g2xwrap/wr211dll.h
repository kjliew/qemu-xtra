#include <cstdlib>
#include <cstdio>
#include <cstring>

#ifndef _G2WRAP_H
#define _G2WRAP_H

#include "sdk2_3dfx.h"
#include "sdk2_glide.h"

#if FXSIZEOF_INT_P != 4
# error "sizeof (intptr_t) != 4"
#endif

#ifdef __cplusplus
extern "C" {
#endif

FX_ENTRY void FX_CALL
wrap_grBufferSwap( int swap_interval );

FX_ENTRY void FX_CALL 
wrap_grLfbBegin( void );

FX_ENTRY void FX_CALL 
wrap_grLfbBypassMode( GrLfbBypassMode_t mode );

FX_ENTRY void FX_CALL 
wrap_grLfbEnd( void );

FX_ENTRY const FxU32 * FX_CALL 
wrap_grLfbGetReadPtr( GrBuffer_t buffer );

FX_ENTRY void * FX_CALL 
wrap_grLfbGetWritePtr( GrBuffer_t buffer );

FX_ENTRY void FX_CALL 
wrap_grLfbOrigin(GrOriginLocation_t origin);

FX_ENTRY void FX_CALL 
wrap_grLfbWriteMode( GrLfbWriteMode_t mode );

FX_ENTRY FxBool FX_CALL 
wrap_grSstOpen(
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          GrSmoothingMode_t    smoothing_filter,
          int                  num_buffers );

FX_ENTRY void FX_CALL
wrap_grSstPassthruMode( GrPassthruMode_t mode);

FX_ENTRY void FX_CALL
wrap_guFbReadRegion(
               const int src_x, const int src_y,
               const int w, const int h, const void *dst,
               const int strideInBytes
               );

FX_ENTRY void FX_CALL
wrap_guFbWriteRegion(
                const int dst_x, const int dst_y,
                const int w, const int h, const void *src,
                const int strideInBytes
                );

#ifdef __cplusplus
}
#endif

#endif //_G2WRAP_H
