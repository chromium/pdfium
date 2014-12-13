// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcrt/fx_ext.h"
#include "../../../include/fxge/fx_ge.h"
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
#include "apple_int.h"
#include "../../../include/fxge/fx_ge_apple.h"
#include "../agg/include/fxfx_agg_clip_liang_barsky.h"
#include "../ge/text_int.h"
#include "../dib/dib_int.h"
#include "../agg/include/fx_agg_driver.h"
#include "../../../include/fxge/fx_freetype.h"
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
void CFX_AggDeviceDriver::InitPlatform()
{
    CQuartz2D & quartz2d = ((CApplePlatform *) CFX_GEModule::Get()->GetPlatformData())->_quartz2d;
    m_pPlatformGraphics = quartz2d.createGraphics(m_pBitmap);
}
void CFX_AggDeviceDriver::DestroyPlatform()
{
    CQuartz2D & quartz2d = ((CApplePlatform *) CFX_GEModule::Get()->GetPlatformData())->_quartz2d;
    if (m_pPlatformGraphics) {
        quartz2d.destroyGraphics(m_pPlatformGraphics);
        m_pPlatformGraphics = NULL;
    }
}
void CFX_FaceCache::InitPlatform() {}
void CFX_FaceCache::DestroyPlatform() {}
CFX_GlyphBitmap* CFX_FaceCache::RenderGlyph_Nativetext(CFX_Font *				pFont,
        FX_DWORD					glyph_index,
        const CFX_AffineMatrix *	pMatrix,
        int						dest_width,
        int						anti_alias)
{
    return NULL;
}
static FX_BOOL _CGDrawGlyphRun(CGContextRef               pContext,
                               int                        nChars,
                               const FXTEXT_CHARPOS*      pCharPos,
                               CFX_Font*                  pFont,
                               CFX_FontCache*             pCache,
                               const CFX_AffineMatrix*    pObject2Device,
                               FX_FLOAT                   font_size,
                               FX_DWORD                   argb,
                               int                        alpha_flag,
                               void*                      pIccTransform)
{
    if (nChars == 0) {
        return TRUE;
    }
    CFX_AffineMatrix new_matrix;
    FX_BOOL bNegSize = font_size < 0;
    if (bNegSize) {
        font_size = -font_size;
    }
    FX_FLOAT ori_x = pCharPos[0].m_OriginX, ori_y = pCharPos[0].m_OriginY;
    new_matrix.Transform(ori_x, ori_y);
    if (pObject2Device) {
        new_matrix.Concat(*pObject2Device);
    }
    CQuartz2D& quartz2d = ((CApplePlatform *) CFX_GEModule::Get()->GetPlatformData())->_quartz2d;
    if (!pFont->m_pPlatformFont) {
        if (pFont->GetPsName() == CFX_WideString::FromLocal("DFHeiStd-W5")) {
            return FALSE;
        }
        pFont->m_pPlatformFont = quartz2d.CreateFont(pFont->m_pFontData, pFont->m_dwSize);
        if (NULL == pFont->m_pPlatformFont) {
            return FALSE;
        }
    }
    CFX_FixedBufGrow<FX_WORD, 32> glyph_indices(nChars);
    CFX_FixedBufGrow<CGPoint, 32> glyph_positions(nChars);
    for (int i = 0; i < nChars; i++ ) {
        glyph_indices[i] = pCharPos[i].m_ExtGID;
        if (bNegSize) {
            glyph_positions[i].x = -pCharPos[i].m_OriginX;
        } else {
            glyph_positions[i].x = pCharPos[i].m_OriginX;
        }
        glyph_positions[i].y = pCharPos[i].m_OriginY;
    }
    if (bNegSize) {
        new_matrix.a = -new_matrix.a;
    } else {
        new_matrix.b = -new_matrix.b;
        new_matrix.d = -new_matrix.d;
    }
    quartz2d.setGraphicsTextMatrix(pContext, &new_matrix);
    return quartz2d.drawGraphicsString(pContext,
                                       pFont->m_pPlatformFont,
                                       font_size,
                                       glyph_indices,
                                       glyph_positions,
                                       nChars,
                                       argb,
                                       NULL);
}
static void _DoNothing(void *info, const void *data, size_t size) {}
FX_BOOL CFX_AggDeviceDriver::DrawDeviceText(int						 nChars,
        const FXTEXT_CHARPOS *	 pCharPos,
        CFX_Font *				 pFont,
        CFX_FontCache *			 pCache,
        const CFX_AffineMatrix * pObject2Device,
        FX_FLOAT				 font_size,
        FX_DWORD				 argb,
        int alpha_flag, void* pIccTransform)
{
    if (!pFont) {
        return FALSE;
    }
    FX_BOOL bBold = pFont->IsBold();
    if (!bBold && pFont->GetSubstFont() &&
            pFont->GetSubstFont()->m_Weight >= 500 &&
            pFont->GetSubstFont()->m_Weight <= 600) {
        return FALSE;
    }
    for (int i = 0; i < nChars; i ++) {
        if (pCharPos[i].m_bGlyphAdjust) {
            return FALSE;
        }
    }
    CGContextRef ctx = CGContextRef(m_pPlatformGraphics);
    if (NULL == ctx) {
        return FALSE;
    }
    CGContextSaveGState(ctx);
    CGContextSetTextDrawingMode(ctx, kCGTextFillClip);
    CGRect rect_cg;
    CGImageRef pImageCG = NULL;
    if (m_pClipRgn) {
        rect_cg = CGRectMake(m_pClipRgn->GetBox().left, m_pClipRgn->GetBox().top, m_pClipRgn->GetBox().Width(), m_pClipRgn->GetBox().Height());
        const CFX_DIBitmap*	pClipMask = m_pClipRgn->GetMask();
        if (pClipMask) {
            CGDataProviderRef pClipMaskDataProvider = CGDataProviderCreateWithData(NULL,
                    pClipMask->GetBuffer(),
                    pClipMask->GetPitch() * pClipMask->GetHeight(),
                    _DoNothing);
            CGFloat decode_f[2] = {255.f, 0.f};
            pImageCG = CGImageMaskCreate(pClipMask->GetWidth(), pClipMask->GetHeight(),
                                         8, 8, pClipMask->GetPitch(), pClipMaskDataProvider,
                                         decode_f, FALSE);
            CGDataProviderRelease(pClipMaskDataProvider);
        }
    } else {
        rect_cg = CGRectMake(0, 0, m_pBitmap->GetWidth(), m_pBitmap->GetHeight());
    }
    rect_cg = CGContextConvertRectToDeviceSpace(ctx, rect_cg);
    if (pImageCG) {
        CGContextClipToMask(ctx, rect_cg, pImageCG);
    } else {
        CGContextClipToRect(ctx, rect_cg);
    }
    FX_BOOL ret = _CGDrawGlyphRun(ctx, nChars, pCharPos, pFont, pCache, pObject2Device, font_size, argb, alpha_flag, pIccTransform);
    if (pImageCG) {
        CGImageRelease(pImageCG);
    }
    CGContextRestoreGState(ctx);
    return ret;
}
void CFX_Font::ReleasePlatformResource()
{
    if (m_pPlatformFont) {
        CQuartz2D & quartz2d = ((CApplePlatform *) CFX_GEModule::Get()->GetPlatformData())->_quartz2d;
        quartz2d.DestroyFont(m_pPlatformFont);
        m_pPlatformFont = NULL;
    }
}
#endif
#endif
