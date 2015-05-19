// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_dib.h"
#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxcodec/fx_codec.h"
const FX_DWORD g_dwWinPalette[256] = {
    0xff000000, 0xff800000, 0xff008000, 0xff808000, 0xff000080, 0xff800080,
    0xff008080, 0xff808080, 0xffC0DCC0, 0xffA6CAF0, 0xff2A3FAA, 0xff2A3FFF,
    0xff2A5F00, 0xff2A5F55, 0xff2A5FAA, 0xff2A5FFF, 0xff2A7F00, 0xff2A7F55,
    0xff2A7FAA, 0xff2A7FFF, 0xff2A9F00, 0xff2A9F55, 0xff2A9FAA, 0xff2A9FFF,
    0xff2ABF00, 0xff2ABF55, 0xff2ABFAA, 0xff2ABFFF, 0xff2ADF00, 0xff2ADF55,
    0xff2ADFAA, 0xff2ADFFF, 0xff2AFF00, 0xff2AFF55, 0xff2AFFAA, 0xff2AFFFF,
    0xff550000, 0xff550055, 0xff5500AA, 0xff5500FF, 0xff551F00, 0xff551F55,
    0xff551FAA, 0xff551FFF, 0xff553F00, 0xff553F55, 0xff553FAA, 0xff553FFF,
    0xff555F00, 0xff555F55, 0xff555FAA, 0xff555FFF, 0xff557F00, 0xff557F55,
    0xff557FAA, 0xff557FFF, 0xff559F00, 0xff559F55, 0xff559FAA, 0xff559FFF,
    0xff55BF00, 0xff55BF55, 0xff55BFAA, 0xff55BFFF, 0xff55DF00, 0xff55DF55,
    0xff55DFAA, 0xff55DFFF, 0xff55FF00, 0xff55FF55, 0xff55FFAA, 0xff55FFFF,
    0xff7F0000, 0xff7F0055, 0xff7F00AA, 0xff7F00FF, 0xff7F1F00, 0xff7F1F55,
    0xff7F1FAA, 0xff7F1FFF, 0xff7F3F00, 0xff7F3F55, 0xff7F3FAA, 0xff7F3FFF,
    0xff7F5F00, 0xff7F5F55, 0xff7F5FAA, 0xff7F5FFF, 0xff7F7F00, 0xff7F7F55,
    0xff7F7FAA, 0xff7F7FFF, 0xff7F9F00, 0xff7F9F55, 0xff7F9FAA, 0xff7F9FFF,
    0xff7FBF00, 0xff7FBF55, 0xff7FBFAA, 0xff7FBFFF, 0xff7FDF00, 0xff7FDF55,
    0xff7FDFAA, 0xff7FDFFF, 0xff00FF7F, 0xff7FFF55, 0xff7FFFAA, 0xff7FFFFF,
    0xffAA0000, 0xffAA0055, 0xffAA00AA, 0xffAA00FF, 0xffAA1F00, 0xffAA1F55,
    0xffAA1FAA, 0xffAA1FFF, 0xffAA3F00, 0xffAA3F55, 0xffAA3FAA, 0xffAA3FFF,
    0xffAA5F00, 0xffAA5F55, 0xffAA5FAA, 0xffAA5FFF, 0xffAA7F00, 0xffAA7F55,
    0xffAA7FAA, 0xffAA7FFF, 0xffAA9F00, 0xffAA9F55, 0xffAA9FAA, 0xffAA9FFF,
    0xffAABF00, 0xffAABF55, 0xffAABFAA, 0xffAABFFF, 0xffAADF00, 0xffAADF55,
    0xffAADFAA, 0xffAADFFF, 0xffAAFF00, 0xffAAFF55, 0xffAAFFAA, 0xffAAFFFF,
    0xffD40000, 0xffD40055, 0xffD400AA, 0xffD400FF, 0xffD41F00, 0xffD41F55,
    0xffD41FAA, 0xffD41FFF, 0xffD43F00, 0xffD43F55, 0xffD43FAA, 0xffD43FFF,
    0xffD45F00, 0xffD45F55, 0xffD45FAA, 0xffD45FFF, 0xffD47F00, 0xffD47F55,
    0xffD47FAA, 0xffD4F7FF, 0xffD49F00, 0xffD49F55, 0xffD49FAA, 0xffD49FFF,
    0xffD4BF00, 0xffD4BF55, 0xffD4BFAA, 0xffD4BFFF, 0xffD4DF00, 0xffD4DF55,
    0xffD4DFAA, 0xffD4DFFF, 0xffD4FF00, 0xffD4FF55, 0xffD4FFAA, 0xffD4FFFF,
    0xffFF0055, 0xffFF00AA, 0xffFF1F00, 0xffFF1F55, 0xffFF1FAA, 0xffFF1FFF,
    0xffFF3F00, 0xffFF3F55, 0xffFF3FAA, 0xffFF3FFF, 0xffFF5F00, 0xffFF5F55,
    0xffFF5FAA, 0xffFF5FFF, 0xffFF7F00, 0xffFF7F55, 0xffFF7FAA, 0xffFF7FFF,
    0xffFF9F00, 0xffFF9F55, 0xffFF9FAA, 0xffFF9FFF, 0xffFFBF00, 0xffFFBF55,
    0xffFFBFAA, 0xffFFBFFF, 0xffFFDF00, 0xffFFDF55, 0xffFFDFAA, 0xffFFDFFF,
    0xffFFFF55, 0xffFFFFAA, 0xffCCCCFF, 0xffFFCCFF, 0xff33FFFF, 0xff66FFFF,
    0xff99FFFF, 0xffCCFFFF, 0xff007F00, 0xff007F55, 0xff007FAA, 0xff007FFF,
    0xff009F00, 0xff009F55, 0xff009FAA, 0xff009FFF, 0xff00BF00, 0xff00BF55,
    0xff00BFAA, 0xff00BFFF, 0xff00DF00, 0xff00DF55, 0xff00DFAA, 0xff00DFFF,
    0xff00FF55, 0xff00FFAA, 0xff2A0000, 0xff2A0055, 0xff2A00AA, 0xff2A00FF,
    0xff2A1F00, 0xff2A1F55, 0xff2A1FAA, 0xff2A1FFF, 0xff2A3F00, 0xff2A3F55,
    0xffFFFBF0, 0xffA0A0A4, 0xff808080, 0xffFF0000, 0xff00FF00, 0xffFF0000,
    0xff0000FF, 0xffFF00FF, 0xff00FFFF, 0xffFFFFFF
};
const FX_DWORD g_dwMacPalette[256] = {
    0xffFFFFFF, 0xffFFFFCC, 0xffFFFF99, 0xffFFFF66, 0xffFFFF33, 0xffFFFF00,
    0xffFFCCFF, 0xffFFCCCC, 0xffFFCC99, 0xffFFCC66, 0xffFFCC33, 0xffFFCC00,
    0xffFF99FF, 0xffFF99CC, 0xffFF9999, 0xffFF9966, 0xffFF9933, 0xffFF9900,
    0xffFF66FF, 0xffFF66CC, 0xffFF6699, 0xffFF6666, 0xffFF6633, 0xffFF6600,
    0xffFF33FF, 0xffFF33CC, 0xffFF3399, 0xffFF3366, 0xffFF3333, 0xffFF3300,
    0xffFF00FF, 0xffFF00CC, 0xffFF0099, 0xffFF0066, 0xffFF0033, 0xffFF0000,
    0xffCCFFFF, 0xffCCFFCC, 0xffCCFF99, 0xffCCFF66, 0xffCCFF33, 0xffCCFF00,
    0xffCCCCFF, 0xffCCCCCC, 0xffCCCC99, 0xffCCCC66, 0xffCCCC33, 0xffCCCC00,
    0xffCC99FF, 0xffCC99CC, 0xffCC9999, 0xffCC9966, 0xffCC9933, 0xffCC9900,
    0xffCC66FF, 0xffCC66CC, 0xffCC6699, 0xffCC6666, 0xffCC6633, 0xffCC6600,
    0xffCC33FF, 0xffCC33CC, 0xffCC3399, 0xffCC3366, 0xffCC3333, 0xffCC3300,
    0xffCC00FF, 0xffCC00CC, 0xffCC0099, 0xffCC0066, 0xffCC0033, 0xffCC0000,
    0xff99FFFF, 0xff99FFCC, 0xff99FF99, 0xff99FF66, 0xff99FF33, 0xff99FF00,
    0xff99CCFF, 0xff99CCCC, 0xff99CC99, 0xff99CC66, 0xff99CC33, 0xff99CC00,
    0xff9999FF, 0xff9999CC, 0xff999999, 0xff999966, 0xff999933, 0xff999900,
    0xff9966FF, 0xff9966CC, 0xff996699, 0xff996666, 0xff996633, 0xff996600,
    0xff9933FF, 0xff9933CC, 0xff993399, 0xff993366, 0xff993333, 0xff993300,
    0xff9900FF, 0xff9900CC, 0xff990099, 0xff990066, 0xff990033, 0xff990000,
    0xff66FFFF, 0xff66FFCC, 0xff66FF99, 0xff66FF66, 0xff66FF33, 0xff66FF00,
    0xff66CCFF, 0xff66CCCC, 0xff66CC99, 0xff66CC66, 0xff66CC33, 0xff66CC00,
    0xff6699FF, 0xff6699CC, 0xff669999, 0xff669966, 0xff669933, 0xff669900,
    0xff6666FF, 0xff6666CC, 0xff666699, 0xff666666, 0xff666633, 0xff666600,
    0xff6633FF, 0xff6633CC, 0xff663399, 0xff663366, 0xff663333, 0xff663300,
    0xff6600FF, 0xff6600CC, 0xff660099, 0xff660066, 0xff660033, 0xff660000,
    0xff33FFFF, 0xff33FFCC, 0xff33FF99, 0xff33FF66, 0xff33FF33, 0xff33FF00,
    0xff33CCFF, 0xff33CCCC, 0xff33CC99, 0xff33CC66, 0xff33CC33, 0xff33CC00,
    0xff3399FF, 0xff3399CC, 0xff339999, 0xff339966, 0xff339933, 0xff339900,
    0xff3366FF, 0xff3366CC, 0xff336699, 0xff336666, 0xff336633, 0xff336600,
    0xff3333FF, 0xff3333CC, 0xff333399, 0xff333366, 0xff333333, 0xff333300,
    0xff3300FF, 0xff3300CC, 0xff330099, 0xff330066, 0xff330033, 0xff330000,
    0xff00FFFF, 0xff00FFCC, 0xff00FF99, 0xff00FF66, 0xff00FF33, 0xff00FF00,
    0xff00CCFF, 0xff00CCCC, 0xff00CC99, 0xff00CC66, 0xff00CC33, 0xff00CC00,
    0xff0099FF, 0xff0099CC, 0xff009999, 0xff009966, 0xff009933, 0xff009900,
    0xff0066FF, 0xff0066CC, 0xff006699, 0xff006666, 0xff006633, 0xff006600,
    0xff0033FF, 0xff0033CC, 0xff003399, 0xff003366, 0xff003333, 0xff003300,
    0xff0000FF, 0xff0000CC, 0xff000099, 0xff000066, 0xff000033,
    0xffEE0000, 0xffDD0000, 0xffBB0000, 0xffAA0000, 0xff880000, 0xff770000,
    0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xff00EE00, 0xff00DD00,
    0xff00BB00, 0xff00AA00, 0xff008800, 0xff007700, 0xff005500, 0xff004400,
    0xff002200, 0xff001100, 0xff0000EE, 0xff0000DD, 0xff0000BB, 0xff0000AA,
    0xff000088, 0xff000077, 0xff000055, 0xff000044, 0xff000022, 0xff000011,
    0xffEEEEEE, 0xffDDDDDD, 0xffBBBBBB, 0xffAAAAAA, 0xff888888, 0xff777777,
    0xff555555, 0xff444444, 0xff222222, 0xff111111, 0xff000000
};
class CFX_Palette
{
public:
    CFX_Palette();
    ~CFX_Palette();
public:
    FX_BOOL      BuildPalette(const CFX_DIBSource* pBitmap, int dwPaletteType);
    FX_DWORD*    GetPalette() const
    {
        return m_pPalette;
    }

