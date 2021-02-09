#include <windows.h>
#include "wr301dll.h"

//#define DEBUG_WRAP3X
#ifdef DEBUG_WRAP3X
#define DPRINTF(fmt, ...) \
    do { fprintf(stderr, "  " fmt , ## __VA_ARGS__); } while(0)
#else
#define DPRINTF(fmt, ...)
#endif

#define VLUT_ELEM 12

static struct vertexLayout {
    const int slen[VLUT_ELEM] = {8, 4, 4, 4, 4, 4, 12, 4, 8, 4, 8, 4};
    int last_vlut[VLUT_ELEM];
    int vlut[VLUT_ELEM];
    int viewPort[4] = { 0, 0, 640, 480 };
    float depthRange[2] = { 0.f, 65535.f };
    float zRange[2] = { 65535.f, 0.f };
    int coordSpace = 0;
    void load(const void *p) { memcpy((void *)p, vlut, sizeof(vlut)); };
    void store(const void *p) { memcpy(vlut, p, sizeof(vlut)); };
    void reset(void) { memset(vlut, 0, sizeof(vlut)); };
    void vvars(int param, int offs, int mode) 
    { 
        vlut[GR_PARAM_IDX(param)] = (mode)? offs:0;
        if (GR_PARAM_PARGB == param) {
            vlut[GR_PARAM_IDX(GR_PARAM_A)] = 0;
            vlut[GR_PARAM_IDX(GR_PARAM_RGB)] = 0;
        }
        if (GR_PARAM_RGB == param)
            vlut[GR_PARAM_IDX(GR_PARAM_PARGB)] = 0;
    }
    void debug(void)
    {
        if (memcmp(last_vlut, vlut, sizeof(vlut))) {
            memcpy(last_vlut, vlut, sizeof(vlut));
#if 0
            DPRINTF("coord:%-2d xy:%-2d z:%-2d w:%-2d q:%-2d a:%-2d rgb:%-2d pargb:%-2d st0:%-2d q0:%-2d st1:%-2d q1:%-2d\n",
                    coordSpace,
                    vlut[GR_PARAM_IDX(GR_PARAM_XY)],
                    vlut[GR_PARAM_IDX(GR_PARAM_Z)],
                    vlut[GR_PARAM_IDX(GR_PARAM_W)],
                    vlut[GR_PARAM_IDX(GR_PARAM_Q)],
                    vlut[GR_PARAM_IDX(GR_PARAM_A)],
                    vlut[GR_PARAM_IDX(GR_PARAM_RGB)],
                    vlut[GR_PARAM_IDX(GR_PARAM_PARGB)],
                    vlut[GR_PARAM_IDX(GR_PARAM_ST0)],
                    vlut[GR_PARAM_IDX(GR_PARAM_Q0)],
                    vlut[GR_PARAM_IDX(GR_PARAM_ST1)],
                    vlut[GR_PARAM_IDX(GR_PARAM_Q1)] );
#endif
            /* Turok 2 fogging fix */
            if (vlut[GR_PARAM_IDX(GR_PARAM_Q0)] &&
                (vlut[GR_PARAM_IDX(GR_PARAM_Q0)] == vlut[GR_PARAM_IDX(GR_PARAM_Q1)])) {
                GrFog_t ft[GR_FOG_TABLE_SIZE];
                wrap3x_grFogTable(ft);
            }
        }
    };
    void wrap(const void *v, const void *p)
    {
        const int v2offs[VLUT_ELEM] = { 0, 24, 32, 32, 0, 28, 12, 0, 36, 44, 48, 52 };
        memset((void *)v, 0, sizeof(GrVertex));
        memcpy((void *)v, p, slen[0]);
        for (int i = 1; i < VLUT_ELEM; i++) {
            if (i == GR_PARAM_IDX(GR_PARAM_FOG_EXT))
                continue;
            if (vlut[i] && (i == GR_PARAM_IDX(GR_PARAM_PARGB))) {
                float argb[4];
                FxU32 pargb = *(FxU32 *)((unsigned char *)p + vlut[i]);
                argb[0] = (float)((pargb & 0xFF000000U) >> 24) * 1.f;
                argb[1] = (float)((pargb &   0xFF0000U) >> 16) * 1.f;
                argb[2] = (float)((pargb &     0xFF00U) >> 8 ) * 1.f;
                argb[3] = (float)((pargb &       0xFFU)      ) * 1.f;
                memcpy((unsigned char *)v + v2offs[GR_PARAM_IDX(GR_PARAM_A)], argb, sizeof(float));
                memcpy((unsigned char *)v + v2offs[GR_PARAM_IDX(GR_PARAM_RGB)], &argb[1], sizeof(float[3]));
                continue;
            }
            if (vlut[i])
                memcpy((unsigned char *)v + v2offs[i], (unsigned char *)p + vlut[i], slen[i]);
        }
        if (vlut[GR_PARAM_IDX(GR_PARAM_Q0)] /*&&
            (vlut[GR_PARAM_IDX(GR_PARAM_Q0)] == vlut[GR_PARAM_IDX(GR_PARAM_Q1)])*/)
            ((GrVertex *)v)->oow = ((GrVertex *)v)->tmuvtx[0].oow;
        if (coordSpace == 1) {
            ((GrVertex *)v)->oow = 1.f / ((GrVertex *)v)->oow;
            ((GrVertex *)v)->x = ((GrVertex *)v)->x * ((GrVertex *)v)->oow * 0.5f * (2*viewPort[0] + viewPort[2]) + 0.5f * (2*viewPort[0] + viewPort[2]);
            ((GrVertex *)v)->y = ((GrVertex *)v)->y * ((GrVertex *)v)->oow * 0.5f * (2*viewPort[1] + viewPort[3]) + 0.5f * (2*viewPort[1] + viewPort[3]);
            ((GrVertex *)v)->ooz = ((GrVertex *)v)->ooz * (depthRange[1] - depthRange[0]) * 0.5f * 65535.f
                + (depthRange[1] + depthRange[0]) * 0.5f * 65535.f;
            ((GrVertex *)v)->r *= 255.f;
            ((GrVertex *)v)->g *= 255.f;
            ((GrVertex *)v)->b *= 255.f;
            ((GrVertex *)v)->a *= 255.f;
        }
    };
} vertex3x;

static void
texInfo3x_wrap( GrTexInfo *info )
{
    info->smallLod = G3_LOD_TRANSLATE(info->smallLod);
    info->largeLod = G3_LOD_TRANSLATE(info->largeLod);
    info->aspectRatio = G3_ASPECT_TRANSLATE(info->aspectRatio);
}

static int trace_g3ext;
static int res;
struct tblGlideResolution {
    int w;
    int	h;
};

static const struct tblGlideResolution tblRes[] = {
  { .w = 320, .h = 200   }, //0x0
  { .w = 320, .h = 240   }, //0x1
  { .w = 400, .h = 256   }, //0x2
  { .w = 512, .h = 384   }, //0x3
  { .w = 640, .h = 200   }, //0x4
  { .w = 640, .h = 350   }, //0x5
  { .w = 640, .h = 400   }, //0x6
  { .w = 640, .h = 480   }, //0x7
  { .w = 800, .h = 600   }, //0x8
  { .w = 960, .h = 720   }, //0x9
  { .w = 856, .h = 480   }, //0xa
  { .w = 512, .h = 256   }, //0xb
  { .w = 1024, .h = 768  }, //0xC
  { .w = 1280, .h = 1024 }, //0xD
  { .w = 1600, .h = 1200 }, //0xE
  { .w = 400, .h = 300   }, //0xF
  { .w = 0, .h = 0},
};

static GrHwConfiguration hwconfig;

FX_ENTRY void FX_CALL 
grEnable( GrEnableMode_t mode )
{
    if (GR_PASSTHRU == mode)
        grSstControl(GR_CONTROL_DEACTIVATE);
}

FX_ENTRY void FX_CALL 
grDisable( GrEnableMode_t mode )
{
    if (GR_PASSTHRU == mode)
        grSstControl(GR_CONTROL_ACTIVATE);
}

FX_ENTRY FxU32 FX_CALL
grGet( FxU32 pname, FxU32 plength, FxI32 *params )
{
    static FxU32 bitsRGBA[4] = { 5, 6, 5, 0 };
    static FxU32 dataU32[] = {
        0, 1, 2, 3,
        16,
        32,
        64, /* GR_FOG_TABLE_SIZE */
        128,
        256,
        sizeof(GrState),
    };
    /* GR_MEMORY                FB        TMU       UMA */
    static FxU32 chipMem[3] = { 0x400000, 0x400000, 0x00 };
    static FxU32 chipRev[2] = { 0x02, 0x01 };
    static FxU32 texAlign = 0x08;
    static const struct {
        FxU32 name;
        FxU32 len;
        void *params;
    } getTbl[] = {
        { GR_PENDING_BUFFERSWAPS, 4,      &dataU32[0] },
        { GR_BITS_DEPTH, 4,               &dataU32[4] },
        { GR_BITS_RGBA, 16, bitsRGBA },
        { GR_BITS_GAMMA, 4,               &dataU32[4] },
        { GR_GAMMA_TABLE_ENTRIES, 4,      &dataU32[8] },
        { GR_FOG_TABLE_ENTRIES, 4,        &dataU32[6] },
        { GR_GLIDE_STATE_SIZE, 4,         &dataU32[9] },
        { GR_GLIDE_VERTEXLAYOUT_SIZE, 4,  &dataU32[8] },
        { GR_MAX_TEXTURE_SIZE, 4,         &dataU32[8] },
        { GR_MAX_TEXTURE_ASPECT_RATIO, 4, &dataU32[3] },
        { GR_MEMORY_FB, 4,                &chipMem[0] },
        { GR_MEMORY_TMU, 4,               &chipMem[1] },
        { GR_MEMORY_UMA, 4,               &chipMem[2] },
        { GR_NUM_BOARDS, 4,               &dataU32[1] },
        { GR_NUM_FB, 4,                   &dataU32[1] },
        { GR_NUM_TMU, 4,                  &dataU32[1] },
        { GR_REVISION_FB, 4,              &chipRev[0] },
        { GR_REVISION_TMU, 4,             &chipRev[1] },
        { GR_SUPPORTS_PASSTHRU, 4,        &dataU32[1] },
        { GR_TEXTURE_ALIGN, 4,            &texAlign },
        { GR_VIEWPORT, 16, vertex3x.viewPort },
        { GR_WDEPTH_MIN_MAX, 8, vertex3x.depthRange },
        { GR_ZDEPTH_MIN_MAX, 8, vertex3x.zRange },
        { GR_MEMTYPE, 4,                  &dataU32[1] },
        { 0, 0, 0 },
    };

    FxU32 ret = 0;
    int i;
    const char *chipstr = (pname == GR_NUM_BOARDS)? " ":grGetString(GR_HARDWARE);

//#define VOODOO_TYPE " GETGAMMA ","Voodoo2"
//#define VOODOO_TYPE " GETGAMMA ","Voodoo Graphics"
#define VOODOO_TYPE " GETGAMMA TEXUMA ","Voodoo Banshee"

    if (hwconfig.num_sst == 0) {
        hwconfig.num_sst = 1;
        hwconfig.SSTs[0].sstBoard.VoodooConfig.tmuConfig[0].tmuRam = (4 << 20);
        hwconfig.SSTs[0].sstBoard.VoodooConfig.fbRam = (4 << 20);
    }

    if (!strncmp(chipstr, "Voodoo Banshee", sizeof("Voodoo Banshee"))) {
        chipMem[2] = hwconfig.SSTs[0].sstBoard.VoodooConfig.tmuConfig[0].tmuRam << 20;
        chipMem[1] = chipMem[0] = hwconfig.SSTs[0].sstBoard.VoodooConfig.fbRam << 20;
        chipRev[0] = 0x1002; chipRev[1] = 0x1001;
    }
    if (!strncmp(chipstr, "Voodoo2", sizeof("Voodoo2"))) {
        chipMem[1] = hwconfig.SSTs[0].sstBoard.VoodooConfig.tmuConfig[0].tmuRam << 20;
        chipMem[0] = hwconfig.SSTs[0].sstBoard.VoodooConfig.fbRam << 20;
    }

    vertex3x.viewPort[2] = tblRes[res].w;
    vertex3x.viewPort[3] = tblRes[res].h;

    for (i = 0; getTbl[i].len != 0; i++) {
        if (params && (pname == getTbl[i].name) &&
            (plength == getTbl[i].len)) {
            if (getTbl[i].params) {
                memcpy(params, (void *)getTbl[i].params, plength);
                ret = plength;
            }
            break;
        }
    }

    if (getTbl[i].len == 0) {
        if (params && plength) {
            memset(params, 0, plength);
            ret = plength;
        }
        DPRINTF("Unsupported grGet() pname=0x%02X plength=%d\n", pname, plength);
    }

    if (ret == 0)
        DPRINTF("Failed grGet() pname=0x%02X\n", pname);

    return ret;
}

FX_ENTRY const char * FX_CALL
grGetString( FxU32 pname )
{
    static const char *cstrTbl[] = {
        VOODOO_TYPE,
        "Glide",
        "3Dfx Interactive",
        "3.01",
    };
    const char *p = 0;
    trace_g3ext = 1;
    if ((pname & 0x0F) < 0x05)
        p = cstrTbl[pname & 0x0F];
    return p;
}

FX_ENTRY void FX_CALL 
grCoordinateSpace( GrCoordinateSpaceMode_t mode )
{
    /*
    if (vertex3x.coordSpace != mode)
        DPRINTF("grCoordinateSpace() mode %d\n", mode);
        */
    vertex3x.coordSpace = (mode)? 1:0;
}

FX_ENTRY void FX_CALL 
grDepthRange( FxFloat n, FxFloat f )
{
    /*
    if ((vertex3x.depthRange[0] != n) ||
        (vertex3x.depthRange[1] != f))
        DPRINTF("grDepthRange() n=%f f=%f\n", n, f);
        */
    vertex3x.depthRange[0] = n;
    vertex3x.depthRange[1] = f;
}

FX_ENTRY void FX_CALL 
grViewport( FxI32 x, FxI32 y, FxI32 width, FxI32 height )
{
    /*
    if ((vertex3x.viewPort[0] != x) || (vertex3x.viewPort[1] != y) ||
        (vertex3x.viewPort[2] != width) || (vertex3x.viewPort[3] != height))
        DPRINTF("grViewPort() x=%d y=%d w=%d h=%d\n", x, y, width, height);
        */
    vertex3x.viewPort[0] = x;
    vertex3x.viewPort[1] = y;
    vertex3x.viewPort[2] = width;
    vertex3x.viewPort[3] = height;
}

FX_ENTRY void FX_CALL 
grFinish(void) {}

FX_ENTRY void FX_CALL 
grFlush(void) {}

static void FX_CALL
grGetGammaTableExt( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue)
{
    struct {
        unsigned short red[256];
        unsigned short green[256];
        unsigned short blue[256];
    } ramp;

    //GetGammaTable(&ramp);

    for (int i = 0; i < nentries; i++) {
        red[i]   = ramp.red[i]   & 0xFFU;
        green[i] = ramp.green[i] & 0xFFU;
        blue[i]  = ramp.blue[i]  & 0xFFU;
    }
#if 0
    DPRINTF("grGetGammaTableExt() n=%d\n", nentries);
    for (int i = 0; i < nentries; i++) {
        if (i && (i%0x10 == 0))
            fprintf(stderr, "\n");
        fprintf(stderr, "%04X ", red[i]);
    }
    fprintf(stderr, "\n");
#endif    
}

FX_ENTRY GrProc FX_CALL
grGetProcAddress( char *procName )
{
    if (!memcmp(procName, "grGetGammaTableExt", sizeof("grGetGammaTableExt")))
        return (GrProc)&grGetGammaTableExt;
    if (!memcmp(procName, "grCommandTransportInfoExt2", sizeof("grCommandTransportInfoExt2")))
        return 0;
    if (trace_g3ext)
        DPRINTF("grGetProcAddress() func %s\n", procName);
    return 0;
}

FX_ENTRY FxBool FX_CALL 
grReset( FxU32 what )
{
    switch (what) {
        case GR_VERTEX_PARAMETER:
            vertex3x.reset();
            break;
        default:
            break;
    }
    return FXTRUE;
}

FX_ENTRY void FX_CALL
grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode)
{
    if (mode && (GR_PARAM_IDX(param) > (VLUT_ELEM - 1))) {
        DPRINTF("grVertexLayout() unsupported param 0x%02X\n", param);
        return;
    }
    vertex3x.vvars(param, offset, mode);
    vertex3x.debug();
}

FX_ENTRY void FX_CALL
grGlideGetVertexLayout( void *layout )
{
    vertex3x.load(layout);
}

FX_ENTRY void FX_CALL
grGlideSetVertexLayout( const void *layout )
{
    vertex3x.store(layout);
}

FX_ENTRY void FX_CALL
grSstConfigPipeline( int arg0, int arg1, int arg2 )
{
}

FX_ENTRY FxBool FX_CALL
grSelectContext( GrContext_t context )
{
    return FXTRUE;
}

FX_ENTRY GrContext_t FX_CALL 
wrap3x_grSstWinOpen(
          FxU                  hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int                  nColBuffers,
          int                  nAuxBuffers)
{
    GrContext_t ctx;
    res = screen_resolution;

    ctx = grSstWinOpen(hWnd,
        screen_resolution, refresh_rate,
        color_format, origin_location,
        nColBuffers, nAuxBuffers);

    return ctx;
}

FX_ENTRY FxBool FX_CALL
wrap3x_grSstWinClose( GrContext_t context )
{
    grSstWinClose();
    trace_g3ext = 0;
    return FXTRUE;
}

FX_ENTRY void FX_CALL
wrap3x_grGlideInit( void )
{
    grSstQueryBoards(&hwconfig);
    grGlideInit();
    grSstQueryHardware(&hwconfig);
}

FX_ENTRY void FX_CALL
wrap3x_grGlideShutdown( void )
{
    trace_g3ext = 0;
    grGlideShutdown();
    //vertex3x.reset();
    memset(&hwconfig, 0, sizeof(GrHwConfiguration));
}

FX_ENTRY void FX_CALL 
wrap3x_grFogMode( GrFogMode_t mode )
{
    //DPRINTF("grFogMode() mode=%d\n", mode);
    grFogMode(mode);
}

FX_ENTRY void FX_CALL 
wrap3x_grFogTable( const GrFog_t ft[] )
{
    GrFog_t fto[GR_FOG_TABLE_SIZE];
    memcpy(fto, ft, GR_FOG_TABLE_SIZE);
    if (vertex3x.vlut[GR_PARAM_IDX(GR_PARAM_Q0)] && 
        (vertex3x.vlut[GR_PARAM_IDX(GR_PARAM_Q0)] == vertex3x.vlut[GR_PARAM_IDX(GR_PARAM_Q1)])) {
        guFogGenerateLinear(fto, guFogTableIndexToW(0), guFogTableIndexToW(63));
#if 0
        fprintf(stderr, "  FogTable override:\n  ");
        for (int i = 0; i < GR_FOG_TABLE_SIZE; i++) {
            if (i && ((i % 0x10) == 0))
                fprintf(stderr, "\n  ");
            fprintf(stderr, "%02X ", fto[i]);
        }
        fprintf(stderr, "\n");
#endif
    }
    grFogTable(fto);
}

FX_ENTRY void FX_CALL
wrap3x_grDrawPoint( const void *pt )
{
    GrVertex wpt;
    vertex3x.wrap(&wpt, pt);
    grDrawPoint(&wpt);
}

FX_ENTRY void FX_CALL
wrap3x_grDrawLine( const void *v1, const void *v2 )
{
    GrVertex wv1, wv2;
    vertex3x.wrap(&wv1, v1);
    vertex3x.wrap(&wv2, v2);
    grDrawLine(&wv1, &wv2);

}

FX_ENTRY void FX_CALL
wrap3x_grDrawTriangle( const void *a, const void *b, const void *c )
{
    GrVertex wa, wb, wc;
    vertex3x.wrap(&wa, a);
    vertex3x.wrap(&wb, b);
    vertex3x.wrap(&wc, c);
    grDrawTriangle(&wa, &wb, &wc);
}

FX_ENTRY void FX_CALL
wrap3x_grAADrawTriangle(
                 const void *a, const void *b, const void *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 )
{
    GrVertex wa, wb, wc;
    vertex3x.wrap(&wa, a);
    vertex3x.wrap(&wb, b);
    vertex3x.wrap(&wc, c);
    grAADrawTriangle(&wa, &wb, &wc,
            ab_antialias, bc_antialias, ca_antialias);
}

FX_ENTRY FxU32 FX_CALL 
wrap3x_grTexCalcMemRequired(
                     GrLOD_t lodmin, GrLOD_t lodmax,
                     GrAspectRatio_t aspect, GrTextureFormat_t fmt)
{
    GrTexInfo wti;
    wti.smallLod = lodmin;
    wti.largeLod = lodmax;
    wti.aspectRatio = aspect;
    texInfo3x_wrap(&wti);

    return grTexCalcMemRequired(wti.smallLod, wti.largeLod,
            wti.aspectRatio, fmt);
}


FX_ENTRY FxU32 FX_CALL 
wrap3x_grTexTextureMemRequired( FxU32     evenOdd,
                                 GrTexInfo *info   )
{
    GrTexInfo wti;
    memcpy(&wti, info, sizeof(GrTexInfo));
    texInfo3x_wrap(&wti);
    return grTexTextureMemRequired(evenOdd, &wti);
}

FX_ENTRY void FX_CALL 
wrap3x_grTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info )
{
    GrTexInfo wti;
    memcpy(&wti, info, sizeof(GrTexInfo));
    texInfo3x_wrap(&wti);
    grTexSource(tmu, startAddress, evenOdd, &wti);
}

