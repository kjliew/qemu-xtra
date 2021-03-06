#ifndef WRAPPER_CONFIG_H
#define WRAPPER_CONFIG_H

#include "sdk2_3dfx.h"

#define WRAPPER_FLAG_WINDOWED               (0x1)
#define WRAPPER_FLAG_MIPMAPS                (0x2)
#define WRAPPER_FLAG_PALETTED_TEXTURE_CACHE (0x4)
#define WRAPPER_FLAG_MANAGED_TEXTURES       (0x8)
#define WRAPPER_FLAG_VSYNCOFF               (0x40)
#define WRAPPER_FLAG_QEMU                   (0x80)

#ifdef __cplusplus
extern "C" {
#endif

FX_ENTRY void FX_CALL setConfig(FxU32 flags);
FX_ENTRY void FX_CALL setConfigRes(int res);

#ifdef __cplusplus
}
#endif

#endif /* WRAPPER_CONFIG_H */
