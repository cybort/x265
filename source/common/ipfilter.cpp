/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Deepthi Devaki <deepthidevaki@multicorewareinc.com>,
 *          Rajesh Paulraj <rajesh@multicorewareinc.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@multicorewareinc.com.
 *****************************************************************************/

#include "primitives.h"
#include "TLibCommon/TComRom.h"

#include <cstring>
#include <assert.h>

#if _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant, typical for templated functions
#endif

namespace {
template<int N>
void filterVertical_s_p(short *src, int srcStride, pixel *dst, int dstStride, int width, int height, short const *coeff)
{
    int cStride = srcStride;

    src -= (N / 2 - 1) * cStride;

    int offset;
    short maxVal;
    int headRoom = IF_INTERNAL_PREC - X265_DEPTH;
    int shift = IF_FILTER_PREC;

    shift += headRoom;
    offset = 1 << (shift - 1);
    offset +=  IF_INTERNAL_OFFS << IF_FILTER_PREC;
    maxVal = (1 << X265_DEPTH) - 1;

    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            int sum;

            sum  = src[col + 0 * cStride] * coeff[0];
            sum += src[col + 1 * cStride] * coeff[1];
            if (N >= 4)
            {
                sum += src[col + 2 * cStride] * coeff[2];
                sum += src[col + 3 * cStride] * coeff[3];
            }
            if (N >= 6)
            {
                sum += src[col + 4 * cStride] * coeff[4];
                sum += src[col + 5 * cStride] * coeff[5];
            }
            if (N == 8)
            {
                sum += src[col + 6 * cStride] * coeff[6];
                sum += src[col + 7 * cStride] * coeff[7];
            }

            short val = (short)((sum + offset) >> shift);

            val = (val < 0) ? 0 : val;
            val = (val > maxVal) ? maxVal : val;

            dst[col] = (pixel)val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

template<int N>
void filterHorizontal_p_p(pixel *src, int srcStride, pixel *dst, int dstStride, int width, int height, short const *coeff)
{
    int cStride = 1;

    src -= (N / 2 - 1) * cStride;

    int offset;
    short maxVal;
    int headRoom = IF_INTERNAL_PREC - X265_DEPTH;
    offset =  (1 << (headRoom - 1));
    maxVal = (1 << X265_DEPTH) - 1;

    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            int sum;

            sum  = src[col + 0 * cStride] * coeff[0];
            sum += src[col + 1 * cStride] * coeff[1];
            if (N >= 4)
            {
                sum += src[col + 2 * cStride] * coeff[2];
                sum += src[col + 3 * cStride] * coeff[3];
            }
            if (N >= 6)
            {
                sum += src[col + 4 * cStride] * coeff[4];
                sum += src[col + 5 * cStride] * coeff[5];
            }
            if (N == 8)
            {
                sum += src[col + 6 * cStride] * coeff[6];
                sum += src[col + 7 * cStride] * coeff[7];
            }

            short val = (short)(sum + offset) >> headRoom;

            if (val < 0) val = 0;
            if (val > maxVal) val = maxVal;
            dst[col] = (pixel)val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

template<int N>
void filterVertical_s_s(short *src, int srcStride, short *dst, int dstStride, int width, int height, short const *coeff)
{
    short c[8];

    c[0] = coeff[0];
    c[1] = coeff[1];
    if (N >= 4)
    {
        c[2] = coeff[2];
        c[3] = coeff[3];
    }
    if (N >= 6)
    {
        c[4] = coeff[4];
        c[5] = coeff[5];
    }
    if (N == 8)
    {
        c[6] = coeff[6];
        c[7] = coeff[7];
    }

    int cStride =  srcStride;
    src -= (N / 2 - 1) * cStride;
    int shift = IF_FILTER_PREC;
    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            int sum;

            sum  = src[col + 0 * cStride] * c[0];
            sum += src[col + 1 * cStride] * c[1];
            if (N >= 4)
            {
                sum += src[col + 2 * cStride] * c[2];
                sum += src[col + 3 * cStride] * c[3];
            }
            if (N >= 6)
            {
                sum += src[col + 4 * cStride] * c[4];
                sum += src[col + 5 * cStride] * c[5];
            }
            if (N == 8)
            {
                sum += src[col + 6 * cStride] * c[6];
                sum += src[col + 7 * cStride] * c[7];
            }

            short val = (short)((sum) >> shift);
            dst[col] = val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

template<int N>
void filterVertical_p_s(pixel *src, int srcStride, short *dst, int dstStride, int width, int height, short const *coeff)
{
    short c[8];

    c[0] = coeff[0];
    c[1] = coeff[1];
    if (N >= 4)
    {
        c[2] = coeff[2];
        c[3] = coeff[3];
    }
    if (N >= 6)
    {
        c[4] = coeff[4];
        c[5] = coeff[5];
    }
    if (N == 8)
    {
        c[6] = coeff[6];
        c[7] = coeff[7];
    }

    int Stride =  srcStride;
    src -= (N / 2 - 1) * Stride;
    Int offset;
    Int headRoom = IF_INTERNAL_PREC - X265_DEPTH;
    Int shift = IF_FILTER_PREC;

    shift -=  headRoom;
    offset = -IF_INTERNAL_OFFS << shift;

    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            int sum;

            sum  = src[col + 0 * Stride] * c[0];
            sum += src[col + 1 * Stride] * c[1];
            if (N >= 4)
            {
                sum += src[col + 2 * Stride] * c[2];
                sum += src[col + 3 * Stride] * c[3];
            }
            if (N >= 6)
            {
                sum += src[col + 4 * Stride] * c[4];
                sum += src[col + 5 * Stride] * c[5];
            }
            if (N == 8)
            {
                sum += src[col + 6 * Stride] * c[6];
                sum += src[col + 7 * Stride] * c[7];
            }

            short val = (short)((sum + offset) >> shift);
            dst[col] = val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

template<int N>
void filterHorizontal_p_s(pixel *src, int srcStride, short *dst, int dstStride, int width, int height, short const *coeff)
{
    src -= N / 2 - 1;

    int offset;
    int headRoom = IF_INTERNAL_PREC - X265_DEPTH;
    int shift = IF_FILTER_PREC;

    shift -= headRoom;
    offset = -IF_INTERNAL_OFFS << shift;

    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            int sum;

            sum  = src[col + 0] * coeff[0];
            sum += src[col + 1] * coeff[1];
            if (N >= 4)
            {
                sum += src[col + 2] * coeff[2];
                sum += src[col + 3] * coeff[3];
            }
            if (N >= 6)
            {
                sum += src[col + 4] * coeff[4];
                sum += src[col + 5] * coeff[5];
            }
            if (N == 8)
            {
                sum += src[col + 6] * coeff[6];
                sum += src[col + 7] * coeff[7];
            }

            short val = (short)(sum + offset) >> shift;
            dst[col] = val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

void filterConvertShortToPel(short *src, int srcStride, pixel *dst, int dstStride, int width, int height)
{
    int shift = IF_INTERNAL_PREC - X265_DEPTH;
    short offset = IF_INTERNAL_OFFS;

    offset += shift ? (1 << (shift - 1)) : 0;
    short maxVal = (1 << X265_DEPTH) - 1;
    short minVal = 0;
    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            short val = src[col];
            val = (val + offset) >> shift;
            if (val < minVal) val = minVal;
            if (val > maxVal) val = maxVal;
            dst[col] = (pixel)val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

void filterConvertPelToShort(pixel *src, int srcStride, short *dst, int dstStride, int width, int height)
{
    int shift = IF_INTERNAL_PREC - X265_DEPTH;
    int row, col;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            short val = src[col] << shift;
            dst[col] = val - (short)IF_INTERNAL_OFFS;
        }

        src += srcStride;
        dst += dstStride;
    }
}

template<int N>
void filterVertical_p_p(pixel *src, int srcStride, pixel *dst, int dstStride, int width, int height, short const *coeff)
{
    short c[8];

    c[0] = coeff[0];
    c[1] = coeff[1];
    if (N >= 4)
    {
        c[2] = coeff[2];
        c[3] = coeff[3];
    }
    if (N >= 6)
    {
        c[4] = coeff[4];
        c[5] = coeff[5];
    }
    if (N == 8)
    {
        c[6] = coeff[6];
        c[7] = coeff[7];
    }

    int Stride = srcStride;
    src -= (N / 2 - 1) * Stride;

    int offset;
    short maxVal;
    //int headRoom = IF_INTERNAL_PREC - X265_DEPTH;
    int shift = IF_FILTER_PREC;

    //shift += headRoom;
    offset = 1 << (shift - 1);
    //offset += IF_INTERNAL_OFFS << IF_FILTER_PREC;
    maxVal = (1 << X265_DEPTH) - 1;

    int row, col;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            int sum;

            sum  = src[col + 0 * Stride] * c[0];
            sum += src[col + 1 * Stride] * c[1];
            if (N >= 4)
            {
                sum += src[col + 2 * Stride] * c[2];
                sum += src[col + 3 * Stride] * c[3];
            }
            if (N >= 6)
            {
                sum += src[col + 4 * Stride] * c[4];
                sum += src[col + 5 * Stride] * c[5];
            }
            if (N == 8)
            {
                sum += src[col + 6 * Stride] * c[6];
                sum += src[col + 7 * Stride] * c[7];
            }

            short val = (short)((sum + offset) >> shift);
            val = (val < 0) ? 0 : val;
            val = (val > maxVal) ? maxVal : val;

            dst[col] = (pixel)val;
        }

        src += srcStride;
        dst += dstStride;
    }
}

void filterVertical_short_pel_multiplane(short *src, int srcStride, pixel *dstE, pixel *dstI, pixel *dstP, int dstStride, int block_width, int block_height)
{
    filterVertical_s_p<8>(src, srcStride, dstI, dstStride, block_width, block_height, g_lumaFilter[2]);
    filterVertical_s_p<8>(src, srcStride, dstE, dstStride, block_width, block_height, g_lumaFilter[1]);
    filterVertical_s_p<8>(src, srcStride, dstP, dstStride, block_width, block_height, g_lumaFilter[3]);
}

void extendPicCompBorder(pixel* txt, int stride, int width, int height, int marginX, int marginY)
{
    int   x, y;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < marginX; x++)
        {
            txt[-marginX + x] = txt[0];
            txt[width + x] = txt[width - 1];
        }

        txt += stride;
    }

    txt -= (stride + marginX);
    for (y = 0; y < marginY; y++)
    {
        ::memcpy(txt + (y + 1) * stride, txt, sizeof(pixel) * (width + (marginX << 1)));
    }

    txt -= ((height - 1) * stride);
    for (y = 0; y < marginY; y++)
    {
        ::memcpy(txt - (y + 1) * stride, txt, sizeof(pixel) * (width + (marginX << 1)));
    }
}

void filterVerticalMultiplaneExtend(short *src, int srcStride, pixel *dstE, pixel *dstI, pixel *dstP, int dstStride, int block_width, int block_height, int marginX, int marginY)
{
    filterVertical_s_p<8>(src, srcStride, dstI, dstStride, block_width, block_height, g_lumaFilter[2]);
    filterVertical_s_p<8>(src, srcStride, dstE, dstStride, block_width, block_height, g_lumaFilter[1]);
    filterVertical_s_p<8>(src, srcStride, dstP, dstStride, block_width, block_height, g_lumaFilter[3]);
    extendPicCompBorder(dstE, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstI, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstP, dstStride, block_width, block_height, marginX, marginY);
}

void filterHorizontalMultiplaneExtend(pixel *src, int srcStride, short *midF, short* midA, short* midB, short* midC, int midStride, pixel *pDstA, pixel *pDstB, pixel *pDstC, int pDstStride, int block_width, int block_height, int marginX, int marginY)
{
    filterConvertPelToShort(src, srcStride, midF, midStride, block_width, block_height);
    filterHorizontal_p_s<8>(src, srcStride, midB, midStride, block_width, block_height, g_lumaFilter[2]);
    filterHorizontal_p_s<8>(src, srcStride, midA, midStride, block_width, block_height, g_lumaFilter[1]);
    filterHorizontal_p_s<8>(src, srcStride, midC, midStride, block_width, block_height, g_lumaFilter[3]);
    filterConvertShortToPel(midA, midStride, pDstA, pDstStride, block_width, block_height);
    filterConvertShortToPel(midB, midStride, pDstB, pDstStride, block_width, block_height);
    filterConvertShortToPel(midC, midStride, pDstC, pDstStride, block_width, block_height);

    extendPicCompBorder(pDstA, pDstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(pDstB, pDstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(pDstC, pDstStride, block_width, block_height, marginX, marginY);
}

void weightUnidir(short *src, pixel *dst, int srcStride, int dstStride, int width, int height, int scale, int round, int shift, int offset)
{
    Int shiftNum = IF_INTERNAL_PREC - X265_DEPTH;
    shift = shift + shiftNum;
    round = shift ? (1 << (shift - 1)) : 0;

    int x, y;
    for (y = height - 1; y >= 0; y--)
    {
        for (x = width - 1; x >= 0; )
        {
            // note: luma min width is 4
            dst[x] = (pixel)Clip3(0, ((1 << X265_DEPTH) - 1), ((scale * (src[x] + IF_INTERNAL_OFFS) + round) >> shift) + offset);
            x--;
            dst[x] = (pixel)Clip3(0, ((1 << X265_DEPTH) - 1), ((scale * (src[x] + IF_INTERNAL_OFFS) + round) >> shift) + offset);
            x--;
        }

        src += srcStride;
        dst  += dstStride;
    }
}

// filterHorizontal, Multiplane, Weighted
void filterHorizontalWeighted(pixel *src, int srcStride, short *midF, short* midA, short* midB, short* midC, int midStride,
                              pixel *dstF, pixel *dstA, pixel *dstB, pixel *dstC, int dstStride, int block_width, int block_height,
                              int marginX, int marginY, int scale, int round, int shift, int offset)
{
    filterConvertPelToShort(src, srcStride, midF, midStride, block_width, block_height);
    filterHorizontal_p_s<8>(src, srcStride, midB, midStride, block_width, block_height, g_lumaFilter[2]);
    filterHorizontal_p_s<8>(src, srcStride, midA, midStride, block_width, block_height, g_lumaFilter[1]);
    filterHorizontal_p_s<8>(src, srcStride, midC, midStride, block_width, block_height, g_lumaFilter[3]);

    weightUnidir(midF, dstF, midStride, dstStride, block_width, block_height, scale, round, shift, offset);
    weightUnidir(midA, dstA, midStride, dstStride, block_width, block_height, scale, round, shift, offset);
    weightUnidir(midB, dstB, midStride, dstStride, block_width, block_height, scale, round, shift, offset);
    weightUnidir(midC, dstC, midStride, dstStride, block_width, block_height, scale, round, shift, offset);

    extendPicCompBorder(dstF, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstA, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstB, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstC, dstStride, block_width, block_height, marginX, marginY);
}

// filterVertical, Multiplane, Weighted
void filterVerticalWeighted(short *src, int srcStride, pixel *dstE, pixel *dstI, pixel *dstP,
                            int dstStride, int block_width, int block_height, int marginX, int marginY,
                            int scale, int round, int shift, int offset)
{
    short* intI, *intE, *intP;    
    int intStride = block_width;

    intI = (short*)X265_MALLOC(short, block_height * block_width);
    intE = (short*)X265_MALLOC(short, block_height * block_width);
    intP = (short*)X265_MALLOC(short, block_height * block_width);

    filterVertical_s_s<8>(src, srcStride, intI, intStride, block_width, block_height, g_lumaFilter[2]);
    filterVertical_s_s<8>(src, srcStride, intE, intStride, block_width, block_height, g_lumaFilter[1]);
    filterVertical_s_s<8>(src, srcStride, intP, intStride, block_width, block_height, g_lumaFilter[3]);

    weightUnidir(intI, dstI, intStride, dstStride,block_width, block_height, scale, round, shift, offset);
    weightUnidir(intE, dstE, intStride, dstStride,block_width, block_height, scale, round, shift, offset);
    weightUnidir(intP, dstP, intStride, dstStride,block_width, block_height, scale, round, shift, offset);

    extendPicCompBorder(dstE, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstI, dstStride, block_width, block_height, marginX, marginY);
    extendPicCompBorder(dstP, dstStride, block_width, block_height, marginX, marginY);

    X265_FREE(intI);
    X265_FREE(intE);
    X265_FREE(intP);
}
}

namespace x265 {
// x265 private namespace

void Setup_C_IPFilterPrimitives(EncoderPrimitives& p)
{
    p.ipfilter_pp[FILTER_H_P_P_8] = filterHorizontal_p_p<8>;
    p.ipfilter_ps[FILTER_H_P_S_8] = filterHorizontal_p_s<8>;
    p.ipfilter_ps[FILTER_V_P_S_8] = filterVertical_p_s<8>;
    p.ipfilter_sp[FILTER_V_S_P_8] = filterVertical_s_p<8>;
    p.ipfilter_pp[FILTER_H_P_P_4] = filterHorizontal_p_p<4>;
    p.ipfilter_ps[FILTER_H_P_S_4] = filterHorizontal_p_s<4>;
    p.ipfilter_ps[FILTER_V_P_S_4] = filterVertical_p_s<4>;
    p.ipfilter_sp[FILTER_V_S_P_4] = filterVertical_s_p<4>;
    p.ipfilter_pp[FILTER_V_P_P_8] = filterVertical_p_p<8>;
    p.ipfilter_pp[FILTER_V_P_P_4] = filterVertical_p_p<4>;
    p.ipfilter_ss[FILTER_V_S_S_8] = filterVertical_s_s<8>;
    p.ipfilter_ss[FILTER_V_S_S_4] = filterVertical_s_s<4>;

    p.ipfilter_p2s = filterConvertPelToShort;
    p.ipfilter_s2p = filterConvertShortToPel;

    p.filterVmulti = filterVerticalMultiplaneExtend;
    p.filterHmulti = filterHorizontalMultiplaneExtend;

    p.filterVwghtd = filterVerticalWeighted;         
    p.filterHwghtd = filterHorizontalWeighted;
}
}