FX_ENTRY void FX_CALL 
wrap3x_grTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info )
{
    GrTexInfo wti;
    memcpy(&wti, info, sizeof(GrTexInfo));
    texInfo3x_wrap(&wti);
    grTexDownloadMipMap(tmu, startAddress, evenOdd, &wti);
}

FX_ENTRY void FX_CALL 
wrap3x_grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data )
{
    GrTexInfo wti;
    wti.smallLod = thisLod;
    wti.largeLod = largeLod;
    wti.aspectRatio = aspectRatio;
    texInfo3x_wrap(&wti);

    grTexDownloadMipMapLevel(tmu, startAddress,
            wti.smallLod, wti.largeLod, wti.aspectRatio,
            format, evenOdd, data);
}

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
                                 int               end )
{
    GrTexInfo wti;
    wti.smallLod = thisLod;
    wti.largeLod = largeLod;
    wti.aspectRatio = aspectRatio;
    texInfo3x_wrap(&wti);

    grTexDownloadMipMapLevelPartial(tmu, startAddress,
            wti.smallLod, wti.largeLod, wti.aspectRatio,
            format, evenOdd, data, start, end);

    return FXTRUE;
}

FX_ENTRY void FX_CALL
wrap3x_grTexDownloadTable( GrTexTable_t type,
                    void         *data )
{
    grTexDownloadTable(0, type, data);
}

