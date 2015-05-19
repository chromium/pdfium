// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_
#include <windows.h>
#include "../../../include/fxge/fx_ge_win32.h"
#include "win32_int.h"
#include "../../../include/fxge/fx_freetype.h"
#include "../ge/text_int.h"
#include "../dib/dib_int.h"
#define SIZETHRESHOLD 1000
#define OUTPUTPSLEN 4096
CGdiPrinterDriver::CGdiPrinterDriver(HDC hDC) : CGdiDeviceDriver(hDC, FXDC_PRINTER)
{
    m_HorzSize = ::GetDeviceCaps(m_hDC, HORZSIZE);
    m_VertSize = ::GetDeviceCaps(m_hDC, VERTSIZE);
    m_bSupportROP = TRUE;
}
int CGdiPrinterDriver::GetDeviceCaps(int caps_id)
{
    if (caps_id == FXDC_HORZ_SIZE) {
        return m_HorzSize;
    }
    if (caps_id == FXDC_VERT_SIZE) {
        return m_VertSize;
    }
    return CGdiDeviceDriver::GetDeviceCaps(caps_id);
}
FX_BOOL CGdiPrinterDriver::SetDIBits(const CFX_DIBSource* pSource, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                                     int alpha_flag, void* pIccTransform)
{
    if (pSource->IsAlphaMask()) {
        FX_RECT clip_rect(left, top, left + pSrcRect->Width(), top + pSrcRect->Height());
        return StretchDIBits(pSource, color, left - pSrcRect->left, top - pSrcRect->top, pSource->GetWidth(), pSource->GetHeight(),
                             &clip_rect, 0, alpha_flag, pIccTransform, FXDIB_BLEND_NORMAL);
    }
    ASSERT(pSource != NULL && !pSource->IsAlphaMask() && pSrcRect != NULL);
    ASSERT(blend_type == FXDIB_BLEND_NORMAL);
    if (pSource->HasAlpha()) {
        return FALSE;
    }
    CFX_DIBExtractor temp(pSource);
    CFX_DIBitmap* pBitmap = temp;
    if (pBitmap == NULL) {
        return FALSE;
    }
    return GDI_SetDIBits(pBitmap, pSrcRect, left, top, pIccTransform);
}
FX_BOOL CGdiPrinterDriver::StretchDIBits(const CFX_DIBSource* pSource, FX_DWORD color, int dest_left, int dest_top,
        int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
        int alpha_flag, void* pIccTransform, int blend_type)
{
    if (pSource->IsAlphaMask()) {
        int alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXARGB_A(color);
        if (pSource->GetBPP() != 1 || alpha != 255 || !m_bSupportROP) {
            return FALSE;
        }
        if (dest_width < 0 || dest_height < 0) {
            CFX_DIBitmap* pFlipped = pSource->FlipImage(dest_width < 0, dest_height < 0);
            if (pFlipped == NULL) {
                return FALSE;
            }
            if (dest_width < 0) {
                dest_left += dest_width;
            }
            if (dest_height < 0) {
                dest_top += dest_height;
            }
            FX_BOOL ret = GDI_StretchBitMask(pFlipped, dest_left, dest_top, abs(dest_width), abs(dest_height), color, flags, alpha_flag, pIccTransform);
            delete pFlipped;
            return ret;
        }
        CFX_DIBExtractor temp(pSource);
        CFX_DIBitmap* pBitmap = temp;
        if (pBitmap == NULL) {
            return FALSE;
        }
        return GDI_StretchBitMask(pBitmap, dest_left, dest_top, dest_width, dest_height, color, flags, alpha_flag, pIccTransform);
    } else {
        ASSERT(pSource != NULL);
        if (pSource->HasAlpha()) {
            return FALSE;
        }
        if (dest_width < 0 || dest_height < 0) {
            CFX_DIBitmap* pFlipped = pSource->FlipImage(dest_width < 0, dest_height < 0);
            if (pFlipped == NULL) {
                return FALSE;
            }
            if (dest_width < 0) {
                dest_left += dest_width;
            }
            if (dest_height < 0) {
                dest_top += dest_height;
            }
            FX_BOOL ret = GDI_StretchDIBits(pFlipped, dest_left, dest_top, abs(dest_width), abs(dest_height), flags, pIccTransform);
            delete pFlipped;
            return ret;
        }
        CFX_DIBExtractor temp(pSource);
        CFX_DIBitmap* pBitmap = temp;
        if (pBitmap == NULL) {
            return FALSE;
        }
        return GDI_StretchDIBits(pBitmap, dest_left, dest_top, dest_width, dest_height, flags, pIccTransform);
    }
}
static CFX_DIBitmap* Transform1bppBitmap(const CFX_DIBSource* pSrc, const CFX_AffineMatrix* pDestMatrix)
{
    ASSERT(pSrc->GetFormat() == FXDIB_1bppRgb || pSrc->GetFormat() == FXDIB_1bppMask || pSrc->GetFormat() == FXDIB_1bppCmyk);
    CFX_FloatRect unit_rect = pDestMatrix->GetUnitRect();
    FX_RECT full_rect = unit_rect.GetOutterRect();
    int full_left = full_rect.left;
    int full_top = full_rect.top;
    CFX_DIBExtractor src_bitmap(pSrc);
    CFX_DIBitmap* pSrcBitmap = src_bitmap;
    if (pSrcBitmap == NULL) {
        return NULL;
    }
    int src_width = pSrcBitmap->GetWidth(), src_height = pSrcBitmap->GetHeight();
    FX_LPBYTE src_buf = pSrcBitmap->GetBuffer();
    FX_DWORD src_pitch = pSrcBitmap->GetPitch();
    FX_FLOAT dest_area = pDestMatrix->GetUnitArea();
    FX_FLOAT area_scale = FXSYS_Div((FX_FLOAT)(src_width * src_height), dest_area);
    FX_FLOAT size_scale = FXSYS_sqrt(area_scale);
    CFX_AffineMatrix adjusted_matrix(*pDestMatrix);
    adjusted_matrix.Scale(size_scale, size_scale);
    CFX_FloatRect result_rect_f = adjusted_matrix.GetUnitRect();
    FX_RECT result_rect = result_rect_f.GetOutterRect();
    CFX_AffineMatrix src2result;
    src2result.e = adjusted_matrix.c + adjusted_matrix.e;
    src2result.f = adjusted_matrix.d + adjusted_matrix.f;
    src2result.a = adjusted_matrix.a / pSrcBitmap->GetWidth();
    src2result.b = adjusted_matrix.b / pSrcBitmap->GetWidth();
    src2result.c = -adjusted_matrix.c / pSrcBitmap->GetHeight();
    src2result.d = -adjusted_matrix.d / pSrcBitmap->GetHeight();
    src2result.TranslateI(-result_rect.left, -result_rect.top);
    CFX_AffineMatrix result2src;
    result2src.SetReverse(src2result);
    CPDF_FixedMatrix result2src_fix(result2src, 8);
    int result_width = result_rect.Width();
    int result_height = result_rect.Height();
    CFX_DIBitmap* pTempBitmap = new CFX_DIBitmap;
    if (!pTempBitmap->Create(result_width, result_height, pSrc->GetFormat())) {
        delete pTempBitmap;
        if (pSrcBitmap != src_bitmap) {
            delete pSrcBitmap;
        }
        return NULL;
    }
    pTempBitmap->CopyPalette(pSrc->GetPalette());
    FX_LPBYTE dest_buf = pTempBitmap->GetBuffer();
    int dest_pitch = pTempBitmap->GetPitch();
    FXSYS_memset8(dest_buf, pSrc->IsAlphaMask() ? 0 : 0xff, dest_pitch * result_height);
    if (pSrcBitmap->IsAlphaMask()) {
        for (int dest_y = 0; dest_y < result_height; dest_y ++) {
            FX_LPBYTE dest_scan = dest_buf + dest_y * dest_pitch;
            for (int dest_x = 0; dest_x < result_width; dest_x ++) {
                int src_x, src_y;
                result2src_fix.Transform(dest_x, dest_y, src_x, src_y);
                if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) {
                    continue;
                }
                if (!((src_buf + src_pitch * src_y)[src_x / 8] & (1 << (7 - src_x % 8)))) {
                    continue;
                }
                dest_scan[dest_x / 8] |= 1 << (7 - dest_x % 8);
            }
        }
    } else {
        for (int dest_y = 0; dest_y < result_height; dest_y ++) {
            FX_LPBYTE dest_scan = dest_buf + dest_y * dest_pitch;
            for (int dest_x = 0; dest_x < result_width; dest_x ++) {
                int src_x, src_y;
                result2src_fix.Transform(dest_x, dest_y, src_x, src_y);
                if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) {
                    continue;
                }
                if ((src_buf + src_pitch * src_y)[src_x / 8] & (1 << (7 - src_x % 8))) {
                    continue;
                }
                dest_scan[dest_x / 8] &= ~(1 << (7 - dest_x % 8));
            }
        }
    }
    if (pSrcBitmap != src_bitmap) {
        delete pSrcBitmap;
    }
    return pTempBitmap;
}
FX_BOOL	CGdiPrinterDriver::StartDIBits(const CFX_DIBSource* pSource, int bitmap_alpha, FX_DWORD color,
                                       const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, FX_LPVOID& handle,
                                       int alpha_flag, void* pIccTransform, int blend_type)
{
    if (bitmap_alpha < 255 || pSource->HasAlpha() || (pSource->IsAlphaMask() && (pSource->GetBPP() != 1 || !m_bSupportROP))) {
        return FALSE;
    }
    CFX_FloatRect unit_rect = pMatrix->GetUnitRect();
    FX_RECT full_rect = unit_rect.GetOutterRect();
    if (FXSYS_fabs(pMatrix->b) < 0.5f && pMatrix->a != 0 && FXSYS_fabs(pMatrix->c) < 0.5f && pMatrix->d != 0) {
        FX_BOOL bFlipX = pMatrix->a < 0;
        FX_BOOL bFlipY = pMatrix->d > 0;
        return StretchDIBits(pSource, color, bFlipX ? full_rect.right : full_rect.left, bFlipY ? full_rect.bottom : full_rect.top,
                             bFlipX ? -full_rect.Width() : full_rect.Width(), bFlipY ? -full_rect.Height() : full_rect.Height(), NULL, 0,
                             alpha_flag, pIccTransform, blend_type);
    }
    if (FXSYS_fabs(pMatrix->a) < 0.5f && FXSYS_fabs(pMatrix->d) < 0.5f) {
        CFX_DIBitmap* pTransformed = pSource->SwapXY(pMatrix->c > 0, pMatrix->b < 0);
        if (pTransformed == NULL) {
            return FALSE;
        }
        FX_BOOL ret = StretchDIBits(pTransformed, color, full_rect.left, full_rect.top, full_rect.Width(), full_rect.Height(), NULL, 0,
                                    alpha_flag, pIccTransform, blend_type);
        delete pTransformed;
        return ret;
    }
    if (pSource->GetBPP() == 1) {
        CFX_DIBitmap* pTransformed = Transform1bppBitmap(pSource, pMatrix);
        if (pIccTransform == NULL) {
            return FALSE;
        }
        SaveState();
        CFX_PathData path;
        path.AppendRect(0, 0, 1.0f, 1.0f);
        SetClip_PathFill(&path, pMatrix, WINDING);
        FX_BOOL ret = StretchDIBits(pTransformed, color, full_rect.left, full_rect.top, full_rect.Width(), full_rect.Height(), NULL, 0,
                                    alpha_flag, pIccTransform, blend_type);
        RestoreState();
        delete pTransformed;
        handle = NULL;
        return ret;
    }
    return FALSE;
}
CPSOutput::CPSOutput(HDC hDC)
{
    m_hDC = hDC;
    m_pBuf = NULL;
}
CPSOutput::~CPSOutput()
{
    if (m_pBuf) {
        FX_Free(m_pBuf);
    }
}
void CPSOutput::Init()
{
    m_pBuf = FX_Alloc(FX_CHAR, 1026);
}
void CPSOutput::OutputPS(FX_LPCSTR string, int len)
{
    if (len < 0) {
        len = (int)FXSYS_strlen(string);
    }
    int sent_len = 0;
    while (len > 0) {
        int send_len = len > 1024 ? 1024 : len;
        *(FX_WORD*)m_pBuf = send_len;
        FXSYS_memcpy(m_pBuf + 2, string + sent_len, send_len);
        int ret = ExtEscape(m_hDC, PASSTHROUGH, send_len + 2, m_pBuf, 0, NULL);
        sent_len += send_len;
        len -= send_len;
    }
}
CPSPrinterDriver::CPSPrinterDriver()
{
    m_pPSOutput = NULL;
    m_bCmykOutput = FALSE;
}
CPSPrinterDriver::~CPSPrinterDriver()
{
    EndRendering();
    if (m_pPSOutput) {
        delete m_pPSOutput;
    }
}
FX_BOOL CPSPrinterDriver::Init(HDC hDC, int pslevel, FX_BOOL bCmykOutput)
{
    m_hDC = hDC;
    m_HorzSize = ::GetDeviceCaps(m_hDC, HORZSIZE);
    m_VertSize = ::GetDeviceCaps(m_hDC, VERTSIZE);
    m_Width = ::GetDeviceCaps(m_hDC, HORZRES);
    m_Height = ::GetDeviceCaps(m_hDC, VERTRES);
    m_nBitsPerPixel = ::GetDeviceCaps(m_hDC, BITSPIXEL);
    m_pPSOutput = new CPSOutput(hDC);
    ((CPSOutput*)m_pPSOutput)->Init();
    m_PSRenderer.Init(m_pPSOutput, pslevel, m_Width, m_Height, bCmykOutput);
    m_bCmykOutput = bCmykOutput;
    HRGN hRgn = ::CreateRectRgn(0, 0, 1, 1);
    int ret = ::GetClipRgn(hDC, hRgn);
    if (ret == 1) {
        ret = ::GetRegionData(hRgn, 0, NULL);
        if (ret) {
            RGNDATA* pData = (RGNDATA*)FX_Alloc(FX_BYTE, ret);
            ret = ::GetRegionData(hRgn, ret, pData);
            if (ret) {
                CFX_PathData path;
                path.AllocPointCount(pData->rdh.nCount * 5);
                for (FX_DWORD i = 0; i < pData->rdh.nCount; i ++) {
                    RECT* pRect = (RECT*)(pData->Buffer + pData->rdh.nRgnSize * i);
                    path.AppendRect((FX_FLOAT)pRect->left, (FX_FLOAT)pRect->bottom, (FX_FLOAT)pRect->right, (FX_FLOAT)pRect->top);
                }
                m_PSRenderer.SetClip_PathFill(&path, NULL, FXFILL_WINDING);
            }
            FX_Free(pData);
        }
    }
    ::DeleteObject(hRgn);
    return TRUE;
}
int CPSPrinterDriver::GetDeviceCaps(int caps_id)
{
    switch (caps_id) {
        case FXDC_DEVICE_CLASS:
            return FXDC_PRINTER;
        case FXDC_PIXEL_WIDTH:
            return m_Width;
        case FXDC_PIXEL_HEIGHT:
            return m_Height;
        case FXDC_BITS_PIXEL:
            return m_nBitsPerPixel;
        case FXDC_RENDER_CAPS:
            return m_bCmykOutput ? FXRC_BIT_MASK | FXRC_CMYK_OUTPUT : FXRC_BIT_MASK;
        case FXDC_HORZ_SIZE:
            return m_HorzSize;
        case FXDC_VERT_SIZE:
            return m_VertSize;
    }
    return 0;
}
FX_BOOL CPSPrinterDriver::StartRendering()
{
    return m_PSRenderer.StartRendering();
}
void CPSPrinterDriver::EndRendering()
{
    m_PSRenderer.EndRendering();
}
void CPSPrinterDriver::SaveState()
{
    m_PSRenderer.SaveState();
}
void CPSPrinterDriver::RestoreState(FX_BOOL bKeepSaved)
{
    m_PSRenderer.RestoreState(bKeepSaved);
}
FX_BOOL	CPSPrinterDriver::SetClip_PathFill(const CFX_PathData* pPathData, const CFX_AffineMatrix* pObject2Device,
        int fill_mode)
{
    m_PSRenderer.SetClip_PathFill(pPathData, pObject2Device, fill_mode);
    return TRUE;
}
FX_BOOL	CPSPrinterDriver::SetClip_PathStroke(const CFX_PathData* pPathData,
        const CFX_AffineMatrix* pObject2Device,
        const CFX_GraphStateData* pGraphState)
{
    m_PSRenderer.SetClip_PathStroke(pPathData, pObject2Device, pGraphState);
    return TRUE;
}
FX_BOOL	CPSPrinterDriver::DrawPath(const CFX_PathData* pPathData,
                                   const CFX_AffineMatrix* pObject2Device,
                                   const CFX_GraphStateData* pGraphState, FX_ARGB fill_color, FX_ARGB stroke_color,
                                   int fill_mode, int alpha_flag, void* pIccTransform, int blend_type)
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    return m_PSRenderer.DrawPath(pPathData, pObject2Device, pGraphState, fill_color, stroke_color, fill_mode & 3, alpha_flag, pIccTransform);
}
FX_BOOL CPSPrinterDriver::GetClipBox(FX_RECT* pRect)
{
    *pRect = m_PSRenderer.GetClipBox();
    return TRUE;
}
FX_BOOL CPSPrinterDriver::SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                                    int alpha_flag, void* pIccTransform)
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    return m_PSRenderer.SetDIBits(pBitmap, color, left, top, alpha_flag, pIccTransform);
}
FX_BOOL CPSPrinterDriver::StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                        int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                        int alpha_flag, void* pIccTransform, int blend_type)
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    return m_PSRenderer.StretchDIBits(pBitmap, color, dest_left, dest_top, dest_width, dest_height, flags, alpha_flag, pIccTransform);
}
FX_BOOL	CPSPrinterDriver::StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                      const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, FX_LPVOID& handle,
                                      int alpha_flag, void* pIccTransform, int blend_type)
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    if (bitmap_alpha < 255) {
        return FALSE;
    }
    handle = NULL;
    return m_PSRenderer.DrawDIBits(pBitmap, color, pMatrix, render_flags, alpha_flag, pIccTransform);
}
FX_BOOL	CPSPrinterDriver::DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
        CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
        int alpha_flag, void* pIccTransform)
{
    return m_PSRenderer.DrawText(nChars, pCharPos, pFont, pCache, pObject2Device, font_size, color, alpha_flag, pIccTransform);
}
#endif