    FX_DWORD*    GetColorLut()const
    {
        return m_cLut;
    }
    FX_DWORD*    GetAmountLut()const
    {
        return m_aLut;
    }
    FX_INT32     Getlut()const
    {
        return m_lut;
    }
protected:
    FX_DWORD*    m_pPalette;
    FX_DWORD*    m_cLut;
    FX_DWORD*    m_aLut;
    int          m_lut;
};
int _Partition(FX_DWORD* alut, FX_DWORD* clut, int l, int r)
{
    FX_DWORD p_a = alut[l];
    FX_DWORD p_c = clut[l];
    while(l < r) {
        while(l < r && alut[r] >= p_a) {
            r--;
        }
        if (l < r) {
            alut[l] = alut[r];
            clut[l++] = clut[r];
        }
        while(l < r && alut[l] <= p_a) {
            l++;
        }
        if (l < r) {
            alut[r] = alut[l];
            clut[r--] = clut[l];
        }
    }
    alut[l] = p_a;
    clut[l] = p_c;
    return l;
}
void _Qsort(FX_DWORD* alut, FX_DWORD* clut, int l, int r)
{
    if(l < r) {
        int pI = _Partition(alut, clut, l, r);
        _Qsort(alut, clut, l, pI - 1);
        _Qsort(alut, clut, pI + 1, r);
    }
}
void _ColorDecode(FX_DWORD pal_v, FX_BYTE& r, FX_BYTE& g, FX_BYTE& b)
{
    r = (FX_BYTE)((pal_v & 0xf00) >> 4);
    g = (FX_BYTE)(pal_v & 0x0f0);
    b = (FX_BYTE)((pal_v & 0x00f) << 4);
}
void _Obtain_Pal(FX_DWORD* aLut, FX_DWORD*cLut, FX_DWORD* dest_pal, int pal_type, FX_DWORD* win_mac_pal, FX_DWORD lut)
{
    int row, col;
    FX_DWORD lut_1 = lut - 1;
    if (pal_type == FXDIB_PALETTE_LOC) {
        for (row = 0; row < 256; row++) {
            int lut_offset = lut_1 - row;
            if (lut_offset < 0) {
                lut_offset += 256;
            }
            FX_DWORD color = cLut[lut_offset];
            FX_BYTE r, g, b;
            _ColorDecode(color, r, g, b);
            dest_pal[row] = ((FX_DWORD)r << 16) | ((FX_DWORD)g << 8) | b | 0xff000000;
            aLut[lut_offset] = row;
        }
    } else {
        for (row = 0; row < 256; row++) {
            int lut_offset = lut_1 - row;
            if (lut_offset < 0) {
                lut_offset += 256;
            }
            FX_BYTE r, g, b;
            _ColorDecode(cLut[lut_offset], r, g, b);
            int error, min_error = 1000000;
            int c_index = 0;
            for (col = 0; col < 256; col++) {
                FX_DWORD p_color = win_mac_pal[col];
                int d_r = r - (FX_BYTE)(p_color >> 16);
                int d_g = g - (FX_BYTE)(p_color >> 8);
                int d_b = b - (FX_BYTE)p_color;
                error = d_r * d_r + d_g * d_g + d_b * d_b;
                if (error < min_error) {
                    min_error = error;
                    c_index = col;
                }
            }
            dest_pal[row] =  win_mac_pal[c_index];
            aLut[lut_offset] = row;
        }
    }
}
CFX_Palette::CFX_Palette()
{
    m_pPalette = NULL;
    m_cLut = NULL;
    m_aLut = NULL;
    m_lut = 0;
}
CFX_Palette::~CFX_Palette()
{
    if (m_pPalette) {
        FX_Free(m_pPalette);
    }
    if (m_cLut) {
        FX_Free(m_cLut);
    }
    if (m_aLut) {
        FX_Free(m_aLut);
    }
    m_lut = 0;
}
FX_BOOL CFX_Palette::BuildPalette(const CFX_DIBSource* pBitmap, int pal_type)
{
    if (pBitmap == NULL) {
        return FALSE;
    }
    if (m_pPalette != NULL) {
        FX_Free(m_pPalette);
    }
    m_pPalette = FX_Alloc(FX_DWORD, 256);
    int bpp    = pBitmap->GetBPP() / 8;
    int width  = pBitmap->GetWidth();
    int height = pBitmap->GetHeight();
    if (m_cLut) {
        FX_Free(m_cLut);
        m_cLut = NULL;
    }
    if (m_aLut) {
        FX_Free(m_aLut);
        m_aLut = NULL;
    }
    m_cLut = FX_Alloc(FX_DWORD, 4096);
    m_aLut = FX_Alloc(FX_DWORD, 4096);
    int row, col;
    m_lut = 0;
    for (row = 0; row < height; row++) {
        FX_BYTE* scan_line = (FX_BYTE*)pBitmap->GetScanline(row);
        for (col = 0; col < width; col++) {
            FX_BYTE* src_port = scan_line + col * bpp;
            FX_DWORD b = src_port[0] & 0xf0;
            FX_DWORD g = src_port[1] & 0xf0;
            FX_DWORD r = src_port[2] & 0xf0;
            FX_DWORD index = (r << 4) + g + (b >> 4);
            m_aLut[index]++;
        }
    }
    for (row = 0; row < 4096; row++) {
        if (m_aLut[row] != 0) {
            m_aLut[m_lut] = m_aLut[row];
            m_cLut[m_lut] = row;
            m_lut++;
        }
    }
    _Qsort(m_aLut, m_cLut, 0, m_lut - 1);
    FX_DWORD* win_mac_pal = NULL;
    if (pal_type == FXDIB_PALETTE_WIN) {
        win_mac_pal = (FX_DWORD*)g_dwWinPalette;
    } else if (pal_type == FXDIB_PALETTE_MAC) {
        win_mac_pal = (FX_DWORD*)g_dwMacPalette;
    }
    _Obtain_Pal(m_aLut, m_cLut, m_pPalette, pal_type, win_mac_pal, m_lut);
    return TRUE;
}
FX_BOOL _ConvertBuffer_1bppMask2Gray(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                     const CFX_DIBSource* pSrcBitmap, int src_left, int src_top)
{
    FX_BYTE set_gray, reset_gray;
    set_gray = 0xff;
    reset_gray = 0x00;
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FXSYS_memset8(dest_scan, reset_gray, width);
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row);
        for (int col = src_left; col < src_left + width; col ++) {
            if (src_scan[col / 8] & (1 << (7 - col % 8))) {
                *dest_scan = set_gray;
            }
            dest_scan ++;
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_8bppMask2Gray(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                     const CFX_DIBSource* pSrcBitmap, int src_left, int src_top)
{
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
        FXSYS_memcpy32(dest_scan, src_scan, width);
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_1bppPlt2Gray(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                    const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    FX_DWORD* src_plt = pSrcBitmap->GetPalette();
    FX_BYTE gray[2];
    if (pIccTransform) {
        FX_DWORD plt[2];
        if (pSrcBitmap->IsCmykImage()) {
            plt[0] = FXCMYK_TODIB(src_plt[0]);
            plt[1] = FXCMYK_TODIB(src_plt[1]);
        } else {
            FX_LPBYTE bgr_ptr = (FX_LPBYTE)plt;
            bgr_ptr[0] = FXARGB_B(src_plt[0]);
            bgr_ptr[1] = FXARGB_G(src_plt[0]);
            bgr_ptr[2] = FXARGB_R(src_plt[0]);
            bgr_ptr[3] = FXARGB_B(src_plt[1]);
            bgr_ptr[4] = FXARGB_G(src_plt[1]);
            bgr_ptr[5] = FXARGB_R(src_plt[1]);
        }
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, gray, (FX_LPCBYTE)plt, 2);
    } else {
        FX_BYTE reset_r, reset_g, reset_b,
                set_r, set_g, set_b;
        if (pSrcBitmap->IsCmykImage()) {
            AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[0]), FXSYS_GetMValue(src_plt[0]), FXSYS_GetYValue(src_plt[0]), FXSYS_GetKValue(src_plt[0]),
                               reset_r, reset_g, reset_b);
            AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[1]), FXSYS_GetMValue(src_plt[1]), FXSYS_GetYValue(src_plt[1]), FXSYS_GetKValue(src_plt[1]),
                               set_r, set_g, set_b);
        } else {
            reset_r = FXARGB_R(src_plt[0]);
            reset_g = FXARGB_G(src_plt[0]);
            reset_b = FXARGB_B(src_plt[0]);
            set_r = FXARGB_R(src_plt[1]);
            set_g = FXARGB_G(src_plt[1]);
            set_b = FXARGB_B(src_plt[1]);
        }
        gray[0] = FXRGB2GRAY(reset_r, reset_g, reset_b);
        gray[1] = FXRGB2GRAY(set_r, set_g, set_b);
    }
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FXSYS_memset8(dest_scan, gray[0], width);
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row);
        for (int col = src_left; col < src_left + width; col ++) {
            if (src_scan[col / 8] & (1 << (7 - col % 8))) {
                *dest_scan = gray[1];
            }
            dest_scan ++;
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_8bppPlt2Gray(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                    const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    FX_DWORD* src_plt = pSrcBitmap->GetPalette();
    FX_BYTE gray[256];
    if (pIccTransform) {
        FX_DWORD plt[256];
        if (pSrcBitmap->IsCmykImage()) {
            for (int i = 0; i < 256; i ++) {
                plt[i] = FXCMYK_TODIB(src_plt[i]);
            }
        } else {
            FX_LPBYTE bgr_ptr = (FX_LPBYTE)plt;
            for (int i = 0; i < 256; i ++) {
                *bgr_ptr++ = FXARGB_B(src_plt[i]);
                *bgr_ptr++ = FXARGB_G(src_plt[i]);
                *bgr_ptr++ = FXARGB_R(src_plt[i]);
            }
        }
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, gray, (FX_LPCBYTE)plt, 256);
    } else {
        if (pSrcBitmap->IsCmykImage()) {
            FX_BYTE r, g, b;
            for (int i = 0; i < 256; i ++) {
                AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[i]), FXSYS_GetMValue(src_plt[i]), FXSYS_GetYValue(src_plt[i]), FXSYS_GetKValue(src_plt[i]),
                                   r, g, b);
                gray[i] = FXRGB2GRAY(r, g, b);
            }
        } else
            for (int i = 0; i < 256; i ++) {
                gray[i] = FXRGB2GRAY(FXARGB_R(src_plt[i]), FXARGB_G(src_plt[i]), FXARGB_B(src_plt[i]));
            }
    }
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
        for (int col = 0; col < width; col ++) {
            *dest_scan++ = gray[*src_scan++];
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_RgbOrCmyk2Gray(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                      const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    int Bpp = pSrcBitmap->GetBPP() / 8;
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        if (Bpp == 3 || pSrcBitmap->IsCmykImage()) {
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
                FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
                pIccModule->TranslateScanline(pIccTransform, dest_scan, src_scan, width);
            }
        } else {
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
                FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
                for (int col = 0; col < width; col ++) {
                    pIccModule->TranslateScanline(pIccTransform, dest_scan, src_scan, 1);
                    dest_scan++;
                    src_scan += 4;
                }
            }
        }
    } else {
        if (pSrcBitmap->IsCmykImage()) {
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
                FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
                for (int col = 0; col < width; col ++) {
                    FX_BYTE r, g, b;
                    AdobeCMYK_to_sRGB1(FXSYS_GetCValue((FX_DWORD)src_scan[0]), FXSYS_GetMValue((FX_DWORD)src_scan[1]), FXSYS_GetYValue((FX_DWORD)src_scan[2]), FXSYS_GetKValue((FX_DWORD)src_scan[3]),
                                       r, g, b);
                    *dest_scan++ = FXRGB2GRAY(r, g, b);
                    src_scan += 4;
                }
            }
        } else {
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
                FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
                for (int col = 0; col < width; col ++) {
                    *dest_scan++ = FXRGB2GRAY(src_scan[2], src_scan[1], src_scan[0]);
                    src_scan += Bpp;
                }
            }
        }
    }
    return TRUE;
}
inline void _ConvertBuffer_IndexCopy(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                     const CFX_DIBSource* pSrcBitmap, int src_left, int src_top)
{
    if (pSrcBitmap->GetBPP() == 1) {
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FXSYS_memset32(dest_scan, 0, width);
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row);
            for (int col = src_left; col < src_left + width; col ++) {
                if (src_scan[col / 8] & (1 << (7 - col % 8))) {
                    *dest_scan = 1;
                }
                dest_scan ++;
            }
        }
    } else {
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
            FXSYS_memcpy32(dest_scan, src_scan, width);
        }
    }
}
FX_BOOL _ConvertBuffer_Plt2PltRgb8(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                   const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, FX_DWORD* dst_plt, void* pIccTransform)
{
    _ConvertBuffer_IndexCopy(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
    FX_DWORD* src_plt = pSrcBitmap->GetPalette();
    int plt_size = pSrcBitmap->GetPaletteSize();
    if (pIccTransform) {
        FX_DWORD plt[256];
        FX_LPBYTE bgr_ptr = (FX_LPBYTE)plt;
        if (pSrcBitmap->IsCmykImage()) {
            for (int i = 0; i < plt_size; i ++) {
                plt[i] = FXCMYK_TODIB(src_plt[i]);
            }
        } else {
            for (int i = 0; i < plt_size; i ++) {
                *bgr_ptr++ = FXARGB_B(src_plt[i]);
                *bgr_ptr++ = FXARGB_G(src_plt[i]);
                *bgr_ptr++ = FXARGB_R(src_plt[i]);
            }
            bgr_ptr = (FX_LPBYTE)plt;
        }
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, (FX_LPBYTE)plt, (FX_LPCBYTE)plt, plt_size);
        for (int i = 0; i < plt_size; i ++) {
            dst_plt[i] = FXARGB_MAKE(0xff, bgr_ptr[2], bgr_ptr[1], bgr_ptr[0]);
            bgr_ptr += 3;
        }
    } else {
        if (pSrcBitmap->IsCmykImage()) {
            for (int i = 0; i < plt_size; i ++) {
                FX_BYTE r, g, b;
                AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[i]), FXSYS_GetMValue(src_plt[i]), FXSYS_GetYValue(src_plt[i]), FXSYS_GetKValue(src_plt[i]),
                                   r, g, b);
                dst_plt[i] = FXARGB_MAKE(0xff, r, g, b);
            }
        } else {
            FXSYS_memcpy32(dst_plt, src_plt, plt_size * 4);
        }
    }
    return TRUE;
}
inline FX_BOOL _ConvertBuffer_Rgb2PltRgb8_NoTransform(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
        const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, FX_DWORD* dst_plt)
{
    int bpp = pSrcBitmap->GetBPP() / 8;
    int row, col;
    CFX_Palette palette;
    palette.BuildPalette(pSrcBitmap, FXDIB_PALETTE_LOC);
    FX_DWORD* cLut = palette.GetColorLut();
    FX_DWORD* aLut = palette.GetAmountLut();
    if (cLut == NULL || aLut == NULL) {
        return FALSE;
    }
    int lut = palette.Getlut();
    FX_DWORD* pPalette = palette.GetPalette();
    if (lut > 256) {
        int err, min_err;
        int lut_256 = lut - 256;
        for (row = 0; row < lut_256; row++) {
            min_err = 1000000;
            FX_BYTE r, g, b;
            _ColorDecode(cLut[row], r, g, b);
            int clrindex = 0;
            for (int col = 0; col < 256; col++) {
                FX_DWORD p_color = *(pPalette + col);
                int d_r = r - (FX_BYTE)(p_color >> 16);
                int d_g = g - (FX_BYTE)(p_color >> 8);
                int d_b = b - (FX_BYTE)(p_color);
                err = d_r * d_r + d_g * d_g + d_b * d_b;
                if (err < min_err) {
                    min_err = err;
                    clrindex = col;
                }
            }
            aLut[row] = clrindex;
        }
    }
    FX_INT32 lut_1 = lut - 1;
    for (row = 0; row < height; row ++) {
        FX_BYTE* src_scan = (FX_BYTE*)pSrcBitmap->GetScanline(src_top + row) + src_left;
        FX_BYTE* dest_scan = dest_buf + row * dest_pitch;
        for (col = 0; col < width; col++) {
            FX_BYTE* src_port = src_scan + col * bpp;
            int r = src_port[2] & 0xf0;
            int g = src_port[1] & 0xf0;
            int b = src_port[0] & 0xf0;
            FX_DWORD clrindex = (r << 4) + g + (b >> 4);
            for (int i = lut_1; i >= 0; i--)
                if (clrindex == cLut[i]) {
                    *(dest_scan + col) = (FX_BYTE)(aLut[i]);
                    break;
                }
        }
    }
    FXSYS_memcpy32(dst_plt, pPalette, sizeof(FX_DWORD) * 256);
    return TRUE;
}
FX_BOOL _ConvertBuffer_Rgb2PltRgb8(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                   const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, FX_DWORD* dst_plt, void* pIccTransform)
{
    FX_BOOL ret = _ConvertBuffer_Rgb2PltRgb8_NoTransform(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, dst_plt);
    if (ret && pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        for (int i = 0; i < 256; i++) {
            FX_ARGB* plt = dst_plt + i;
            FX_ARGB plt_entry = FXARGB_TODIB(*plt);
            pIccModule->TranslateScanline(pIccTransform, (FX_LPBYTE)&plt_entry, (FX_LPCBYTE)&plt_entry, 1);
            *plt = FXARGB_TODIB(plt_entry);
        }
    }
    return ret;
}
FX_BOOL _ConvertBuffer_1bppMask2Rgb(FXDIB_Format dst_format, FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                    const CFX_DIBSource* pSrcBitmap, int src_left, int src_top)
{
    int comps = (dst_format & 0xff) / 8;
    FX_BYTE set_gray, reset_gray;
    set_gray = 0xff;
    reset_gray = 0x00;
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row);
        for (int col = src_left; col < src_left + width; col ++) {
            if (src_scan[col / 8] & (1 << (7 - col % 8))) {
                dest_scan[0] = set_gray;
                dest_scan[1] = set_gray;
                dest_scan[2] = set_gray;
            } else {
                dest_scan[0] = reset_gray;
                dest_scan[1] = reset_gray;
                dest_scan[2] = reset_gray;
            }
            dest_scan += comps;
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_8bppMask2Rgb(FXDIB_Format dst_format, FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                    const CFX_DIBSource* pSrcBitmap, int src_left, int src_top)
{
    int comps = (dst_format & 0xff) / 8;
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
        FX_BYTE src_pixel;
        for (int col = 0; col < width; col ++) {
            src_pixel = *src_scan++;
            *dest_scan++ = src_pixel;
            *dest_scan++ = src_pixel;
            *dest_scan   = src_pixel;
            dest_scan += comps - 2;
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_1bppPlt2Rgb(FXDIB_Format dst_format, FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                   const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    int comps = (dst_format & 0xff) / 8;
    FX_DWORD* src_plt = pSrcBitmap->GetPalette();
    FX_DWORD plt[2];
    FX_LPBYTE bgr_ptr = (FX_LPBYTE)plt;
    if (pSrcBitmap->IsCmykImage()) {
        plt[0] = FXCMYK_TODIB(src_plt[0]);
        plt[1] = FXCMYK_TODIB(src_plt[1]);
    } else {
        bgr_ptr[0] = FXARGB_B(src_plt[0]);
        bgr_ptr[1] = FXARGB_G(src_plt[0]);
        bgr_ptr[2] = FXARGB_R(src_plt[0]);
        bgr_ptr[3] = FXARGB_B(src_plt[1]);
        bgr_ptr[4] = FXARGB_G(src_plt[1]);
        bgr_ptr[5] = FXARGB_R(src_plt[1]);
    }
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, (FX_LPBYTE)plt, (FX_LPCBYTE)plt, 2);
    } else {
        if (pSrcBitmap->IsCmykImage()) {
            AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[0]), FXSYS_GetMValue(src_plt[0]), FXSYS_GetYValue(src_plt[0]), FXSYS_GetKValue(src_plt[0]),
                               bgr_ptr[2], bgr_ptr[1], bgr_ptr[0]);
            AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[1]), FXSYS_GetMValue(src_plt[1]), FXSYS_GetYValue(src_plt[1]), FXSYS_GetKValue(src_plt[1]),
                               bgr_ptr[5], bgr_ptr[4], bgr_ptr[3]);
        }
    }
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row);
        for (int col = src_left; col < src_left + width; col ++) {
            if (src_scan[col / 8] & (1 << (7 - col % 8))) {
                *dest_scan++ = bgr_ptr[3];
                *dest_scan++ = bgr_ptr[4];
                *dest_scan   = bgr_ptr[5];
            } else {
                *dest_scan++ = bgr_ptr[0];
                *dest_scan++ = bgr_ptr[1];
                *dest_scan   = bgr_ptr[2];
            }
            dest_scan += comps - 2;
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_8bppPlt2Rgb(FXDIB_Format dst_format, FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                   const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    int comps = (dst_format & 0xff) / 8;
    FX_DWORD* src_plt = pSrcBitmap->GetPalette();
    FX_DWORD plt[256];
    FX_LPBYTE bgr_ptr = (FX_LPBYTE)plt;
    if (!pSrcBitmap->IsCmykImage()) {
        for (int i = 0; i < 256; i++) {
            *bgr_ptr++ = FXARGB_B(src_plt[i]);
            *bgr_ptr++ = FXARGB_G(src_plt[i]);
            *bgr_ptr++ = FXARGB_R(src_plt[i]);
        }
        bgr_ptr = (FX_LPBYTE)plt;
    }
    if (pIccTransform) {
        if (pSrcBitmap->IsCmykImage()) {
            for (int i = 0; i < 256; i++) {
                plt[i] = FXCMYK_TODIB(src_plt[i]);
            }
        }
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, (FX_LPBYTE)plt, (FX_LPCBYTE)plt, 256);
    } else {
        if (pSrcBitmap->IsCmykImage()) {
            for (int i = 0; i < 256; i++) {
                AdobeCMYK_to_sRGB1(FXSYS_GetCValue(src_plt[i]), FXSYS_GetMValue(src_plt[i]), FXSYS_GetYValue(src_plt[i]), FXSYS_GetKValue(src_plt[i]),
                                   bgr_ptr[2], bgr_ptr[1], bgr_ptr[0]);
                bgr_ptr += 3;
            }
            bgr_ptr = (FX_LPBYTE)plt;
        }
    }
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
        for (int col = 0; col < width; col ++) {
            FX_LPBYTE src_pixel = bgr_ptr + 3 * (*src_scan++);
            *dest_scan++ = *src_pixel++;
            *dest_scan++ = *src_pixel++;
            *dest_scan   = *src_pixel++;
            dest_scan += comps - 2;
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_24bppRgb2Rgb24(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                      const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 3;
            pIccModule->TranslateScanline(pIccTransform, dest_scan, src_scan, width);
        }
    } else {
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 3;
            FXSYS_memcpy32(dest_scan, src_scan, width * 3);
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_32bppRgb2Rgb24(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                      const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
        for (int col = 0; col < width; col ++) {
            *dest_scan++ = *src_scan++;
            *dest_scan++ = *src_scan++;
            *dest_scan++ = *src_scan++;
            src_scan++;
        }
    }
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            pIccModule->TranslateScanline(pIccTransform, dest_scan, dest_scan, width);
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_Rgb2Rgb32(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                 const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    int comps = pSrcBitmap->GetBPP() / 8;
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * comps;
            for (int col = 0; col < width; col ++) {
                pIccModule->TranslateScanline(pIccTransform, dest_scan, src_scan, 1);
                dest_scan += 4;
                src_scan += comps;
            }
        }
    } else {
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * comps;
            for (int col = 0; col < width; col ++) {
                *dest_scan++ = *src_scan++;
                *dest_scan++ = *src_scan++;
                *dest_scan++ = *src_scan++;
                dest_scan++;
                src_scan += comps - 3;
            }
        }
    }
    return TRUE;
}
FX_BOOL _ConvertBuffer_32bppCmyk2Rgb32(FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                                       const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
            for (int col = 0; col < width; col ++) {
                pIccModule->TranslateScanline(pIccTransform, dest_scan, src_scan, 1);
                dest_scan += 4;
                src_scan += 4;
            }
        }
    } else {
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
            FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
            for (int col = 0; col < width; col ++) {
                AdobeCMYK_to_sRGB1(src_scan[0], src_scan[1], src_scan[2], src_scan[3],
                                   dest_scan[2], dest_scan[1], dest_scan[0]);
                dest_scan += 4;
                src_scan += 4;
            }
        }
    }
    return TRUE;
}
FX_BOOL ConvertBuffer(FXDIB_Format dest_format, FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                      const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, FX_DWORD*& d_pal, void* pIccTransform)
{
    FXDIB_Format src_format = pSrcBitmap->GetFormat();
    if (!CFX_GEModule::Get()->GetCodecModule() || !CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
        pIccTransform = NULL;
    }
    switch (dest_format) {
        case FXDIB_Invalid:
        case FXDIB_1bppCmyk:
        case FXDIB_1bppMask:
        case FXDIB_1bppRgb:
            ASSERT(FALSE);
            return FALSE;
        case FXDIB_8bppMask: {
                if ((src_format & 0xff) == 1) {
                    if (pSrcBitmap->GetPalette()) {
                        return _ConvertBuffer_1bppPlt2Gray(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_1bppMask2Gray(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
                } else if ((src_format & 0xff) == 8) {
                    if (pSrcBitmap->GetPalette()) {
                        return _ConvertBuffer_8bppPlt2Gray(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_8bppMask2Gray(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
                } else if ((src_format & 0xff) >= 24) {
                    return _ConvertBuffer_RgbOrCmyk2Gray(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                }
                return FALSE;
            }
        case FXDIB_8bppRgb:
        case FXDIB_8bppRgba: {
                if ((src_format & 0xff) == 8 && pSrcBitmap->GetPalette() == NULL) {
                    return ConvertBuffer(FXDIB_8bppMask, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, d_pal, pIccTransform);
                }
                d_pal = FX_Alloc(FX_DWORD, 256);
                if (((src_format & 0xff) == 1 || (src_format & 0xff) == 8) && pSrcBitmap->GetPalette()) {
                    return _ConvertBuffer_Plt2PltRgb8(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, d_pal, pIccTransform);
                } else if ((src_format & 0xff) >= 24) {
                    return _ConvertBuffer_Rgb2PltRgb8(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, d_pal, pIccTransform);
                }
                return FALSE;
            }
        case FXDIB_Rgb:
        case FXDIB_Rgba: {
                if ((src_format & 0xff) == 1) {
                    if (pSrcBitmap->GetPalette()) {
                        return _ConvertBuffer_1bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_1bppMask2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
                } else if ((src_format & 0xff) == 8) {
                    if (pSrcBitmap->GetPalette()) {
                        return _ConvertBuffer_8bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_8bppMask2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
                } else if ((src_format & 0xff) == 24) {
                    return _ConvertBuffer_24bppRgb2Rgb24(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                } else if ((src_format & 0xff) == 32) {
                    return _ConvertBuffer_32bppRgb2Rgb24(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                }
                return FALSE;
            }
        case FXDIB_Argb:
        case FXDIB_Rgb32: {
                if ((src_format & 0xff) == 1) {
                    if (pSrcBitmap->GetPalette()) {
                        return _ConvertBuffer_1bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_1bppMask2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
                } else if ((src_format & 0xff) == 8) {
                    if (pSrcBitmap->GetPalette()) {
                        return _ConvertBuffer_8bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_8bppMask2Rgb(dest_format, dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top);
                } else if ((src_format & 0xff) >= 24) {
                    if (src_format & 0x0400) {
                        return _ConvertBuffer_32bppCmyk2Rgb32(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                    }
                    return _ConvertBuffer_Rgb2Rgb32(dest_buf, dest_pitch, width, height, pSrcBitmap, src_left, src_top, pIccTransform);
                }
                return FALSE;
            }
        default:
            return FALSE;
    }
    return FALSE;
}
CFX_DIBitmap* CFX_DIBSource::CloneConvert(FXDIB_Format dest_format, const FX_RECT* pClip, void* pIccTransform) const
{
    if(dest_format == GetFormat() && pIccTransform == NULL) {
        return Clone(pClip);
    }
    if (pClip) {
        CFX_DIBitmap* pClone = Clone(pClip);
        if (pClone == NULL) {
            return NULL;
        }
        if(!pClone->ConvertFormat(dest_format, pIccTransform)) {
            delete pClone;
            return NULL;
        }
        return pClone;
    }
    CFX_DIBitmap* pClone = new CFX_DIBitmap;
    if(!pClone->Create(m_Width, m_Height, dest_format)) {
        delete pClone;
        return NULL;
    }
    FX_BOOL ret = TRUE;
    CFX_DIBitmap* pSrcAlpha = NULL;
    if (m_AlphaFlag & 2) {
        pSrcAlpha = (GetFormat() == FXDIB_Argb) ? GetAlphaMask() : m_pAlphaMask;
        if (pSrcAlpha == NULL) {
            delete pClone;
            return NULL;
        }
    }
    if (dest_format & 0x0200) {
        if (dest_format == FXDIB_Argb)
            ret = pSrcAlpha ?
                  pClone->LoadChannel(FXDIB_Alpha, pSrcAlpha, FXDIB_Alpha) :
                  pClone->LoadChannel(FXDIB_Alpha, 0xff);
        else {
            ret = pClone->CopyAlphaMask(pSrcAlpha);
        }
    }
    if (pSrcAlpha && pSrcAlpha != m_pAlphaMask) {
        delete pSrcAlpha;
        pSrcAlpha = NULL;
    }
    if (!ret) {
        delete pClone;
        return NULL;
    }
    FX_DWORD* pal_8bpp = NULL;
    ret = ConvertBuffer(dest_format, pClone->GetBuffer(), pClone->GetPitch(), m_Width, m_Height, this, 0, 0, pal_8bpp, pIccTransform);
    if (!ret) {
        if (pal_8bpp) {
            FX_Free(pal_8bpp);
        }
        delete pClone;
        return NULL;
    }
    if (pal_8bpp) {
        pClone->CopyPalette(pal_8bpp);
        FX_Free(pal_8bpp);
        pal_8bpp = NULL;
    }
    return pClone;
}
FX_BOOL CFX_DIBitmap::ConvertFormat(FXDIB_Format dest_format, void* pIccTransform)
{
    FXDIB_Format src_format = GetFormat();
    if (dest_format == src_format && pIccTransform == NULL) {
        return TRUE;
    }
    if (dest_format == FXDIB_8bppMask && src_format == FXDIB_8bppRgb && m_pPalette == NULL) {
        m_AlphaFlag = 1;
        return TRUE;
    }
    if (dest_format == FXDIB_Argb && src_format == FXDIB_Rgb32 && pIccTransform == NULL) {
        m_AlphaFlag = 2;
        for (int row = 0; row < m_Height; row ++) {
            FX_LPBYTE scanline = m_pBuffer + row * m_Pitch + 3;
            for (int col = 0; col < m_Width; col ++) {
                *scanline = 0xff;
                scanline += 4;
            }
        }
        return TRUE;
    }
    int dest_bpp = dest_format & 0xff;
    int dest_pitch = (dest_bpp * m_Width + 31) / 32 * 4;
    FX_LPBYTE dest_buf = FX_TryAlloc(FX_BYTE, dest_pitch * m_Height + 4);
    if (dest_buf == NULL) {
        return FALSE;
    }
    CFX_DIBitmap* pAlphaMask = NULL;
    if (dest_format == FXDIB_Argb) {
        FXSYS_memset8(dest_buf, 0xff, dest_pitch * m_Height + 4);
        if (m_pAlphaMask) {
            for (int row = 0; row < m_Height; row ++) {
                FX_LPBYTE pDstScanline = dest_buf + row * dest_pitch + 3;
                FX_LPCBYTE pSrcScanline = m_pAlphaMask->GetScanline(row);
                for (int col = 0; col < m_Width; col ++) {
                    *pDstScanline = *pSrcScanline++;
                    pDstScanline += 4;
                }
            }
        }
    } else if (dest_format & 0x0200) {
        if (src_format == FXDIB_Argb) {
            pAlphaMask = GetAlphaMask();
            if (pAlphaMask == NULL) {
                FX_Free(dest_buf);
                return FALSE;
            }
        } else {
            if (m_pAlphaMask == NULL) {
                if (!BuildAlphaMask()) {
                    FX_Free(dest_buf);
                    return FALSE;
                }
                pAlphaMask = m_pAlphaMask;
                m_pAlphaMask = NULL;
            } else {
                pAlphaMask = m_pAlphaMask;
            }
        }
    }
    FX_BOOL ret = FALSE;
    FX_DWORD* pal_8bpp = NULL;
    ret = ConvertBuffer(dest_format, dest_buf, dest_pitch, m_Width, m_Height, this, 0, 0, pal_8bpp, pIccTransform);
    if (!ret) {
        if (pal_8bpp) {
            FX_Free(pal_8bpp);
        }
        if (pAlphaMask != m_pAlphaMask) {
            delete pAlphaMask;
        }
        if (dest_buf) {
            FX_Free(dest_buf);
        }
        return FALSE;
    }
    if (m_pAlphaMask && pAlphaMask != m_pAlphaMask) {
        delete m_pAlphaMask;
    }
    m_pAlphaMask = pAlphaMask;
    if (m_pPalette) {
        FX_Free(m_pPalette);
    }
    m_pPalette = pal_8bpp;
    if (!m_bExtBuf) {
        FX_Free(m_pBuffer);
    }
    m_bExtBuf = FALSE;
    m_pBuffer = dest_buf;
    m_bpp = (FX_BYTE)dest_format;
    m_AlphaFlag = (FX_BYTE)(dest_format >> 8);
    m_Pitch = dest_pitch;
    return TRUE;
}