FX_ENTRY void FX_CALL
wrap3x_grTexDownloadTablePartial( GrTexTable_t type,
                           void         *data,
                           int          start,
                           int          end )
{
    grTexDownloadTablePartial(0, type, data, start, end);
}

FX_ENTRY void FX_CALL
wrap3x_grTexNCCTable( GrNCCTable_t table )
{
    grTexNCCTable(0, table);
}

FX_ENTRY FxBool FX_CALL
wrap3x_gu3dfGetInfo( const char *filename, Gu3dfInfo *info )
{
    static const FxI32 g3Lod[9] = {8, 7, 6, 5, 4, 3, 2, 1, 0};
    static const FxI32 g3Aspect[7] = {3, 2, 1, 0, -1, -2, -3};
    FxBool ret;
    ret = gu3dfGetInfo(filename, info);
    info->header.small_lod = g3Lod[info->header.small_lod];
    info->header.large_lod = g3Lod[info->header.large_lod];
    info->header.aspect_ratio = g3Aspect[info->header.aspect_ratio];

    return ret;
}

FX_ENTRY FxBool FX_CALL
wrap3x_gu3dfLoad( const char *filename, Gu3dfInfo *data )
{
    Gu3dfInfo fi;
    memcpy(&fi, data, sizeof(Gu3dfInfo));
    fi.header.small_lod = G3_LOD_TRANSLATE(fi.header.small_lod);
    fi.header.large_lod = G3_LOD_TRANSLATE(fi.header.large_lod);
    fi.header.aspect_ratio = G3_ASPECT_TRANSLATE(fi.header.aspect_ratio);

    return gu3dfLoad(filename, &fi);
}

