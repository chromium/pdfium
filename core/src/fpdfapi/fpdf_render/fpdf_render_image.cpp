// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_render.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "../../fxcrt/fx_safe_types.h"
#include "../fpdf_page/pageint.h"
#include "render_int.h"
FX_BOOL CPDF_RenderStatus::ProcessImage(CPDF_ImageObject* pImageObj, const CFX_AffineMatrix* pObj2Device)
{
    CPDF_ImageRenderer render;
    if (render.Start(this, pImageObj, pObj2Device, m_bStdCS, m_curBlend)) {
        render.Continue(NULL);
    }
    return render.m_Result;
}
void CPDF_RenderStatus::CompositeDIBitmap(CFX_DIBitmap* pDIBitmap, int left, int top, FX_ARGB mask_argb,
        int bitmap_alpha, int blend_mode, int Transparency)
{
    if (pDIBitmap == NULL) {
        return;
    }
    FX_BOOL bIsolated = Transparency & PDFTRANS_ISOLATED;
    FX_BOOL bGroup = Transparency & PDFTRANS_GROUP;
    if (blend_mode == FXDIB_BLEND_NORMAL) {
        if (!pDIBitmap->IsAlphaMask()) {
            if (bitmap_alpha < 255) {
                pDIBitmap->MultiplyAlpha(bitmap_alpha);
            }
            if (m_pDevice->SetDIBits(pDIBitmap, left, top)) {
                return;
            }
        } else {
            FX_DWORD fill_argb = m_Options.TranslateColor(mask_argb);
            if (bitmap_alpha < 255) {
                ((FX_BYTE*)&fill_argb)[3] = ((FX_BYTE*)&fill_argb)[3] * bitmap_alpha / 255;
            }
            if (m_pDevice->SetBitMask(pDIBitmap, left, top, fill_argb)) {
                return;
            }
        }
    }
    FX_BOOL bBackAlphaRequired = blend_mode && bIsolated && !m_bDropObjects;
    FX_BOOL bGetBackGround = ((m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT)) ||
                             (!(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT) && (m_pDevice->GetRenderCaps()
                                     & FXRC_GET_BITS) && !bBackAlphaRequired);
    if (bGetBackGround) {
        if (bIsolated || !bGroup) {
            if (pDIBitmap->IsAlphaMask()) {
                return;
            }
            m_pDevice->SetDIBits(pDIBitmap, left, top, blend_mode);
        } else {
            FX_RECT rect(left, top, left + pDIBitmap->GetWidth(), top + pDIBitmap->GetHeight());
            rect.Intersect(m_pDevice->GetClipBox());
            CFX_DIBitmap* pClone = NULL;
            FX_BOOL bClone = FALSE;
            if (m_pDevice->GetBackDrop() && m_pDevice->GetBitmap()) {
                bClone = TRUE;
                pClone = m_pDevice->GetBackDrop()->Clone(&rect);
                CFX_DIBitmap *pForeBitmap = m_pDevice->GetBitmap();
                pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(), pForeBitmap, rect.left, rect.top);
                left = left >= 0 ? 0 : left;
                top = top >= 0 ? 0 : top;
                if (!pDIBitmap->IsAlphaMask())
                    pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(), pDIBitmap,
                                            left, top, blend_mode);
                else
                    pClone->CompositeMask(0, 0, pClone->GetWidth(), pClone->GetHeight(), pDIBitmap,
                                          mask_argb, left, top, blend_mode);
            } else {
                pClone = pDIBitmap;
            }
            if (m_pDevice->GetBackDrop()) {
                m_pDevice->SetDIBits(pClone, rect.left, rect.top);
            } else {
                if (pDIBitmap->IsAlphaMask()) {
                    return;
                }
                m_pDevice->SetDIBits(pDIBitmap, rect.left, rect.top, blend_mode);
            }
            if (bClone) {
                delete pClone;
            }
        }
        return;
    }
    int back_left, back_top;
    FX_RECT rect(left, top, left + pDIBitmap->GetWidth(), top + pDIBitmap->GetHeight());
    CFX_DIBitmap* pBackdrop = GetBackdrop(m_pCurObj, rect, back_left, back_top, blend_mode > FXDIB_BLEND_NORMAL && bIsolated);
    if (!pBackdrop) {
        return;
    }
    if (!pDIBitmap->IsAlphaMask())
        pBackdrop->CompositeBitmap(left - back_left, top - back_top, pDIBitmap->GetWidth(), pDIBitmap->GetHeight(), pDIBitmap,
                                   0, 0, blend_mode);
    else
        pBackdrop->CompositeMask(left - back_left, top - back_top, pDIBitmap->GetWidth(), pDIBitmap->GetHeight(), pDIBitmap,
                                 mask_argb, 0, 0, blend_mode);
    CFX_DIBitmap* pBackdrop1 = new CFX_DIBitmap;
    pBackdrop1->Create(pBackdrop->GetWidth(), pBackdrop->GetHeight(), FXDIB_Rgb32);
    pBackdrop1->Clear((FX_DWORD) - 1);
    pBackdrop1->CompositeBitmap(0, 0, pBackdrop->GetWidth(), pBackdrop->GetHeight(), pBackdrop, 0, 0);
    delete pBackdrop;
    pBackdrop = pBackdrop1;
    m_pDevice->SetDIBits(pBackdrop, back_left, back_top);
    delete pBackdrop;
}
FX_COLORREF CPDF_TransferFunc::TranslateColor(FX_COLORREF rgb)
{
    return FXSYS_RGB(m_Samples[FXSYS_GetRValue(rgb)], m_Samples[256 + FXSYS_GetGValue(rgb)],
                     m_Samples[512 + FXSYS_GetBValue(rgb)]);
}
CFX_DIBSource* CPDF_TransferFunc::TranslateImage(const CFX_DIBSource* pSrc, FX_BOOL bAutoDropSrc)
{
    CPDF_DIBTransferFunc* pDest = new CPDF_DIBTransferFunc(this);
    pDest->LoadSrc(pSrc, bAutoDropSrc);
    return pDest;
}
FXDIB_Format CPDF_DIBTransferFunc::GetDestFormat()
{
    if (m_pSrc->IsAlphaMask()) {
        return FXDIB_8bppMask;
    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    return (m_pSrc->HasAlpha()) ? FXDIB_Argb : FXDIB_Rgb32;
#else
    return (m_pSrc->HasAlpha()) ? FXDIB_Argb : FXDIB_Rgb;
#endif
}
CPDF_DIBTransferFunc::CPDF_DIBTransferFunc(const CPDF_TransferFunc* pTransferFunc)
{
    m_RampR = pTransferFunc->m_Samples;
    m_RampG = &pTransferFunc->m_Samples[256];
    m_RampB = &pTransferFunc->m_Samples[512];
}
void CPDF_DIBTransferFunc::TranslateScanline(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf) const
{
    int i;
    FX_BOOL bSkip = FALSE;
    switch (m_pSrc->GetFormat()) {
        case FXDIB_1bppRgb: {
                int r0 = m_RampR[0], g0 = m_RampG[0], b0 = m_RampB[0];
                int r1 = m_RampR[255], g1 = m_RampG[255], b1 = m_RampB[255];
                for (i = 0; i < m_Width; i ++) {
                    if (src_buf[i / 8] & (1 << (7 - i % 8))) {
                        *dest_buf++ = b1;
                        *dest_buf++ = g1;
                        *dest_buf++ = r1;
                    } else {
                        *dest_buf++ = b0;
                        *dest_buf++ = g0;
                        *dest_buf++ = r0;
                    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                    dest_buf++;
#endif
                }
                break;
            }
        case FXDIB_1bppMask: {
                int m0 = m_RampR[0], m1 = m_RampR[255];
                for (i = 0; i < m_Width; i ++) {
                    if (src_buf[i / 8] & (1 << (7 - i % 8))) {
                        *dest_buf++ = m1;
                    } else {
                        *dest_buf++ = m0;
                    }
                }
                break;
            }
        case FXDIB_8bppRgb: {
                FX_ARGB* pPal = m_pSrc->GetPalette();
                for (i = 0; i < m_Width; i ++) {
                    if (pPal) {
                        FX_ARGB src_argb = pPal[*src_buf];
                        *dest_buf++ = m_RampB[FXARGB_R(src_argb)];
                        *dest_buf++ = m_RampG[FXARGB_G(src_argb)];
                        *dest_buf++ = m_RampR[FXARGB_B(src_argb)];
                    } else {
                        FX_DWORD src_byte = *src_buf;
                        *dest_buf++ = m_RampB[src_byte];
                        *dest_buf++ = m_RampG[src_byte];
                        *dest_buf++ = m_RampR[src_byte];
                    }
                    src_buf ++;
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                    dest_buf++;
#endif
                }
                break;
            }
        case FXDIB_8bppMask:
            for (i = 0; i < m_Width; i ++) {
                *dest_buf++ = m_RampR[*(src_buf++)];
            }
            break;
        case FXDIB_Rgb:
            for (i = 0; i < m_Width; i ++) {
                *dest_buf++ = m_RampB[*(src_buf++)];
                *dest_buf++ = m_RampG[*(src_buf++)];
                *dest_buf++ = m_RampR[*(src_buf++)];
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                dest_buf++;
#endif
            }
            break;
        case FXDIB_Rgb32:
            bSkip = TRUE;
        case FXDIB_Argb:
            for (i = 0; i < m_Width; i ++) {
                *dest_buf++ = m_RampB[*(src_buf++)];
                *dest_buf++ = m_RampG[*(src_buf++)];
                *dest_buf++ = m_RampR[*(src_buf++)];
                if (!bSkip) {
                    *dest_buf++ = *src_buf;
                }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                else {
                    dest_buf++;
                }
#endif
                src_buf ++;
            }
            break;
        default:
            break;
    }
}
void CPDF_DIBTransferFunc::TranslateDownSamples(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf, int pixels, int Bpp) const
{
    if (Bpp == 8) {
        for (int i = 0; i < pixels; i ++) {
            *dest_buf++ = m_RampR[*(src_buf++)];
        }
    } else if (Bpp == 24) {
        for (int i = 0; i < pixels; i ++) {
            *dest_buf++ = m_RampB[*(src_buf++)];
            *dest_buf++ = m_RampG[*(src_buf++)];
            *dest_buf++ = m_RampR[*(src_buf++)];
        }
    } else {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
        if (!m_pSrc->HasAlpha()) {
            for (int i = 0; i < pixels; i ++) {
                *dest_buf++ = m_RampB[*(src_buf++)];
                *dest_buf++ = m_RampG[*(src_buf++)];
                *dest_buf++ = m_RampR[*(src_buf++)];
                dest_buf++;
                src_buf++;
            }
        } else
#endif
            for (int i = 0; i < pixels; i ++) {
                *dest_buf++ = m_RampB[*(src_buf++)];
                *dest_buf++ = m_RampG[*(src_buf++)];
                *dest_buf++ = m_RampR[*(src_buf++)];
                *dest_buf++ = *(src_buf++);
            }
    }
}
static FX_BOOL _IsSupported(CPDF_ColorSpace* pCS)
{
    if (pCS->GetFamily() == PDFCS_DEVICERGB || pCS->GetFamily() == PDFCS_DEVICEGRAY ||
            pCS->GetFamily() == PDFCS_DEVICECMYK || pCS->GetFamily() == PDFCS_CALGRAY ||
            pCS->GetFamily() == PDFCS_CALRGB) {
        return TRUE;
    }
    if (pCS->GetFamily() == PDFCS_INDEXED && _IsSupported(pCS->GetBaseCS())) {
        return TRUE;
    }
    return FALSE;
}
CPDF_ImageRenderer::CPDF_ImageRenderer()
{
    m_pRenderStatus = NULL;
    m_pImageObject = NULL;
    m_Result = TRUE;
    m_Status = 0;
    m_pQuickStretcher = NULL;
    m_pTransformer = NULL;
    m_DeviceHandle = NULL;
    m_LoadHandle = NULL;
    m_pClone = NULL;
    m_bStdCS = FALSE;
    m_bPatternColor = FALSE;
    m_BlendType = FXDIB_BLEND_NORMAL;
    m_pPattern = NULL;
    m_pObj2Device = NULL;
}
CPDF_ImageRenderer::~CPDF_ImageRenderer()
{
    if (m_pQuickStretcher) {
        delete m_pQuickStretcher;
    }
    if (m_pTransformer) {
        delete m_pTransformer;
    }
    if (m_DeviceHandle) {
        m_pRenderStatus->m_pDevice->CancelDIBits(m_DeviceHandle);
    }
    if (m_LoadHandle) {
        delete (CPDF_ProgressiveImageLoaderHandle*)m_LoadHandle;
    }
    if (m_pClone) {
        delete m_pClone;
    }
}
FX_BOOL CPDF_ImageRenderer::StartLoadDIBSource()
{
    CFX_FloatRect image_rect_f = m_ImageMatrix.GetUnitRect();
    FX_RECT image_rect = image_rect_f.GetOutterRect();
    int dest_width = image_rect.Width();
    int dest_height = image_rect.Height();
    if (m_ImageMatrix.a < 0) {
        dest_width = -dest_width;
    }
    if (m_ImageMatrix.d > 0) {
        dest_height = -dest_height;
    }
    if (m_Loader.StartLoadImage(m_pImageObject, m_pRenderStatus->m_pContext->m_pPageCache, m_LoadHandle, m_bStdCS,
                                m_pRenderStatus->m_GroupFamily, m_pRenderStatus->m_bLoadMask, m_pRenderStatus, dest_width, dest_height)) {
        if (m_LoadHandle != NULL) {
            m_Status = 4;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}
FX_BOOL CPDF_ImageRenderer::StartRenderDIBSource()
{
    if (m_Loader.m_pBitmap == NULL) {
        return FALSE;
    }
    m_BitmapAlpha = 255;
    const CPDF_GeneralStateData* pGeneralState = m_pImageObject->m_GeneralState;
    if (pGeneralState) {
        m_BitmapAlpha = FXSYS_round(pGeneralState->m_FillAlpha * 255);
    }
    m_pDIBSource = m_Loader.m_pBitmap;
    if (m_pRenderStatus->m_Options.m_ColorMode == RENDER_COLOR_ALPHA && m_Loader.m_pMask == NULL) {
        return StartBitmapAlpha();
    }
    if (pGeneralState && pGeneralState->m_pTR) {
        if (!pGeneralState->m_pTransferFunc) {
            ((CPDF_GeneralStateData*)pGeneralState)->m_pTransferFunc = m_pRenderStatus->GetTransferFunc(pGeneralState->m_pTR);
        }
        if (pGeneralState->m_pTransferFunc && !pGeneralState->m_pTransferFunc->m_bIdentity) {
            m_pDIBSource = m_Loader.m_pBitmap = pGeneralState->m_pTransferFunc->TranslateImage(m_Loader.m_pBitmap, !m_Loader.m_bCached);
            if (m_Loader.m_bCached && m_Loader.m_pMask) {
                m_Loader.m_pMask = m_Loader.m_pMask->Clone();
            }
            m_Loader.m_bCached = FALSE;
        }
    }
    m_FillArgb = 0;
    m_bPatternColor = FALSE;
    m_pPattern = NULL;
    if (m_pDIBSource->IsAlphaMask()) {
        CPDF_Color* pColor = m_pImageObject->m_ColorState.GetFillColor();
        if (pColor && pColor->IsPattern()) {
            m_pPattern = pColor->GetPattern();
            if (m_pPattern != NULL) {
                m_bPatternColor = TRUE;
            }
        }
        m_FillArgb = m_pRenderStatus->GetFillArgb(m_pImageObject);
    } else if (m_pRenderStatus->m_Options.m_ColorMode == RENDER_COLOR_GRAY) {
        m_pClone = m_pDIBSource->Clone();
        m_pClone->ConvertColorScale(m_pRenderStatus->m_Options.m_BackColor, m_pRenderStatus->m_Options.m_ForeColor);
        m_pDIBSource = m_pClone;
    }
    m_Flags = 0;
    if (m_pRenderStatus->m_Options.m_Flags & RENDER_FORCE_DOWNSAMPLE) {
        m_Flags |= RENDER_FORCE_DOWNSAMPLE;
    } else if (m_pRenderStatus->m_Options.m_Flags & RENDER_FORCE_HALFTONE) {
        m_Flags |= RENDER_FORCE_HALFTONE;
    }
    if (m_pRenderStatus->m_pDevice->GetDeviceClass() != FXDC_DISPLAY) {
        CPDF_Object* pFilters = m_pImageObject->m_pImage->GetStream()->GetDict()->GetElementValue(FX_BSTRC("Filter"));
        if (pFilters) {
            if (pFilters->GetType() == PDFOBJ_NAME) {
                CFX_ByteStringC bsDecodeType = pFilters->GetConstString();
                if (bsDecodeType == FX_BSTRC("DCTDecode") || bsDecodeType == FX_BSTRC("JPXDecode")) {
                    m_Flags |= FXRENDER_IMAGE_LOSSY;
                }
            } else if (pFilters->GetType() == PDFOBJ_ARRAY) {
                CPDF_Array* pArray = (CPDF_Array*)pFilters;
                for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
                    CFX_ByteStringC bsDecodeType = pArray->GetConstString(i);
                    if (bsDecodeType == FX_BSTRC("DCTDecode") || bsDecodeType == FX_BSTRC("JPXDecode")) {
                        m_Flags |= FXRENDER_IMAGE_LOSSY;
                        break;
                    }
                }
            }
        }
    }
    if (m_pRenderStatus->m_Options.m_Flags & RENDER_NOIMAGESMOOTH) {
        m_Flags |= FXDIB_NOSMOOTH;
    } else if (m_pImageObject->m_pImage->IsInterpol()) {
        m_Flags |= FXDIB_INTERPOL;
    }
    if (m_Loader.m_pMask) {
        return DrawMaskedImage();
    }
    if (m_bPatternColor) {
        return DrawPatternImage(m_pObj2Device);
    }
    if (m_BitmapAlpha == 255 && pGeneralState && pGeneralState->m_FillOP &&
            pGeneralState->m_OPMode == 0 && pGeneralState->m_BlendType == FXDIB_BLEND_NORMAL && pGeneralState->m_StrokeAlpha == 1 && pGeneralState->m_FillAlpha == 1) {
        CPDF_Document* pDocument = NULL;
        CPDF_Page* pPage = NULL;
        if (m_pRenderStatus->m_pContext->m_pPageCache) {
            pPage = m_pRenderStatus->m_pContext->m_pPageCache->GetPage();
            pDocument = pPage->m_pDocument;
        } else {
            pDocument = m_pImageObject->m_pImage->GetDocument();
        }
        CPDF_Dictionary* pPageResources = pPage ? pPage->m_pPageResources : NULL;
        CPDF_Object* pCSObj = m_pImageObject->m_pImage->GetStream()->GetDict()->GetElementValue(FX_BSTRC("ColorSpace"));
        CPDF_ColorSpace* pColorSpace = pDocument->LoadColorSpace(pCSObj, pPageResources);
        if (pColorSpace) {
            int format = pColorSpace->GetFamily();
            if (format == PDFCS_DEVICECMYK || format == PDFCS_SEPARATION || format == PDFCS_DEVICEN) {
                m_BlendType = FXDIB_BLEND_DARKEN;
            }
            pDocument->GetPageData()->ReleaseColorSpace(pCSObj);
        }
    }
    return StartDIBSource();
}
FX_BOOL CPDF_ImageRenderer::Start(CPDF_RenderStatus* pStatus, const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStdCS, int blendType)
{
    m_pRenderStatus = pStatus;
    m_bStdCS = bStdCS;
    m_pImageObject = (CPDF_ImageObject*)pObj;
    m_BlendType = blendType;
    m_pObj2Device = pObj2Device;
    CPDF_Dictionary* pOC = m_pImageObject->m_pImage->GetOC();
    if (pOC && m_pRenderStatus->m_Options.m_pOCContext && !m_pRenderStatus->m_Options.m_pOCContext->CheckOCGVisible(pOC)) {
        return FALSE;
    }
    m_ImageMatrix = m_pImageObject->m_Matrix;
    m_ImageMatrix.Concat(*pObj2Device);
    if (StartLoadDIBSource()) {
        return TRUE;
    }
    return StartRenderDIBSource();
}
FX_BOOL CPDF_ImageRenderer::Start(CPDF_RenderStatus* pStatus, const CFX_DIBSource* pDIBSource, FX_ARGB bitmap_argb,
                                  int bitmap_alpha, const CFX_AffineMatrix* pImage2Device, FX_DWORD flags, FX_BOOL bStdCS, int blendType)
{
    m_pRenderStatus = pStatus;
    m_pDIBSource = pDIBSource;
    m_FillArgb = bitmap_argb;
    m_BitmapAlpha = bitmap_alpha;
    m_ImageMatrix = *pImage2Device;
    m_Flags = flags;
    m_bStdCS = bStdCS;
    m_BlendType = blendType;
    return StartDIBSource();
}
FX_BOOL	CPDF_ImageRenderer::DrawPatternImage(const CFX_Matrix* pObj2Device)
{
    if (m_pRenderStatus->m_bPrint && !(m_pRenderStatus->m_pDevice->GetRenderCaps() & FXRC_BLEND_MODE)) {
        m_Result = FALSE;
        return FALSE;
    }
    FX_RECT rect = m_ImageMatrix.GetUnitRect().GetOutterRect();
    rect.Intersect(m_pRenderStatus->m_pDevice->GetClipBox());
    if (rect.IsEmpty()) {
        return FALSE;
    }
    CFX_AffineMatrix new_matrix = m_ImageMatrix;
    new_matrix.TranslateI(-rect.left, -rect.top);
    int width = rect.Width();
    int height = rect.Height();
    CFX_FxgeDevice bitmap_device1;
    if (!bitmap_device1.Create(rect.Width(), rect.Height(), FXDIB_Rgb32)) {
        return TRUE;
    }
    bitmap_device1.GetBitmap()->Clear(0xffffff);
    {
        CPDF_RenderStatus bitmap_render;
        bitmap_render.Initialize(m_pRenderStatus->m_pContext, &bitmap_device1, NULL, NULL,
                                 NULL, NULL, &m_pRenderStatus->m_Options, 0, m_pRenderStatus->m_bDropObjects, NULL, TRUE);
        CFX_Matrix patternDevice = *pObj2Device;
        patternDevice.Translate((FX_FLOAT) - rect.left, (FX_FLOAT) - rect.top);
        if(m_pPattern->m_PatternType == PATTERN_TILING) {
            bitmap_render.DrawTilingPattern((CPDF_TilingPattern*)m_pPattern, m_pImageObject, &patternDevice, FALSE);
        } else {
            bitmap_render.DrawShadingPattern((CPDF_ShadingPattern*)m_pPattern, m_pImageObject, &patternDevice, FALSE);
        }
    }
    {
        CFX_FxgeDevice bitmap_device2;
        if (!bitmap_device2.Create(rect.Width(), rect.Height(), FXDIB_8bppRgb)) {
            return TRUE;
        }
        bitmap_device2.GetBitmap()->Clear(0);
        CPDF_RenderStatus bitmap_render;
        bitmap_render.Initialize(m_pRenderStatus->m_pContext, &bitmap_device2, NULL, NULL,
                                 NULL, NULL, NULL, 0, m_pRenderStatus->m_bDropObjects, NULL, TRUE);
        CPDF_ImageRenderer image_render;
        if (image_render.Start(&bitmap_render, m_pDIBSource, 0xffffffff, 255, &new_matrix, m_Flags, TRUE)) {
            image_render.Continue(NULL);
        }
        if (m_Loader.m_MatteColor != 0xffffffff) {
            int matte_r = FXARGB_R(m_Loader.m_MatteColor);
            int matte_g = FXARGB_G(m_Loader.m_MatteColor);
            int matte_b = FXARGB_B(m_Loader.m_MatteColor);
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = (FX_LPBYTE)bitmap_device1.GetBitmap()->GetScanline(row);
                FX_LPCBYTE mask_scan = bitmap_device2.GetBitmap()->GetScanline(row);
                for (int col = 0; col < width; col ++) {
                    int alpha = *mask_scan ++;
                    if (alpha) {
                        int orig = (*dest_scan - matte_b) * 255 / alpha + matte_b;
                        if (orig < 0) {
                            orig = 0;
                        } else if (orig > 255) {
                            orig = 255;
                        }
                        *dest_scan++ = orig;
                        orig = (*dest_scan - matte_g) * 255 / alpha + matte_g;
                        if (orig < 0) {
                            orig = 0;
                        } else if (orig > 255) {
                            orig = 255;
                        }
                        *dest_scan++ = orig;
                        orig = (*dest_scan - matte_r) * 255 / alpha + matte_r;
                        if (orig < 0) {
                            orig = 0;
                        } else if (orig > 255) {
                            orig = 255;
                        }
                        *dest_scan++ = orig;
                        dest_scan ++;
                    } else {
                        dest_scan += 4;
                    }
                }
            }
        }
        bitmap_device2.GetBitmap()->ConvertFormat(FXDIB_8bppMask);
        bitmap_device1.GetBitmap()->MultiplyAlpha(bitmap_device2.GetBitmap());
        bitmap_device1.GetBitmap()->MultiplyAlpha(255);
    }
    m_pRenderStatus->m_pDevice->SetDIBits(bitmap_device1.GetBitmap(), rect.left, rect.top, m_BlendType);
    return FALSE;
}
FX_BOOL CPDF_ImageRenderer::DrawMaskedImage()
{
    if (m_pRenderStatus->m_bPrint && !(m_pRenderStatus->m_pDevice->GetRenderCaps() & FXRC_BLEND_MODE)) {
        m_Result = FALSE;
        return FALSE;
    }
    FX_RECT rect = m_ImageMatrix.GetUnitRect().GetOutterRect();
    rect.Intersect(m_pRenderStatus->m_pDevice->GetClipBox());
    if (rect.IsEmpty()) {
        return FALSE;
    }
    CFX_AffineMatrix new_matrix = m_ImageMatrix;
    new_matrix.TranslateI(-rect.left, -rect.top);
    int width = rect.Width();
    int height = rect.Height();
    CFX_FxgeDevice bitmap_device1;
    if (!bitmap_device1.Create(width, height, FXDIB_Rgb32)) {
        return TRUE;
    }
    bitmap_device1.GetBitmap()->Clear(0xffffff);
    {
        CPDF_RenderStatus bitmap_render;
        bitmap_render.Initialize(m_pRenderStatus->m_pContext, &bitmap_device1, NULL, NULL,
                                 NULL, NULL, NULL, 0, m_pRenderStatus->m_bDropObjects, NULL, TRUE);
        CPDF_ImageRenderer image_render;
        if (image_render.Start(&bitmap_render, m_pDIBSource, 0, 255, &new_matrix, m_Flags, TRUE)) {
            image_render.Continue(NULL);
        }
    }
    {
        CFX_FxgeDevice bitmap_device2;
        if (!bitmap_device2.Create(width, height, FXDIB_8bppRgb)) {
            return TRUE;
        }
        bitmap_device2.GetBitmap()->Clear(0);
        CPDF_RenderStatus bitmap_render;
        bitmap_render.Initialize(m_pRenderStatus->m_pContext, &bitmap_device2, NULL, NULL,
                                 NULL, NULL, NULL, 0, m_pRenderStatus->m_bDropObjects, NULL, TRUE);
        CPDF_ImageRenderer image_render;
        if (image_render.Start(&bitmap_render, m_Loader.m_pMask, 0xffffffff, 255, &new_matrix, m_Flags, TRUE)) {
            image_render.Continue(NULL);
        }
        if (m_Loader.m_MatteColor != 0xffffffff) {
            int matte_r = FXARGB_R(m_Loader.m_MatteColor);
            int matte_g = FXARGB_G(m_Loader.m_MatteColor);
            int matte_b = FXARGB_B(m_Loader.m_MatteColor);
            for (int row = 0; row < height; row ++) {
                FX_LPBYTE dest_scan = (FX_LPBYTE)bitmap_device1.GetBitmap()->GetScanline(row);
                FX_LPCBYTE mask_scan = bitmap_device2.GetBitmap()->GetScanline(row);
                for (int col = 0; col < width; col ++) {
                    int alpha = *mask_scan ++;
                    if (alpha) {
                        int orig = (*dest_scan - matte_b) * 255 / alpha + matte_b;
                        if (orig < 0) {
                            orig = 0;
                        } else if (orig > 255) {
                            orig = 255;
                        }
                        *dest_scan++ = orig;
                        orig = (*dest_scan - matte_g) * 255 / alpha + matte_g;
                        if (orig < 0) {
                            orig = 0;
                        } else if (orig > 255) {
                            orig = 255;
                        }
                        *dest_scan++ = orig;
                        orig = (*dest_scan - matte_r) * 255 / alpha + matte_r;
                        if (orig < 0) {
                            orig = 0;
                        } else if (orig > 255) {
                            orig = 255;
                        }
                        *dest_scan++ = orig;
                        dest_scan ++;
                    } else {
                        dest_scan += 4;
                    }
                }
            }
        }
        bitmap_device2.GetBitmap()->ConvertFormat(FXDIB_8bppMask);
        bitmap_device1.GetBitmap()->MultiplyAlpha(bitmap_device2.GetBitmap());
        if (m_BitmapAlpha < 255) {
            bitmap_device1.GetBitmap()->MultiplyAlpha(m_BitmapAlpha);
        }
    }
    m_pRenderStatus->m_pDevice->SetDIBits(bitmap_device1.GetBitmap(), rect.left, rect.top, m_BlendType);
    return FALSE;
}
FX_BOOL CPDF_ImageRenderer::StartDIBSource()
{
    if (!(m_Flags & RENDER_FORCE_DOWNSAMPLE) && m_pDIBSource->GetBPP() > 1) {
        int image_size = m_pDIBSource->GetBPP() / 8 * m_pDIBSource->GetWidth() * m_pDIBSource->GetHeight();
        if (image_size > FPDF_HUGE_IMAGE_SIZE && !(m_Flags & RENDER_FORCE_HALFTONE)) {
            m_Flags |= RENDER_FORCE_DOWNSAMPLE;
        }
    }
    if (m_pRenderStatus->m_pDevice->StartDIBits(m_pDIBSource, m_BitmapAlpha, m_FillArgb,
            &m_ImageMatrix, m_Flags, m_DeviceHandle, 0, NULL, m_BlendType)) {
        if (m_DeviceHandle != NULL) {
            m_Status = 3;
            return TRUE;
        }
        return FALSE;
    }
    CFX_FloatRect image_rect_f = m_ImageMatrix.GetUnitRect();
    FX_RECT image_rect = image_rect_f.GetOutterRect();
    int dest_width = image_rect.Width();
    int dest_height = image_rect.Height();
    if ((FXSYS_fabs(m_ImageMatrix.b) >= 0.5f || m_ImageMatrix.a == 0) ||
            (FXSYS_fabs(m_ImageMatrix.c) >= 0.5f || m_ImageMatrix.d == 0) ) {
        if (m_pRenderStatus->m_bPrint && !(m_pRenderStatus->m_pDevice->GetRenderCaps() & FXRC_BLEND_MODE)) {
            m_Result = FALSE;
            return FALSE;
        }
        FX_RECT clip_box = m_pRenderStatus->m_pDevice->GetClipBox();
        clip_box.Intersect(image_rect);
        m_Status = 2;
        m_pTransformer = new CFX_ImageTransformer;
        m_pTransformer->Start(m_pDIBSource, &m_ImageMatrix, m_Flags, &clip_box);
        return TRUE;
    }
    if (m_ImageMatrix.a < 0) {
        dest_width = -dest_width;
    }
    if (m_ImageMatrix.d > 0) {
        dest_height = -dest_height;
    }
    int dest_left, dest_top;
    dest_left = dest_width > 0 ? image_rect.left : image_rect.right;
    dest_top = dest_height > 0 ? image_rect.top : image_rect.bottom;
    if (m_pDIBSource->IsOpaqueImage() && m_BitmapAlpha == 255) {
        if (m_pRenderStatus->m_pDevice->StretchDIBits(m_pDIBSource, dest_left, dest_top,
                dest_width, dest_height, m_Flags, NULL, m_BlendType)) {
            return FALSE;
        }
    }
    if (m_pDIBSource->IsAlphaMask()) {
        if (m_BitmapAlpha != 255) {
            m_FillArgb = FXARGB_MUL_ALPHA(m_FillArgb, m_BitmapAlpha);
        }
        if (m_pRenderStatus->m_pDevice->StretchBitMask(m_pDIBSource, dest_left, dest_top, dest_width, dest_height, m_FillArgb, m_Flags)) {
            return FALSE;
        }
    }
    if (m_pRenderStatus->m_bPrint && !(m_pRenderStatus->m_pDevice->GetRenderCaps() & FXRC_BLEND_MODE)) {
        m_Result = FALSE;
        return TRUE;
    }
    FX_RECT clip_box = m_pRenderStatus->m_pDevice->GetClipBox();
    FX_RECT dest_rect = clip_box;
    dest_rect.Intersect(image_rect);
    FX_RECT dest_clip(dest_rect.left - image_rect.left, dest_rect.top - image_rect.top,
                      dest_rect.right - image_rect.left, dest_rect.bottom - image_rect.top);
    CFX_DIBitmap* pStretched = m_pDIBSource->StretchTo(dest_width, dest_height, m_Flags, &dest_clip);
    if (pStretched) {
        m_pRenderStatus->CompositeDIBitmap(pStretched, dest_rect.left, dest_rect.top, m_FillArgb,
                                           m_BitmapAlpha, m_BlendType, FALSE);
        delete pStretched;
        pStretched = NULL;
    }
    return FALSE;
}
FX_BOOL CPDF_ImageRenderer::StartBitmapAlpha()
{
    if (m_pDIBSource->IsOpaqueImage()) {
        CFX_PathData path;
        path.AppendRect(0, 0, 1, 1);
        path.Transform(&m_ImageMatrix);
        FX_DWORD fill_color = ArgbEncode(0xff, m_BitmapAlpha, m_BitmapAlpha, m_BitmapAlpha);
        m_pRenderStatus->m_pDevice->DrawPath(&path, NULL, NULL, fill_color, 0, FXFILL_WINDING);
    } else {
        const CFX_DIBSource* pAlphaMask = m_pDIBSource->IsAlphaMask() ? m_pDIBSource : m_pDIBSource->GetAlphaMask();
        if (FXSYS_fabs(m_ImageMatrix.b) >= 0.5f || FXSYS_fabs(m_ImageMatrix.c) >= 0.5f) {
            int left, top;
            CFX_DIBitmap* pTransformed = pAlphaMask->TransformTo(&m_ImageMatrix, left, top);
            if (pTransformed == NULL) {
                return TRUE;
            }
            m_pRenderStatus->m_pDevice->SetBitMask(pTransformed, left, top, ArgbEncode(0xff, m_BitmapAlpha, m_BitmapAlpha, m_BitmapAlpha));
            delete pTransformed;
        } else {
            CFX_FloatRect image_rect_f = m_ImageMatrix.GetUnitRect();
            FX_RECT image_rect = image_rect_f.GetOutterRect();
            int dest_width = m_ImageMatrix.a > 0 ? image_rect.Width() : -image_rect.Width();
            int dest_height = m_ImageMatrix.d > 0 ? -image_rect.Height() : image_rect.Height();
            int left = dest_width > 0 ? image_rect.left : image_rect.right;
            int top = dest_height > 0 ? image_rect.top : image_rect.bottom;
            m_pRenderStatus->m_pDevice->StretchBitMask(pAlphaMask, left, top, dest_width, dest_height,
                    ArgbEncode(0xff, m_BitmapAlpha, m_BitmapAlpha, m_BitmapAlpha));
        }
        if (m_pDIBSource != pAlphaMask) {
            delete pAlphaMask;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_ImageRenderer::Continue(IFX_Pause* pPause)
{
    if (m_Status == 1) {
        if (m_pQuickStretcher->Continue(pPause)) {
            return TRUE;
        }
        if (m_pQuickStretcher->m_pBitmap->IsAlphaMask())
            m_pRenderStatus->m_pDevice->SetBitMask(m_pQuickStretcher->m_pBitmap, m_pQuickStretcher->m_ResultLeft,
                                                   m_pQuickStretcher->m_ResultTop, m_FillArgb);
        else
            m_pRenderStatus->m_pDevice->SetDIBits(m_pQuickStretcher->m_pBitmap, m_pQuickStretcher->m_ResultLeft,
                                                  m_pQuickStretcher->m_ResultTop, m_BlendType);
        return FALSE;
    } else if (m_Status == 2) {
        if (m_pTransformer->Continue(pPause)) {
            return TRUE;
        }
        CFX_DIBitmap* pBitmap = m_pTransformer->m_Storer.Detach();
        if (pBitmap == NULL) {
            return FALSE;
        }
        if (pBitmap->IsAlphaMask()) {
            if (m_BitmapAlpha != 255) {
                m_FillArgb = FXARGB_MUL_ALPHA(m_FillArgb, m_BitmapAlpha);
            }
            m_Result = m_pRenderStatus->m_pDevice->SetBitMask(pBitmap,
                       m_pTransformer->m_ResultLeft, m_pTransformer->m_ResultTop, m_FillArgb);
        } else {
            if (m_BitmapAlpha != 255) {
                pBitmap->MultiplyAlpha(m_BitmapAlpha);
            }
            m_Result = m_pRenderStatus->m_pDevice->SetDIBits(pBitmap,
                       m_pTransformer->m_ResultLeft, m_pTransformer->m_ResultTop, m_BlendType);
        }
        delete pBitmap;
        return FALSE;
    } else if (m_Status == 3) {
        return m_pRenderStatus->m_pDevice->ContinueDIBits(m_DeviceHandle, pPause);
    } else if (m_Status == 4) {
        if (m_Loader.Continue(m_LoadHandle, pPause)) {
            return TRUE;
        }
        if (StartRenderDIBSource()) {
            return Continue(pPause);
        }
        return FALSE;
    }
    return FALSE;
}
CPDF_QuickStretcher::CPDF_QuickStretcher()
{
    m_pBitmap = NULL;
    m_pDecoder = NULL;
    m_pCS = NULL;
}
CPDF_QuickStretcher::~CPDF_QuickStretcher()
{
    if (m_pBitmap) {
        delete m_pBitmap;
    }
    if (m_pCS) {
        m_pCS->ReleaseCS();
    }
    if (m_pDecoder) {
        delete m_pDecoder;
    }
}
ICodec_ScanlineDecoder* FPDFAPI_CreateFlateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        int nComps, int bpc, const CPDF_Dictionary* pParams);
FX_BOOL CPDF_QuickStretcher::Start(CPDF_ImageObject* pImageObj, CFX_AffineMatrix* pImage2Device, const FX_RECT* pClipBox)
{
    if (FXSYS_fabs(pImage2Device->a) < FXSYS_fabs(pImage2Device->b) * 10 && FXSYS_fabs(pImage2Device->d) < FXSYS_fabs(pImage2Device->c) * 10) {
        return FALSE;
    }
    CFX_FloatRect image_rect_f = pImage2Device->GetUnitRect();
    FX_RECT image_rect = image_rect_f.GetOutterRect();
    m_DestWidth = image_rect.Width();
    m_DestHeight = image_rect.Height();
    m_bFlipX = pImage2Device->a < 0;
    m_bFlipY = pImage2Device->d > 0;
    FX_RECT result_rect = *pClipBox;
    result_rect.Intersect(image_rect);
    if (result_rect.IsEmpty()) {
        return FALSE;
    }
    m_ResultWidth = result_rect.Width();
    m_ResultHeight = result_rect.Height();
    m_ResultLeft = result_rect.left;
    m_ResultTop = result_rect.top;
    m_ClipLeft = result_rect.left - image_rect.left;
    m_ClipTop = result_rect.top - image_rect.top;
    CPDF_Dictionary* pDict = pImageObj->m_pImage->GetDict();
    if (pDict->GetInteger(FX_BSTRC("BitsPerComponent")) != 8) {
        return FALSE;
    }
    if (pDict->KeyExist(FX_BSTRC("SMask")) || pDict->KeyExist(FX_BSTRC("Mask"))) {
        return FALSE;
    }
    m_SrcWidth = pDict->GetInteger(FX_BSTRC("Width"));
    m_SrcHeight = pDict->GetInteger(FX_BSTRC("Height"));
    m_pCS = NULL;
    m_Bpp = 3;
    CPDF_Object* pCSObj = pDict->GetElementValue(FX_BSTRC("ColorSpace"));
    if (pCSObj == NULL) {
        return FALSE;
    }
    m_pCS = CPDF_ColorSpace::Load(pImageObj->m_pImage->GetDocument(), pCSObj);
    if (m_pCS == NULL) {
        return FALSE;
    }
    if (!_IsSupported(m_pCS)) {
        return FALSE;
    }
    m_Bpp = m_pCS->CountComponents();
    if (m_pCS->sRGB()) {
        m_pCS->ReleaseCS();
        m_pCS = NULL;
    }
    CPDF_Stream* pStream = pImageObj->m_pImage->GetStream();
    m_StreamAcc.LoadAllData(pStream, FALSE, m_SrcWidth * m_SrcHeight * m_Bpp, TRUE);
    m_pDecoder = NULL;
    if (!m_StreamAcc.GetImageDecoder().IsEmpty()) {
        if (m_StreamAcc.GetImageDecoder() == FX_BSTRC("DCTDecode")) {
            const CPDF_Dictionary* pParam = m_StreamAcc.GetImageParam();
            m_pDecoder = CPDF_ModuleMgr::Get()->GetJpegModule()->CreateDecoder(
                             m_StreamAcc.GetData(), m_StreamAcc.GetSize(), m_SrcWidth, m_SrcHeight, m_Bpp,
                             pParam ? pParam->GetInteger(FX_BSTRC("ColorTransform"), 1) : 1);
        } else if (m_StreamAcc.GetImageDecoder() == FX_BSTRC("FlateDecode")) {
            m_pDecoder = FPDFAPI_CreateFlateDecoder(
                             m_StreamAcc.GetData(), m_StreamAcc.GetSize(), m_SrcWidth, m_SrcHeight, m_Bpp, 8,
                             m_StreamAcc.GetImageParam());
        } else {
            return FALSE;
        }
        m_pDecoder->DownScale(m_DestWidth, m_DestHeight);
    }
    m_pBitmap = new CFX_DIBitmap;
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    m_pBitmap->Create(m_ResultWidth, m_ResultHeight, FXDIB_Rgb32);
#else
    m_pBitmap->Create(m_ResultWidth, m_ResultHeight, FXDIB_Rgb);
#endif
    m_LineIndex = 0;
    return TRUE;
}
FX_BOOL CPDF_QuickStretcher::Continue(IFX_Pause* pPause)
{
    FX_LPBYTE result_buf = m_pBitmap->GetBuffer();
    int src_width = m_pDecoder ? m_pDecoder->GetWidth() : m_SrcWidth;
    int src_height = m_pDecoder ? m_pDecoder->GetHeight() : m_SrcHeight;
    int src_pitch = src_width * m_Bpp;
    while (m_LineIndex < m_ResultHeight) {
        int dest_y, src_y;
        if (m_bFlipY) {
            dest_y = m_ResultHeight - m_LineIndex - 1;
            src_y = (m_DestHeight - (dest_y + m_ClipTop) - 1) * src_height / m_DestHeight;
        } else {
            dest_y = m_LineIndex;
            src_y = (dest_y + m_ClipTop) * src_height / m_DestHeight;
        }
        FX_LPCBYTE src_scan;
        if (m_pDecoder) {
            src_scan = m_pDecoder->GetScanline(src_y);
            if (src_scan == NULL) {
                break;
            }
        } else {
            src_scan = m_StreamAcc.GetData();
            if (src_scan == NULL) {
                break;
            }
            src_scan += src_y * src_pitch;
        }
        FX_LPBYTE result_scan = result_buf + dest_y * m_pBitmap->GetPitch();
        for (int x = 0; x < m_ResultWidth; x ++) {
            int dest_x = m_ClipLeft + x;
            int src_x = (m_bFlipX ? (m_DestWidth - dest_x - 1) : dest_x) * src_width / m_DestWidth;
            FX_LPCBYTE src_pixel = src_scan + src_x * m_Bpp;
            if (m_pCS == NULL) {
                *result_scan = src_pixel[2];
                result_scan ++;
                *result_scan = src_pixel[1];
                result_scan ++;
                *result_scan = src_pixel[0];
                result_scan ++;
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                result_scan ++;
#endif
            } else {
                m_pCS->TranslateImageLine(result_scan, src_pixel, 1, 0, 0);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                result_scan += 4;
#else
                result_scan += 3;
#endif
            }
        }
        m_LineIndex ++;
        if (pPause && pPause->NeedToPauseNow()) {
            return TRUE;
        }
    }
    return FALSE;
}
CFX_DIBitmap* CPDF_RenderStatus::LoadSMask(CPDF_Dictionary* pSMaskDict,
        FX_RECT* pClipRect, const CFX_AffineMatrix* pMatrix)
{
    if (pSMaskDict == NULL) {
        return NULL;
    }
    CFX_DIBitmap* pMask = NULL;
    int width = pClipRect->right - pClipRect->left;
    int height = pClipRect->bottom - pClipRect->top;
    FX_BOOL bLuminosity = FALSE;
    bLuminosity = pSMaskDict->GetConstString(FX_BSTRC("S")) != FX_BSTRC("Alpha");
    CPDF_Stream* pGroup = pSMaskDict->GetStream(FX_BSTRC("G"));
    if (pGroup == NULL) {
        return NULL;
    }
    CPDF_Function* pFunc = NULL;
    CPDF_Object* pFuncObj = pSMaskDict->GetElementValue(FX_BSTRC("TR"));
    if (pFuncObj && (pFuncObj->GetType() == PDFOBJ_DICTIONARY || pFuncObj->GetType() == PDFOBJ_STREAM)) {
        pFunc = CPDF_Function::Load(pFuncObj);
    }
    CFX_AffineMatrix matrix = *pMatrix;
    matrix.TranslateI(-pClipRect->left, -pClipRect->top);
    CPDF_Form form(m_pContext->m_pDocument, m_pContext->m_pPageResources, pGroup);
    form.ParseContent(NULL, NULL, NULL, NULL);
    CFX_FxgeDevice bitmap_device;
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    if (!bitmap_device.Create(width, height, bLuminosity ? FXDIB_Rgb32 : FXDIB_8bppMask)) {
        return NULL;
    }
#else
    if (!bitmap_device.Create(width, height, bLuminosity ? FXDIB_Rgb : FXDIB_8bppMask)) {
        return NULL;
    }
#endif
    CFX_DIBitmap& bitmap = *bitmap_device.GetBitmap();
    CPDF_Object* pCSObj = NULL;
    CPDF_ColorSpace* pCS = NULL;
    if (bLuminosity) {
        CPDF_Array* pBC = pSMaskDict->GetArray(FX_BSTRC("BC"));
        FX_ARGB back_color = 0xff000000;
        if (pBC) {
            CPDF_Dictionary* pDict = pGroup->GetDict();
            if (pDict && pDict->GetDict(FX_BSTRC("Group")))
                pCSObj = pDict->GetDict(FX_BSTRC("Group"))->GetElementValue(FX_BSTRC("CS"));
            else
                pCSObj = NULL;
            pCS = m_pContext->m_pDocument->LoadColorSpace(pCSObj);
            if (pCS) {
                FX_FLOAT R, G, B;
                FX_DWORD comps = 8;
                if (pCS->CountComponents() > static_cast<FX_INT32>(comps)) {
                    comps = (FX_DWORD)pCS->CountComponents();
                }
                CFX_FixedBufGrow<FX_FLOAT, 8> float_array(comps);
                FX_FLOAT* pFloats = float_array;
                FX_SAFE_DWORD num_floats = comps;
                num_floats *= sizeof(FX_FLOAT);
                if (!num_floats.IsValid()) {
                    return NULL;  
                } 
                FXSYS_memset32(pFloats, 0, num_floats.ValueOrDie());
                int count = pBC->GetCount() > 8 ? 8 : pBC->GetCount();
                for (int i = 0; i < count; i ++) {
                    pFloats[i] = pBC->GetNumber(i);
                }
                pCS->GetRGB(pFloats, R, G, B);
                back_color = 0xff000000 | ((FX_INT32)(R * 255) << 16) | ((FX_INT32)(G * 255) << 8) | (FX_INT32)(B * 255);
                m_pContext->m_pDocument->GetPageData()->ReleaseColorSpace(pCSObj);
            }
        }
        bitmap.Clear(back_color);
    } else {
        bitmap.Clear(0);
    }
    CPDF_Dictionary* pFormResource = NULL;
    if (form.m_pFormDict) {
        pFormResource = form.m_pFormDict->GetDict(FX_BSTRC("Resources"));
    }
    CPDF_RenderOptions options;
    options.m_ColorMode = bLuminosity ? RENDER_COLOR_NORMAL : RENDER_COLOR_ALPHA;
    CPDF_RenderStatus status;
    status.Initialize(m_pContext, &bitmap_device, NULL, NULL, NULL, NULL,
                      &options, 0, m_bDropObjects, pFormResource, TRUE, NULL, 0, pCS ? pCS->GetFamily() : 0, bLuminosity);
    status.RenderObjectList(&form, &matrix);
    pMask = new CFX_DIBitmap;
    if (!pMask->Create(width, height, FXDIB_8bppMask)) {
        delete pMask;
        return NULL;
    }
    FX_LPBYTE dest_buf = pMask->GetBuffer();
    int dest_pitch = pMask->GetPitch();
    FX_LPBYTE src_buf = bitmap.GetBuffer();
    int src_pitch = bitmap.GetPitch();
    FX_LPBYTE pTransfer = FX_Alloc(FX_BYTE, 256);
    if (pFunc) {
        CFX_FixedBufGrow<FX_FLOAT, 16> results(pFunc->CountOutputs());
        for (int i = 0; i < 256; i ++) {
            FX_FLOAT input = (FX_FLOAT)i / 255.0f;
            int nresult;
            pFunc->Call(&input, 1, results, nresult);
            pTransfer[i] = FXSYS_round(results[0] * 255);
        }
    } else {
        for (int i = 0; i < 256; i ++) {
            pTransfer[i] = i;
        }
    }
    if (bLuminosity) {
        int Bpp = bitmap.GetBPP() / 8;
        for (int row = 0; row < height; row ++) {
            FX_LPBYTE dest_pos = dest_buf + row * dest_pitch;
            FX_LPBYTE src_pos = src_buf + row * src_pitch;
            for (int col = 0; col < width; col ++) {
                *dest_pos ++ = pTransfer[FXRGB2GRAY(src_pos[2], src_pos[1], *src_pos)];
                src_pos += Bpp;
            }
        }
    } else if (pFunc) {
        int size = dest_pitch * height;
        for (int i = 0; i < size; i ++) {
            dest_buf[i] = pTransfer[src_buf[i]];
        }
    } else {
        FXSYS_memcpy32(dest_buf, src_buf, dest_pitch * height);
    }
    if (pFunc) {
        delete pFunc;
    }
    FX_Free(pTransfer);
    return pMask;
}
