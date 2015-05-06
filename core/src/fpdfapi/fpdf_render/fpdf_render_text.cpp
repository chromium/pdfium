// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fpdfapi/fpdf_render.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "../fpdf_page/pageint.h"
#include "render_int.h"
extern FX_BOOL IsAvailableMatrix(const CFX_AffineMatrix& matrix);
CPDF_Type3Cache::~CPDF_Type3Cache()
{
    FX_POSITION pos = m_SizeMap.GetStartPosition();
    CFX_ByteString Key;
    CPDF_Type3Glyphs* pSizeCache = NULL;
    while(pos) {
        pSizeCache = (CPDF_Type3Glyphs*)m_SizeMap.GetNextValue(pos);
        delete pSizeCache;
    }
    m_SizeMap.RemoveAll();
}
CFX_GlyphBitmap* CPDF_Type3Cache::LoadGlyph(FX_DWORD charcode, const CFX_AffineMatrix* pMatrix, FX_FLOAT retinaScaleX, FX_FLOAT retinaScaleY)
{
    _CPDF_UniqueKeyGen keygen;
    keygen.Generate(4, FXSYS_round(pMatrix->a * 10000), FXSYS_round(pMatrix->b * 10000),
                    FXSYS_round(pMatrix->c * 10000), FXSYS_round(pMatrix->d * 10000));
    CFX_ByteStringC FaceGlyphsKey(keygen.m_Key, keygen.m_KeyLen);
    CPDF_Type3Glyphs* pSizeCache = NULL;
    if(!m_SizeMap.Lookup(FaceGlyphsKey, (void*&)pSizeCache)) {
        pSizeCache = new CPDF_Type3Glyphs;
        m_SizeMap.SetAt(FaceGlyphsKey, pSizeCache);
    }
    CFX_GlyphBitmap* pGlyphBitmap;
    if(pSizeCache->m_GlyphMap.Lookup((FX_LPVOID)(FX_UINTPTR)charcode, (void*&)pGlyphBitmap)) {
        return pGlyphBitmap;
    }
    pGlyphBitmap = RenderGlyph(pSizeCache, charcode, pMatrix, retinaScaleX, retinaScaleY);
    pSizeCache->m_GlyphMap.SetAt((FX_LPVOID)(FX_UINTPTR)charcode, pGlyphBitmap);
    return pGlyphBitmap;
}
CPDF_Type3Glyphs::~CPDF_Type3Glyphs()
{
    FX_POSITION pos = m_GlyphMap.GetStartPosition();
    FX_LPVOID Key;
    CFX_GlyphBitmap* pGlyphBitmap;
    while(pos) {
        m_GlyphMap.GetNextAssoc(pos, Key, (void*&)pGlyphBitmap);
        delete pGlyphBitmap;
    }
}
static int _AdjustBlue(FX_FLOAT pos, int& count, int blues[])
{
    FX_FLOAT min_distance = 1000000.0f * 1.0f;
    int closest_pos = -1;
    for (int i = 0; i < count; i ++) {
        FX_FLOAT distance = (FX_FLOAT)FXSYS_fabs(pos - (FX_FLOAT)blues[i]);
        if (distance < 1.0f * 80.0f / 100.0f && distance < min_distance) {
            min_distance = distance;
            closest_pos = i;
        }
    }
    if (closest_pos >= 0) {
        return blues[closest_pos];
    }
    int new_pos = FXSYS_round(pos);
    if (count == TYPE3_MAX_BLUES) {
        return new_pos;
    }
    blues[count++] = new_pos;
    return new_pos;
}
void CPDF_Type3Glyphs::AdjustBlue(FX_FLOAT top, FX_FLOAT bottom, int& top_line, int& bottom_line)
{
    top_line = _AdjustBlue(top, m_TopBlueCount, m_TopBlue);
    bottom_line = _AdjustBlue(bottom, m_BottomBlueCount, m_BottomBlue);
}
static FX_BOOL _IsScanLine1bpp(FX_LPBYTE pBuf, int width)
{
    int size = width / 8;
    for (int i = 0; i < size; i ++)
        if (pBuf[i]) {
            return TRUE;
        }
    if (width % 8)
        if (pBuf[width / 8] & (0xff << (8 - width % 8))) {
            return TRUE;
        }
    return FALSE;
}
static FX_BOOL _IsScanLine8bpp(FX_LPBYTE pBuf, int width)
{
    for (int i = 0; i < width; i ++)
        if (pBuf[i] > 0x40) {
            return TRUE;
        }
    return FALSE;
}
static int _DetectFirstLastScan(const CFX_DIBitmap* pBitmap, FX_BOOL bFirst)
{
    int height = pBitmap->GetHeight(), pitch = pBitmap->GetPitch(), width = pBitmap->GetWidth();
    int bpp = pBitmap->GetBPP();
    if (bpp > 8) {
        width *= bpp / 8;
    }
    FX_LPBYTE pBuf = pBitmap->GetBuffer();
    int line = bFirst ? 0 : height - 1;
    int line_step = bFirst ? 1 : -1;
    int line_end = bFirst ? height : -1;
    while (line != line_end) {
        if (bpp == 1) {
            if (_IsScanLine1bpp(pBuf + line * pitch, width)) {
                return line;
            }
        } else {
            if (_IsScanLine8bpp(pBuf + line * pitch, width)) {
                return line;
            }
        }
        line += line_step;
    }
    return -1;
}
CFX_GlyphBitmap* CPDF_Type3Cache::RenderGlyph(CPDF_Type3Glyphs* pSize, FX_DWORD charcode, const CFX_AffineMatrix* pMatrix, FX_FLOAT retinaScaleX, FX_FLOAT retinaScaleY)
{
    CPDF_Type3Char* pChar = m_pFont->LoadChar(charcode);
    if (pChar == NULL || pChar->m_pBitmap == NULL) {
        return NULL;
    }
    CFX_DIBitmap* pBitmap = pChar->m_pBitmap;
    CFX_AffineMatrix image_matrix, text_matrix;
    image_matrix = pChar->m_ImageMatrix;
    text_matrix.Set(pMatrix->a, pMatrix->b, pMatrix->c, pMatrix->d, 0, 0);
    image_matrix.Concat(text_matrix);
    CFX_DIBitmap* pResBitmap = NULL;
    int left, top;
    if (FXSYS_fabs(image_matrix.b) < FXSYS_fabs(image_matrix.a) / 100 && FXSYS_fabs(image_matrix.c) < FXSYS_fabs(image_matrix.d) / 100) {
        int top_line, bottom_line;
        top_line = _DetectFirstLastScan(pBitmap, TRUE);
        bottom_line = _DetectFirstLastScan(pBitmap, FALSE);
        if (top_line == 0 && bottom_line == pBitmap->GetHeight() - 1) {
            FX_FLOAT top_y = image_matrix.d + image_matrix.f;
            FX_FLOAT bottom_y = image_matrix.f;
            FX_BOOL bFlipped = top_y > bottom_y;
            if (bFlipped) {
                FX_FLOAT temp = top_y;
                top_y = bottom_y;
                bottom_y = temp;
            }
            pSize->AdjustBlue(top_y, bottom_y, top_line, bottom_line);
            pResBitmap = pBitmap->StretchTo((int)(FXSYS_round(image_matrix.a) * retinaScaleX), (int)((bFlipped ? top_line - bottom_line : bottom_line - top_line) * retinaScaleY));
            top = top_line;
            if (image_matrix.a < 0) {
                image_matrix.Scale(retinaScaleX, retinaScaleY);
                left = FXSYS_round(image_matrix.e + image_matrix.a);
            } else {
                left = FXSYS_round(image_matrix.e);
            }
        } else {
        }
    }
    if (pResBitmap == NULL) {
        image_matrix.Scale(retinaScaleX, retinaScaleY);
        pResBitmap = pBitmap->TransformTo(&image_matrix, left, top);
    }
    if (pResBitmap == NULL) {
        return NULL;
    }
    CFX_GlyphBitmap* pGlyph = new CFX_GlyphBitmap;
    pGlyph->m_Left = left;
    pGlyph->m_Top = -top;
    pGlyph->m_Bitmap.TakeOver(pResBitmap);
    delete pResBitmap;
    return pGlyph;
}
void _CPDF_UniqueKeyGen::Generate(int count, ...)
{
    va_list argList;
    va_start(argList, count);
    for (int i = 0; i < count; i ++) {
        int p = va_arg(argList, int);
        ((FX_DWORD*)m_Key)[i] = p;
    }
    va_end(argList);
    m_KeyLen = count * sizeof(FX_DWORD);
}
FX_BOOL CPDF_RenderStatus::ProcessText(const CPDF_TextObject* textobj, const CFX_AffineMatrix* pObj2Device, CFX_PathData* pClippingPath)
{
    if(textobj->m_nChars == 0) {
        return TRUE;
    }
    int text_render_mode = textobj->m_TextState.GetObject()->m_TextMode;
    if (text_render_mode == 3) {
        return TRUE;
    }
    CPDF_Font* pFont = textobj->m_TextState.GetFont();
    if (pFont->GetFontType() == PDFFONT_TYPE3) {
        return ProcessType3Text(textobj, pObj2Device);
    }
    FX_BOOL bFill = FALSE, bStroke = FALSE, bClip = FALSE;
    if (pClippingPath) {
        bClip = TRUE;
    } else {
        switch (text_render_mode) {
            case 0:
            case 4:
                bFill = TRUE;
                break;
            case 1:
            case 5:
                if (pFont->GetFace() == NULL && !(pFont->GetSubstFont()->m_SubstFlags & FXFONT_SUBST_GLYPHPATH)) {
                    bFill = TRUE;
                } else {
                    bStroke = TRUE;
                }
                break;
            case 2:
            case 6:
                if (pFont->GetFace() == NULL && !(pFont->GetSubstFont()->m_SubstFlags & FXFONT_SUBST_GLYPHPATH)) {
                    bFill = TRUE;
                } else {
                    bFill = bStroke = TRUE;
                }
                break;
            case 3:
            case 7:
                return TRUE;
            default:
                bFill = TRUE;
        }
    }
    FX_ARGB stroke_argb = 0, fill_argb = 0;
    FX_BOOL bPattern = FALSE;
    if (bStroke) {
        if (textobj->m_ColorState.GetStrokeColor()->IsPattern()) {
            bPattern = TRUE;
        } else {
            stroke_argb = GetStrokeArgb(textobj);
        }
    }
    if (bFill) {
        if (textobj->m_ColorState.GetFillColor()->IsPattern()) {
            bPattern = TRUE;
        } else {
            fill_argb = GetFillArgb(textobj);
        }
    }
    CFX_AffineMatrix text_matrix;
    textobj->GetTextMatrix(&text_matrix);
    if(IsAvailableMatrix(text_matrix) == FALSE) {
        return TRUE;
    }
    FX_FLOAT font_size = textobj->m_TextState.GetFontSize();
    if (bPattern) {
        DrawTextPathWithPattern(textobj, pObj2Device, pFont, font_size, &text_matrix, bFill, bStroke);
        return TRUE;
    }
    if (bClip || bStroke) {
        const CFX_AffineMatrix* pDeviceMatrix = pObj2Device;
        CFX_AffineMatrix device_matrix;
        if (bStroke) {
            const FX_FLOAT* pCTM = textobj->m_TextState.GetObject()->m_CTM;
            if (pCTM[0] != 1.0f || pCTM[3] != 1.0f) {
                CFX_AffineMatrix ctm(pCTM[0], pCTM[1], pCTM[2], pCTM[3], 0, 0);
                text_matrix.ConcatInverse(ctm);
                device_matrix.Copy(ctm);
                device_matrix.Concat(*pObj2Device);
                pDeviceMatrix = &device_matrix;
            }
        }
        int flag = 0;
        if (bStroke && bFill) {
            flag |= FX_FILL_STROKE;
            flag |= FX_STROKE_TEXT_MODE;
        }
        const CPDF_GeneralStateData* pGeneralData = ((CPDF_PageObject*)textobj)->m_GeneralState;
        if (pGeneralData && pGeneralData->m_StrokeAdjust) {
            flag |= FX_STROKE_ADJUST;
        }
        if (m_Options.m_Flags & RENDER_NOTEXTSMOOTH) {
            flag |= FXFILL_NOPATHSMOOTH;
        }
        return CPDF_TextRenderer::DrawTextPath(m_pDevice, textobj->m_nChars, textobj->m_pCharCodes, textobj->m_pCharPos, pFont, font_size,
                                               &text_matrix, pDeviceMatrix, textobj->m_GraphState, fill_argb, stroke_argb, pClippingPath, flag);
    }
    text_matrix.Concat(*pObj2Device);
    return CPDF_TextRenderer::DrawNormalText(m_pDevice, textobj->m_nChars, textobj->m_pCharCodes, textobj->m_pCharPos, pFont, font_size,
            &text_matrix, fill_argb, &m_Options);
}
CPDF_Type3Cache* CPDF_RenderStatus::GetCachedType3(CPDF_Type3Font* pFont)
{
    if (pFont->m_pDocument == NULL) {
        return NULL;
    }
    pFont->m_pDocument->GetPageData()->GetFont(pFont->GetFontDict(), FALSE);
    return pFont->m_pDocument->GetRenderData()->GetCachedType3(pFont);
}
static void ReleaseCachedType3(CPDF_Type3Font* pFont)
{
    if (pFont->m_pDocument == NULL) {
        return;
    }
    pFont->m_pDocument->GetRenderData()->ReleaseCachedType3(pFont);
    pFont->m_pDocument->GetPageData()->ReleaseFont(pFont->GetFontDict());
}
FX_BOOL CPDF_Type3Char::LoadBitmap(CPDF_RenderContext* pContext)
{
    if (m_pBitmap != NULL || m_pForm == NULL) {
        return TRUE;
    }
    if (m_pForm->CountObjects() == 1 && !m_bColored) {
        CPDF_PageObject *pPageObj = m_pForm->GetObjectAt(m_pForm->GetFirstObjectPosition());
        if (pPageObj->m_Type == PDFPAGE_IMAGE) {
            CPDF_ImageObject* pImage = (CPDF_ImageObject*)pPageObj;
            m_ImageMatrix = pImage->m_Matrix;
            const CFX_DIBSource* pSource = pImage->m_pImage->LoadDIBSource();
            if (pSource) {
                m_pBitmap = pSource->Clone();
                delete pSource;
            }
            delete m_pForm;
            m_pForm = NULL;
            return TRUE;
        }
        if (pPageObj->m_Type == PDFPAGE_INLINES) {
            CPDF_InlineImages *pInlines = (CPDF_InlineImages *)pPageObj;
            if (pInlines->m_pStream) {
                m_ImageMatrix = pInlines->m_Matrices[0];
                CPDF_DIBSource dibsrc;
                if (!dibsrc.Load(pContext->m_pDocument, pInlines->m_pStream, NULL, NULL, NULL, NULL)) {
                    return FALSE;
                }
                m_pBitmap = dibsrc.Clone();
                delete m_pForm;
                m_pForm = NULL;
                return TRUE;
            }
        }
    }
    return FALSE;
}
class CPDF_RefType3Cache
{
public:
    CPDF_RefType3Cache(CPDF_Type3Font* pType3Font)
    {
        m_dwCount = 0;
        m_pType3Font = pType3Font;
    }
    ~CPDF_RefType3Cache()
    {
        while(m_dwCount--) {
            ReleaseCachedType3(m_pType3Font);
        }
    }
    FX_DWORD m_dwCount;
    CPDF_Type3Font* m_pType3Font;
};
FX_BOOL CPDF_RenderStatus::ProcessType3Text(const CPDF_TextObject* textobj, const CFX_AffineMatrix* pObj2Device)
{
    CPDF_Type3Font* pType3Font = textobj->m_TextState.GetFont()->GetType3Font();
    for (int j = 0; j < m_Type3FontCache.GetSize(); j++)
        if ((CPDF_Type3Font*)m_Type3FontCache.GetAt(j) == pType3Font) {
            return TRUE;
        }
    CFX_Matrix dCTM = m_pDevice->GetCTM();
    FX_FLOAT sa = FXSYS_fabs(dCTM.a);
    FX_FLOAT sd = FXSYS_fabs(dCTM.d);
    CFX_AffineMatrix text_matrix;
    textobj->GetTextMatrix(&text_matrix);
    CFX_AffineMatrix char_matrix = pType3Font->GetFontMatrix();
    FX_FLOAT font_size = textobj->m_TextState.GetFontSize();
    char_matrix.Scale(font_size, font_size);
    FX_ARGB fill_argb = GetFillArgb(textobj, TRUE);
    int fill_alpha = FXARGB_A(fill_argb);
    int device_class = m_pDevice->GetDeviceClass();
    FXTEXT_GLYPHPOS* pGlyphAndPos = NULL;
    if (device_class == FXDC_DISPLAY) {
        pGlyphAndPos = FX_Alloc(FXTEXT_GLYPHPOS, textobj->m_nChars);
    } else if (fill_alpha < 255) {
        return FALSE;
    }
    CPDF_RefType3Cache refTypeCache(pType3Font);
    FX_DWORD *pChars = textobj->m_pCharCodes;
    if (textobj->m_nChars == 1) {
        pChars = (FX_DWORD*)(&textobj->m_pCharCodes);
    }
    for (int iChar = 0; iChar < textobj->m_nChars; iChar ++) {
        FX_DWORD charcode = pChars[iChar];
        if (charcode == (FX_DWORD) - 1) {
            continue;
        }
        CPDF_Type3Char* pType3Char = pType3Font->LoadChar(charcode);
        if (pType3Char == NULL) {
            continue;
        }
        CFX_AffineMatrix matrix = char_matrix;
        matrix.e += iChar ? textobj->m_pCharPos[iChar - 1] : 0;
        matrix.Concat(text_matrix);
        matrix.Concat(*pObj2Device);
        if (!pType3Char->LoadBitmap(m_pContext)) {
            if (pGlyphAndPos) {
                for (int i = 0; i < iChar; i ++) {
                    FXTEXT_GLYPHPOS& glyph = pGlyphAndPos[i];
                    if (glyph.m_pGlyph == NULL) {
                        continue;
                    }
                    m_pDevice->SetBitMask(&glyph.m_pGlyph->m_Bitmap,
                                          glyph.m_OriginX + glyph.m_pGlyph->m_Left,
                                          glyph.m_OriginY - glyph.m_pGlyph->m_Top, fill_argb);
                }
                FX_Free(pGlyphAndPos);
                pGlyphAndPos = NULL;
            }
            CPDF_GraphicStates* pStates = CloneObjStates(textobj, FALSE);
            CPDF_RenderOptions Options = m_Options;
            Options.m_Flags |= RENDER_FORCE_HALFTONE | RENDER_RECT_AA;
            Options.m_Flags &= ~RENDER_FORCE_DOWNSAMPLE;
            CPDF_Dictionary* pFormResource = NULL;
            if (pType3Char->m_pForm && pType3Char->m_pForm->m_pFormDict) {
                pFormResource = pType3Char->m_pForm->m_pFormDict->GetDict(FX_BSTRC("Resources"));
            }
            if (fill_alpha == 255) {
                CPDF_RenderStatus status;
                status.Initialize(m_pContext, m_pDevice, NULL, NULL, this, pStates, &Options,
                                  pType3Char->m_pForm->m_Transparency, m_bDropObjects, pFormResource, FALSE, pType3Char, fill_argb);
                status.m_Type3FontCache.Append(m_Type3FontCache);
                status.m_Type3FontCache.Add(pType3Font);
                m_pDevice->SaveState();
                status.RenderObjectList(pType3Char->m_pForm, &matrix);
                m_pDevice->RestoreState();
            } else {
                CFX_FloatRect rect_f = pType3Char->m_pForm->CalcBoundingBox();
                rect_f.Transform(&matrix);
                FX_RECT rect = rect_f.GetOutterRect();
                CFX_FxgeDevice bitmap_device;
                if (!bitmap_device.Create((int)(rect.Width() * sa), (int)(rect.Height() * sd), FXDIB_Argb)) {
                    return TRUE;
                }
                bitmap_device.GetBitmap()->Clear(0);
                CPDF_RenderStatus status;
                status.Initialize(m_pContext, &bitmap_device, NULL, NULL, this, pStates, &Options,
                                  pType3Char->m_pForm->m_Transparency, m_bDropObjects, pFormResource, FALSE, pType3Char, fill_argb);
                status.m_Type3FontCache.Append(m_Type3FontCache);
                status.m_Type3FontCache.Add(pType3Font);
                matrix.TranslateI(-rect.left, -rect.top);
                matrix.Scale(sa, sd);
                status.RenderObjectList(pType3Char->m_pForm, &matrix);
                m_pDevice->SetDIBits(bitmap_device.GetBitmap(), rect.left, rect.top);
            }
            delete pStates;
        } else if (pType3Char->m_pBitmap) {
            if (device_class == FXDC_DISPLAY) {
                CPDF_Type3Cache* pCache = GetCachedType3(pType3Font);
                refTypeCache.m_dwCount++;
                CFX_GlyphBitmap* pBitmap = pCache->LoadGlyph(charcode, &matrix, sa, sd);
                if (pBitmap == NULL) {
                    continue;
                }
                int origin_x = FXSYS_round(matrix.e);
                int origin_y = FXSYS_round(matrix.f);
                if (pGlyphAndPos) {
                    pGlyphAndPos[iChar].m_pGlyph = pBitmap;
                    pGlyphAndPos[iChar].m_OriginX = origin_x;
                    pGlyphAndPos[iChar].m_OriginY = origin_y;
                } else {
                    m_pDevice->SetBitMask(&pBitmap->m_Bitmap, origin_x + pBitmap->m_Left, origin_y - pBitmap->m_Top, fill_argb);
                }
            } else {
                CFX_AffineMatrix image_matrix = pType3Char->m_ImageMatrix;
                image_matrix.Concat(matrix);
                CPDF_ImageRenderer renderer;
                if (renderer.Start(this, pType3Char->m_pBitmap, fill_argb, 255, &image_matrix, 0, FALSE)) {
                    renderer.Continue(NULL);
                }
                if (!renderer.m_Result) {
                    return FALSE;
                }
            }
        }
    }
    if (pGlyphAndPos) {
        FX_RECT rect = FXGE_GetGlyphsBBox(pGlyphAndPos, textobj->m_nChars, 0, sa, sd);
        CFX_DIBitmap bitmap;
        if (!bitmap.Create((int)(rect.Width() * sa), (int)(rect.Height() * sd), FXDIB_8bppMask)) {
            FX_Free(pGlyphAndPos);
            return TRUE;
        }
        bitmap.Clear(0);
        for (int iChar = 0; iChar < textobj->m_nChars; iChar ++) {
            FXTEXT_GLYPHPOS& glyph = pGlyphAndPos[iChar];
            if (glyph.m_pGlyph == NULL) {
                continue;
            }
            bitmap.TransferBitmap((int)((glyph.m_OriginX + glyph.m_pGlyph->m_Left - rect.left) * sa),
                                  (int)((glyph.m_OriginY - glyph.m_pGlyph->m_Top - rect.top) * sd),
                                  glyph.m_pGlyph->m_Bitmap.GetWidth(), glyph.m_pGlyph->m_Bitmap.GetHeight(),
                                  &glyph.m_pGlyph->m_Bitmap, 0, 0);
        }
        m_pDevice->SetBitMask(&bitmap, rect.left, rect.top, fill_argb);
        FX_Free(pGlyphAndPos);
    }
    return TRUE;
}
class CPDF_CharPosList
{
public:
    CPDF_CharPosList();
    ~CPDF_CharPosList();
    void				Load(int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos, CPDF_Font* pFont, FX_FLOAT font_size);
    FXTEXT_CHARPOS*		m_pCharPos;
    FX_DWORD			m_nChars;
};
FX_FLOAT _CIDTransformToFloat(FX_BYTE ch);
CPDF_CharPosList::CPDF_CharPosList()
{
    m_pCharPos = NULL;
}
CPDF_CharPosList::~CPDF_CharPosList()
{
    if (m_pCharPos) {
        FX_Free(m_pCharPos);
    }
}
void CPDF_CharPosList::Load(int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos, CPDF_Font* pFont,
                            FX_FLOAT FontSize)
{
    m_pCharPos = FX_Alloc(FXTEXT_CHARPOS, nChars);
    m_nChars = 0;
    CPDF_CIDFont* pCIDFont = pFont->GetCIDFont();
    FX_BOOL bVertWriting = pCIDFont && pCIDFont->IsVertWriting();
    for (int iChar = 0; iChar < nChars; iChar ++) {
        FX_DWORD CharCode = nChars == 1 ? (FX_DWORD)(FX_UINTPTR)pCharCodes : pCharCodes[iChar];
        if (CharCode == (FX_DWORD) - 1) {
            continue;
        }
        FX_BOOL bVert = FALSE;
        FXTEXT_CHARPOS& charpos = m_pCharPos[m_nChars++];
        if (pCIDFont) {
            charpos.m_bFontStyle = pCIDFont->IsFontStyleFromCharCode(CharCode);
        }
        charpos.m_GlyphIndex = pFont->GlyphFromCharCode(CharCode, &bVert);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
        charpos.m_ExtGID = pFont->GlyphFromCharCodeExt(CharCode);
#endif
        if (!pFont->IsEmbedded() && pFont->GetFontType() != PDFFONT_CIDFONT) {
            charpos.m_FontCharWidth = pFont->GetCharWidthF(CharCode);
        } else {
            charpos.m_FontCharWidth = 0;
        }
        charpos.m_OriginX = iChar ? pCharPos[iChar - 1] : 0;
        charpos.m_OriginY = 0;
        charpos.m_bGlyphAdjust = FALSE;
        if (pCIDFont == NULL) {
            continue;
        }
        FX_WORD CID = pCIDFont->CIDFromCharCode(CharCode);
        if (bVertWriting) {
            charpos.m_OriginY = charpos.m_OriginX;
            charpos.m_OriginX = 0;
            short vx, vy;
            pCIDFont->GetVertOrigin(CID, vx, vy);
            charpos.m_OriginX -= FontSize * vx / 1000;
            charpos.m_OriginY -= FontSize * vy / 1000;
        }
        FX_LPCBYTE pTransform = pCIDFont->GetCIDTransform(CID);
        if (pTransform && !bVert) {
            charpos.m_AdjustMatrix[0] = _CIDTransformToFloat(pTransform[0]);
            charpos.m_AdjustMatrix[2] = _CIDTransformToFloat(pTransform[2]);
            charpos.m_AdjustMatrix[1] = _CIDTransformToFloat(pTransform[1]);
            charpos.m_AdjustMatrix[3] = _CIDTransformToFloat(pTransform[3]);
            charpos.m_OriginX += _CIDTransformToFloat(pTransform[4]) * FontSize;
            charpos.m_OriginY += _CIDTransformToFloat(pTransform[5]) * FontSize;
            charpos.m_bGlyphAdjust = TRUE;
        }
    }
}
FX_BOOL CPDF_TextRenderer::DrawTextPath(CFX_RenderDevice* pDevice, int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos,
                                        CPDF_Font* pFont, FX_FLOAT font_size,
                                        const CFX_AffineMatrix* pText2User, const CFX_AffineMatrix* pUser2Device,
                                        const CFX_GraphStateData* pGraphState,
                                        FX_ARGB fill_argb, FX_ARGB stroke_argb, CFX_PathData* pClippingPath, int nFlag)
{
    CFX_FontCache* pCache = pFont->m_pDocument ? pFont->m_pDocument->GetRenderData()->GetFontCache() : NULL;
    CPDF_CharPosList CharPosList;
    CharPosList.Load(nChars, pCharCodes, pCharPos, pFont, font_size);
    return pDevice->DrawTextPath(CharPosList.m_nChars, CharPosList.m_pCharPos,
                                 &pFont->m_Font, pCache, font_size, pText2User, pUser2Device,
                                 pGraphState, fill_argb, stroke_argb, pClippingPath, nFlag);
}
void CPDF_TextRenderer::DrawTextString(CFX_RenderDevice* pDevice, int left, int top, CPDF_Font* pFont, int height,
                                       const CFX_ByteString& str, FX_ARGB argb)
{
    FX_RECT font_bbox;
    pFont->GetFontBBox(font_bbox);
    FX_FLOAT font_size = (FX_FLOAT)height * 1000.0f / (FX_FLOAT)(font_bbox.top - font_bbox.bottom);
    FX_FLOAT origin_x = (FX_FLOAT)left;
    FX_FLOAT origin_y = (FX_FLOAT)top + font_size * (FX_FLOAT)font_bbox.top / 1000.0f;
    CFX_AffineMatrix matrix(1.0f, 0, 0, -1.0f, 0, 0);
    DrawTextString(pDevice, origin_x, origin_y, pFont, font_size, &matrix, str, argb);
}
void CPDF_TextRenderer::DrawTextString(CFX_RenderDevice* pDevice, FX_FLOAT origin_x, FX_FLOAT origin_y, CPDF_Font* pFont, FX_FLOAT font_size,
                                       const CFX_AffineMatrix* pMatrix, const CFX_ByteString& str, FX_ARGB fill_argb,
                                       FX_ARGB stroke_argb, const CFX_GraphStateData* pGraphState, const CPDF_RenderOptions* pOptions)
{
    int nChars = pFont->CountChar(str, str.GetLength());
    if (nChars == 0) {
        return;
    }
    FX_DWORD charcode;
    int offset = 0;
    FX_DWORD* pCharCodes;
    FX_FLOAT* pCharPos;
    if (nChars == 1) {
        charcode = pFont->GetNextChar(str, str.GetLength(), offset);
        pCharCodes = (FX_DWORD*)(FX_UINTPTR)charcode;
        pCharPos = NULL;
    } else {
        pCharCodes = FX_Alloc(FX_DWORD, nChars);
        pCharPos = FX_Alloc(FX_FLOAT, nChars - 1);
        FX_FLOAT cur_pos = 0;
        for (int i = 0; i < nChars; i ++) {
            pCharCodes[i] = pFont->GetNextChar(str, str.GetLength(), offset);
            if (i) {
                pCharPos[i - 1] = cur_pos;
            }
            cur_pos += pFont->GetCharWidthF(pCharCodes[i]) * font_size / 1000;
        }
    }
    CFX_AffineMatrix matrix;
    if (pMatrix) {
        matrix = *pMatrix;
    }
    matrix.e = origin_x;
    matrix.f = origin_y;
    if (pFont->GetFontType() == PDFFONT_TYPE3)
        ;
    else if (stroke_argb == 0) {
        DrawNormalText(pDevice, nChars, pCharCodes, pCharPos, pFont, font_size, &matrix, fill_argb, pOptions);
    } else
        DrawTextPath(pDevice, nChars, pCharCodes, pCharPos, pFont, font_size, &matrix, NULL, pGraphState,
                     fill_argb, stroke_argb, NULL);
    if (nChars > 1) {
        FX_Free(pCharCodes);
        FX_Free(pCharPos);
    }
}
FX_BOOL CPDF_TextRenderer::DrawNormalText(CFX_RenderDevice* pDevice, int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos,
        CPDF_Font* pFont, FX_FLOAT font_size,
        const CFX_AffineMatrix* pText2Device,
        FX_ARGB fill_argb, const CPDF_RenderOptions* pOptions)
{
    CFX_FontCache* pCache = pFont->m_pDocument ? pFont->m_pDocument->GetRenderData()->GetFontCache() : NULL;
    CPDF_CharPosList CharPosList;
    CharPosList.Load(nChars, pCharCodes, pCharPos, pFont, font_size);
    int FXGE_flags = 0;
    if (pOptions) {
        FX_DWORD dwFlags = pOptions->m_Flags;
        if (dwFlags & RENDER_CLEARTYPE) {
            FXGE_flags |= FXTEXT_CLEARTYPE;
            if (dwFlags & RENDER_BGR_STRIPE) {
                FXGE_flags |= FXTEXT_BGR_STRIPE;
            }
        }
        if (dwFlags & RENDER_NOTEXTSMOOTH) {
            FXGE_flags |= FXTEXT_NOSMOOTH;
        }
        if (dwFlags & RENDER_PRINTGRAPHICTEXT) {
            FXGE_flags |= FXTEXT_PRINTGRAPHICTEXT;
        }
        if (dwFlags & RENDER_NO_NATIVETEXT) {
            FXGE_flags |= FXTEXT_NO_NATIVETEXT;
        }
        if (dwFlags & RENDER_PRINTIMAGETEXT) {
            FXGE_flags |= FXTEXT_PRINTIMAGETEXT;
        }
    } else {
        FXGE_flags = FXTEXT_CLEARTYPE;
    }
    if (pFont->GetFontType() & PDFFONT_CIDFONT) {
        FXGE_flags |= FXFONT_CIDFONT;
    }
    return pDevice->DrawNormalText(CharPosList.m_nChars, CharPosList.m_pCharPos, &pFont->m_Font, pCache, font_size, pText2Device, fill_argb, FXGE_flags);
}
void CPDF_RenderStatus::DrawTextPathWithPattern(const CPDF_TextObject* textobj, const CFX_AffineMatrix* pObj2Device,
        CPDF_Font* pFont, FX_FLOAT font_size,
        const CFX_AffineMatrix* pTextMatrix, FX_BOOL bFill, FX_BOOL bStroke)
{
    if (!bStroke) {
        CPDF_PathObject path;
        CPDF_TextObject* pCopy = new CPDF_TextObject;
        pCopy->Copy(textobj);
        path.m_bStroke = FALSE;
        path.m_FillType = FXFILL_WINDING;
        path.m_ClipPath.AppendTexts(&pCopy, 1);
        path.m_ColorState = textobj->m_ColorState;
        path.m_Path.New()->AppendRect(textobj->m_Left, textobj->m_Bottom, textobj->m_Right, textobj->m_Top);
        path.m_Left = textobj->m_Left;
        path.m_Bottom = textobj->m_Bottom;
        path.m_Right = textobj->m_Right;
        path.m_Top = textobj->m_Top;
        RenderSingleObject(&path, pObj2Device);
        return;
    }
    CFX_FontCache* pCache;
    if (pFont->m_pDocument) {
        pCache = pFont->m_pDocument->GetRenderData()->GetFontCache();
    } else {
        pCache = CFX_GEModule::Get()->GetFontCache();
    }
    CFX_FaceCache* pFaceCache = pCache->GetCachedFace(&pFont->m_Font);
    FX_FONTCACHE_DEFINE(pCache, &pFont->m_Font);
    CPDF_CharPosList CharPosList;
    CharPosList.Load(textobj->m_nChars, textobj->m_pCharCodes, textobj->m_pCharPos, pFont, font_size);
    for (FX_DWORD i = 0; i < CharPosList.m_nChars; i ++) {
        FXTEXT_CHARPOS& charpos = CharPosList.m_pCharPos[i];
        const CFX_PathData* pPath = pFaceCache->LoadGlyphPath(&pFont->m_Font, charpos.m_GlyphIndex,
                                    charpos.m_FontCharWidth);
        if (pPath == NULL) {
            continue;
        }
        CPDF_PathObject path;
        path.m_GraphState = textobj->m_GraphState;
        path.m_ColorState = textobj->m_ColorState;
        CFX_AffineMatrix matrix;
        if (charpos.m_bGlyphAdjust)
            matrix.Set(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                       charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3], 0, 0);
        matrix.Concat(font_size, 0, 0, font_size, charpos.m_OriginX, charpos.m_OriginY);
        path.m_Path.New()->Append(pPath, &matrix);
        path.m_Matrix = *pTextMatrix;
        path.m_bStroke = bStroke;
        path.m_FillType = bFill ? FXFILL_WINDING : 0;
        path.CalcBoundingBox();
        ProcessPath(&path, pObj2Device);
    }
}
CFX_PathData* CPDF_Font::LoadGlyphPath(FX_DWORD charcode, int dest_width)
{
    int glyph_index = GlyphFromCharCode(charcode);
    if (m_Font.m_Face == NULL) {
        return NULL;
    }
    return m_Font.LoadGlyphPath(glyph_index, dest_width);
}