FX_ENTRY FxBool FX_CALL
wrap3x_grLfbWriteRegion( GrBuffer_t dst_buffer,
                  FxU32 dst_x, FxU32 dst_y,
                  GrLfbSrcFmt_t src_format,
                  FxU32 src_width, FxU32 src_height,
                  FxBool pixelPipeline,
                  FxI32 src_stride, void *src_data )
{
    return grLfbWriteRegion(dst_buffer, dst_x, dst_y, 
            src_format, src_width, src_height,
            src_stride, src_data);
}

FX_ENTRY void FX_CALL
grLoadGammaTable( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue)
{
    struct {
        unsigned short red[256];
        unsigned short green[256];
        unsigned short blue[256];
    } ramp;
#if 0
    DPRINTF("grLoadGammaTable() n=%d\n", nentries);
    for (int i = 0; i < nentries; i++) {
        if (i && (i%0x10 == 0))
            fprintf(stderr, "\n");
        fprintf(stderr, "%04X ", red[i]);
    }
    fprintf(stderr, "\n");
#endif
    for (int i = 0; i < nentries; i++) {
        ramp.red[i] =   ((unsigned short)red[i]   << 8 | red[i]  ) & 0xFFFFU;
        ramp.green[i] = ((unsigned short)green[i] << 8 | green[i]) & 0xFFFFU;
        ramp.blue[i] =  ((unsigned short)blue[i]  << 8 | blue[i] ) & 0xFFFFU;
    }
    //SetGammaTable(&ramp);
}

