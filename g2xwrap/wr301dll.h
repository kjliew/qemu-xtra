#ifndef __G3WRAP_H__
#define __G3WRAP_H__

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "sdk2_glide.h"
#include "sdk2_glide3.h"

#define G3_LOD_TRANSLATE(lod)                       (0x8-lod)
#define G3_ASPECT_TRANSLATE(aspect)                 (0x3-(aspect))

#define GR_PARAM_IDX(p) ((p&0xF0U)? ((p&0x01U)? ((p>>4) + 6):((p>>4) + GR_PARAM_Q)):(p-1))

#endif // __G3WRAP_H__
