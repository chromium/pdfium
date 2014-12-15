// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_dib.h"
#include "../../../include/fxge/fx_ge.h"
#include "dib_int.h"
#include <limits.h>
extern int SDP_Table[513];
void CWeightTable::Calc(int dest_len, int dest_min, int dest_max, int src_len, int src_min, int src_max, int flags)
{
    if (m_pWeightTables) {
        FX_Free(m_pWeightTables);
        m_pWeightTables = NULL;
    }
    double scale, base;
    scale = FXSYS_Div((FX_FLOAT)(src_len), (FX_FLOAT)(dest_len));
    if (dest_len < 0) {
        base = (FX_FLOAT)(src_len);
    } else {
        base = 0;
    }
    int ext_size = flags & FXDIB_BICUBIC_INTERPOL ? 3 : 1;
    m_ItemSize = sizeof(int) * 2 + (int)(sizeof(int) * (FXSYS_ceil(FXSYS_fabs((FX_FLOAT)scale)) + ext_size));
    m_DestMin = dest_min;
    if ((dest_max - dest_min) > (int)((1U << 30) - 4) / m_ItemSize) {
        return;
    }
    m_pWeightTables = FX_AllocNL(FX_BYTE, (dest_max - dest_min) * m_ItemSize + 4);
    if (m_pWeightTables == NULL) {
        return;
    }
    if ((flags & FXDIB_NOSMOOTH) != 0 || FXSYS_fabs((FX_FLOAT)scale) < 1.0f) {
        for (int dest_pixel = dest_min; dest_pixel < dest_max; dest_pixel ++) {
            PixelWeight& pixel_weights = *GetPixelWeight(dest_pixel);
            double src_pos = dest_pixel * scale + scale / 2 + base;
            if (flags & FXDIB_INTERPOL) {
                pixel_weights.m_SrcStart = (int)FXSYS_floor((FX_FLOAT)src_pos - 1.0f / 2);
                pixel_weights.m_SrcEnd = (int)FXSYS_floor((FX_FLOAT)src_pos + 1.0f / 2);
                if (pixel_weights.m_SrcStart < src_min) {
                    pixel_weights.m_SrcStart = src_min;
                }
                if (pixel_weights.m_SrcEnd >= src_max) {
                    pixel_weights.m_SrcEnd = src_max - 1;
                }
                if (pixel_weights.m_SrcStart == pixel_weights.m_SrcEnd) {
                    pixel_weights.m_Weights[0] = 65536;
                } else {
                    pixel_weights.m_Weights[1] = FXSYS_round((FX_FLOAT)(src_pos - pixel_weights.m_SrcStart - 1.0f / 2) * 65536);
                    pixel_weights.m_Weights[0] = 65536 - pixel_weights.m_Weights[1];
                }
            } else if (flags & FXDIB_BICUBIC_INTERPOL) {
                pixel_weights.m_SrcStart = (int)FXSYS_floor((FX_FLOAT)src_pos - 1.0f / 2);
                pixel_weights.m_SrcEnd = (int)FXSYS_floor((FX_FLOAT)src_pos + 1.0f / 2);
                int start = pixel_weights.m_SrcStart - 1;
                int end = pixel_weights.m_SrcEnd + 1;
                if (start < src_min) {
                    start = src_min;
                }
                if (end >= src_max) {
                    end = src_max - 1;
                }
                if (pixel_weights.m_SrcStart < src_min) {
                    src_pos += src_min - pixel_weights.m_SrcStart;
                    pixel_weights.m_SrcStart = src_min;
                }
                if (pixel_weights.m_SrcEnd >= src_max) {
                    pixel_weights.m_SrcEnd = src_max - 1;
                }
                int weight;
                weight = FXSYS_round((FX_FLOAT)(src_pos - pixel_weights.m_SrcStart - 1.0f / 2) * 256);
                if (start == end) {
                    pixel_weights.m_Weights[0] = (SDP_Table[256 + weight] + SDP_Table[weight] + SDP_Table[256 - weight] + SDP_Table[512 - weight]) << 8;
                } else if ((start == pixel_weights.m_SrcStart && (pixel_weights.m_SrcStart == pixel_weights.m_SrcEnd ||
                            end == pixel_weights.m_SrcEnd) && start < end) || (start < pixel_weights.m_SrcStart && pixel_weights.m_SrcStart == pixel_weights.m_SrcEnd && end == pixel_weights.m_SrcEnd)) {
                    if (start < pixel_weights.m_SrcStart) {
                        pixel_weights.m_Weights[0] = SDP_Table[256 + weight] << 8;
                        pixel_weights.m_Weights[1] = (SDP_Table[weight] + SDP_Table[256 - weight] + SDP_Table[512 - weight]) << 8;
                    } else {
                        if (pixel_weights.m_SrcStart == pixel_weights.m_SrcEnd) {
                            pixel_weights.m_Weights[0] = (SDP_Table[256 + weight] + SDP_Table[weight] + SDP_Table[256 - weight]) << 8;
                            pixel_weights.m_Weights[1] = SDP_Table[512 - weight] << 8;
                        } else {
                            pixel_weights.m_Weights[0] = (SDP_Table[256 + weight] + SDP_Table[weight]) << 8;
                            pixel_weights.m_Weights[1] = (SDP_Table[256 - weight] + SDP_Table[512 - weight]) << 8;
                        }
                    }
                    if (pixel_weights.m_SrcStart == pixel_weights.m_SrcEnd) {
                        pixel_weights.m_SrcEnd = end;
                    }
                    if (start < pixel_weights.m_SrcStart) {
                        pixel_weights.m_SrcStart = start;
                    }
                } else if (start == pixel_weights.m_SrcStart &&
                           start < pixel_weights.m_SrcEnd &&
                           pixel_weights.m_SrcEnd < end) {
                    pixel_weights.m_Weights[0] = (SDP_Table[256 + weight] + SDP_Table[weight]) << 8;
                    pixel_weights.m_Weights[1] = SDP_Table[256 - weight] << 8;
                    pixel_weights.m_Weights[2] = SDP_Table[512 - weight] << 8;
                    pixel_weights.m_SrcEnd = end;
                } else if (start < pixel_weights.m_SrcStart &&
                           pixel_weights.m_SrcStart < pixel_weights.m_SrcEnd &&
                           pixel_weights.m_SrcEnd == end) {
                    pixel_weights.m_Weights[0] = SDP_Table[256 + weight] << 8;
                    pixel_weights.m_Weights[1] = SDP_Table[weight] << 8;
                    pixel_weights.m_Weights[2] = (SDP_Table[256 - weight] + SDP_Table[512 - weight]) << 8;
                    pixel_weights.m_SrcStart = start;
                } else {
                    pixel_weights.m_Weights[0] = SDP_Table[256 + weight] << 8;
                    pixel_weights.m_Weights[1] = SDP_Table[weight] << 8;
                    pixel_weights.m_Weights[2] = SDP_Table[256 - weight] << 8;
                    pixel_weights.m_Weights[3] = SDP_Table[512 - weight] << 8;
                    pixel_weights.m_SrcStart = start;
                    pixel_weights.m_SrcEnd = end;
                }
            } else {
                pixel_weights.m_SrcStart = pixel_weights.m_SrcEnd = (int)FXSYS_floor((FX_FLOAT)src_pos);
                if (pixel_weights.m_SrcStart < src_min) {
                    pixel_weights.m_SrcStart = src_min;
                }
                if (pixel_weights.m_SrcEnd >= src_max) {
                    pixel_weights.m_SrcEnd = src_max - 1;
                }
                pixel_weights.m_Weights[0] = 65536;
            }
        }
        return;
    }
    for (int dest_pixel = dest_min; dest_pixel < dest_max; dest_pixel ++) {
        PixelWeight& pixel_weights = *GetPixelWeight(dest_pixel);
        double src_start = dest_pixel * scale + base;
        double src_end = src_start + scale;
        int start_i, end_i;
        if (src_start < src_end) {
            start_i = (int)FXSYS_floor((FX_FLOAT)src_start);
            end_i = (int)FXSYS_ceil((FX_FLOAT)src_end);
        } else {
            start_i = (int)FXSYS_floor((FX_FLOAT)src_end);
            end_i = (int)FXSYS_ceil((FX_FLOAT)src_start);
        }
        if (start_i < src_min) {
            start_i = src_min;
        }
        if (end_i >= src_max) {
            end_i = src_max - 1;
        }
        if (start_i > end_i) {
            if (start_i >= src_max) {
                start_i = src_max - 1;
            }
            pixel_weights.m_SrcStart = start_i;
            pixel_weights.m_SrcEnd = start_i;
            continue;
        }
        pixel_weights.m_SrcStart = start_i;
        pixel_weights.m_SrcEnd = end_i;
        for (int j = start_i; j <= end_i; j ++) {
            double dest_start = FXSYS_Div((FX_FLOAT)(j) - base, scale);
            double dest_end = FXSYS_Div((FX_FLOAT)(j + 1) - base, scale);
            if (dest_start > dest_end) {
                double temp = dest_start;
                dest_start = dest_end;
                dest_end = temp;
            }
            double area_start = dest_start > (FX_FLOAT)(dest_pixel) ? dest_start : (FX_FLOAT)(dest_pixel);
            double area_end = dest_end > (FX_FLOAT)(dest_pixel + 1) ? (FX_FLOAT)(dest_pixel + 1) : dest_end;
            double weight = area_start >= area_end ? 0.0f : area_end - area_start;
            if (weight == 0 && j == end_i) {
                pixel_weights.m_SrcEnd --;
                break;
            }
            pixel_weights.m_Weights[j - start_i] = FXSYS_round((FX_FLOAT)(weight * 65536));
        }
    }
}
CStretchEngine::CStretchEngine(IFX_ScanlineComposer* pDestBitmap, FXDIB_Format dest_format,
                               int dest_width, int dest_height, const FX_RECT& clip_rect,
                               const CFX_DIBSource* pSrcBitmap, int flags)
{
    m_State = 0;
    m_DestFormat = dest_format;
    m_DestBpp = dest_format & 0xff;
    m_SrcBpp = pSrcBitmap->GetFormat() & 0xff;
    m_bHasAlpha = pSrcBitmap->GetFormat() & 0x200;
    m_pSrcPalette = pSrcBitmap->GetPalette();
    m_pDestBitmap = pDestBitmap;
    m_DestWidth = dest_width;
    m_DestHeight = dest_height;
    m_pInterBuf = NULL;
    m_pExtraAlphaBuf = NULL;
    m_pDestMaskScanline = NULL;
    m_DestClip = clip_rect;
    FX_DWORD size = clip_rect.Width();
    if (size && m_DestBpp > (int)(INT_MAX / size)) {
        return;
    }
    size *= m_DestBpp;
    if (size > INT_MAX - 31) {
        return;
    }
    size += 31;
    size = size / 32 * 4;
    m_pDestScanline = FX_AllocNL(FX_BYTE, size);
    if (m_pDestScanline == NULL) {
        return;
    }
    if (dest_format == FXDIB_Rgb32) {
        FXSYS_memset8(m_pDestScanline, 255, size);
    }
    m_InterPitch = (m_DestClip.Width() * m_DestBpp + 31) / 32 * 4;
    m_ExtraMaskPitch = (m_DestClip.Width() * 8 + 31) / 32 * 4;
    m_pInterBuf = NULL;
    m_pSource = pSrcBitmap;
    m_SrcWidth = pSrcBitmap->GetWidth();
    m_SrcHeight = pSrcBitmap->GetHeight();
    m_SrcPitch = (m_SrcWidth * m_SrcBpp + 31) / 32 * 4;
    if ((flags & FXDIB_NOSMOOTH) == 0) {
        FX_BOOL bInterpol = flags & FXDIB_INTERPOL || flags & FXDIB_BICUBIC_INTERPOL;
        if (!bInterpol && FXSYS_abs(dest_width) != 0 && FXSYS_abs(dest_height) < m_SrcWidth * m_SrcHeight * 8 / FXSYS_abs(dest_width)) {
            flags = FXDIB_INTERPOL;
        }
        m_Flags = flags;
    } else {
        m_Flags = FXDIB_NOSMOOTH;
        if (flags & FXDIB_DOWNSAMPLE) {
            m_Flags |= FXDIB_DOWNSAMPLE;
        }
    }
    double scale_x = FXSYS_Div((FX_FLOAT)(m_SrcWidth), (FX_FLOAT)(m_DestWidth));
    double scale_y = FXSYS_Div((FX_FLOAT)(m_SrcHeight), (FX_FLOAT)(m_DestHeight));
    double base_x = m_DestWidth > 0 ? 0.0f : (FX_FLOAT)(m_DestWidth);
    double base_y = m_DestHeight > 0 ? 0.0f : (FX_FLOAT)(m_DestHeight);
    double src_left = FXSYS_Mul(scale_x, (FX_FLOAT)(clip_rect.left) + base_x);
    double src_right = FXSYS_Mul(scale_x, (FX_FLOAT)(clip_rect.right) + base_x);
    double src_top = FXSYS_Mul(scale_y, (FX_FLOAT)(clip_rect.top) + base_y);
    double src_bottom = FXSYS_Mul(scale_y, (FX_FLOAT)(clip_rect.bottom) + base_y);
    if (src_left > src_right) {
        double temp = src_left;
        src_left = src_right;
        src_right = temp;
    }
    if (src_top > src_bottom) {
        double temp = src_top;
        src_top = src_bottom;
        src_bottom = temp;
    }
    m_SrcClip.left = (int)FXSYS_floor((FX_FLOAT)src_left);
    m_SrcClip.right = (int)FXSYS_ceil((FX_FLOAT)src_right);
    m_SrcClip.top = (int)FXSYS_floor((FX_FLOAT)src_top);
    m_SrcClip.bottom = (int)FXSYS_ceil((FX_FLOAT)src_bottom);
    FX_RECT src_rect(0, 0, m_SrcWidth, m_SrcHeight);
    m_SrcClip.Intersect(src_rect);
    if (m_SrcBpp == 1) {
        if (m_DestBpp == 8) {
            m_TransMethod = 1;
        } else {
            m_TransMethod = 2;
        }
    } else if (m_SrcBpp == 8) {
        if (m_DestBpp == 8) {
            if (!m_bHasAlpha) {
                m_TransMethod = 3;
            } else {
                m_TransMethod = 4;
            }
        } else {
            if (!m_bHasAlpha) {
                m_TransMethod = 5;
            } else {
                m_TransMethod = 6;
            }
        }
    } else {
        if (!m_bHasAlpha) {
            m_TransMethod = 7;
        } else {
            m_TransMethod = 8;
        }
    }
}
FX_BOOL CStretchEngine::Continue(IFX_Pause* pPause)
{
    while (m_State == 1) {
        if (ContinueStretchHorz(pPause)) {
            return TRUE;
        }
        m_State = 2;
        StretchVert();
    }
    return FALSE;
}
CStretchEngine::~CStretchEngine()
{
    if (m_pDestScanline) {
        FX_Free(m_pDestScanline);
    }
    if (m_pInterBuf) {
        FX_Free(m_pInterBuf);
    }
    if (m_pExtraAlphaBuf) {
        FX_Free(m_pExtraAlphaBuf);
    }
    if (m_pDestMaskScanline) {
        FX_Free(m_pDestMaskScanline);
    }
}
FX_BOOL CStretchEngine::StartStretchHorz()
{
    if (m_DestWidth == 0 || m_pDestScanline == NULL || m_SrcClip.Height() > (int)((1U << 29) / m_InterPitch) || m_SrcClip.Height() == 0) {
        return FALSE;
    }
    m_pInterBuf = FX_AllocNL(unsigned char, m_SrcClip.Height() * m_InterPitch);
    if (m_pInterBuf == NULL) {
        return FALSE;
    }
    if (m_pSource && m_bHasAlpha && m_pSource->m_pAlphaMask) {
        m_pExtraAlphaBuf = FX_Alloc(unsigned char, m_SrcClip.Height() * m_ExtraMaskPitch);
        if (!m_pExtraAlphaBuf) {
            return FALSE;
        }
        FX_DWORD size = (m_DestClip.Width() * 8 + 31) / 32 * 4;
        m_pDestMaskScanline = FX_AllocNL(unsigned char, size);
        if (!m_pDestMaskScanline) {
            return FALSE;
        }
    }
    m_WeightTable.Calc(m_DestWidth, m_DestClip.left, m_DestClip.right, m_SrcWidth, m_SrcClip.left, m_SrcClip.right, m_Flags);
    if (m_WeightTable.m_pWeightTables == NULL) {
        return FALSE;
    }
    m_CurRow = m_SrcClip.top;
    m_State = 1;
    return TRUE;
}
#define FX_STRECH_PAUSE_ROWS	10
FX_BOOL CStretchEngine::ContinueStretchHorz(IFX_Pause* pPause)
{
    if (!m_DestWidth) {
        return 0;
    }
    if (m_pSource->SkipToScanline(m_CurRow, pPause)) {
        return TRUE;
    }
    int Bpp = m_DestBpp / 8;
    int rows_to_go = FX_STRECH_PAUSE_ROWS;
    for (; m_CurRow < m_SrcClip.bottom; m_CurRow ++) {
        if (rows_to_go == 0) {
            if (pPause && pPause->NeedToPauseNow()) {
                return TRUE;
            } else {
                rows_to_go = FX_STRECH_PAUSE_ROWS;
            }
        }
        FX_LPCBYTE src_scan = m_pSource->GetScanline(m_CurRow);
        FX_LPBYTE dest_scan = m_pInterBuf + (m_CurRow - m_SrcClip.top) * m_InterPitch;
        FX_LPCBYTE src_scan_mask = NULL;
        FX_LPBYTE dest_scan_mask = NULL;
        if (m_pExtraAlphaBuf) {
            src_scan_mask = m_pSource->m_pAlphaMask->GetScanline(m_CurRow);
            dest_scan_mask = m_pExtraAlphaBuf + (m_CurRow - m_SrcClip.top) * m_ExtraMaskPitch;
        }
        switch (m_TransMethod) {
            case 1:
            case 2: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_a = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            if (src_scan[j / 8] & (1 << (7 - j % 8))) {
                                dest_a += pixel_weight * 255;
                            }
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_a = dest_a < 0 ? 0 : dest_a > 16711680 ? 16711680 : dest_a;
                        }
                        *dest_scan++ = (FX_BYTE)(dest_a >> 16);
                    }
                    break;
                }
            case 3: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_a = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            dest_a += pixel_weight * src_scan[j];
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_a = dest_a < 0 ? 0 : dest_a > 16711680 ? 16711680 : dest_a;
                        }
                        *dest_scan++ = (FX_BYTE)(dest_a >> 16);
                    }
                    break;
                }
            case 4: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_a = 0, dest_r = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            pixel_weight = pixel_weight * src_scan_mask[j] / 255;
                            dest_r += pixel_weight * src_scan[j];
                            dest_a += pixel_weight;
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_r = dest_r < 0 ? 0 : dest_r > 16711680 ? 16711680 : dest_r;
                            dest_a = dest_a < 0 ? 0 : dest_a > 65536 ? 65536 : dest_a;
                        }
                        *dest_scan++ = (FX_BYTE)(dest_r >> 16);
                        *dest_scan_mask++ = (FX_BYTE)((dest_a * 255) >> 16);
                    }
                    break;
                }
            case 5: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_r_y = 0, dest_g_m = 0, dest_b_c = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            unsigned long argb_cmyk = m_pSrcPalette[src_scan[j]];
                            if (m_DestFormat == FXDIB_Rgb) {
                                dest_r_y += pixel_weight * (FX_BYTE)(argb_cmyk >> 16);
                                dest_g_m += pixel_weight * (FX_BYTE)(argb_cmyk >> 8);
                                dest_b_c += pixel_weight * (FX_BYTE)argb_cmyk;
                            } else {
                                dest_b_c += pixel_weight * (FX_BYTE)(argb_cmyk >> 24);
                                dest_g_m += pixel_weight * (FX_BYTE)(argb_cmyk >> 16);
                                dest_r_y += pixel_weight * (FX_BYTE)(argb_cmyk >> 8);
                            }
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_r_y = dest_r_y < 0 ? 0 : dest_r_y > 16711680 ? 16711680 : dest_r_y;
                            dest_g_m = dest_g_m < 0 ? 0 : dest_g_m > 16711680 ? 16711680 : dest_g_m;
                            dest_b_c = dest_b_c < 0 ? 0 : dest_b_c > 16711680 ? 16711680 : dest_b_c;
                        }
                        *dest_scan++ = (FX_BYTE)(dest_b_c >> 16);
                        *dest_scan++ = (FX_BYTE)(dest_g_m >> 16);
                        *dest_scan++ = (FX_BYTE)(dest_r_y >> 16);
                    }
                    break;
                }
            case 6: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_a = 0, dest_r_y = 0, dest_g_m = 0, dest_b_c = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            pixel_weight = pixel_weight * src_scan_mask[j] / 255;
                            unsigned long argb_cmyk = m_pSrcPalette[src_scan[j]];
                            if (m_DestFormat == FXDIB_Rgba) {
                                dest_r_y += pixel_weight * (FX_BYTE)(argb_cmyk >> 16);
                                dest_g_m += pixel_weight * (FX_BYTE)(argb_cmyk >> 8);
                                dest_b_c += pixel_weight * (FX_BYTE)argb_cmyk;
                            } else {
                                dest_b_c += pixel_weight * (FX_BYTE)(argb_cmyk >> 24);
                                dest_g_m += pixel_weight * (FX_BYTE)(argb_cmyk >> 16);
                                dest_r_y += pixel_weight * (FX_BYTE)(argb_cmyk >> 8);
                            }
                            dest_a += pixel_weight;
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_b_c = dest_b_c < 0 ? 0 : dest_b_c > 16711680 ? 16711680 : dest_b_c;
                            dest_g_m = dest_g_m < 0 ? 0 : dest_g_m > 16711680 ? 16711680 : dest_g_m;
                            dest_r_y = dest_r_y < 0 ? 0 : dest_r_y > 16711680 ? 16711680 : dest_r_y;
                            dest_a = dest_a < 0 ? 0 : dest_a > 65536 ? 65536 : dest_a;
                        }
                        *dest_scan++ = (FX_BYTE)(dest_b_c >> 16);
                        *dest_scan++ = (FX_BYTE)(dest_g_m >> 16);
                        *dest_scan++ = (FX_BYTE)(dest_r_y >> 16);
                        *dest_scan_mask++ = (FX_BYTE)((dest_a * 255) >> 16);
                    }
                    break;
                }
            case 7: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_r_y = 0, dest_g_m = 0, dest_b_c = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            FX_LPCBYTE src_pixel = src_scan + j * Bpp;
                            dest_b_c += pixel_weight * (*src_pixel++);
                            dest_g_m += pixel_weight * (*src_pixel++);
                            dest_r_y += pixel_weight * (*src_pixel);
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_b_c = dest_b_c < 0 ? 0 : dest_b_c > 16711680 ? 16711680 : dest_b_c;
                            dest_g_m = dest_g_m < 0 ? 0 : dest_g_m > 16711680 ? 16711680 : dest_g_m;
                            dest_r_y = dest_r_y < 0 ? 0 : dest_r_y > 16711680 ? 16711680 : dest_r_y;
                        }
                        *dest_scan++ = (FX_BYTE)((dest_b_c) >> 16);
                        *dest_scan++ = (FX_BYTE)((dest_g_m) >> 16);
                        *dest_scan++ = (FX_BYTE)((dest_r_y) >> 16);
                        dest_scan += Bpp - 3;
                    }
                    break;
                }
            case 8: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        PixelWeight* pPixelWeights = m_WeightTable.GetPixelWeight(col);
                        int dest_a = 0, dest_r_y = 0, dest_g_m = 0, dest_b_c = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            FX_LPCBYTE src_pixel = src_scan + j * Bpp;
                            if (m_DestFormat == FXDIB_Argb) {
                                pixel_weight = pixel_weight * src_pixel[3] / 255;
                            } else {
                                pixel_weight = pixel_weight * src_scan_mask[j] / 255;
                            }
                            dest_b_c += pixel_weight * (*src_pixel++);
                            dest_g_m += pixel_weight * (*src_pixel++);
                            dest_r_y += pixel_weight * (*src_pixel);
                            dest_a += pixel_weight;
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_r_y = dest_r_y < 0 ? 0 : dest_r_y > 16711680 ? 16711680 : dest_r_y;
                            dest_g_m = dest_g_m < 0 ? 0 : dest_g_m > 16711680 ? 16711680 : dest_g_m;
                            dest_b_c = dest_b_c < 0 ? 0 : dest_b_c > 16711680 ? 16711680 : dest_b_c;
                            dest_a = dest_a < 0 ? 0 : dest_a > 65536 ? 65536 : dest_a;
                        }
                        *dest_scan++ = (FX_BYTE)((dest_b_c) >> 16);
                        *dest_scan++ = (FX_BYTE)((dest_g_m) >> 16);
                        *dest_scan++ = (FX_BYTE)((dest_r_y) >> 16);
                        if (m_DestFormat == FXDIB_Argb) {
                            *dest_scan = (FX_BYTE)((dest_a * 255) >> 16);
                        }
                        if (dest_scan_mask) {
                            *dest_scan_mask++ = (FX_BYTE)((dest_a * 255) >> 16);
                        }
                        dest_scan += Bpp - 3;
                    }
                    break;
                }
        }
        rows_to_go --;
    }
    return FALSE;
}
void CStretchEngine::StretchVert()
{
    if (m_DestHeight == 0) {
        return;
    }
    CWeightTable table;
    table.Calc(m_DestHeight, m_DestClip.top, m_DestClip.bottom, m_SrcHeight, m_SrcClip.top, m_SrcClip.bottom, m_Flags);
    if (table.m_pWeightTables == NULL) {
        return;
    }
    int DestBpp = m_DestBpp / 8;
    for (int row = m_DestClip.top; row < m_DestClip.bottom; row ++) {
        unsigned char* dest_scan = m_pDestScanline;
        unsigned char* dest_sacn_mask = m_pDestMaskScanline;
        PixelWeight* pPixelWeights = table.GetPixelWeight(row);
        switch(m_TransMethod) {
            case 1:
            case 2:
            case 3: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        unsigned char* src_scan = m_pInterBuf + (col - m_DestClip.left) * DestBpp;
                        int dest_a = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            dest_a += pixel_weight * src_scan[(j - m_SrcClip.top) * m_InterPitch];
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_a = dest_a < 0 ? 0 : dest_a > 16711680 ? 16711680 : dest_a;
                        }
                        *dest_scan = (FX_BYTE)(dest_a >> 16);
                        dest_scan += DestBpp;
                    }
                    break;
                }
            case 4: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        unsigned char* src_scan = m_pInterBuf + (col - m_DestClip.left) * DestBpp;
                        unsigned char* src_scan_mask = m_pExtraAlphaBuf + (col - m_DestClip.left);
                        int dest_a = 0, dest_k = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            dest_k += pixel_weight * src_scan[(j - m_SrcClip.top) * m_InterPitch];
                            dest_a += pixel_weight * src_scan_mask[(j - m_SrcClip.top) * m_ExtraMaskPitch];
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_k = dest_k < 0 ? 0 : dest_k > 16711680 ? 16711680 : dest_k;
                            dest_a = dest_a < 0 ? 0 : dest_a > 16711680 ? 16711680 : dest_a;
                        }
                        *dest_scan = (FX_BYTE)(dest_k >> 16);
                        dest_scan += DestBpp;
                        *dest_sacn_mask++ = (FX_BYTE)(dest_a >> 16);
                    }
                    break;
                }
            case 5:
            case 7: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        unsigned char* src_scan = m_pInterBuf + (col - m_DestClip.left) * DestBpp;
                        int dest_r_y = 0, dest_g_m = 0, dest_b_c = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            FX_LPCBYTE src_pixel = src_scan + (j - m_SrcClip.top) * m_InterPitch;
                            dest_b_c += pixel_weight * (*src_pixel++);
                            dest_g_m += pixel_weight * (*src_pixel++);
                            dest_r_y += pixel_weight * (*src_pixel);
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_r_y = dest_r_y < 0 ? 0 : dest_r_y > 16711680 ? 16711680 : dest_r_y;
                            dest_g_m = dest_g_m < 0 ? 0 : dest_g_m > 16711680 ? 16711680 : dest_g_m;
                            dest_b_c = dest_b_c < 0 ? 0 : dest_b_c > 16711680 ? 16711680 : dest_b_c;
                        }
                        dest_scan[0] = (FX_BYTE)((dest_b_c) >> 16);
                        dest_scan[1] = (FX_BYTE)((dest_g_m) >> 16);
                        dest_scan[2] = (FX_BYTE)((dest_r_y) >> 16);
                        dest_scan += DestBpp;
                    }
                    break;
                }
            case 6:
            case 8: {
                    for (int col = m_DestClip.left; col < m_DestClip.right; col ++) {
                        unsigned char* src_scan = m_pInterBuf + (col - m_DestClip.left) * DestBpp;
                        unsigned char* src_scan_mask = NULL;
                        if (m_DestFormat != FXDIB_Argb) {
                            src_scan_mask = m_pExtraAlphaBuf + (col - m_DestClip.left);
                        }
                        int dest_a = 0, dest_r_y = 0, dest_g_m = 0, dest_b_c = 0;
                        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd; j ++) {
                            int pixel_weight = pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
                            FX_LPCBYTE src_pixel = src_scan + (j - m_SrcClip.top) * m_InterPitch;
                            int mask_v = 255;
                            if (src_scan_mask) {
                                mask_v = src_scan_mask[(j - m_SrcClip.top) * m_ExtraMaskPitch];
                            }
                            dest_b_c += pixel_weight * (*src_pixel++);
                            dest_g_m += pixel_weight * (*src_pixel++);
                            dest_r_y += pixel_weight * (*src_pixel);
                            if (m_DestFormat == FXDIB_Argb) {
                                dest_a += pixel_weight * (*(src_pixel + 1));
                            } else {
                                dest_a += pixel_weight * mask_v;
                            }
                        }
                        if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                            dest_r_y = dest_r_y < 0 ? 0 : dest_r_y > 16711680 ? 16711680 : dest_r_y;
                            dest_g_m = dest_g_m < 0 ? 0 : dest_g_m > 16711680 ? 16711680 : dest_g_m;
                            dest_b_c = dest_b_c < 0 ? 0 : dest_b_c > 16711680 ? 16711680 : dest_b_c;
                            dest_a = dest_a < 0 ? 0 : dest_a > 16711680 ? 16711680 : dest_a;
                        }
                        if (dest_a) {
                            int r = ((FX_DWORD)dest_r_y) * 255 / dest_a;
                            int g = ((FX_DWORD)dest_g_m) * 255 / dest_a;
                            int b = ((FX_DWORD)dest_b_c) * 255 / dest_a;
                            dest_scan[0] = b > 255 ? 255 : b < 0 ? 0 : b;
                            dest_scan[1] = g > 255 ? 255 : g < 0 ? 0 : g;
                            dest_scan[2] = r > 255 ? 255 : r < 0 ? 0 : r;
                        }
                        if (m_DestFormat == FXDIB_Argb) {
                            dest_scan[3] = (FX_BYTE)((dest_a) >> 16);
                        } else {
                            *dest_sacn_mask = (FX_BYTE)((dest_a) >> 16);
                        }
                        dest_scan += DestBpp;
                        if (dest_sacn_mask) {
                            dest_sacn_mask++;
                        }
                    }
                    break;
                }
        }
        m_pDestBitmap->ComposeScanline(row - m_DestClip.top, m_pDestScanline, m_pDestMaskScanline);
    }
}
CFX_ImageStretcher::CFX_ImageStretcher()
{
    m_pScanline = NULL;
    m_pStretchEngine = NULL;
    m_pMaskScanline = NULL;
}
CFX_ImageStretcher::~CFX_ImageStretcher()
{
    if (m_pScanline) {
        FX_Free(m_pScanline);
    }
    if (m_pStretchEngine) {
        delete m_pStretchEngine;
    }
    if (m_pMaskScanline) {
        FX_Free(m_pMaskScanline);
    }
}
FXDIB_Format _GetStretchedFormat(const CFX_DIBSource* pSrc)
{
    FXDIB_Format format = pSrc->GetFormat();
    if (format == FXDIB_1bppMask) {
        format = FXDIB_8bppMask;
    } else if (format == FXDIB_1bppRgb) {
        format = FXDIB_8bppRgb;
    } else if (format == FXDIB_8bppRgb) {
        if (pSrc->GetPalette()) {
            format = FXDIB_Rgb;
        }
    }
    return format;
}
FX_BOOL CFX_ImageStretcher::Start(IFX_ScanlineComposer* pDest,
                                  const CFX_DIBSource* pSource, int dest_width, int dest_height,
                                  const FX_RECT& rect, FX_DWORD flags)
{
    m_DestFormat = _GetStretchedFormat(pSource);
    m_DestBPP = m_DestFormat & 0xff;
    m_pDest = pDest;
    m_pSource = pSource;
    m_DestWidth = dest_width;
    m_DestHeight = dest_height;
    m_ClipRect = rect;
    m_Flags = flags;
    if (pSource->GetFormat() == FXDIB_1bppRgb && pSource->GetPalette()) {
        FX_ARGB pal[256];
        int a0, r0, g0, b0, a1, r1, g1, b1;
        ArgbDecode(pSource->GetPaletteEntry(0), a0, r0, g0, b0);
        ArgbDecode(pSource->GetPaletteEntry(1), a1, r1, g1, b1);
        for (int i = 0; i < 256; i ++) {
            int a = a0 + (a1 - a0) * i / 255;
            int r = r0 + (r1 - r0) * i / 255;
            int g = g0 + (g1 - g0) * i / 255;
            int b = b0 + (b1 - b0) * i / 255;
            pal[i] = ArgbEncode(a, r, g, b);
        }
        if (!pDest->SetInfo(rect.Width(), rect.Height(), m_DestFormat, pal)) {
            return FALSE;
        }
    } else if (pSource->GetFormat() == FXDIB_1bppCmyk && pSource->GetPalette()) {
        FX_CMYK pal[256];
        int c0, m0, y0, k0, c1, m1, y1, k1;
        CmykDecode(pSource->GetPaletteEntry(0), c0, m0, y0, k0);
        CmykDecode(pSource->GetPaletteEntry(1), c1, m1, y1, k1);
        for (int i = 0; i < 256; i ++) {
            int c = c0 + (c1 - c0) * i / 255;
            int m = m0 + (m1 - m0) * i / 255;
            int y = y0 + (y1 - y0) * i / 255;
            int k = k0 + (k1 - k0) * i / 255;
            pal[i] = CmykEncode(c, m, y, k);
        }
        if (!pDest->SetInfo(rect.Width(), rect.Height(), m_DestFormat, pal)) {
            return FALSE;
        }
    } else if (!pDest->SetInfo(rect.Width(), rect.Height(), m_DestFormat, NULL)) {
        return FALSE;
    }
    if (flags & FXDIB_DOWNSAMPLE) {
        return StartQuickStretch();
    } else {
        return StartStretch();
    }
}
FX_BOOL CFX_ImageStretcher::Continue(IFX_Pause* pPause)
{
    if (m_Flags & FXDIB_DOWNSAMPLE) {
        return ContinueQuickStretch(pPause);
    } else {
        return ContinueStretch(pPause);
    }
}
#define MAX_PROGRESSIVE_STRETCH_PIXELS	1000000
FX_BOOL CFX_ImageStretcher::StartStretch()
{
    m_pStretchEngine = FX_NEW CStretchEngine(m_pDest, m_DestFormat, m_DestWidth, m_DestHeight, m_ClipRect, m_pSource, m_Flags);
    if (!m_pStretchEngine) {
        return FALSE;
    }
    m_pStretchEngine->StartStretchHorz();
    if (m_pSource->GetWidth() * m_pSource->GetHeight() < MAX_PROGRESSIVE_STRETCH_PIXELS) {
        m_pStretchEngine->Continue(NULL);
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CFX_ImageStretcher::ContinueStretch(IFX_Pause* pPause)
{
    if (m_pStretchEngine == NULL) {
        return FALSE;
    }
    return m_pStretchEngine->Continue(pPause);
}
FX_BOOL CFX_ImageStretcher::StartQuickStretch()
{
    m_bFlipX = FALSE;
    m_bFlipY = FALSE;
    if (m_DestWidth < 0) {
        m_bFlipX = TRUE;
        m_DestWidth = -m_DestWidth;
    }
    if (m_DestHeight < 0) {
        m_bFlipY = TRUE;
        m_DestHeight = -m_DestHeight;
    }
    m_LineIndex = 0;
    FX_DWORD size = m_ClipRect.Width();
    if (size && m_DestBPP > (int)(INT_MAX / size)) {
        return FALSE;
    }
    size *= m_DestBPP;
    m_pScanline = FX_Alloc(FX_BYTE, (size / 8 + 3) / 4 * 4);
    if (!m_pScanline) {
        return FALSE;
    }
    if (m_pSource->m_pAlphaMask) {
        m_pMaskScanline = FX_Alloc(FX_BYTE, (m_ClipRect.Width() + 3) / 4 * 4);
        if (!m_pMaskScanline) {
            return FALSE;
        }
    }
    if (m_pSource->GetWidth() * m_pSource->GetHeight() < MAX_PROGRESSIVE_STRETCH_PIXELS) {
        ContinueQuickStretch(NULL);
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CFX_ImageStretcher::ContinueQuickStretch(IFX_Pause* pPause)
{
    if (m_pScanline == NULL) {
        return FALSE;
    }
    int result_width = m_ClipRect.Width(), result_height = m_ClipRect.Height();
    int src_height = m_pSource->GetHeight();
    for (; m_LineIndex < result_height; m_LineIndex ++) {
        int dest_y, src_y;
        if (m_bFlipY) {
            dest_y = result_height - m_LineIndex - 1;
            src_y = (m_DestHeight - (dest_y + m_ClipRect.top) - 1) * src_height / m_DestHeight;
        } else {
            dest_y = m_LineIndex;
            src_y = (dest_y + m_ClipRect.top) * src_height / m_DestHeight;
        }
        if (src_y >= src_height) {
            src_y = src_height - 1;
        }
        if (src_y < 0) {
            src_y = 0;
        }
        if (m_pSource->SkipToScanline(src_y, pPause)) {
            return TRUE;
        }
        m_pSource->DownSampleScanline(src_y, m_pScanline, m_DestBPP, m_DestWidth, m_bFlipX, m_ClipRect.left, result_width);
        if (m_pMaskScanline) {
            m_pSource->m_pAlphaMask->DownSampleScanline(src_y, m_pMaskScanline, 1, m_DestWidth, m_bFlipX, m_ClipRect.left, result_width);
        }
        m_pDest->ComposeScanline(dest_y, m_pScanline, m_pMaskScanline);
    }
    return FALSE;
}
