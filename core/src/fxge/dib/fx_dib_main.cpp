// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_dib.h"
#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "dib_int.h"
#include <limits.h>
FX_BOOL ConvertBuffer(FXDIB_Format dest_format, FX_LPBYTE dest_buf, int dest_pitch, int width, int height,
                      const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, FX_DWORD*& pal, void* pIccTransform);
void CmykDecode(FX_DWORD cmyk, int& c, int& m, int& y, int& k)
{
    c = FXSYS_GetCValue(cmyk);
    m = FXSYS_GetMValue(cmyk);
    y = FXSYS_GetYValue(cmyk);
    k = FXSYS_GetKValue(cmyk);
}
void ArgbDecode(FX_DWORD argb, int& a, int& r, int& g, int& b)
{
    a = FXARGB_A(argb);
    r = FXARGB_R(argb);
    g = FXARGB_G(argb);
    b = FXARGB_B(argb);
}
void ArgbDecode(FX_DWORD argb, int& a, FX_COLORREF& rgb)
{
    a = FXARGB_A(argb);
    rgb = FXSYS_RGB(FXARGB_R(argb), FXARGB_G(argb), FXARGB_B(argb));
}
FX_DWORD ArgbEncode(int a, FX_COLORREF rgb)
{
    return FXARGB_MAKE(a, FXSYS_GetRValue(rgb), FXSYS_GetGValue(rgb), FXSYS_GetBValue(rgb));
}
CFX_DIBSource::CFX_DIBSource()
{
    m_bpp = 0;
    m_AlphaFlag = 0;
    m_Width = m_Height = 0;
    m_Pitch = 0;
    m_pPalette = NULL;
    m_pAlphaMask = NULL;
}
CFX_DIBSource::~CFX_DIBSource()
{
    if (m_pPalette) {
        FX_Free(m_pPalette);
    }
    if (m_pAlphaMask) {
        delete m_pAlphaMask;
    }
}
CFX_DIBitmap::CFX_DIBitmap()
{
    m_bExtBuf = FALSE;
    m_pBuffer = NULL;
    m_pPalette = NULL;
}
#define _MAX_OOM_LIMIT_	12000000
FX_BOOL CFX_DIBitmap::Create(int width, int height, FXDIB_Format format, FX_LPBYTE pBuffer, int pitch)
{
    m_pBuffer = NULL;
    m_bpp = (FX_BYTE)format;
    m_AlphaFlag = (FX_BYTE)(format >> 8);
    m_Width = m_Height = m_Pitch = 0;
    if (width <= 0 || height <= 0 || pitch < 0) {
        return FALSE;
    }
    if ((INT_MAX - 31) / width < (format & 0xff)) {
        return FALSE;
    }
    if (!pitch) {
        pitch = (width * (format & 0xff) + 31) / 32 * 4;
    }
    if ((1 << 30) / pitch < height) {
        return FALSE;
    }
    if (pBuffer) {
        m_pBuffer = pBuffer;
        m_bExtBuf = TRUE;
    } else {
        int size = pitch * height + 4;
        int oomlimit = _MAX_OOM_LIMIT_;
        if (oomlimit >= 0 && size >= oomlimit) {
            m_pBuffer = FX_AllocNL(FX_BYTE, size);
        } else {
            m_pBuffer = FX_Alloc(FX_BYTE, size);
        }
        if (m_pBuffer == NULL) {
            return FALSE;
        }
    }
    m_Width = width;
    m_Height = height;
    m_Pitch = pitch;
    if (HasAlpha() && format != FXDIB_Argb) {
        FX_BOOL ret = TRUE;
        ret = BuildAlphaMask();
        if (!ret) {
            if (!m_bExtBuf && m_pBuffer) {
                FX_Free(m_pBuffer);
                m_pBuffer = NULL;
                m_Width = m_Height = m_Pitch = 0;
                return FALSE;
            }
        }
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::Copy(const CFX_DIBSource* pSrc)
{
    if (m_pBuffer) {
        return FALSE;
    }
    if (!Create(pSrc->GetWidth(), pSrc->GetHeight(), pSrc->GetFormat())) {
        return FALSE;
    }
    CopyPalette(pSrc->GetPalette());
    CopyAlphaMask(pSrc->m_pAlphaMask);
    for (int row = 0; row < pSrc->GetHeight(); row ++) {
        FXSYS_memcpy32(m_pBuffer + row * m_Pitch, pSrc->GetScanline(row), m_Pitch);
    }
    return TRUE;
}
CFX_DIBitmap::~CFX_DIBitmap()
{
    if (m_pBuffer && !m_bExtBuf) {
        FX_Free(m_pBuffer);
    }
    m_pBuffer = NULL;
}
void CFX_DIBitmap::TakeOver(CFX_DIBitmap* pSrcBitmap)
{
    if (m_pBuffer && !m_bExtBuf) {
        FX_Free(m_pBuffer);
    }
    if (m_pPalette) {
        FX_Free(m_pPalette);
    }
    if (m_pAlphaMask) {
        delete m_pAlphaMask;
    }
    m_pBuffer = pSrcBitmap->m_pBuffer;
    m_pPalette = pSrcBitmap->m_pPalette;
    m_pAlphaMask = pSrcBitmap->m_pAlphaMask;
    pSrcBitmap->m_pBuffer = NULL;
    pSrcBitmap->m_pPalette = NULL;
    pSrcBitmap->m_pAlphaMask = NULL;
    m_bpp = pSrcBitmap->m_bpp;
    m_bExtBuf = pSrcBitmap->m_bExtBuf;
    m_AlphaFlag = pSrcBitmap->m_AlphaFlag;
    m_Width = pSrcBitmap->m_Width;
    m_Height = pSrcBitmap->m_Height;
    m_Pitch = pSrcBitmap->m_Pitch;
}
CFX_DIBitmap* CFX_DIBSource::Clone(const FX_RECT* pClip) const
{
    FX_RECT rect(0, 0, m_Width, m_Height);
    if (pClip) {
        rect.Intersect(*pClip);
        if (rect.IsEmpty()) {
            return NULL;
        }
    }
    CFX_DIBitmap* pNewBitmap = FX_NEW CFX_DIBitmap;
    if (!pNewBitmap) {
        return NULL;
    }
    if (!pNewBitmap->Create(rect.Width(), rect.Height(), GetFormat())) {
        delete pNewBitmap;
        return NULL;
    }
    pNewBitmap->CopyPalette(m_pPalette);
    pNewBitmap->CopyAlphaMask(m_pAlphaMask, pClip);
    if (GetBPP() == 1 && rect.left % 8 != 0) {
        int left_shift = rect.left % 32;
        int right_shift = 32 - left_shift;
        int dword_count = pNewBitmap->m_Pitch / 4;
        for (int row = rect.top; row < rect.bottom; row ++) {
            FX_DWORD* src_scan = (FX_DWORD*)GetScanline(row) + rect.left / 32;
            FX_DWORD* dest_scan = (FX_DWORD*)pNewBitmap->GetScanline(row - rect.top);
            for (int i = 0; i < dword_count; i ++) {
                dest_scan[i] = (src_scan[i] << left_shift) | (src_scan[i + 1] >> right_shift);
            }
        }
    } else {
        int copy_len = (pNewBitmap->GetWidth() * pNewBitmap->GetBPP() + 7) / 8;
        if (m_Pitch < (FX_DWORD)copy_len) {
            copy_len = m_Pitch;
        }
        for (int row = rect.top; row < rect.bottom; row ++) {
            FX_LPCBYTE src_scan = GetScanline(row) + rect.left * m_bpp / 8;
            FX_LPBYTE dest_scan = (FX_LPBYTE)pNewBitmap->GetScanline(row - rect.top);
            FXSYS_memcpy32(dest_scan, src_scan, copy_len);
        }
    }
    return pNewBitmap;
}
void CFX_DIBSource::BuildPalette()
{
    if (m_pPalette) {
        return;
    }
    if (GetBPP() == 1) {
        m_pPalette = FX_Alloc(FX_DWORD, 2);
        if (!m_pPalette) {
            return;
        }
        if(IsCmykImage()) {
            m_pPalette[0] = 0xff;
            m_pPalette[1] = 0;
        } else {
            m_pPalette[0] = 0xff000000;
            m_pPalette[1] = 0xffffffff;
        }
    } else if (GetBPP() == 8) {
        m_pPalette = FX_Alloc(FX_DWORD, 256);
        if (!m_pPalette) {
            return;
        }
        if(IsCmykImage()) {
            for (int i = 0; i < 256; i ++) {
                m_pPalette[i] = 0xff - i;
            }
        } else {
            for (int i = 0; i < 256; i ++) {
                m_pPalette[i] = 0xff000000 | (i * 0x10101);
            }
        }
    }
}
FX_BOOL CFX_DIBSource::BuildAlphaMask()
{
    if (m_pAlphaMask) {
        return TRUE;
    }
    m_pAlphaMask = FX_NEW CFX_DIBitmap;
    if (!m_pAlphaMask) {
        return FALSE;
    }
    if (!m_pAlphaMask->Create(m_Width, m_Height, FXDIB_8bppMask)) {
        delete m_pAlphaMask;
        m_pAlphaMask = NULL;
        return FALSE;
    }
    FXSYS_memset8(m_pAlphaMask->GetBuffer(), 0xff, m_pAlphaMask->GetHeight()*m_pAlphaMask->GetPitch());
    return TRUE;
}
FX_DWORD CFX_DIBSource::GetPaletteEntry(int index) const
{
    ASSERT((GetBPP() == 1 || GetBPP() == 8) && !IsAlphaMask());
    if (m_pPalette) {
        return m_pPalette[index];
    }
    if (IsCmykImage()) {
        if (GetBPP() == 1) {
            return index ? 0 : 0xff;
        }
        return 0xff - index;
    }
    if (GetBPP() == 1) {
        return index ? 0xffffffff : 0xff000000;
    }
    return index * 0x10101 | 0xff000000;
}
void CFX_DIBSource::SetPaletteEntry(int index, FX_DWORD color)
{
    ASSERT((GetBPP() == 1 || GetBPP() == 8) && !IsAlphaMask());
    if (m_pPalette == NULL) {
        BuildPalette();
    }
    m_pPalette[index] = color;
}
int CFX_DIBSource::FindPalette(FX_DWORD color) const
{
    ASSERT((GetBPP() == 1 || GetBPP() == 8) && !IsAlphaMask());
    if (m_pPalette == NULL) {
        if (IsCmykImage()) {
            if (GetBPP() == 1) {
                return ((FX_BYTE)color == 0xff) ? 0 : 1;
            }
            return 0xff - (FX_BYTE)color;
        }
        if (GetBPP() == 1) {
            return ((FX_BYTE)color == 0xff) ? 1 : 0;
        }
        return (FX_BYTE)color;
    }
    int palsize = (1 << GetBPP());
    for (int i = 0; i < palsize; i ++)
        if (m_pPalette[i] == color) {
            return i;
        }
    return -1;
}
void CFX_DIBitmap::Clear(FX_DWORD color)
{
    if (m_pBuffer == NULL) {
        return;
    }
    switch (GetFormat()) {
        case FXDIB_1bppMask:
            FXSYS_memset8(m_pBuffer, (color & 0xff000000) ? 0xff : 0, m_Pitch * m_Height);
            break;
        case FXDIB_1bppRgb: {
                int index = FindPalette(color);
                FXSYS_memset8(m_pBuffer, index ? 0xff : 0, m_Pitch * m_Height);
                break;
            }
        case FXDIB_8bppMask:
            FXSYS_memset8(m_pBuffer, color >> 24, m_Pitch * m_Height);
            break;
        case FXDIB_8bppRgb: {
                int index = FindPalette(color);
                FXSYS_memset8(m_pBuffer, index, m_Pitch * m_Height);
                break;
            }
        case FXDIB_Rgb:
        case FXDIB_Rgba: {
                int a, r, g, b;
                ArgbDecode(color, a, r, g, b);
                if (r == g && g == b) {
                    FXSYS_memset8(m_pBuffer, r, m_Pitch * m_Height);
                } else {
                    int byte_pos = 0;
                    for (int col = 0; col < m_Width; col ++) {
                        m_pBuffer[byte_pos++] = b;
                        m_pBuffer[byte_pos++] = g;
                        m_pBuffer[byte_pos++] = r;
                    }
                    for (int row = 1; row < m_Height; row ++) {
                        FXSYS_memcpy32(m_pBuffer + row * m_Pitch, m_pBuffer, m_Pitch);
                    }
                }
                break;
            }
        case FXDIB_Rgb32:
        case FXDIB_Argb: {
                color = IsCmykImage() ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
                for (int i = 0; i < m_Width; i ++) {
                    ((FX_DWORD*)m_pBuffer)[i] = color;
                }
                for (int row = 1; row < m_Height; row ++) {
                    FXSYS_memcpy32(m_pBuffer + row * m_Pitch, m_pBuffer, m_Pitch);
                }
                break;
            }
        default:
            break;
    }
}
void CFX_DIBSource::GetOverlapRect(int& dest_left, int& dest_top, int& width, int& height,
                                   int src_width, int src_height, int& src_left, int& src_top,
                                   const CFX_ClipRgn* pClipRgn)
{
    if (width == 0 || height == 0) {
        return;
    }
    ASSERT(width > 0 && height > 0);
    if (dest_left > m_Width || dest_top > m_Height) {
        width = 0;
        height = 0;
        return;
    }
    int x_offset = dest_left - src_left;
    int y_offset = dest_top - src_top;
    FX_RECT src_rect(src_left, src_top, src_left + width, src_top + height);
    FX_RECT src_bound(0, 0, src_width, src_height);
    src_rect.Intersect(src_bound);
    FX_RECT dest_rect(src_rect.left + x_offset, src_rect.top + y_offset,
                      src_rect.right + x_offset, src_rect.bottom + y_offset);
    FX_RECT dest_bound(0, 0, m_Width, m_Height);
    dest_rect.Intersect(dest_bound);
    if (pClipRgn) {
        dest_rect.Intersect(pClipRgn->GetBox());
    }
    dest_left = dest_rect.left;
    dest_top = dest_rect.top;
    src_left = dest_left - x_offset;
    src_top = dest_top - y_offset;
    width = dest_rect.right - dest_rect.left;
    height = dest_rect.bottom - dest_rect.top;
}
FX_BOOL CFX_DIBitmap::TransferBitmap(int dest_left, int dest_top, int width, int height,
                                     const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    GetOverlapRect(dest_left, dest_top, width, height, pSrcBitmap->GetWidth(), pSrcBitmap->GetHeight(), src_left, src_top, NULL);
    if (width == 0 || height == 0) {
        return TRUE;
    }
    FXDIB_Format dest_format = GetFormat();
    FXDIB_Format src_format = pSrcBitmap->GetFormat();
    if (dest_format == src_format && pIccTransform == NULL) {
        if (GetBPP() == 1) {
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = m_pBuffer + (dest_top + row) * m_Pitch;
                FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row);
                for (int col = 0; col < width; col ++) {
                    if (src_scan[(src_left + col) / 8] & (1 << (7 - (src_left + col) % 8))) {
                        dest_scan[(dest_left + col) / 8] |= 1 << (7 - (dest_left + col) % 8);
                    } else {
                        dest_scan[(dest_left + col) / 8] &= ~(1 << (7 - (dest_left + col) % 8));
                    }
                }
            }
        } else {
            int Bpp = GetBPP() / 8;
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = m_pBuffer + (dest_top + row) * m_Pitch + dest_left * Bpp;
                FX_LPCBYTE src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
                FXSYS_memcpy32(dest_scan, src_scan, width * Bpp);
            }
        }
    } else {
        if (m_pPalette) {
            return FALSE;
        }
        if (m_bpp == 8) {
            dest_format = FXDIB_8bppMask;
        }
        FX_LPBYTE dest_buf = m_pBuffer + dest_top * m_Pitch + dest_left * GetBPP() / 8;
        FX_DWORD* d_plt = NULL;
        if(!ConvertBuffer(dest_format, dest_buf, m_Pitch, width, height, pSrcBitmap, src_left, src_top, d_plt, pIccTransform)) {
            return FALSE;
        }
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::TransferMask(int dest_left, int dest_top, int width, int height,
                                   const CFX_DIBSource* pMask, FX_DWORD color, int src_left, int src_top, int alpha_flag, void* pIccTransform)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    ASSERT(HasAlpha() && (m_bpp >= 24));
    ASSERT(pMask->IsAlphaMask());
    if (!HasAlpha() || !pMask->IsAlphaMask() || m_bpp < 24) {
        return FALSE;
    }
    GetOverlapRect(dest_left, dest_top, width, height, pMask->GetWidth(), pMask->GetHeight(), src_left, src_top, NULL);
    if (width == 0 || height == 0) {
        return TRUE;
    }
    int src_bpp = pMask->GetBPP();
    int alpha;
    FX_DWORD dst_color;
    if (alpha_flag >> 8) {
        alpha = alpha_flag & 0xff;
        dst_color = FXCMYK_TODIB(color);
    } else {
        alpha = FXARGB_A(color);
        dst_color = FXARGB_TODIB(color);
    }
    FX_LPBYTE color_p = (FX_LPBYTE)&dst_color;
    if (pIccTransform && CFX_GEModule::Get()->GetCodecModule() && CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, color_p, color_p, 1);
    } else {
        if (alpha_flag >> 8 && !IsCmykImage())
            AdobeCMYK_to_sRGB1(FXSYS_GetCValue(color), FXSYS_GetMValue(color), FXSYS_GetYValue(color), FXSYS_GetKValue(color),
                               color_p[2], color_p[1], color_p[0]);
        else if (!(alpha_flag >> 8) && IsCmykImage()) {
            return FALSE;
        }
    }
    if(!IsCmykImage()) {
        color_p[3] = (FX_BYTE)alpha;
    }
    if (GetFormat() == FXDIB_Argb) {
        for (int row = 0; row < height; row ++) {
            FX_DWORD* dest_pos = (FX_DWORD*)(m_pBuffer + (dest_top + row) * m_Pitch + dest_left * 4);
            FX_LPCBYTE src_scan = pMask->GetScanline(src_top + row);
            if (src_bpp == 1) {
                for (int col = 0; col < width; col ++) {
                    int src_bitpos = src_left + col;
                    if (src_scan[src_bitpos / 8] & (1 << (7 - src_bitpos % 8))) {
                        *dest_pos = dst_color;
                    } else {
                        *dest_pos = 0;
                    }
                    dest_pos ++;
                }
            } else {
                src_scan += src_left;
                dst_color = FXARGB_TODIB(dst_color);
                dst_color &= 0xffffff;
                for (int col = 0; col < width; col ++) {
                    FXARGB_SETDIB(dest_pos++, dst_color | ((alpha * (*src_scan++) / 255) << 24));
                }
            }
        }
    } else {
        int comps = m_bpp / 8;
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_color_pos = m_pBuffer + (dest_top + row) * m_Pitch + dest_left * comps;
            FX_LPBYTE dest_alpha_pos = (FX_LPBYTE)m_pAlphaMask->GetScanline(dest_top + row) + dest_left;
            FX_LPCBYTE src_scan = pMask->GetScanline(src_top + row);
            if (src_bpp == 1) {
                for (int col = 0; col < width; col ++) {
                    int src_bitpos = src_left + col;
                    if (src_scan[src_bitpos / 8] & (1 << (7 - src_bitpos % 8))) {
                        FXSYS_memcpy32(dest_color_pos, color_p, comps);
                        *dest_alpha_pos = 0xff;
                    } else {
                        FXSYS_memset32(dest_color_pos, 0, comps);
                        *dest_alpha_pos = 0;
                    }
                    dest_color_pos += comps;
                    dest_alpha_pos ++;
                }
            } else {
                src_scan += src_left;
                for (int col = 0; col < width; col ++) {
                    FXSYS_memcpy32(dest_color_pos, color_p, comps);
                    dest_color_pos += comps;
                    *dest_alpha_pos++ = (alpha * (*src_scan++) / 255);
                }
            }
        }
    }
    return TRUE;
}
void CFX_DIBSource::CopyPalette(const FX_DWORD* pSrc, FX_DWORD size)
{
    if (pSrc == NULL || GetBPP() > 8) {
        if (m_pPalette) {
            FX_Free(m_pPalette);
        }
        m_pPalette = NULL;
    } else {
        FX_DWORD pal_size = 1 << GetBPP();
        if (m_pPalette == NULL) {
            m_pPalette = FX_Alloc(FX_DWORD, pal_size);
        }
        if (!m_pPalette) {
            return;
        }
        if (pal_size > size) {
            pal_size = size;
        }
        FXSYS_memcpy32(m_pPalette, pSrc, pal_size * sizeof(FX_DWORD));
    }
}
void CFX_DIBSource::GetPalette(FX_DWORD* pal, int alpha) const
{
    ASSERT(GetBPP() <= 8 && !IsCmykImage());
    if (GetBPP() == 1) {
        pal[0] = ((m_pPalette ? m_pPalette[0] : 0xff000000) & 0xffffff) | (alpha << 24);
        pal[1] = ((m_pPalette ? m_pPalette[1] : 0xffffffff) & 0xffffff) | (alpha << 24);
        return;
    }
    if (m_pPalette) {
        for (int i = 0; i < 256; i ++) {
            pal[i] = (m_pPalette[i] & 0x00ffffff) | (alpha << 24);
        }
    } else {
        for (int i = 0; i < 256; i ++) {
            pal[i] = (i * 0x10101) | (alpha << 24);
        }
    }
}
CFX_DIBitmap* CFX_DIBSource::GetAlphaMask(const FX_RECT* pClip) const
{
    ASSERT(GetFormat() == FXDIB_Argb);
    FX_RECT rect(0, 0, m_Width, m_Height);
    if (pClip) {
        rect.Intersect(*pClip);
        if (rect.IsEmpty()) {
            return NULL;
        }
    }
    CFX_DIBitmap* pMask = FX_NEW CFX_DIBitmap;
    if (!pMask) {
        return NULL;
    }
    if (!pMask->Create(rect.Width(), rect.Height(), FXDIB_8bppMask)) {
        delete pMask;
        return NULL;
    }
    for (int row = rect.top; row < rect.bottom; row ++) {
        FX_LPCBYTE src_scan = GetScanline(row) + rect.left * 4 + 3;
        FX_LPBYTE dest_scan = (FX_LPBYTE)pMask->GetScanline(row - rect.top);
        for (int col = rect.left; col < rect.right; col ++) {
            *dest_scan ++ = *src_scan;
            src_scan += 4;
        }
    }
    return pMask;
}
FX_BOOL CFX_DIBSource::CopyAlphaMask(const CFX_DIBSource* pAlphaMask, const FX_RECT* pClip)
{
    if (!HasAlpha() || GetFormat() == FXDIB_Argb) {
        return FALSE;
    }
    if (pAlphaMask) {
        FX_RECT rect(0, 0, pAlphaMask->m_Width, pAlphaMask->m_Height);
        if (pClip) {
            rect.Intersect(*pClip);
            if (rect.IsEmpty() || rect.Width() != m_Width || rect.Height() != m_Height) {
                return FALSE;
            }
        } else {
            if (pAlphaMask->m_Width != m_Width || pAlphaMask->m_Height != m_Height) {
                return FALSE;
            }
        }
        for (int row = 0; row < m_Height; row ++)
            FXSYS_memcpy32((void*)m_pAlphaMask->GetScanline(row),
                           pAlphaMask->GetScanline(row + rect.top) + rect.left, m_pAlphaMask->m_Pitch);
    } else {
        m_pAlphaMask->Clear(0xff000000);
    }
    return TRUE;
}
const int g_ChannelOffset[] = {0, 2, 1, 0, 0, 1, 2, 3, 3};
FX_BOOL CFX_DIBitmap::LoadChannel(FXDIB_Channel destChannel, const CFX_DIBSource* pSrcBitmap, FXDIB_Channel srcChannel)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    CFX_DIBSource* pSrcClone = (CFX_DIBSource*)pSrcBitmap;
    CFX_DIBitmap* pDst = this;
    int destOffset, srcOffset;
    if (srcChannel == FXDIB_Alpha) {
        if (!pSrcBitmap->HasAlpha() && !pSrcBitmap->IsAlphaMask()) {
            return FALSE;
        }
        if (pSrcBitmap->GetBPP() == 1) {
            pSrcClone = pSrcBitmap->CloneConvert(FXDIB_8bppMask);
            if (pSrcClone == NULL) {
                return FALSE;
            }
        }
        if(pSrcBitmap->GetFormat() == FXDIB_Argb) {
            srcOffset = 3;
        } else {
            srcOffset = 0;
        }
    } else {
        if (pSrcBitmap->IsAlphaMask()) {
            return FALSE;
        }
        if (pSrcBitmap->GetBPP() < 24) {
            if (pSrcBitmap->IsCmykImage()) {
                pSrcClone = pSrcBitmap->CloneConvert((FXDIB_Format)((pSrcBitmap->GetFormat() & 0xff00) | 0x20));
            } else {
                pSrcClone = pSrcBitmap->CloneConvert((FXDIB_Format)((pSrcBitmap->GetFormat() & 0xff00) | 0x18));
            }
            if (pSrcClone == NULL) {
                return FALSE;
            }
        }
        srcOffset = g_ChannelOffset[srcChannel];
    }
    if (destChannel == FXDIB_Alpha) {
        if (IsAlphaMask()) {
            if(!ConvertFormat(FXDIB_8bppMask)) {
                if (pSrcClone != pSrcBitmap) {
                    delete pSrcClone;
                }
                return FALSE;
            }
            destOffset = 0;
        } else {
            destOffset = 0;
            if(!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb)) {
                if (pSrcClone != pSrcBitmap) {
                    delete pSrcClone;
                }
                return FALSE;
            }
            if (GetFormat() == FXDIB_Argb) {
                destOffset = 3;
            }
        }
    } else {
        if (IsAlphaMask()) {
            if (pSrcClone != pSrcBitmap) {
                delete pSrcClone;
            }
            return FALSE;
        }
        if (GetBPP() < 24) {
            if (HasAlpha()) {
                if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb)) {
                    if (pSrcClone != pSrcBitmap) {
                        delete pSrcClone;
                    }
                    return FALSE;
                }
            } else
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb32)) {
#else
                if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb)) {
#endif
                    if (pSrcClone != pSrcBitmap) {
                        delete pSrcClone;
                    }
                    return FALSE;
                }
        }
        destOffset = g_ChannelOffset[destChannel];
    }
    if (srcChannel == FXDIB_Alpha && pSrcClone->m_pAlphaMask) {
        CFX_DIBitmap* pAlphaMask = pSrcClone->m_pAlphaMask;
        if (pSrcClone->GetWidth() != m_Width || pSrcClone->GetHeight() != m_Height) {
            if (pAlphaMask) {
                pAlphaMask = pAlphaMask->StretchTo(m_Width, m_Height);
                if (pAlphaMask == NULL) {
                    if (pSrcClone != pSrcBitmap) {
                        delete pSrcClone;
                    }
                    return FALSE;
                }
            }
        }
        if (pSrcClone != pSrcBitmap) {
            pSrcClone->m_pAlphaMask = NULL;
            delete pSrcClone;
        }
        pSrcClone = pAlphaMask;
        srcOffset = 0;
    } else if (pSrcClone->GetWidth() != m_Width || pSrcClone->GetHeight() != m_Height) {
        CFX_DIBitmap* pSrcMatched = pSrcClone->StretchTo(m_Width, m_Height);
        if (pSrcClone != pSrcBitmap) {
            delete pSrcClone;
        }
        if (pSrcMatched == NULL) {
            return FALSE;
        }
        pSrcClone = pSrcMatched;
    }
    if (destChannel == FXDIB_Alpha && m_pAlphaMask) {
        pDst = m_pAlphaMask;
        destOffset = 0;
    }
    int srcBytes = pSrcClone->GetBPP() / 8;
    int destBytes = pDst->GetBPP() / 8;
    for (int row = 0; row < m_Height; row ++) {
        FX_LPBYTE dest_pos = (FX_LPBYTE)pDst->GetScanline(row) + destOffset;
        FX_LPCBYTE src_pos = pSrcClone->GetScanline(row) + srcOffset;
        for (int col = 0; col < m_Width; col ++) {
            *dest_pos = *src_pos;
            dest_pos += destBytes;
            src_pos += srcBytes;
        }
    }
    if (pSrcClone != pSrcBitmap && pSrcClone != pSrcBitmap->m_pAlphaMask) {
        delete pSrcClone;
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::LoadChannel(FXDIB_Channel destChannel, int value)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    int destOffset;
    if (destChannel == FXDIB_Alpha) {
        if (IsAlphaMask()) {
            if(!ConvertFormat(FXDIB_8bppMask)) {
                return FALSE;
            }
            destOffset = 0;
        } else {
            destOffset = 0;
            if(!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb)) {
                return FALSE;
            }
            if (GetFormat() == FXDIB_Argb) {
                destOffset = 3;
            }
        }
    } else {
        if (IsAlphaMask()) {
            return FALSE;
        }
        if (GetBPP() < 24) {
            if (HasAlpha()) {
                if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb)) {
                    return FALSE;
                }
            } else
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb)) {
                    return FALSE;
                }