FX_ENTRY void FX_CALL
guGammaCorrectionRGB( FxFloat red, FxFloat green, FxFloat blue )
{
    if ((red == green) && (green == blue))
        grGammaCorrectionValue(red);
    else
        DPRINTF("guGammaCorrectionRGB() r=%f g=%f b=%f\n", red, green, blue);
}

FX_ENTRY FxI32 FX_CALL
grQueryResolutions( const GrResolution *resTemplate, GrResolution *output )
{
    static GrResolution resTbl[] = {
        { .resolution = 0x07, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x08, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x09, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x0A, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x0B, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x0C, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x0D, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0x0E, .refresh = 0x00, .numColorBuffers = 2, .numAuxBuffers = 1 },
        { .resolution = 0xFF, .refresh = 0xFF },
    };

    FxI32 ret = 0;

    if (output == NULL)
        ret = sizeof(GrResolution[8]);
    else {
        int i;
        for (i = 0; resTbl[i].resolution != 0xFF; i++) {
            if (resTemplate->resolution == resTbl[i].resolution) {
                ret = sizeof(GrResolution[1]);
                memcpy(output, &resTbl[i], sizeof(GrResolution[1]));
                break;
            }
        }

        if (resTbl[i].resolution == 0xFF) {
            ret = sizeof(GrResolution[8]);
            memcpy(output, resTbl, sizeof(GrResolution[8]));
        }
    }
            
    return ret;
}

