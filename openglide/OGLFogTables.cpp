
#include "OGLTables.h"

float tableIndexToW[ GR_FOG_TABLE_SIZE ] =
{
        1.000000f,      1.142857f,      1.333333f,      1.600000f, 
        2.000000f,      2.285714f,      2.666667f,      3.200000f, 
        4.000000f,      4.571429f,      5.333333f,      6.400000f, 
        8.000000f,      9.142858f,     10.666667f,     12.800000f, 
       16.000000f,     18.285715f,     21.333334f,     25.600000f, 
       32.000000f,     36.571430f,     42.666668f,     51.200001f, 
       64.000000f,     73.142860f,     85.333336f,    102.400002f, 
      128.000000f,    146.285721f,    170.666672f,    204.800003f, 
      256.000000f,    292.571442f,    341.333344f,    409.600006f, 
      512.000000f,    585.142883f,    682.666687f,    819.200012f, 
     1024.000000f,   1170.285767f,   1365.333374f,   1638.400024f, 
     2048.000000f,   2340.571533f,   2730.666748f,   3276.800049f, 
     4096.000000f,   4681.143066f,   5461.333496f,   6553.600098f, 
     8192.000000f,   9362.286133f,  10922.666992f,  13107.200195f, 
    16384.000000f,  18724.572266f,  21845.333984f,  26214.400391f, 
    32768.000000f,  37449.144531f,  43690.667969f,  52428.800781f
};

float OGLFogDistance(const float w)
{
    int i;
    for (i = 0; ((i < GR_FOG_TABLE_SIZE) && (tableIndexToW[i] < w)); i++);
    i = (i == GR_FOG_TABLE_SIZE)? (GR_FOG_TABLE_SIZE - 1):i;
    return Glide.FogTable[i] * D1OVER255;
}