#else
                if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb32)) {
                    return FALSE;
                }
#endif
        }
        destOffset = g_ChannelOffset[destChannel];
    }
    int Bpp = GetBPP() / 8;
    if (Bpp == 1) {
        FXSYS_memset8(m_pBuffer, value, m_Height * m_Pitch);
        return TRUE;
    }
    if (destChannel == FXDIB_Alpha && m_pAlphaMask) {
        FXSYS_memset8(m_pAlphaMask->GetBuffer(), value, m_pAlphaMask->GetHeight()*m_pAlphaMask->GetPitch());
        return TRUE;
    }
    for (int row = 0; row < m_Height; row ++) {
        FX_LPBYTE scan_line = m_pBuffer + row * m_Pitch + destOffset;
        for (int col = 0; col < m_Width; col ++) {
            *scan_line = value;
            scan_line += Bpp;
        }
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::MultiplyAlpha(const CFX_DIBSource* pSrcBitmap)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    ASSERT(pSrcBitmap->IsAlphaMask());
    if (!pSrcBitmap->IsAlphaMask()) {
        return FALSE;
    }
    if (!IsAlphaMask() && !HasAlpha()) {
        return LoadChannel(FXDIB_Alpha, pSrcBitmap, FXDIB_Alpha);
    }
    CFX_DIBitmap* pSrcClone = (CFX_DIBitmap*)pSrcBitmap;
    if (pSrcBitmap->GetWidth() != m_Width || pSrcBitmap->GetHeight() != m_Height) {
        pSrcClone = pSrcBitmap->StretchTo(m_Width, m_Height);
        ASSERT(pSrcClone != NULL);
        if (pSrcClone == NULL) {
            return FALSE;
        }
    }
    if (IsAlphaMask()) {
        if(!ConvertFormat(FXDIB_8bppMask)) {
            if (pSrcClone != pSrcBitmap) {
                delete pSrcClone;
            }
            return FALSE;
        }
        for (int row = 0; row < m_Height; row ++) {
            FX_LPBYTE dest_scan = m_pBuffer + m_Pitch * row;
            FX_LPBYTE src_scan = pSrcClone->m_pBuffer + pSrcClone->m_Pitch * row;
            if (pSrcClone->GetBPP() == 1) {
                for (int col = 0; col < m_Width; col ++) {
                    if (!((1 << (7 - col % 8)) & src_scan[col / 8])) {
                        dest_scan[col] = 0;
                    }
                }
            } else {
                for (int col = 0; col < m_Width; col ++) {
                    *dest_scan = (*dest_scan) * src_scan[col] / 255;
                    dest_scan ++;
                }
            }
        }
    } else {
        if(GetFormat() == FXDIB_Argb) {
            if (pSrcClone->GetBPP() == 1) {
                if (pSrcClone != pSrcBitmap) {
                    delete pSrcClone;
                }
                return FALSE;
            }
            for (int row = 0; row < m_Height; row ++) {
                FX_LPBYTE dest_scan = m_pBuffer + m_Pitch * row + 3;
                FX_LPBYTE src_scan = pSrcClone->m_pBuffer + pSrcClone->m_Pitch * row;
                for (int col = 0; col < m_Width; col ++) {
                    *dest_scan = (*dest_scan) * src_scan[col] / 255;
                    dest_scan += 4;
                }
            }
        } else {
            m_pAlphaMask->MultiplyAlpha(pSrcClone);
        }
    }
    if (pSrcClone != pSrcBitmap) {
        delete pSrcClone;
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::GetGrayData(void* pIccTransform)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    switch (GetFormat()) {
        case FXDIB_1bppRgb: {
                if (m_pPalette == NULL) {
                    return FALSE;
                }
                FX_BYTE gray[2];
                for (int i = 0; i < 2; i ++) {
                    int r = (FX_BYTE)(m_pPalette[i] >> 16);
                    int g = (FX_BYTE)(m_pPalette[i] >> 8);
                    int b = (FX_BYTE)m_pPalette[i];
                    gray[i] = (FX_BYTE)FXRGB2GRAY(r, g, b);
                }
                CFX_DIBitmap* pMask = FX_NEW CFX_DIBitmap;
                if (!pMask) {
                    return FALSE;
                }
                if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask)) {
                    delete pMask;
                    return FALSE;
                }
                FXSYS_memset8(pMask->GetBuffer(), gray[0], pMask->GetPitch() * m_Height);
                for (int row = 0; row < m_Height; row ++) {
                    FX_LPBYTE src_pos = m_pBuffer + row * m_Pitch;
                    FX_LPBYTE dest_pos = (FX_LPBYTE)pMask->GetScanline(row);
                    for (int col = 0; col < m_Width; col ++) {
                        if (src_pos[col / 8] & (1 << (7 - col % 8))) {
                            *dest_pos = gray[1];
                        }
                        dest_pos ++;
                    }
                }
                TakeOver(pMask);
                delete pMask;
                break;
            }
        case FXDIB_8bppRgb: {
                if (m_pPalette == NULL) {
                    return FALSE;
                }
                FX_BYTE gray[256];
                for (int i = 0; i < 256; i ++) {
                    int r = (FX_BYTE)(m_pPalette[i] >> 16);
                    int g = (FX_BYTE)(m_pPalette[i] >> 8);
                    int b = (FX_BYTE)m_pPalette[i];
                    gray[i] = (FX_BYTE)FXRGB2GRAY(r, g, b);
                }
                CFX_DIBitmap* pMask = FX_NEW CFX_DIBitmap;
                if (!pMask) {
                    return FALSE;
                }
                if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask)) {
                    delete pMask;
                    return FALSE;
                }
                for (int row = 0; row < m_Height; row ++) {
                    FX_LPBYTE dest_pos = pMask->GetBuffer() + row * pMask->GetPitch();
                    FX_LPBYTE src_pos = m_pBuffer + row * m_Pitch;
                    for (int col = 0; col < m_Width; col ++) {
                        *dest_pos ++ = gray[*src_pos ++];
                    }
                }
                TakeOver(pMask);
                delete pMask;
                break;
            }
        case FXDIB_Rgb: {
                CFX_DIBitmap* pMask = FX_NEW CFX_DIBitmap;
                if (!pMask) {
                    return FALSE;
                }
                if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask)) {
                    delete pMask;
                    return FALSE;
                }
                for (int row = 0; row < m_Height; row ++) {
                    FX_LPBYTE src_pos = m_pBuffer + row * m_Pitch;
                    FX_LPBYTE dest_pos = pMask->GetBuffer() + row * pMask->GetPitch();
                    for (int col = 0; col < m_Width; col ++) {
                        *dest_pos ++ = FXRGB2GRAY(src_pos[2], src_pos[1], *src_pos);
                        src_pos += 3;
                    }
                }
                TakeOver(pMask);
                delete pMask;
                break;
            }
        case FXDIB_Rgb32: {
                CFX_DIBitmap* pMask = FX_NEW CFX_DIBitmap;
                if (!pMask) {
                    return FALSE;
                }
                if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask)) {
                    delete pMask;
                    return FALSE;
                }
                for (int row = 0; row < m_Height; row ++) {
                    FX_LPBYTE src_pos = m_pBuffer + row * m_Pitch;
                    FX_LPBYTE dest_pos = pMask->GetBuffer() + row * pMask->GetPitch();
                    for (int col = 0; col < m_Width; col ++) {
                        *dest_pos ++ = FXRGB2GRAY(src_pos[2], src_pos[1], *src_pos);
                        src_pos += 4;
                    }
                }
                TakeOver(pMask);
                delete pMask;
                break;
            }
        default:
            return FALSE;
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::MultiplyAlpha(int alpha)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    switch (GetFormat()) {
        case FXDIB_1bppMask:
            if (!ConvertFormat(FXDIB_8bppMask)) {
                return FALSE;
            }
            MultiplyAlpha(alpha);
            break;
        case FXDIB_8bppMask: {
                for (int row = 0; row < m_Height; row ++) {
                    FX_LPBYTE scan_line = m_pBuffer + row * m_Pitch;
                    for (int col = 0; col < m_Width; col ++) {
                        scan_line[col] = scan_line[col] * alpha / 255;
                    }
                }
                break;
            }
        case FXDIB_Argb: {
                for (int row = 0; row < m_Height; row ++) {
                    FX_LPBYTE scan_line = m_pBuffer + row * m_Pitch + 3;
                    for (int col = 0; col < m_Width; col ++) {
                        *scan_line = (*scan_line) * alpha / 255;
                        scan_line += 4;
                    }
                }
                break;
            }
        default:
            if (HasAlpha()) {
                m_pAlphaMask->MultiplyAlpha(alpha);
            } else if (IsCmykImage()) {
                if (!ConvertFormat((FXDIB_Format)(GetFormat() | 0x0200))) {
                    return FALSE;
                }
                m_pAlphaMask->MultiplyAlpha(alpha);
            } else {
                if (!ConvertFormat(FXDIB_Argb)) {
                    return FALSE;
                }
                MultiplyAlpha(alpha);
            }
            break;
    }
    return TRUE;
}
FX_DWORD CFX_DIBitmap::GetPixel(int x, int y) const
{
    if (m_pBuffer == NULL) {
        return 0;
    }
    FX_LPBYTE pos = m_pBuffer + y * m_Pitch + x * GetBPP() / 8;
    switch (GetFormat()) {
        case FXDIB_1bppMask: {
                if ((*pos) & (1 << (7 - x % 8))) {
                    return 0xff000000;
                }
                return 0;
            }
        case FXDIB_1bppRgb: {
                if ((*pos) & (1 << (7 - x % 8))) {
                    return m_pPalette ? m_pPalette[1] : 0xffffffff;
                } else {
                    return m_pPalette ? m_pPalette[0] : 0xff000000;
                }
                break;
            }
        case FXDIB_8bppMask:
            return (*pos) << 24;
        case FXDIB_8bppRgb:
            return m_pPalette ? m_pPalette[*pos] : (0xff000000 | ((*pos) * 0x10101));
        case FXDIB_Rgb:
        case FXDIB_Rgba:
        case FXDIB_Rgb32:
            return FXARGB_GETDIB(pos) | 0xff000000;
        case FXDIB_Argb:
            return FXARGB_GETDIB(pos);
        default:
            break;
    }
    return 0;
}
void CFX_DIBitmap::SetPixel(int x, int y, FX_DWORD color)
{
    if (m_pBuffer == NULL) {
        return;
    }
    if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
        return;
    }
    FX_LPBYTE pos = m_pBuffer + y * m_Pitch + x * GetBPP() / 8;
    switch (GetFormat()) {
        case FXDIB_1bppMask:
            if (color >> 24) {
                *pos |= 1 << (7 - x % 8);
            } else {
                *pos &= ~(1 << (7 - x % 8));
            }
            break;
        case FXDIB_1bppRgb:
            if (m_pPalette) {
                if (color == m_pPalette[1]) {
                    *pos |= 1 << (7 - x % 8);
                } else {
                    *pos &= ~(1 << (7 - x % 8));
                }
            } else {
                if (color == 0xffffffff) {
                    *pos |= 1 << (7 - x % 8);
                } else {
                    *pos &= ~(1 << (7 - x % 8));
                }
            }
            break;
        case FXDIB_8bppMask:
            *pos = (FX_BYTE)(color >> 24);
            break;
        case FXDIB_8bppRgb: {
                if (m_pPalette) {
                    for (int i = 0; i < 256; i ++) {
                        if (m_pPalette[i] == color) {
                            *pos = (FX_BYTE)i;
                            return;
                        }
                    }
                    *pos = 0;
                } else {
                    *pos = FXRGB2GRAY(FXARGB_R(color), FXARGB_G(color), FXARGB_B(color));
                }
                break;
            }
        case FXDIB_Rgb:
        case FXDIB_Rgb32: {
                int alpha = FXARGB_A(color);
                pos[0] = (FXARGB_B(color) * alpha + pos[0] * (255 - alpha)) / 255;
                pos[1] = (FXARGB_G(color) * alpha + pos[1] * (255 - alpha)) / 255;
                pos[2] = (FXARGB_R(color) * alpha + pos[2] * (255 - alpha)) / 255;
                break;
            }
        case FXDIB_Rgba: {
                pos[0] = FXARGB_B(color);
                pos[1] = FXARGB_G(color);
                pos[2] = FXARGB_R(color);
                break;
            }
        case FXDIB_Argb:
            FXARGB_SETDIB(pos, color);
            break;
        default:
            break;
    }
}
void CFX_DIBitmap::DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
                                      int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const
{
    if (m_pBuffer == NULL) {
        return;
    }
    int src_Bpp = m_bpp / 8;
    FX_LPBYTE scanline = m_pBuffer + line * m_Pitch;
    if (src_Bpp == 0) {
        for (int i = 0; i < clip_width; i ++) {
            FX_DWORD dest_x = clip_left + i;
            FX_DWORD src_x = dest_x * m_Width / dest_width;
            if (bFlipX) {
                src_x = m_Width - src_x - 1;
            }
            src_x %= m_Width;
            dest_scan[i] = (scanline[src_x / 8] & (1 << (7 - src_x % 8))) ? 255 : 0;
        }
    } else if (src_Bpp == 1) {
        for (int i = 0; i < clip_width; i ++) {
            FX_DWORD dest_x = clip_left + i;
            FX_DWORD src_x = dest_x * m_Width / dest_width;
            if (bFlipX) {
                src_x = m_Width - src_x - 1;
            }
            src_x %= m_Width;
            int dest_pos = i;
            if (m_pPalette) {
                if (!IsCmykImage()) {
                    dest_pos *= 3;
                    FX_ARGB argb = m_pPalette[scanline[src_x]];
                    dest_scan[dest_pos] = FXARGB_B(argb);
                    dest_scan[dest_pos + 1] = FXARGB_G(argb);
                    dest_scan[dest_pos + 2] = FXARGB_R(argb);
                } else {
                    dest_pos *= 4;
                    FX_CMYK cmyk = m_pPalette[scanline[src_x]];
                    dest_scan[dest_pos] = FXSYS_GetCValue(cmyk);
                    dest_scan[dest_pos + 1] = FXSYS_GetMValue(cmyk);
                    dest_scan[dest_pos + 2] = FXSYS_GetYValue(cmyk);
                    dest_scan[dest_pos + 3] = FXSYS_GetKValue(cmyk);
                }
            } else {
                dest_scan[dest_pos] = scanline[src_x];
            }
        }
    } else {
        for (int i = 0; i < clip_width; i ++) {
            FX_DWORD dest_x = clip_left + i;
            FX_DWORD src_x = bFlipX ? (m_Width - dest_x * m_Width / dest_width - 1) * src_Bpp : (dest_x * m_Width / dest_width) * src_Bpp;
            src_x %= m_Width * src_Bpp;
            int dest_pos = i * src_Bpp;
            for (int b = 0; b < src_Bpp; b ++) {
                dest_scan[dest_pos + b] = scanline[src_x + b];
            }
        }
    }
}
FX_BOOL CFX_DIBitmap::ConvertColorScale(FX_DWORD forecolor, FX_DWORD backcolor)
{
    ASSERT(!IsAlphaMask());
    if (m_pBuffer == NULL || IsAlphaMask()) {
        return FALSE;
    }
    int fc, fm, fy, fk, bc, bm, by, bk;
    int fr, fg, fb, br, bg, bb;
    FX_BOOL isCmykImage = IsCmykImage();
    if (isCmykImage) {
        fc = FXSYS_GetCValue(forecolor);
        fm = FXSYS_GetMValue(forecolor);
        fy = FXSYS_GetYValue(forecolor);
        fk = FXSYS_GetKValue(forecolor);
        bc = FXSYS_GetCValue(backcolor);
        bm = FXSYS_GetMValue(backcolor);
        by = FXSYS_GetYValue(backcolor);
        bk = FXSYS_GetKValue(backcolor);
    } else {
        fr = FXSYS_GetRValue(forecolor);
        fg = FXSYS_GetGValue(forecolor);
        fb = FXSYS_GetBValue(forecolor);
        br = FXSYS_GetRValue(backcolor);
        bg = FXSYS_GetGValue(backcolor);
        bb = FXSYS_GetBValue(backcolor);
    }
    if (m_bpp <= 8) {
        if (isCmykImage) {
            if (forecolor == 0xff && backcolor == 0 && m_pPalette == NULL) {
                return TRUE;
            }
        } else if (forecolor == 0 && backcolor == 0xffffff && m_pPalette == NULL) {
            return TRUE;
        }
        if (m_pPalette == NULL) {
            BuildPalette();
        }
        int size = 1 << m_bpp;
        if (isCmykImage) {
            for (int i = 0; i < size; i ++) {
                FX_BYTE b, g, r;
                AdobeCMYK_to_sRGB1(FXSYS_GetCValue(m_pPalette[i]), FXSYS_GetMValue(m_pPalette[i]), FXSYS_GetYValue(m_pPalette[i]), FXSYS_GetKValue(m_pPalette[i]),
                                   r, g, b);
                int gray = 255 - FXRGB2GRAY(r, g, b);
                m_pPalette[i] = CmykEncode(bc + (fc - bc) * gray / 255, bm + (fm - bm) * gray / 255,
                                           by + (fy - by) * gray / 255, bk + (fk - bk) * gray / 255);
            }
        } else
            for (int i = 0; i < size; i ++) {
                int gray = FXRGB2GRAY(FXARGB_R(m_pPalette[i]), FXARGB_G(m_pPalette[i]), FXARGB_B(m_pPalette[i]));
                m_pPalette[i] = FXARGB_MAKE(0xff, br + (fr - br) * gray / 255, bg + (fg - bg) * gray / 255,
                                            bb + (fb - bb) * gray / 255);
            }
        return TRUE;
    }
    if (isCmykImage) {
        if (forecolor == 0xff && backcolor == 0x00) {
            for (int row = 0; row < m_Height; row ++) {
                FX_LPBYTE scanline = m_pBuffer + row * m_Pitch;
                for (int col = 0; col < m_Width; col ++) {
                    FX_BYTE b, g, r;
                    AdobeCMYK_to_sRGB1(scanline[0], scanline[1], scanline[2], scanline[3],
                                       r, g, b);
                    *scanline ++ = 0;
                    *scanline ++ = 0;
                    *scanline ++ = 0;
                    *scanline ++ = 255 - FXRGB2GRAY(r, g, b);
                }
            }
            return TRUE;
        }
    } else if (forecolor == 0 && backcolor == 0xffffff) {
        for (int row = 0; row < m_Height; row ++) {
            FX_LPBYTE scanline = m_pBuffer + row * m_Pitch;
            int gap = m_bpp / 8 - 2;
            for (int col = 0; col < m_Width; col ++) {
                int gray = FXRGB2GRAY(scanline[2], scanline[1], scanline[0]);
                *scanline ++ = gray;
                *scanline ++ = gray;
                *scanline    = gray;
                scanline += gap;
            }
        }
        return TRUE;
    }
    if (isCmykImage) {
        for (int row = 0; row < m_Height; row ++) {
            FX_LPBYTE scanline = m_pBuffer + row * m_Pitch;
            for (int col = 0; col < m_Width; col ++) {
                FX_BYTE b, g, r;
                AdobeCMYK_to_sRGB1(scanline[0], scanline[1], scanline[2], scanline[3],
                                   r, g, b);
                int gray = 255 - FXRGB2GRAY(r, g, b);
                *scanline ++ = bc + (fc - bc) * gray / 255;
                *scanline ++ = bm + (fm - bm) * gray / 255;
                *scanline ++ = by + (fy - by) * gray / 255;
                *scanline ++ = bk + (fk - bk) * gray / 255;
            }
        }
    } else {
        for (int row = 0; row < m_Height; row ++) {
            FX_LPBYTE scanline = m_pBuffer + row * m_Pitch;
            int gap = m_bpp / 8 - 2;
            for (int col = 0; col < m_Width; col ++) {
                int gray = FXRGB2GRAY(scanline[2], scanline[1], scanline[0]);
                *scanline ++ = bb + (fb - bb) * gray / 255;
                *scanline ++ = bg + (fg - bg) * gray / 255;
                *scanline    = br + (fr - br) * gray / 255;
                scanline += gap;
            }
        }
    }
    return TRUE;
}
FX_BOOL CFX_DIBitmap::DitherFS(const FX_DWORD* pPalette, int pal_size, const FX_RECT* pRect)
{
    if (m_pBuffer == NULL) {
        return FALSE;
    }
    if (m_bpp != 8 && m_pPalette != NULL && m_AlphaFlag != 0) {
        return FALSE;
    }
    if (m_Width < 4 && m_Height < 4) {
        return FALSE;
    }
    FX_RECT rect(0, 0, m_Width, m_Height);
    if (pRect) {
        rect.Intersect(*pRect);
    }
    FX_BYTE translate[256];
    for (int i = 0; i < 256; i ++) {
        int err2 = 65536;
        for (int j = 0; j < pal_size; j ++) {
            FX_BYTE entry = (FX_BYTE)pPalette[j];
            int err = (int)entry - i;
            if (err * err < err2) {
                err2 = err * err;
                translate[i] = entry;
            }
        }
    }
    for (int row = rect.top; row < rect.bottom; row ++) {
        FX_LPBYTE scan = m_pBuffer + row * m_Pitch;
        FX_LPBYTE next_scan = m_pBuffer + (row + 1) * m_Pitch;
        for (int col = rect.left; col < rect.right; col ++) {
            int src_pixel = scan[col];
            int dest_pixel = translate[src_pixel];
            scan[col] = (FX_BYTE)dest_pixel;
            int error = -dest_pixel + src_pixel;
            if (col < rect.right - 1) {
                int src = scan[col + 1];
                src += error * 7 / 16;
                if (src > 255) {
                    scan[col + 1] = 255;
                } else if (src < 0) {
                    scan[col + 1] = 0;
                } else {
                    scan[col + 1] = src;
                }
            }
            if (col < rect.right - 1 && row < rect.bottom - 1) {
                int src = next_scan[col + 1];
                src += error * 1 / 16;
                if (src > 255) {
                    next_scan[col + 1] = 255;
                } else if (src < 0) {
                    next_scan[col + 1] = 0;
                } else {
                    next_scan[col + 1] = src;
                }
            }
            if (row < rect.bottom - 1) {
                int src = next_scan[col];
                src += error * 5 / 16;
                if (src > 255) {
                    next_scan[col] = 255;
                } else if (src < 0) {
                    next_scan[col] = 0;
                } else {
                    next_scan[col] = src;
                }
            }
            if (col > rect.left && row < rect.bottom - 1) {
                int src = next_scan[col - 1];
                src += error * 3 / 16;
                if (src > 255) {
                    next_scan[col - 1] = 255;
                } else if (src < 0) {
                    next_scan[col - 1] = 0;
                } else {
                    next_scan[col - 1] = src;
                }
            }
        }
    }
    return TRUE;
}
CFX_DIBitmap* CFX_DIBSource::FlipImage(FX_BOOL bXFlip, FX_BOOL bYFlip) const
{
    CFX_DIBitmap* pFlipped = FX_NEW CFX_DIBitmap;
    if (!pFlipped) {
        return NULL;
    }
    if (!pFlipped->Create(m_Width, m_Height, GetFormat())) {
        delete pFlipped;
        return NULL;
    }
    pFlipped->CopyPalette(m_pPalette);
    FX_LPBYTE pDestBuffer = pFlipped->GetBuffer();
    int Bpp = m_bpp / 8;
    for (int row = 0; row < m_Height; row ++) {
        FX_LPCBYTE src_scan = GetScanline(row);
        FX_LPBYTE dest_scan = pDestBuffer + m_Pitch * (bYFlip ? (m_Height - row - 1) : row);
        if (!bXFlip) {
            FXSYS_memcpy32(dest_scan, src_scan, m_Pitch);
            continue;
        }
        if (m_bpp == 1) {
            FXSYS_memset32(dest_scan, 0, m_Pitch);
            for (int col = 0; col < m_Width; col ++)
                if (src_scan[col / 8] & (1 << (7 - col % 8))) {
                    int dest_col = m_Width - col - 1;
                    dest_scan[dest_col / 8] |= (1 << (7 - dest_col % 8));
                }
        } else {
            dest_scan += (m_Width - 1) * Bpp;
            if (Bpp == 1) {
                for (int col = 0; col < m_Width; col ++) {
                    *dest_scan = *src_scan;
                    dest_scan --;
                    src_scan ++;
                }
            } else if (Bpp == 3) {
                for (int col = 0; col < m_Width; col ++) {
                    dest_scan[0] = src_scan[0];
                    dest_scan[1] = src_scan[1];
                    dest_scan[2] = src_scan[2];
                    dest_scan -= 3;
                    src_scan += 3;
                }
            } else {
                ASSERT(Bpp == 4);
                for (int col = 0; col < m_Width; col ++) {
                    *(FX_DWORD*)dest_scan = *(FX_DWORD*)src_scan;
                    dest_scan -= 4;
                    src_scan += 4;
                }
            }
        }
    }
    if (m_pAlphaMask) {
        pDestBuffer = pFlipped->m_pAlphaMask->GetBuffer();
        FX_DWORD dest_pitch = pFlipped->m_pAlphaMask->GetPitch();
        for (int row = 0; row < m_Height; row ++) {
            FX_LPCBYTE src_scan = m_pAlphaMask->GetScanline(row);
            FX_LPBYTE dest_scan = pDestBuffer + dest_pitch * (bYFlip ? (m_Height - row - 1) : row);
            if (!bXFlip) {
                FXSYS_memcpy32(dest_scan, src_scan, dest_pitch);
                continue;
            }
            dest_scan += (m_Width - 1);
            for (int col = 0; col < m_Width; col ++) {
                *dest_scan = *src_scan;
                dest_scan --;
                src_scan ++;
            }
        }
    }
    return pFlipped;
}
CFX_DIBExtractor::CFX_DIBExtractor(const CFX_DIBSource* pSrc)
{
    m_pBitmap = NULL;
    if (pSrc->GetBuffer() == NULL) {
        m_pBitmap = pSrc->Clone();
    } else {
        m_pBitmap = FX_NEW CFX_DIBitmap;
        if (!m_pBitmap) {
            return;
        }
        if (!m_pBitmap->Create(pSrc->GetWidth(), pSrc->GetHeight(), pSrc->GetFormat(), pSrc->GetBuffer())) {
            delete m_pBitmap;
            m_pBitmap = NULL;
            return;
        }
        m_pBitmap->CopyPalette(pSrc->GetPalette());
        m_pBitmap->CopyAlphaMask(pSrc->m_pAlphaMask);
    }
}
CFX_DIBExtractor::~CFX_DIBExtractor()
{
    if (m_pBitmap) {
        delete m_pBitmap;
    }
}
CFX_FilteredDIB::CFX_FilteredDIB()
{
    m_pScanline = NULL;
    m_pSrc = NULL;
}
CFX_FilteredDIB::~CFX_FilteredDIB()
{
    if (m_pSrc && m_bAutoDropSrc) {
        delete m_pSrc;
    }
    if (m_pScanline) {
        FX_Free(m_pScanline);
    }
}
void CFX_FilteredDIB::LoadSrc(const CFX_DIBSource* pSrc, FX_BOOL bAutoDropSrc)
{
    m_pSrc = pSrc;
    m_bAutoDropSrc = bAutoDropSrc;
    m_Width = pSrc->GetWidth();
    m_Height = pSrc->GetHeight();
    FXDIB_Format format = GetDestFormat();
    m_bpp = (FX_BYTE)format;
    m_AlphaFlag = (FX_BYTE)(format >> 8);
    m_Pitch = (m_Width * (format & 0xff) + 31) / 32 * 4;
    m_pPalette = GetDestPalette();
    m_pScanline = FX_Alloc(FX_BYTE, m_Pitch);
}
FX_LPCBYTE CFX_FilteredDIB::GetScanline(int line) const
{
    TranslateScanline(m_pScanline, m_pSrc->GetScanline(line));
    return m_pScanline;
}
void CFX_FilteredDIB::DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
        int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const
{
    m_pSrc->DownSampleScanline(line, dest_scan, dest_bpp, dest_width, bFlipX, clip_left, clip_width);
    TranslateDownSamples(dest_scan, dest_scan, clip_width, dest_bpp);
}
CFX_ImageRenderer::CFX_ImageRenderer()
{
    m_Status = 0;
    m_pTransformer = NULL;
    m_bRgbByteOrder = FALSE;
    m_BlendType = FXDIB_BLEND_NORMAL;
}
CFX_ImageRenderer::~CFX_ImageRenderer()
{
    if (m_pTransformer) {
        delete m_pTransformer;
    }
}
extern FX_RECT _FXDIB_SwapClipBox(FX_RECT& clip, int width, int height, FX_BOOL bFlipX, FX_BOOL bFlipY);
FX_BOOL CFX_ImageRenderer::Start(CFX_DIBitmap* pDevice, const CFX_ClipRgn* pClipRgn,
                                 const CFX_DIBSource* pSource, int bitmap_alpha,
                                 FX_DWORD mask_color, const CFX_AffineMatrix* pMatrix,
                                 FX_DWORD dib_flags, FX_BOOL bRgbByteOrder,
                                 int alpha_flag, void* pIccTransform, int blend_type)
{
    m_Matrix = *pMatrix;
    CFX_FloatRect image_rect_f = m_Matrix.GetUnitRect();
    FX_RECT image_rect = image_rect_f.GetOutterRect();
    m_ClipBox = pClipRgn ? pClipRgn->GetBox() : FX_RECT(0, 0, pDevice->GetWidth(), pDevice->GetHeight());
    m_ClipBox.Intersect(image_rect);
    if (m_ClipBox.IsEmpty()) {
        return FALSE;
    }
    m_pDevice = pDevice;
    m_pClipRgn = pClipRgn;
    m_MaskColor = mask_color;
    m_BitmapAlpha = bitmap_alpha;
    m_Matrix = *pMatrix;
    m_Flags = dib_flags;
    m_AlphaFlag = alpha_flag;
    m_pIccTransform = pIccTransform;
    m_bRgbByteOrder = bRgbByteOrder;
    m_BlendType = blend_type;
    FX_BOOL ret = TRUE;
    if ((FXSYS_fabs(m_Matrix.b) >= 0.5f || m_Matrix.a == 0) ||
            (FXSYS_fabs(m_Matrix.c) >= 0.5f || m_Matrix.d == 0) ) {
        if (FXSYS_fabs(m_Matrix.a) < FXSYS_fabs(m_Matrix.b) / 20 && FXSYS_fabs(m_Matrix.d) < FXSYS_fabs(m_Matrix.c) / 20 &&
                FXSYS_fabs(m_Matrix.a) < 0.5f && FXSYS_fabs(m_Matrix.d) < 0.5f) {
            int dest_width = image_rect.Width();
            int dest_height = image_rect.Height();
            FX_RECT bitmap_clip = m_ClipBox;
            bitmap_clip.Offset(-image_rect.left, -image_rect.top);
            bitmap_clip = _FXDIB_SwapClipBox(bitmap_clip, dest_width, dest_height, m_Matrix.c > 0, m_Matrix.b < 0);
            m_Composer.Compose(pDevice, pClipRgn, bitmap_alpha, mask_color, m_ClipBox, TRUE,
                               m_Matrix.c > 0, m_Matrix.b < 0, m_bRgbByteOrder, alpha_flag, pIccTransform, m_BlendType);
            if (!m_Stretcher.Start(&m_Composer, pSource, dest_height, dest_width, bitmap_clip, dib_flags)) {
                return FALSE;
            }
            m_Status = 1;
            return TRUE;
        }
        m_Status = 2;
        m_pTransformer = FX_NEW CFX_ImageTransformer;
        if (!m_pTransformer) {
            return FALSE;
        }
        m_pTransformer->Start(pSource, &m_Matrix, dib_flags, &m_ClipBox);
        return TRUE;
    }
    int dest_width = image_rect.Width();
    if (m_Matrix.a < 0) {
        dest_width = -dest_width;
    }
    int dest_height = image_rect.Height();
    if (m_Matrix.d > 0) {
        dest_height = -dest_height;
    }
    if (dest_width == 0 || dest_height == 0) {
        return FALSE;
    }
    FX_RECT bitmap_clip = m_ClipBox;
    bitmap_clip.Offset(-image_rect.left, -image_rect.top);
    m_Composer.Compose(pDevice, pClipRgn, bitmap_alpha, mask_color,
                       m_ClipBox, FALSE, FALSE, FALSE, m_bRgbByteOrder, alpha_flag, pIccTransform, m_BlendType);
    m_Status = 1;
    ret = m_Stretcher.Start(&m_Composer, pSource, dest_width, dest_height, bitmap_clip, dib_flags);
    return ret;
}
FX_BOOL CFX_ImageRenderer::Continue(IFX_Pause* pPause)
{
    if (m_Status == 1) {
        return m_Stretcher.Continue(pPause);
    } else if (m_Status == 2) {
        if (m_pTransformer->Continue(pPause)) {
            return TRUE;
        }
        CFX_DIBitmap* pBitmap = m_pTransformer->m_Storer.Detach();
        if (pBitmap == NULL) {
            return FALSE;
        }
        if (pBitmap->GetBuffer() == NULL) {
            delete pBitmap;
            return FALSE;
        }
        if (pBitmap->IsAlphaMask()) {
            if (m_BitmapAlpha != 255) {
                if (m_AlphaFlag >> 8) {
                    m_AlphaFlag = (((FX_BYTE)((m_AlphaFlag & 0xff) * m_BitmapAlpha / 255)) | ((m_AlphaFlag >> 8) << 8));
                } else {
                    m_MaskColor = FXARGB_MUL_ALPHA(m_MaskColor, m_BitmapAlpha);
                }
            }
            m_pDevice->CompositeMask(m_pTransformer->m_ResultLeft, m_pTransformer->m_ResultTop,
                                     pBitmap->GetWidth(), pBitmap->GetHeight(), pBitmap, m_MaskColor,
                                     0, 0, m_BlendType, m_pClipRgn, m_bRgbByteOrder, m_AlphaFlag, m_pIccTransform);
        } else {
            if (m_BitmapAlpha != 255) {
                pBitmap->MultiplyAlpha(m_BitmapAlpha);
            }
            m_pDevice->CompositeBitmap(m_pTransformer->m_ResultLeft, m_pTransformer->m_ResultTop,
                                       pBitmap->GetWidth(), pBitmap->GetHeight(), pBitmap, 0, 0, m_BlendType, m_pClipRgn, m_bRgbByteOrder, m_pIccTransform);
        }
        delete pBitmap;
        return FALSE;
    }
    return FALSE;
}
CFX_BitmapStorer::CFX_BitmapStorer()
{
    m_pBitmap = NULL;
}
CFX_BitmapStorer::~CFX_BitmapStorer()
{
    if (m_pBitmap) {
        delete m_pBitmap;
    }
}
CFX_DIBitmap* CFX_BitmapStorer::Detach()
{
    CFX_DIBitmap* pBitmap = m_pBitmap;
    m_pBitmap = NULL;
    return pBitmap;
}
void CFX_BitmapStorer::Replace(CFX_DIBitmap* pBitmap)
{
    if (m_pBitmap) {
        delete m_pBitmap;
    }
    m_pBitmap = pBitmap;
}
void CFX_BitmapStorer::ComposeScanline(int line, FX_LPCBYTE scanline, FX_LPCBYTE scan_extra_alpha)
{
    FX_LPBYTE dest_buf = (FX_LPBYTE)m_pBitmap->GetScanline(line);
    FX_LPBYTE dest_alpha_buf = m_pBitmap->m_pAlphaMask ?
                               (FX_LPBYTE)m_pBitmap->m_pAlphaMask->GetScanline(line) : NULL;
    if (dest_buf) {
        FXSYS_memcpy32(dest_buf, scanline, m_pBitmap->GetPitch());
    }
    if (dest_alpha_buf) {
        FXSYS_memcpy32(dest_alpha_buf, scan_extra_alpha, m_pBitmap->m_pAlphaMask->GetPitch());
    }
}
FX_BOOL CFX_BitmapStorer::SetInfo(int width, int height, FXDIB_Format src_format, FX_DWORD* pSrcPalette)
{
    m_pBitmap = FX_NEW CFX_DIBitmap;
    if (!m_pBitmap) {
        return FALSE;
    }
    if (!m_pBitmap->Create(width, height, src_format)) {
        delete m_pBitmap;
        m_pBitmap = NULL;
        return FALSE;
    }
    if (pSrcPalette) {
        m_pBitmap->CopyPalette(pSrcPalette);
    }
    return TRUE;
}