FX_ENTRY void FX_CALL
grDrawVertexArray(FxU32 mode, FxU32 Count, void *pointers)
{
    switch(mode) {
        case GR_POINTS:
            {
                void **v = (void **)pointers;
                for (int i = 0; i < Count; i++)
                    wrap3x_grDrawPoint(v[i]);
            }
            break;
        case GR_LINE_STRIP:
            {
                void **v = (void **)pointers;
                for (int i = 0; (i+1) < Count; i++)
                    wrap3x_grDrawLine(v[i], v[i+1]);
            }
            break;
        case GR_LINES:
            {
                void **v = (void **)pointers;
                for (int i = 0; i < Count; i+=2)
                    wrap3x_grDrawLine(v[i], v[i+1]);
            }
            break;
        case GR_POLYGON:
            //DPRINTF("grDrawVertexArray() mode=%d cnt=%d\n", mode, Count);
            {
                void **v = (void **)pointers;
                int index[Count];
                GrVertex vtx[Count];
                for (int i = 0; i < Count; i++) {
                    vertex3x.wrap(&vtx[i], v[i]);
                    index[i] = i;
                }
                grDrawPolygon(Count, index, vtx);
            }
            break;
        case GR_TRIANGLE_STRIP:
        case GR_TRIANGLE_STRIP_CONTINUE:
            {
                static GrVertex vbuf[6];
                static int vi = 0, vo = 0;
                void **v = (void **)pointers;
                if (mode == GR_TRIANGLE_STRIP) {
                    vi = 0; vo = 0;
                }
                for (int i = 0; i < Count; i++) {
                    if (abs(vi - vo) == 3) {
                        grDrawTriangle(&vbuf[vo], &vbuf[(vo+1)%6], &vbuf[(vo+2)%6]);
                        vo++; vo %= 6;
                    }
                    vertex3x.wrap(&vbuf[vi++], v[i]);
                    vi %= 6;
                }
                if (abs(vi - vo) == 3) {
                    grDrawTriangle(&vbuf[vo], &vbuf[(vo+1)%6], &vbuf[(vo+2)%6]);
                    vo++; vo %= 6;
                }
            }
            break;
        case GR_TRIANGLE_FAN:
        case GR_TRIANGLE_FAN_CONTINUE:
            {
                static GrVertex vbuf[5];
                static int vi = 0, vo = 0;
                void **v = (void **)pointers;
                int i = 0;
                if (mode == GR_TRIANGLE_FAN) {
                    vertex3x.wrap(&vbuf[4], v[0]);
                    vi = 0; vo = 0; i = 1;
                }
                for (; i < Count; i++) {
                    if (abs(vi - vo) == 2) {
                        grDrawTriangle(&vbuf[4], &vbuf[vo], &vbuf[(vo+1)%4]);
                        vo++; vo %= 4;
                    }
                    vertex3x.wrap(&vbuf[vi++], v[i]);
                    vi %= 4;
                }
                if (abs(vi - vo) == 2) {
                    grDrawTriangle(&vbuf[4], &vbuf[vo], &vbuf[(vo+1)%4]);
                    vo++; vo %= 4;
                }
            }
            break;
        case GR_TRIANGLES:
            {
                void **v = (void **)pointers;
                for (int i = 0; i < Count; i+=3)
                    wrap3x_grDrawTriangle(v[i], v[i+1], v[i+2]);
            }
            break;
        default:
            DPRINTF(" Unsupported grDrawVertexArray() mode %d\n", mode);
            break;
    }
}

FX_ENTRY void FX_CALL
grDrawVertexArrayContiguous(FxU32 mode, FxU32 Count, void *pointers, FxU32 stride)
{
#define VA_CONT(n) (v + ((n)*stride))
    switch(mode) {
        case GR_POINTS:
            {
                unsigned char *v = (unsigned char *)pointers;
                for (int i = 0; i < Count; i++)
                    wrap3x_grDrawPoint(VA_CONT(i));
            }
            break;
        case GR_LINE_STRIP:
            {
                unsigned char *v = (unsigned char *)pointers;
                for (int i = 0; (i+1) < Count; i++)
                    wrap3x_grDrawLine(VA_CONT(i), VA_CONT(i+1));
            }
            break;
        case GR_LINES:
            {
                unsigned char *v = (unsigned char *)pointers;
                for (int i = 0; i < Count; i+=2)
                    wrap3x_grDrawLine(VA_CONT(i), VA_CONT(i+1));
            }
            break;
        case GR_POLYGON:
            //DPRINTF("grDrawVertexArrayContiguous() mode=%d cnt=%d\n", mode, Count);
            {
                unsigned char *v = (unsigned char *)pointers;
                int index[Count];
                GrVertex vtx[Count];
                for (int i = 0; i < Count; i++) {
                    vertex3x.wrap(&vtx[i], VA_CONT(i));
                    index[i] = i;
                }
                grDrawPolygon(Count, index, vtx);
            }
            break;
        case GR_TRIANGLE_STRIP:
        case GR_TRIANGLE_STRIP_CONTINUE:
            {
                static GrVertex vbuf[6];
                static int vi = 0, vo = 0;
                unsigned char *v = (unsigned char *)pointers;
                if (mode == GR_TRIANGLE_STRIP) {
                    vi = 0; vo = 0;
                }
                for (int i = 0; i < Count; i++) {
                    if (abs(vi - vo) == 3) {
                        grDrawTriangle(&vbuf[vo], &vbuf[(vo+1)%6], &vbuf[(vo+2)%6]);
                        vo++; vo %= 6;
                    }
                    vertex3x.wrap(&vbuf[vi++], VA_CONT(i));
                    vi %= 6;
                }
                if (abs(vi - vo) == 3) {
                    grDrawTriangle(&vbuf[vo], &vbuf[(vo+1)%6], &vbuf[(vo+2)%6]);
                    vo++; vo %= 6;
                }
            }
            break;
        case GR_TRIANGLE_FAN:
        case GR_TRIANGLE_FAN_CONTINUE:
            {
                static GrVertex vbuf[5];
                static int vi = 0, vo = 0;
                unsigned char *v = (unsigned char *)pointers;
                int i = 0;
                if (mode == GR_TRIANGLE_FAN) {
                    vertex3x.wrap(&vbuf[4], v);
                    vi = 0; vo = 0; i = 1;
                }
                for (i; i < Count; i++) {
                    if (abs(vi - vo) == 2) {
                        grDrawTriangle(&vbuf[4], &vbuf[vo], &vbuf[(vo+1)%4]);
                        vo++; vo %= 4;
                    }
                    vertex3x.wrap(&vbuf[vi++], VA_CONT(i));
                    vi %= 4;
                }
                if (abs(vi - vo) == 2) {
                    grDrawTriangle(&vbuf[4], &vbuf[vo], &vbuf[(vo+1)%4]);
                    vo++; vo %= 4;
                }
            }
            break;
        case GR_TRIANGLES:
            {
                unsigned char *v = (unsigned char *)pointers;
                for (int i = 0; i < Count; i+=3)
                    wrap3x_grDrawTriangle(VA_CONT(i), VA_CONT(i+1), VA_CONT(i+2));
            }
            break;
        default:
            DPRINTF("Unsupported grDrawVertexArrayContinuous() mode %d\n", mode );
            break;
    }
}

BOOL APIENTRY DllMain( HINSTANCE hModule,
        DWORD dwReason,
        LPVOID lpReserved
        )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

