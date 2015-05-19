// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxge/fx_freetype.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "text_int.h"
#undef FX_GAMMA
#undef FX_GAMMA_INVERSE
#define FX_GAMMA(value)			(value)
#define FX_GAMMA_INVERSE(value)	(value)
FX_RECT FXGE_GetGlyphsBBox(FXTEXT_GLYPHPOS* pGlyphAndPos, int nChars, int anti_alias, FX_FLOAT retinaScaleX, FX_FLOAT retinaScaleY)
{
    FX_RECT rect(0, 0, 0, 0);
    FX_BOOL bStarted = FALSE;
    for (int iChar = 0; iChar < nChars; iChar ++) {
        FXTEXT_GLYPHPOS& glyph = pGlyphAndPos[iChar];
        const CFX_GlyphBitmap* pGlyph = glyph.m_pGlyph;
        if (pGlyph == NULL) {
            continue;
        }
        int char_left = glyph.m_OriginX + pGlyph->m_Left;
        int char_width = (int)(pGlyph->m_Bitmap.GetWidth() / retinaScaleX);
        if (anti_alias == FXFT_RENDER_MODE_LCD) {
            char_width /= 3;
        }
        int char_right = char_left + char_width;
        int char_top = glyph.m_OriginY - pGlyph->m_Top;
        int char_bottom = char_top + (int)(pGlyph->m_Bitmap.GetHeight() / retinaScaleY);
        if (!bStarted) {
            rect.left = char_left;
            rect.right = char_right;
            rect.top = char_top;
            rect.bottom = char_bottom;
            bStarted = TRUE;
        } else {
            if (rect.left > char_left) {
                rect.left = char_left;
            }
            if (rect.right < char_right) {
                rect.right = char_right;
            }
            if (rect.top > char_top) {
                rect.top = char_top;
            }
            if (rect.bottom < char_bottom) {
                rect.bottom = char_bottom;
            }
        }
    }
    return rect;
}
static void _AdjustGlyphSpace(FXTEXT_GLYPHPOS* pGlyphAndPos, int nChars)
{
    ASSERT(nChars > 1);
    FX_BOOL bVertical = FALSE;
    if (pGlyphAndPos[nChars - 1].m_OriginX == pGlyphAndPos[0].m_OriginX) {
        bVertical = TRUE;
    } else if (pGlyphAndPos[nChars - 1].m_OriginY != pGlyphAndPos[0].m_OriginY) {
        return;
    }
    int i = nChars - 1;
    int* next_origin = bVertical ? &pGlyphAndPos[i].m_OriginY : &pGlyphAndPos[i].m_OriginX;
    FX_FLOAT next_origin_f = bVertical ? pGlyphAndPos[i].m_fOriginY : pGlyphAndPos[i].m_fOriginX;
    for (i --; i > 0; i --) {
        int* this_origin = bVertical ? &pGlyphAndPos[i].m_OriginY : &pGlyphAndPos[i].m_OriginX;
        FX_FLOAT this_origin_f = bVertical ? pGlyphAndPos[i].m_fOriginY : pGlyphAndPos[i].m_fOriginX;
        int space = (*next_origin) - (*this_origin);
        FX_FLOAT space_f = next_origin_f - this_origin_f;
        FX_FLOAT error = (FX_FLOAT)(FXSYS_fabs(space_f) - FXSYS_fabs((FX_FLOAT)(space)));
        if (error > 0.5f) {
            *this_origin += space > 0 ? -1 : 1;
        }
        next_origin = this_origin;
        next_origin_f = this_origin_f;
    }
}
static const FX_BYTE g_TextGammaAdjust[256] = {
    0, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 15, 16, 17, 18, 19,
    21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
    121, 122, 123, 124, 125, 126, 127, 128, 129, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
    167, 168, 169, 170, 171, 172, 173, 174, 174, 175, 176, 177, 178, 179, 180, 181,
    182, 183, 184, 185, 186, 187, 188, 189, 190, 190, 191, 192, 193, 194, 195, 196,
    197, 198, 199, 200, 201, 202, 203, 204, 204, 205, 206, 207, 208, 209, 210, 211,
    212, 213, 214, 215, 216, 217, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
    227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 239, 240,
    241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 250, 251, 252, 253, 254, 255,
};
#define ADJUST_ALPHA(background, foreground, src_alpha, text_flags, a) \
    src_alpha = g_TextGammaAdjust[(FX_BYTE)src_alpha];
void _Color2Argb(FX_ARGB& argb, FX_DWORD color, int alpha_flag, void* pIccTransform)
{
    if (pIccTransform == NULL && !FXGETFLAG_COLORTYPE(alpha_flag)) {
        argb = color;
        return;
    }
    if (!CFX_GEModule::Get()->GetCodecModule() || !CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
        pIccTransform = NULL;
    }
    FX_BYTE bgra[4];
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        color = FXGETFLAG_COLORTYPE(alpha_flag) ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
        pIccModule->TranslateScanline(pIccTransform, bgra, (FX_LPCBYTE)&color, 1);
        bgra[3] = FXGETFLAG_COLORTYPE(alpha_flag) ?
                  (alpha_flag >> 24) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXGETFLAG_ALPHA_STROKE(alpha_flag) :
                  FXARGB_A(color);
        argb = FXARGB_MAKE(bgra[3], bgra[2], bgra[1], bgra[0]);
        return;
    }
    AdobeCMYK_to_sRGB1(FXSYS_GetCValue(color), FXSYS_GetMValue(color),
                       FXSYS_GetYValue(color), FXSYS_GetKValue(color),
                       bgra[2], bgra[1], bgra[0]);
    bgra[3] = (alpha_flag >> 24) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXGETFLAG_ALPHA_STROKE(alpha_flag);
    argb = FXARGB_MAKE(bgra[3], bgra[2], bgra[1], bgra[0]);
}
FX_BOOL CFX_RenderDevice::DrawNormalText(int nChars, const FXTEXT_CHARPOS* pCharPos,
        CFX_Font* pFont, CFX_FontCache* pCache,
        FX_FLOAT font_size, const CFX_AffineMatrix* pText2Device,
        FX_DWORD fill_color, FX_DWORD text_flags,
        int alpha_flag, void* pIccTransform)
{
    int nativetext_flags = text_flags;
    if (m_DeviceClass != FXDC_DISPLAY) {
        if (!(text_flags & FXTEXT_PRINTGRAPHICTEXT)) {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
            if (!(text_flags & FXFONT_CIDFONT) && pFont->GetPsName().Find(CFX_WideString::FromLocal("+ZJHL")) == -1)
#ifdef FOXIT_CHROME_BUILD
                if (pFont->GetPsName() != CFX_WideString::FromLocal("CNAAJI+cmex10"))
#endif
#endif
                    if (m_pDeviceDriver->DrawDeviceText(nChars, pCharPos, pFont, pCache, pText2Device, font_size, fill_color, alpha_flag, pIccTransform)) {
                        return TRUE;
                    }
        }
        int alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXARGB_A(fill_color);
        if (alpha < 255) {
            return FALSE;
        }
    } else if (!(text_flags & FXTEXT_NO_NATIVETEXT)) {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
        if (!(text_flags & FXFONT_CIDFONT))
#ifdef FOXIT_CHROME_BUILD
            if (pFont->GetPsName() != CFX_WideString::FromLocal("CNAAJI+cmex10"))
#endif
#endif
                if (m_pDeviceDriver->DrawDeviceText(nChars, pCharPos, pFont, pCache, pText2Device, font_size, fill_color, alpha_flag, pIccTransform)) {
                    return TRUE;
                }
    }
    CFX_AffineMatrix char2device, deviceCtm, text2Device;
    if (pText2Device) {
        char2device = *pText2Device;
        text2Device = *pText2Device;
    }
    char2device.Scale(font_size, -font_size);
    if (FXSYS_fabs(char2device.a) + FXSYS_fabs(char2device.b) > 50 * 1.0f ||
            ((m_DeviceClass == FXDC_PRINTER && !m_pDeviceDriver->IsPSPrintDriver())
             && !(text_flags & FXTEXT_PRINTIMAGETEXT))) {
        if (pFont->GetFace() != NULL || (pFont->GetSubstFont()->m_SubstFlags & FXFONT_SUBST_GLYPHPATH)) {
            int nPathFlags = (text_flags & FXTEXT_NOSMOOTH) == 0 ? 0 : FXFILL_NOPATHSMOOTH;
            return DrawTextPath(nChars, pCharPos, pFont, pCache, font_size, pText2Device, NULL, NULL, fill_color, 0, NULL, nPathFlags, alpha_flag, pIccTransform);
        }
    }
    int anti_alias = FXFT_RENDER_MODE_MONO;
    FX_BOOL bNormal = FALSE;
    if ((text_flags & FXTEXT_NOSMOOTH) == 0) {
        if (m_DeviceClass == FXDC_DISPLAY && m_bpp > 1) {
            FX_BOOL bClearType;
            if (pFont->GetFace() == NULL && !(pFont->GetSubstFont()->m_SubstFlags & FXFONT_SUBST_CLEARTYPE)) {
                bClearType = FALSE;
            } else {
                bClearType = text_flags & FXTEXT_CLEARTYPE;
            }
            if ((m_RenderCaps & (FXRC_ALPHA_OUTPUT | FXRC_CMYK_OUTPUT))) {
                anti_alias = FXFT_RENDER_MODE_LCD;
                bNormal = TRUE;
            } else if (m_bpp < 16) {
                anti_alias = FXFT_RENDER_MODE_NORMAL;
            } else {
                if (bClearType == FALSE) {
                    anti_alias = FXFT_RENDER_MODE_LCD;
                    bNormal = TRUE;
                } else {
                    anti_alias = FXFT_RENDER_MODE_LCD;
                }
            }
        }
    }
    if (pCache == NULL) {
        pCache = CFX_GEModule::Get()->GetFontCache();
    }
    CFX_FaceCache* pFaceCache = pCache->GetCachedFace(pFont);
    FX_FONTCACHE_DEFINE(pCache, pFont);
    FXTEXT_GLYPHPOS* pGlyphAndPos = FX_Alloc(FXTEXT_GLYPHPOS, nChars);
    int iChar;
    deviceCtm = char2device;
    CFX_AffineMatrix matrixCTM = GetCTM();
    FX_FLOAT scale_x = FXSYS_fabs(matrixCTM.a);
    FX_FLOAT scale_y = FXSYS_fabs(matrixCTM.d);
    deviceCtm.Concat(scale_x, 0, 0, scale_y, 0, 0);
    text2Device.Concat(scale_x, 0, 0, scale_y, 0, 0);
    for (iChar = 0; iChar < nChars; iChar ++) {
        FXTEXT_GLYPHPOS& glyph = pGlyphAndPos[iChar];
        const FXTEXT_CHARPOS& charpos = pCharPos[iChar];
        glyph.m_fOriginX = charpos.m_OriginX;
        glyph.m_fOriginY = charpos.m_OriginY;
        text2Device.Transform(glyph.m_fOriginX, glyph.m_fOriginY);
        if (anti_alias < FXFT_RENDER_MODE_LCD) {
            glyph.m_OriginX = FXSYS_round(glyph.m_fOriginX);
        } else {
            glyph.m_OriginX = (int)FXSYS_floor(glyph.m_fOriginX);
        }
        glyph.m_OriginY = FXSYS_round(glyph.m_fOriginY);
        if (charpos.m_bGlyphAdjust) {
            CFX_AffineMatrix new_matrix(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                                        charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3], 0, 0);
            new_matrix.Concat(deviceCtm);
            glyph.m_pGlyph = pFaceCache->LoadGlyphBitmap(pFont, charpos.m_GlyphIndex, charpos.m_bFontStyle, &new_matrix,
                             charpos.m_FontCharWidth, anti_alias, nativetext_flags);
        } else
            glyph.m_pGlyph = pFaceCache->LoadGlyphBitmap(pFont, charpos.m_GlyphIndex, charpos.m_bFontStyle, &deviceCtm,
                             charpos.m_FontCharWidth, anti_alias, nativetext_flags);
    }
    if (anti_alias < FXFT_RENDER_MODE_LCD && nChars > 1) {
        _AdjustGlyphSpace(pGlyphAndPos, nChars);
    }
    FX_RECT bmp_rect1 = FXGE_GetGlyphsBBox(pGlyphAndPos, nChars, anti_alias);
    if (scale_x > 1 && scale_y > 1) {
        bmp_rect1.left--;
        bmp_rect1.top --;
        bmp_rect1.right ++;
        bmp_rect1.bottom ++;
    }
    FX_RECT bmp_rect(FXSYS_round((FX_FLOAT)(bmp_rect1.left) / scale_x), FXSYS_round((FX_FLOAT)(bmp_rect1.top) / scale_y),
                     FXSYS_round((FX_FLOAT)bmp_rect1.right / scale_x), FXSYS_round((FX_FLOAT)bmp_rect1.bottom / scale_y));
    bmp_rect.Intersect(m_ClipBox);
    if (bmp_rect.IsEmpty()) {
        FX_Free(pGlyphAndPos);
        return TRUE;
    }
    int pixel_width = FXSYS_round(bmp_rect.Width() * scale_x);
    int pixel_height = FXSYS_round(bmp_rect.Height() * scale_y);
    int pixel_left = FXSYS_round(bmp_rect.left * scale_x);
    int pixel_top = FXSYS_round(bmp_rect.top * scale_y);
    if (anti_alias == FXFT_RENDER_MODE_MONO) {
        CFX_DIBitmap bitmap;
        if (!bitmap.Create(pixel_width, pixel_height, FXDIB_1bppMask)) {
            FX_Free(pGlyphAndPos);
            return FALSE;
        }
        bitmap.Clear(0);
        for (iChar = 0; iChar < nChars; iChar ++) {
            FXTEXT_GLYPHPOS& glyph = pGlyphAndPos[iChar];
            if (glyph.m_pGlyph == NULL) {
                continue;
            }
            const CFX_DIBitmap* pGlyph = &glyph.m_pGlyph->m_Bitmap;
            bitmap.TransferBitmap(glyph.m_OriginX + glyph.m_pGlyph->m_Left - pixel_left,
                                  glyph.m_OriginY - glyph.m_pGlyph->m_Top - pixel_top,
                                  pGlyph->GetWidth(), pGlyph->GetHeight(), pGlyph, 0, 0);
        }
        FX_Free(pGlyphAndPos);
        return SetBitMask(&bitmap, bmp_rect.left, bmp_rect.top, fill_color);
    }
    CFX_DIBitmap bitmap;
    if (m_bpp == 8) {
        if (!bitmap.Create(pixel_width, pixel_height, FXDIB_8bppMask)) {
            FX_Free(pGlyphAndPos);
            return FALSE;
        }
    } else {
        if (!CreateCompatibleBitmap(&bitmap, pixel_width, pixel_height)) {
            FX_Free(pGlyphAndPos);
            return FALSE;
        }
    }
    if (!bitmap.HasAlpha() && !bitmap.IsAlphaMask()) {
        bitmap.Clear(0xFFFFFFFF);
        if (!GetDIBits(&bitmap, bmp_rect.left, bmp_rect.top)) {
            FX_Free(pGlyphAndPos);
            return FALSE;
        }
    } else {
        bitmap.Clear(0);
        if (bitmap.m_pAlphaMask) {
            bitmap.m_pAlphaMask->Clear(0);
        }
    }
    int dest_width = pixel_width;
    FX_LPBYTE dest_buf = bitmap.GetBuffer();
    int dest_pitch = bitmap.GetPitch();
    int Bpp = bitmap.GetBPP() / 8;
    int a, r, g, b;
    if (anti_alias == FXFT_RENDER_MODE_LCD) {
        _Color2Argb(fill_color, fill_color, alpha_flag | (1 << 24), pIccTransform);
        ArgbDecode(fill_color, a, r, g, b);
        r = FX_GAMMA(r);
        g = FX_GAMMA(g);
        b = FX_GAMMA(b);
    }
    for (iChar = 0; iChar < nChars; iChar ++) {
        FXTEXT_GLYPHPOS& glyph = pGlyphAndPos[iChar];
        if (glyph.m_pGlyph == NULL) {
            continue;
        }
        const CFX_DIBitmap* pGlyph = &glyph.m_pGlyph->m_Bitmap;
        int left = glyph.m_OriginX + glyph.m_pGlyph->m_Left - pixel_left;
        int top = glyph.m_OriginY - glyph.m_pGlyph->m_Top - pixel_top;
        int ncols = pGlyph->GetWidth();
        int nrows = pGlyph->GetHeight();
        if (anti_alias == FXFT_RENDER_MODE_NORMAL) {
            if (!bitmap.CompositeMask(left, top, ncols, nrows, pGlyph, fill_color,
                                      0, 0, FXDIB_BLEND_NORMAL, NULL, FALSE, alpha_flag, pIccTransform)) {
                FX_Free(pGlyphAndPos);
                return FALSE;
            }
            continue;
        }
        FX_BOOL bBGRStripe = text_flags & FXTEXT_BGR_STRIPE;
        ncols /= 3;
        int x_subpixel = (int)(glyph.m_fOriginX * 3) % 3;
        FX_LPBYTE src_buf = pGlyph->GetBuffer();
        int src_pitch = pGlyph->GetPitch();
        int start_col = left;
        if (start_col < 0) {
            start_col = 0;
        }
        int end_col = left + ncols;
        if (end_col > dest_width) {
            end_col = dest_width;
        }
        if (start_col >= end_col) {
            continue;
        }
        if (bitmap.GetFormat() == FXDIB_Argb) {
            for (int row = 0; row < nrows; row ++) {
                int dest_row = row + top;
                if (dest_row < 0 || dest_row >= bitmap.GetHeight()) {
                    continue;
                }
                FX_LPBYTE src_scan = src_buf + row * src_pitch + (start_col - left) * 3;
                FX_LPBYTE dest_scan = dest_buf + dest_row * dest_pitch + (start_col << 2);
                if (bBGRStripe) {
                    if (x_subpixel == 0) {
                        for (int col = start_col; col < end_col; col ++) {
                            int src_alpha = src_scan[2];
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[1];
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[0];
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                    } else if (x_subpixel == 1) {
                        int src_alpha = src_scan[1];
                        ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                        src_alpha = src_alpha * a / 255;
                        dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                        src_alpha = src_scan[0];
                        ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                        src_alpha = src_alpha * a / 255;
                        dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                        if (start_col > left) {
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                        }
                        dest_scan[3] = 255;
                        dest_scan += 4;
                        src_scan += 3;
                        for (int col = start_col + 1; col < end_col - 1; col ++) {
                            int src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                    } else {
                        int src_alpha = src_scan[0];
                        ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                        src_alpha = src_alpha * a / 255;
                        dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                        if (start_col > left) {
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[-2];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                        }
                        dest_scan[3] = 255;
                        dest_scan += 4;
                        src_scan += 3;
                        for (int col = start_col + 1; col < end_col - 1; col ++) {
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[-2];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                    }
                } else {
                    if (x_subpixel == 0) {
                        for (int col = start_col; col < end_col; col ++) {
                            if (bNormal) {
                                int src_alpha1 = (src_scan[0] + src_scan[1] + src_scan[2]) / 3;
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                                src_alpha1 = src_alpha1 * a / 255;
                                FX_BYTE back_alpha = dest_scan[3];
                                if (back_alpha == 0) {
                                    FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_alpha1, r, g, b));
                                    dest_scan += 4;
                                    src_scan += 3;
                                    continue;
                                }
                                if (src_alpha1 == 0) {
                                    dest_scan += 4;
                                    src_scan += 3;
                                    continue;
                                }
                                FX_BYTE dest_alpha = back_alpha + src_alpha1 - back_alpha * src_alpha1 / 255;
                                dest_scan[3] = dest_alpha;
                                int alpha_ratio = src_alpha1 * 255 / dest_alpha;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, alpha_ratio));
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, alpha_ratio));
                                dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, alpha_ratio));
                                dest_scan += 4;
                                src_scan += 3;
                                continue;
                            }
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[2];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                    } else if (x_subpixel == 1) {
                        if (bNormal) {
                            int src_alpha1 = start_col > left ? ((src_scan[-1] + src_scan[0] + src_scan[1]) / 3) : ((src_scan[0] + src_scan[1]) / 3);
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                            src_alpha1 = src_alpha1 * a / 255;
                            if (src_alpha1 == 0) {
                                dest_scan += 4;
                                src_scan += 3;
                            } else {
                                FX_BYTE back_alpha = dest_scan[3];
                                if (back_alpha == 0) {
                                    FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_alpha1, r, g, b));
                                } else {
                                    FX_BYTE dest_alpha = back_alpha + src_alpha1 - back_alpha * src_alpha1 / 255;
                                    dest_scan[3] = dest_alpha;
                                    int alpha_ratio = src_alpha1 * 255 / dest_alpha;
                                    dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, alpha_ratio));
                                    dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, alpha_ratio));
                                    dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, alpha_ratio));
                                }
                                dest_scan += 4;
                                src_scan += 3;
                            }
                        } else {
                            if (start_col > left) {
                                int src_alpha = src_scan[-1];
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                                src_alpha = src_alpha * a / 255;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            }
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                        for (int col = start_col + 1; col < end_col; col ++) {
                            if (bNormal) {
                                int src_alpha1 = (src_scan[-1] + src_scan[0] + src_scan[1]) / 3;
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                                src_alpha1 = src_alpha1 * a / 255;
                                FX_BYTE back_alpha = dest_scan[3];
                                if (back_alpha == 0) {
                                    FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_alpha1, r, g, b));
                                    dest_scan += 4;
                                    src_scan += 3;
                                    continue;
                                }
                                if (src_alpha1 == 0) {
                                    dest_scan += 4;
                                    src_scan += 3;
                                    continue;
                                }
                                FX_BYTE dest_alpha = back_alpha + src_alpha1 - back_alpha * src_alpha1 / 255;
                                dest_scan[3] = dest_alpha;
                                int alpha_ratio = src_alpha1 * 255 / dest_alpha;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, alpha_ratio));
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, alpha_ratio));
                                dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, alpha_ratio));
                                dest_scan += 4;
                                src_scan += 3;
                                continue;
                            }
                            int src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                    } else {
                        if (bNormal) {
                            int src_alpha1 = start_col > left ? ((src_scan[-2] + src_scan[-1] + src_scan[0]) / 3) : ((src_scan[0]) / 3);
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                            src_alpha1 = src_alpha1 * a / 255;
                            if (src_alpha1 == 0) {
                                dest_scan += 4;
                                src_scan += 3;
                            } else {
                                FX_BYTE back_alpha = dest_scan[3];
                                if (back_alpha == 0) {
                                    FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_alpha1, r, g, b));
                                } else {
                                    FX_BYTE dest_alpha = back_alpha + src_alpha1 - back_alpha * src_alpha1 / 255;
                                    dest_scan[3] = dest_alpha;
                                    int alpha_ratio = src_alpha1 * 255 / dest_alpha;
                                    dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, alpha_ratio));
                                    dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, alpha_ratio));
                                    dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, alpha_ratio));
                                }
                                dest_scan += 4;
                                src_scan += 3;
                            }
                        } else {
                            if (start_col > left) {
                                int src_alpha = src_scan[-2];
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                                src_alpha = src_alpha * a / 255;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                                src_alpha = src_scan[-1];
                                ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                                src_alpha = src_alpha * a / 255;
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            }
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                        for (int col = start_col + 1; col < end_col; col ++) {
                            if (bNormal) {
                                int src_alpha1 = (src_scan[-2] + src_scan[-1] + src_scan[0]) / 3;
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                                src_alpha1 = src_alpha1 * a / 255;
                                FX_BYTE back_alpha = dest_scan[3];
                                if (back_alpha == 0) {
                                    FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_alpha1, r, g, b));
                                    dest_scan += 4;
                                    src_scan += 3;
                                    continue;
                                }
                                if (src_alpha1 == 0) {
                                    dest_scan += 4;
                                    src_scan += 3;
                                    continue;
                                }
                                FX_BYTE dest_alpha = back_alpha + src_alpha1 - back_alpha * src_alpha1 / 255;
                                dest_scan[3] = dest_alpha;
                                int alpha_ratio = src_alpha1 * 255 / dest_alpha;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, alpha_ratio));
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, alpha_ratio));
                                dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, alpha_ratio));
                                dest_scan += 4;
                                src_scan += 3;
                                continue;
                            }
                            int src_alpha = src_scan[-2];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan[3] = 255;
                            dest_scan += 4;
                            src_scan += 3;
                        }
                    }
                }
            }
        } else {
            for (int row = 0; row < nrows; row ++) {
                int dest_row = row + top;
                if (dest_row < 0 || dest_row >= bitmap.GetHeight()) {
                    continue;
                }
                FX_LPBYTE src_scan = src_buf + row * src_pitch + (start_col - left) * 3;
                FX_LPBYTE dest_scan = dest_buf + dest_row * dest_pitch + start_col * Bpp;
                if (bBGRStripe) {
                    if (x_subpixel == 0) {
                        for (int col = start_col; col < end_col; col ++) {
                            int src_alpha = src_scan[2];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                    } else if (x_subpixel == 1) {
                        int src_alpha = src_scan[1];
                        ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                        src_alpha = src_alpha * a / 255;
                        dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                        src_alpha = src_scan[0];
                        ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                        src_alpha = src_alpha * a / 255;
                        dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                        if (start_col > left) {
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                        }
                        dest_scan += Bpp;
                        src_scan += 3;
                        for (int col = start_col + 1; col < end_col - 1; col ++) {
                            int src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                    } else {
                        int src_alpha = src_scan[0];
                        ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                        src_alpha = src_alpha * a / 255;
                        dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                        if (start_col > left) {
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[-2];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                        }
                        dest_scan += Bpp;
                        src_scan += 3;
                        for (int col = start_col + 1; col < end_col - 1; col ++) {
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[-2];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                    }
                } else {
                    if (x_subpixel == 0) {
                        for (int col = start_col; col < end_col; col ++) {
                            if (bNormal) {
                                int src_alpha1 = (src_scan[0] + src_scan[1] + src_scan[2]) / 3;
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                                src_alpha1 = src_alpha1 * a / 255;
                                if (src_alpha1 == 0) {
                                    dest_scan += Bpp;
                                    src_scan += 3;
                                    continue;
                                }
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha1));
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha1));
                                dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha1));
                                dest_scan += Bpp;
                                src_scan += 3;
                                continue;
                            }
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[2];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                    } else if (x_subpixel == 1) {
                        if (bNormal) {
                            int src_alpha1 = start_col > left ? (src_scan[0] + src_scan[1] + src_scan[-1]) / 3 : (src_scan[0] + src_scan[1]) / 3;
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                            src_alpha1 = src_alpha1 * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha1));
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha1));
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha1));
                            dest_scan += Bpp;
                            src_scan += 3;
                        } else {
                            if (start_col > left) {
                                int src_alpha = src_scan[-1];
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                                src_alpha = src_alpha * a / 255;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            }
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                        for (int col = start_col + 1; col < end_col; col ++) {
                            if (bNormal) {
                                int src_alpha1 = (src_scan[0] + src_scan[1] + src_scan[-1]) / 3;
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                                src_alpha1 = src_alpha1 * a / 255;
                                if (src_alpha1 == 0) {
                                    dest_scan += Bpp;
                                    src_scan += 3;
                                    continue;
                                }
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha1));
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha1));
                                dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha1));
                                dest_scan += Bpp;
                                src_scan += 3;
                                continue;
                            }
                            int src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[1];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                    } else {
                        if (bNormal) {
                            int src_alpha1 = start_col > left ? (src_scan[0] + src_scan[-2] + src_scan[-1]) / 3 : src_scan[0] / 3;
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                            src_alpha1 = src_alpha1 * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha1));
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha1));
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha1));
                            dest_scan += Bpp;
                            src_scan += 3;
                        } else {
                            if (start_col > left) {
                                int src_alpha = src_scan[-2];
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                                src_alpha = src_alpha * a / 255;
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                                src_alpha = src_scan[-1];
                                ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                                src_alpha = src_alpha * a / 255;
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            }
                            int src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                        for (int col = start_col + 1; col < end_col; col ++) {
                            if (bNormal) {
                                int src_alpha1 = ((int)(src_scan[0]) + (int)(src_scan[-2]) + (int)(src_scan[-1])) / 3;
                                ADJUST_ALPHA(dest_scan[2], r, src_alpha1, nativetext_flags, a);
                                src_alpha1 = src_alpha1 * a / 255;
                                if (src_alpha1 == 0) {
                                    dest_scan += Bpp;
                                    src_scan += 3;
                                    continue;
                                }
                                dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha1));
                                dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha1));
                                dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha1));
                                dest_scan += Bpp;
                                src_scan += 3;
                                continue;
                            }
                            int src_alpha = src_scan[-2];
                            ADJUST_ALPHA(dest_scan[2], r, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[2] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[2]), r, src_alpha));
                            src_alpha = src_scan[-1];
                            ADJUST_ALPHA(dest_scan[1], g, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[1] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[1]), g, src_alpha));
                            src_alpha = src_scan[0];
                            ADJUST_ALPHA(dest_scan[0], b, src_alpha, nativetext_flags, a);
                            src_alpha = src_alpha * a / 255;
                            dest_scan[0] = FX_GAMMA_INVERSE(FXDIB_ALPHA_MERGE(FX_GAMMA(dest_scan[0]), b, src_alpha));
                            dest_scan += Bpp;
                            src_scan += 3;
                        }
                    }
                }
            }
        }
    }
    if (bitmap.IsAlphaMask()) {
        SetBitMask(&bitmap, bmp_rect.left, bmp_rect.top, fill_color, alpha_flag, pIccTransform);
    } else {
        SetDIBits(&bitmap, bmp_rect.left, bmp_rect.top);
    }
    FX_Free(pGlyphAndPos);
    return TRUE;
}
FX_BOOL CFX_RenderDevice::DrawTextPath(int nChars, const FXTEXT_CHARPOS* pCharPos,
                                       CFX_Font* pFont, CFX_FontCache* pCache,
                                       FX_FLOAT font_size, const CFX_AffineMatrix* pText2User,
                                       const CFX_AffineMatrix* pUser2Device, const CFX_GraphStateData* pGraphState,
                                       FX_DWORD fill_color, FX_ARGB stroke_color, CFX_PathData* pClippingPath, int nFlag,
                                       int alpha_flag, void* pIccTransform, int blend_type)
{
    if (pCache == NULL) {
        pCache = CFX_GEModule::Get()->GetFontCache();
    }
    CFX_FaceCache* pFaceCache = pCache->GetCachedFace(pFont);
    FX_FONTCACHE_DEFINE(pCache, pFont);
    for (int iChar = 0; iChar < nChars; iChar ++) {
        const FXTEXT_CHARPOS& charpos = pCharPos[iChar];
        CFX_AffineMatrix matrix;
        if (charpos.m_bGlyphAdjust)
            matrix.Set(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                       charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3], 0, 0);
        matrix.Concat(font_size, 0, 0, font_size, charpos.m_OriginX, charpos.m_OriginY);
        const CFX_PathData* pPath = pFaceCache->LoadGlyphPath(pFont, charpos.m_GlyphIndex, charpos.m_FontCharWidth);
        if (pPath == NULL) {
            continue;
        }
        matrix.Concat(*pText2User);
        CFX_PathData TransformedPath(*pPath);
        TransformedPath.Transform(&matrix);
        FX_BOOL bHasAlpha = FXGETFLAG_COLORTYPE(alpha_flag) ?
                            (FXGETFLAG_ALPHA_FILL(alpha_flag) || FXGETFLAG_ALPHA_STROKE(alpha_flag)) :
                            (fill_color || stroke_color);
        if (bHasAlpha) {
            int fill_mode = nFlag;
            if (FXGETFLAG_COLORTYPE(alpha_flag)) {
                if (FXGETFLAG_ALPHA_FILL(alpha_flag)) {
                    fill_mode |= FXFILL_WINDING;
                }
            } else {
                if (fill_color) {
                    fill_mode |= FXFILL_WINDING;
                }
            }
            fill_mode |= FX_FILL_TEXT_MODE;
            if (!DrawPath(&TransformedPath, pUser2Device, pGraphState, fill_color, stroke_color, fill_mode, alpha_flag, pIccTransform, blend_type)) {
                return FALSE;
            }
        }
        if (pClippingPath) {
            pClippingPath->Append(&TransformedPath, pUser2Device);
        }
    }
    return TRUE;
}
CFX_FontCache::~CFX_FontCache()
{
    FreeCache(TRUE);
}
CFX_FaceCache* CFX_FontCache::GetCachedFace(CFX_Font* pFont)
{
    FX_BOOL bExternal = pFont->GetFace() == NULL;
    void* face = bExternal ? pFont->GetSubstFont()->m_ExtHandle : pFont->GetFace();
    CFX_FTCacheMap& map =  bExternal ? m_ExtFaceMap : m_FTFaceMap;
    CFX_CountedFaceCache* counted_face_cache = NULL;
    if (map.Lookup((FXFT_Face)face, counted_face_cache)) {
        counted_face_cache->m_nCount++;
        return counted_face_cache->m_Obj;
    }
    CFX_FaceCache* face_cache = new CFX_FaceCache(bExternal ? NULL : (FXFT_Face)face);
    counted_face_cache = new CFX_CountedFaceCache;
    counted_face_cache->m_nCount = 2;
    counted_face_cache->m_Obj = face_cache;
    map.SetAt((FXFT_Face)face, counted_face_cache);
    return face_cache;
}
void CFX_FontCache::ReleaseCachedFace(CFX_Font* pFont)
{
    FX_BOOL bExternal = pFont->GetFace() == NULL;
    void* face = bExternal ? pFont->GetSubstFont()->m_ExtHandle : pFont->GetFace();
    CFX_FTCacheMap& map =  bExternal ? m_ExtFaceMap : m_FTFaceMap;
    CFX_CountedFaceCache* counted_face_cache = NULL;
    if (!map.Lookup((FXFT_Face)face, counted_face_cache)) {
        return;
    }
    if (counted_face_cache->m_nCount > 1) {
        counted_face_cache->m_nCount--;
    }
}
void CFX_FontCache::FreeCache(FX_BOOL bRelease)
{
    {
        FX_POSITION pos;
        pos = m_FTFaceMap.GetStartPosition();
        while (pos) {
            FXFT_Face face;
            CFX_CountedFaceCache* cache;
            m_FTFaceMap.GetNextAssoc(pos, face, cache);
            if (bRelease || cache->m_nCount < 2) {
                delete cache->m_Obj;
                delete cache;
                m_FTFaceMap.RemoveKey(face);
            }
        }
        pos = m_ExtFaceMap.GetStartPosition();
        while (pos) {
            FXFT_Face face;
            CFX_CountedFaceCache* cache;
            m_ExtFaceMap.GetNextAssoc(pos, face, cache);
            if (bRelease || cache->m_nCount < 2) {
                delete cache->m_Obj;
                delete cache;
                m_ExtFaceMap.RemoveKey(face);
            }
        }
    }
}
CFX_FaceCache::CFX_FaceCache(FXFT_Face face)
{
    m_Face = face;
    m_pBitmap = NULL;
}
CFX_FaceCache::~CFX_FaceCache()
{
    FX_POSITION pos = m_SizeMap.GetStartPosition();
    CFX_ByteString Key;
    CFX_SizeGlyphCache* pSizeCache = NULL;
    while(pos) {
        m_SizeMap.GetNextAssoc( pos, Key, (void*&)pSizeCache);
        delete pSizeCache;
    }
    m_SizeMap.RemoveAll();
    pos = m_PathMap.GetStartPosition();
    FX_LPVOID key1;
    CFX_PathData* pPath;
    while (pos) {
        m_PathMap.GetNextAssoc(pos, key1, (FX_LPVOID&)pPath);
        delete pPath;
    }
    if (m_pBitmap) {
        delete m_pBitmap;
    }
    m_PathMap.RemoveAll();
}
#if _FXM_PLATFORM_  != _FXM_PLATFORM_APPLE_
void CFX_FaceCache::InitPlatform()
{
}
#endif
CFX_GlyphBitmap* CFX_FaceCache::LookUpGlyphBitmap(CFX_Font* pFont, const CFX_AffineMatrix* pMatrix,
        CFX_ByteStringC& FaceGlyphsKey, FX_DWORD glyph_index, FX_BOOL bFontStyle,
        int dest_width, int anti_alias)
{
    CFX_SizeGlyphCache* pSizeCache = NULL;
    if (!m_SizeMap.Lookup(FaceGlyphsKey, (void*&)pSizeCache)) {
        pSizeCache = new CFX_SizeGlyphCache;
        m_SizeMap.SetAt(FaceGlyphsKey, pSizeCache);
    }
    CFX_GlyphBitmap* pGlyphBitmap = NULL;
    if (pSizeCache->m_GlyphMap.Lookup((FX_LPVOID)(FX_UINTPTR)glyph_index, (void*&)pGlyphBitmap)) {
        return pGlyphBitmap;
    }
    pGlyphBitmap = RenderGlyph(pFont, glyph_index, bFontStyle, pMatrix, dest_width, anti_alias);
    if (pGlyphBitmap == NULL)	{
        return NULL;
    }
    pSizeCache->m_GlyphMap.SetAt((FX_LPVOID)(FX_UINTPTR)glyph_index, pGlyphBitmap);
    return pGlyphBitmap;
}
const CFX_GlyphBitmap* CFX_FaceCache::LoadGlyphBitmap(CFX_Font* pFont, FX_DWORD glyph_index, FX_BOOL bFontStyle, const CFX_AffineMatrix* pMatrix,
        int dest_width, int anti_alias, int& text_flags)
{
    if (glyph_index == (FX_DWORD) - 1) {
        return NULL;
    }
    _CFX_UniqueKeyGen keygen;
#if _FXM_PLATFORM_  != _FXM_PLATFORM_APPLE_
    if (pFont->GetSubstFont())
        keygen.Generate(9, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                        (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias,
                        pFont->GetSubstFont()->m_Weight, pFont->GetSubstFont()->m_ItalicAngle, pFont->IsVertical());
    else
        keygen.Generate(6, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                        (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias);
#else
    if (text_flags & FXTEXT_NO_NATIVETEXT) {
        if (pFont->GetSubstFont())
            keygen.Generate(9, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                            (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias,
                            pFont->GetSubstFont()->m_Weight, pFont->GetSubstFont()->m_ItalicAngle, pFont->IsVertical());
        else
            keygen.Generate(6, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                            (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias);
    } else {
        if (pFont->GetSubstFont())
            keygen.Generate(10, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                            (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias,
                            pFont->GetSubstFont()->m_Weight, pFont->GetSubstFont()->m_ItalicAngle, pFont->IsVertical(), 3);
        else
            keygen.Generate(7, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                            (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias, 3);
    }
#endif
    CFX_ByteStringC FaceGlyphsKey(keygen.m_Key, keygen.m_KeyLen);
#if _FXM_PLATFORM_  != _FXM_PLATFORM_APPLE_
    return LookUpGlyphBitmap(pFont, pMatrix, FaceGlyphsKey, glyph_index, bFontStyle, dest_width, anti_alias);
#else
    if (text_flags & FXTEXT_NO_NATIVETEXT) {
        return LookUpGlyphBitmap(pFont, pMatrix, FaceGlyphsKey, glyph_index, bFontStyle, dest_width, anti_alias);
    } else {
        CFX_GlyphBitmap* pGlyphBitmap;
        CFX_SizeGlyphCache* pSizeCache = NULL;
        if (m_SizeMap.Lookup(FaceGlyphsKey, (void*&)pSizeCache)) {
            if (pSizeCache->m_GlyphMap.Lookup((FX_LPVOID)(FX_UINTPTR)glyph_index, (void*&)pGlyphBitmap)) {
                return pGlyphBitmap;
            }
            pGlyphBitmap = RenderGlyph_Nativetext(pFont, glyph_index, pMatrix, dest_width, anti_alias);
            if (pGlyphBitmap) {
                pSizeCache->m_GlyphMap.SetAt((FX_LPVOID)(FX_UINTPTR)glyph_index, pGlyphBitmap);
                return pGlyphBitmap;
            }
        } else {
            pGlyphBitmap = RenderGlyph_Nativetext(pFont, glyph_index, pMatrix, dest_width, anti_alias);
            if (pGlyphBitmap) {
                pSizeCache = new CFX_SizeGlyphCache;
                m_SizeMap.SetAt(FaceGlyphsKey, pSizeCache);
                pSizeCache->m_GlyphMap.SetAt((FX_LPVOID)(FX_UINTPTR)glyph_index, pGlyphBitmap);
                return pGlyphBitmap;
            }
        }
        if (pFont->GetSubstFont())
            keygen.Generate(9, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                            (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias,
                            pFont->GetSubstFont()->m_Weight, pFont->GetSubstFont()->m_ItalicAngle, pFont->IsVertical());
        else
            keygen.Generate(6, (int)(pMatrix->a * 10000), (int)(pMatrix->b * 10000),
                            (int)(pMatrix->c * 10000), (int)(pMatrix->d * 10000), dest_width, anti_alias);
        CFX_ByteStringC FaceGlyphsKey(keygen.m_Key, keygen.m_KeyLen);
        text_flags |= FXTEXT_NO_NATIVETEXT;
        return LookUpGlyphBitmap(pFont, pMatrix, FaceGlyphsKey, glyph_index, bFontStyle, dest_width, anti_alias);
    }
#endif
}
CFX_SizeGlyphCache::~CFX_SizeGlyphCache()
{
    FX_POSITION pos = m_GlyphMap.GetStartPosition();
    FX_LPVOID Key;
    CFX_GlyphBitmap* pGlyphBitmap = NULL;
    while(pos) {
        m_GlyphMap.GetNextAssoc(pos, Key, (void*&)pGlyphBitmap);
        delete pGlyphBitmap;
    }
    m_GlyphMap.RemoveAll();
}
#define CONTRAST_RAMP_STEP	1
void CFX_Font::AdjustMMParams(int glyph_index, int dest_width, int weight)
{
    FXFT_MM_Var pMasters = NULL;
    FXFT_Get_MM_Var(m_Face, &pMasters);
    if (pMasters == NULL) {
        return;
    }
    long coords[2];
    if (weight == 0) {
        coords[0] = FXFT_Get_MM_Axis_Def(FXFT_Get_MM_Axis(pMasters, 0)) / 65536;
    } else {
        coords[0] = weight;
    }
    if (dest_width == 0) {
        coords[1] = FXFT_Get_MM_Axis_Def(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
    } else {
        int min_param = FXFT_Get_MM_Axis_Min(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
        int max_param = FXFT_Get_MM_Axis_Max(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
        coords[1] = min_param;
        int error = FXFT_Set_MM_Design_Coordinates(m_Face, 2, coords);
        error = FXFT_Load_Glyph(m_Face, glyph_index, FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
        int min_width = FXFT_Get_Glyph_HoriAdvance(m_Face) * 1000 / FXFT_Get_Face_UnitsPerEM(m_Face);
        coords[1] = max_param;
        error = FXFT_Set_MM_Design_Coordinates(m_Face, 2, coords);
        error = FXFT_Load_Glyph(m_Face, glyph_index, FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
        int max_width = FXFT_Get_Glyph_HoriAdvance(m_Face) * 1000 / FXFT_Get_Face_UnitsPerEM(m_Face);
        if (max_width == min_width) {
            return;
        }
        int param = min_param + (max_param - min_param) * (dest_width - min_width) / (max_width - min_width);
        coords[1] = param;
    }
    FXFT_Free(m_Face, pMasters);
    FXFT_Set_MM_Design_Coordinates(m_Face, 2, coords);
}
static const size_t ANGLESKEW_ARRAY_SIZE = 30;
static const char g_AngleSkew[ANGLESKEW_ARRAY_SIZE] = {
    0, 2, 3, 5, 7, 9, 11, 12, 14, 16,
    18, 19, 21, 23, 25, 27, 29, 31, 32, 34,
    36, 38, 40, 42, 45, 47, 49, 51, 53, 55,
};
static const size_t WEIGHTPOW_ARRAY_SIZE = 100;
static const FX_BYTE g_WeightPow[WEIGHTPOW_ARRAY_SIZE] = {
    0, 3, 6, 7, 8, 9, 11, 12, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 35, 36, 36,
    37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42,
    42, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 47, 47,
    47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 50, 51, 51,
    51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53,
};
static const FX_BYTE g_WeightPow_11[WEIGHTPOW_ARRAY_SIZE] = {
    0, 4, 7, 8, 9, 10, 12, 13, 15, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39, 39, 40, 40,
    41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46,
    46, 43, 47, 47, 48, 48, 48, 48, 45, 50, 50, 50, 46, 51, 51, 51, 52, 52,
    52, 52, 53, 53, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55, 55, 55, 56, 56,
    56, 56, 56, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58,
};
static const FX_BYTE g_WeightPow_SHIFTJIS[WEIGHTPOW_ARRAY_SIZE] = {
    0, 0, 1, 2, 3, 4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 21, 22, 24, 26, 28,
    30, 32, 33, 35, 37, 39, 41, 43, 45, 48, 48, 48, 48, 49, 49, 49, 50, 50, 50, 50,
    51, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 55, 55,
    55, 55, 55, 56, 56, 56, 56, 56 , 56, 57, 57, 57 , 57 , 57, 57, 57, 58, 58, 58, 58, 58,
    58, 58, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60,
};
static void _GammaAdjust(FX_LPBYTE pData, int nWid, int nHei, int src_pitch, FX_LPCBYTE gammaTable)
{
    int count = nHei * src_pitch;
    for(int i = 0; i < count; i++) {
        pData[i] = gammaTable[pData[i]];
    }
}
static void _ContrastAdjust(FX_LPBYTE pDataIn, FX_LPBYTE pDataOut, int nWid, int nHei, int nSrcRowBytes, int nDstRowBytes)
{
    int col, row, temp;
    int max = 0, min = 255;
    FX_FLOAT rate;
    for (row = 0; row < nHei; row ++) {
        FX_LPBYTE pRow = pDataIn + row * nSrcRowBytes;
        for (col = 0; col < nWid; col++) {
            temp = *pRow ++;
            if (temp > max) {
                max = temp;
            }
            if (temp < min) {
                min = temp;
            }
        }
    }
    temp = max - min;
    if (0 == temp || 255 == temp) {
        int rowbytes = FXSYS_abs(nSrcRowBytes) > nDstRowBytes ? nDstRowBytes : FXSYS_abs(nSrcRowBytes);
        for (row = 0; row < nHei; row ++) {
            FXSYS_memcpy32(pDataOut + row * nDstRowBytes, pDataIn + row * nSrcRowBytes, rowbytes);
        }
        return;
    }
    rate = 255.f / temp;
    for (row = 0; row < nHei; row ++) {
        FX_LPBYTE pSrcRow = pDataIn + row * nSrcRowBytes;
        FX_LPBYTE pDstRow = pDataOut + row * nDstRowBytes;
        for (col = 0; col < nWid; col ++) {
            temp = (int)((*(pSrcRow++) - min) * rate + 0.5);
            if (temp > 255)	{
                temp = 255;
            } else if (temp < 0) {
                temp = 0;
            }
            *pDstRow ++ = (FX_BYTE)temp;
        }
    }
}
CFX_GlyphBitmap* CFX_FaceCache::RenderGlyph(CFX_Font* pFont, FX_DWORD glyph_index, FX_BOOL bFontStyle,
        const CFX_AffineMatrix* pMatrix, int dest_width, int anti_alias)
{
    if (m_Face == NULL) {
        return NULL;
    }
    FXFT_Matrix  ft_matrix;
    ft_matrix.xx = (signed long)(pMatrix->GetA() / 64 * 65536);
    ft_matrix.xy = (signed long)(pMatrix->GetC() / 64 * 65536);
    ft_matrix.yx = (signed long)(pMatrix->GetB() / 64 * 65536);
    ft_matrix.yy = (signed long)(pMatrix->GetD() / 64 * 65536);
    FX_BOOL bUseCJKSubFont = FALSE;
    const CFX_SubstFont* pSubstFont = pFont->GetSubstFont();
    if (pSubstFont) {
        bUseCJKSubFont = pSubstFont->m_bSubstOfCJK && bFontStyle;
        int skew = 0;
        if (bUseCJKSubFont) {
            skew = pSubstFont->m_bItlicCJK ? -15 : 0;
        } else {
            skew = pSubstFont->m_ItalicAngle;
        }
        if (skew) {
            skew = skew <= -ANGLESKEW_ARRAY_SIZE ? -58 : -g_AngleSkew[-skew];
            if (pFont->IsVertical()) {
                ft_matrix.yx += ft_matrix.yy * skew / 100;
            } else {
                ft_matrix.xy += -ft_matrix.xx * skew / 100;
            }
        }
        if (pSubstFont->m_SubstFlags & FXFONT_SUBST_MM) {
            pFont->AdjustMMParams(glyph_index, dest_width, pFont->GetSubstFont()->m_Weight);
        }
    }
    FXFT_Set_Transform(m_Face, &ft_matrix, 0);
    int load_flags = (m_Face->face_flags & FT_FACE_FLAG_SFNT) ? FXFT_LOAD_NO_BITMAP : (FXFT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING);
    int error = FXFT_Load_Glyph(m_Face, glyph_index, load_flags);
    if (error) {
        //if an error is returned, try to reload glyphs without hinting.
        if (load_flags & FT_LOAD_NO_HINTING || load_flags & FT_LOAD_NO_SCALE) {
            return NULL;
        }

        load_flags |= FT_LOAD_NO_HINTING;
        error = FXFT_Load_Glyph(m_Face, glyph_index, load_flags);

        if (error) { 
            return NULL;
        }
    }
    int weight = 0;
    if (bUseCJKSubFont) {
        weight = pSubstFont->m_WeightCJK;
    } else {
        weight = pSubstFont ? pSubstFont->m_Weight : 0;
    }
    if (pSubstFont && !(pSubstFont->m_SubstFlags & FXFONT_SUBST_MM) && weight > 400) {
        int index = (weight - 400) / 10;
        if (index >= WEIGHTPOW_ARRAY_SIZE) {
            return NULL;
        }
        int level = 0;
        if (pSubstFont->m_Charset == FXFONT_SHIFTJIS_CHARSET) {
            level = g_WeightPow_SHIFTJIS[index] * 2 * (FXSYS_abs((int)(ft_matrix.xx)) + FXSYS_abs((int)(ft_matrix.xy))) / 36655;
        } else {
            level = g_WeightPow_11[index] * (FXSYS_abs((int)(ft_matrix.xx)) + FXSYS_abs((int)(ft_matrix.xy))) / 36655;
        }
        FXFT_Outline_Embolden(FXFT_Get_Glyph_Outline(m_Face), level);
    }
    FXFT_Library_SetLcdFilter(CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary, FT_LCD_FILTER_DEFAULT);
    error = FXFT_Render_Glyph(m_Face, anti_alias);
    if (error) {
        return NULL;
    }
    int bmwidth = FXFT_Get_Bitmap_Width(FXFT_Get_Glyph_Bitmap(m_Face));
    int bmheight = FXFT_Get_Bitmap_Rows(FXFT_Get_Glyph_Bitmap(m_Face));
    if (bmwidth > 2048 || bmheight > 2048) {
        return NULL;
    }
    int dib_width = bmwidth;
    CFX_GlyphBitmap* pGlyphBitmap = new CFX_GlyphBitmap;
    pGlyphBitmap->m_Bitmap.Create(dib_width, bmheight,
                                  anti_alias == FXFT_RENDER_MODE_MONO ? FXDIB_1bppMask : FXDIB_8bppMask);
    pGlyphBitmap->m_Left = FXFT_Get_Glyph_BitmapLeft(m_Face);
    pGlyphBitmap->m_Top = FXFT_Get_Glyph_BitmapTop(m_Face);
    int dest_pitch = pGlyphBitmap->m_Bitmap.GetPitch();
    int src_pitch = FXFT_Get_Bitmap_Pitch(FXFT_Get_Glyph_Bitmap(m_Face));
    FX_BYTE* pDestBuf = pGlyphBitmap->m_Bitmap.GetBuffer();
    FX_BYTE* pSrcBuf = (FX_BYTE*)FXFT_Get_Bitmap_Buffer(FXFT_Get_Glyph_Bitmap(m_Face));
    if (anti_alias != FXFT_RENDER_MODE_MONO && FXFT_Get_Bitmap_PixelMode(FXFT_Get_Glyph_Bitmap(m_Face)) == FXFT_PIXEL_MODE_MONO) {
        int bytes = anti_alias == FXFT_RENDER_MODE_LCD ? 3 : 1;
        for(int i = 0; i < bmheight; i++)
            for(int n = 0; n < bmwidth; n++) {
                FX_BYTE data = (pSrcBuf[i * src_pitch + n / 8] & (0x80 >> (n % 8))) ? 255 : 0;
                for (int b = 0; b < bytes; b ++) {
                    pDestBuf[i * dest_pitch + n * bytes + b] = data;
                }
            }
    } else {
        FXSYS_memset32(pDestBuf, 0, dest_pitch * bmheight);
        if (anti_alias == FXFT_RENDER_MODE_MONO && FXFT_Get_Bitmap_PixelMode(FXFT_Get_Glyph_Bitmap(m_Face)) == FXFT_PIXEL_MODE_MONO) {
            int rowbytes = FXSYS_abs(src_pitch) > dest_pitch ? dest_pitch : FXSYS_abs(src_pitch);
            for (int row = 0; row < bmheight; row ++) {
                FXSYS_memcpy32(pDestBuf + row * dest_pitch, pSrcBuf + row * src_pitch, rowbytes);
            }
        } else {
            _ContrastAdjust(pSrcBuf, pDestBuf, bmwidth, bmheight, src_pitch, dest_pitch);
            _GammaAdjust(pDestBuf, bmwidth, bmheight, dest_pitch, CFX_GEModule::Get()->GetTextGammaTable());
        }
    }
    return pGlyphBitmap;
}
FX_BOOL _OutputGlyph(void* dib, int x, int y, CFX_Font* pFont,
                     int glyph_index, FX_ARGB argb)
{
    CFX_DIBitmap* pDib = (CFX_DIBitmap*)dib;
    FXFT_Face face = pFont->GetFace();
    int error = FXFT_Load_Glyph(face, glyph_index, FXFT_LOAD_NO_BITMAP);
    if (error) {
        return FALSE;
    }
    error = FXFT_Render_Glyph(face, FXFT_RENDER_MODE_NORMAL);
    if (error) {
        return FALSE;
    }
    int bmwidth = FXFT_Get_Bitmap_Width(FXFT_Get_Glyph_Bitmap(face));
    int bmheight = FXFT_Get_Bitmap_Rows(FXFT_Get_Glyph_Bitmap(face));
    int left = FXFT_Get_Glyph_BitmapLeft(face);
    int top = FXFT_Get_Glyph_BitmapTop(face);
    FX_LPCBYTE src_buf = (FX_LPCBYTE)FXFT_Get_Bitmap_Buffer(FXFT_Get_Glyph_Bitmap(face));
    int src_pitch = FXFT_Get_Bitmap_Pitch(FXFT_Get_Glyph_Bitmap(face));
    CFX_DIBitmap mask;
    mask.Create(bmwidth, bmheight, FXDIB_8bppMask);
    FX_LPBYTE dest_buf = mask.GetBuffer();
    int dest_pitch = mask.GetPitch();
    for (int row = 0; row < bmheight; row ++) {
        FX_LPCBYTE src_scan = src_buf + row * src_pitch;
        FX_LPBYTE dest_scan = dest_buf + row * dest_pitch;
        FXSYS_memcpy32(dest_scan, src_scan, dest_pitch);
    }
    pDib->CompositeMask(x + left, y - top, bmwidth, bmheight, &mask, argb, 0, 0);
    return TRUE;
}
FX_BOOL OutputText(void* dib, int x, int y, CFX_Font* pFont, double font_size,
                   CFX_AffineMatrix* pText_matrix, unsigned short const* text, unsigned long argb)
{
    if (!pFont) {
        return FALSE;
    }
    FXFT_Face face = pFont->GetFace();
    FXFT_Select_Charmap(pFont->m_Face, FXFT_ENCODING_UNICODE);
    if (pText_matrix) {
        FXFT_Matrix  ft_matrix;
        ft_matrix.xx = (signed long)(pText_matrix->a / 64 * 65536);
        ft_matrix.xy = (signed long)(pText_matrix->c / 64 * 65536);
        ft_matrix.yx = (signed long)(pText_matrix->b / 64 * 65536);
        ft_matrix.yy = (signed long)(pText_matrix->d / 64 * 65536);
        FXFT_Set_Transform(face, &ft_matrix, 0);
    }
    FX_FLOAT x_pos = 0;
    for (; *text != 0; text ++) {
        FX_WCHAR unicode = *text;
        int glyph_index = FXFT_Get_Char_Index(pFont->m_Face, unicode);
        if (glyph_index <= 0) {
            continue;
        }
        int err = FXFT_Load_Glyph(pFont->m_Face, glyph_index, FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
        if (err) {
            continue;
        }
        int w = FXFT_Get_Glyph_HoriAdvance(pFont->m_Face);
        int em = FXFT_Get_Face_UnitsPerEM(pFont->m_Face);
        FX_FLOAT x1, y1;
        pText_matrix->Transform(x_pos, 0, x1, y1);
        _OutputGlyph(dib, (int)x1 + x, (int) - y1 + y, pFont,
                     glyph_index, argb);
        x_pos += (FX_FLOAT)w / em;
    }
    return TRUE;
}
FX_BOOL OutputGlyph(void* dib, int x, int y, CFX_Font* pFont, double font_size,
                    CFX_AffineMatrix* pMatrix, unsigned long glyph_index, unsigned long argb)
{
    FXFT_Matrix  ft_matrix;
    if (pMatrix) {
        ft_matrix.xx = (signed long)(pMatrix->a * font_size / 64 * 65536);
        ft_matrix.xy = (signed long)(pMatrix->c * font_size / 64 * 65536);
        ft_matrix.yx = (signed long)(pMatrix->b * font_size / 64 * 65536);
        ft_matrix.yy = (signed long)(pMatrix->d * font_size / 64 * 65536);
    } else {
        ft_matrix.xx = (signed long)(font_size / 64 * 65536);
        ft_matrix.xy = ft_matrix.yx = 0;
        ft_matrix.yy = (signed long)(font_size / 64 * 65536);
    }
    FXFT_Set_Transform(pFont->m_Face, &ft_matrix, 0);
    FX_BOOL ret = _OutputGlyph(dib, x, y, pFont,
                               glyph_index, argb);
    return ret;
}
const CFX_PathData* CFX_FaceCache::LoadGlyphPath(CFX_Font* pFont, FX_DWORD glyph_index, int dest_width)
{
    if (m_Face == NULL || glyph_index == (FX_DWORD) - 1) {
        return NULL;
    }
    CFX_PathData* pGlyphPath = NULL;
    FX_LPVOID key;
    if (pFont->GetSubstFont())
        key = (FX_LPVOID)(FX_UINTPTR)(glyph_index + ((pFont->GetSubstFont()->m_Weight / 16) << 15) +
                                      ((pFont->GetSubstFont()->m_ItalicAngle / 2) << 21) + ((dest_width / 16) << 25) +
                                      (pFont->IsVertical() << 31));
    else {
        key = (FX_LPVOID)(FX_UINTPTR)glyph_index;
    }
    if (m_PathMap.Lookup(key, (FX_LPVOID&)pGlyphPath)) {
        return pGlyphPath;
    }
    pGlyphPath = pFont->LoadGlyphPath(glyph_index, dest_width);
    m_PathMap.SetAt(key, pGlyphPath);
    return pGlyphPath;
}
typedef struct {
    FX_BOOL			m_bCount;
    int				m_PointCount;
    FX_PATHPOINT*	m_pPoints;
    int				m_CurX;
    int				m_CurY;
    FX_FLOAT		m_CoordUnit;
} OUTLINE_PARAMS;
void _Outline_CheckEmptyContour(OUTLINE_PARAMS* param)
{
    if (param->m_PointCount >= 2 && param->m_pPoints[param->m_PointCount - 2].m_Flag == FXPT_MOVETO &&
            param->m_pPoints[param->m_PointCount - 2].m_PointX == param->m_pPoints[param->m_PointCount - 1].m_PointX &&
            param->m_pPoints[param->m_PointCount - 2].m_PointY == param->m_pPoints[param->m_PointCount - 1].m_PointY) {
        param->m_PointCount -= 2;
    }
    if (param->m_PointCount >= 4 && param->m_pPoints[param->m_PointCount - 4].m_Flag == FXPT_MOVETO &&
            param->m_pPoints[param->m_PointCount - 3].m_Flag == FXPT_BEZIERTO &&
            param->m_pPoints[param->m_PointCount - 3].m_PointX == param->m_pPoints[param->m_PointCount - 4].m_PointX &&
            param->m_pPoints[param->m_PointCount - 3].m_PointY == param->m_pPoints[param->m_PointCount - 4].m_PointY &&
            param->m_pPoints[param->m_PointCount - 2].m_PointX == param->m_pPoints[param->m_PointCount - 4].m_PointX &&
            param->m_pPoints[param->m_PointCount - 2].m_PointY == param->m_pPoints[param->m_PointCount - 4].m_PointY &&
            param->m_pPoints[param->m_PointCount - 1].m_PointX == param->m_pPoints[param->m_PointCount - 4].m_PointX &&
            param->m_pPoints[param->m_PointCount - 1].m_PointY == param->m_pPoints[param->m_PointCount - 4].m_PointY) {
        param->m_PointCount -= 4;
    }
}
extern "C" {
    static int _Outline_MoveTo(const FXFT_Vector* to, void* user)
    {
        OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
        if (!param->m_bCount) {
            _Outline_CheckEmptyContour(param);
            param->m_pPoints[param->m_PointCount].m_PointX = to->x / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_PointY = to->y / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_Flag = FXPT_MOVETO;
            param->m_CurX = to->x;
            param->m_CurY = to->y;
            if (param->m_PointCount) {
                param->m_pPoints[param->m_PointCount - 1].m_Flag |= FXPT_CLOSEFIGURE;
            }
        }
        param->m_PointCount ++;
        return 0;
    }
};
extern "C" {
    static int _Outline_LineTo(const FXFT_Vector* to, void* user)
    {
        OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
        if (!param->m_bCount) {
            param->m_pPoints[param->m_PointCount].m_PointX = to->x / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_PointY = to->y / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_Flag = FXPT_LINETO;
            param->m_CurX = to->x;
            param->m_CurY = to->y;
        }
        param->m_PointCount ++;
        return 0;
    }
};
extern "C" {
    static int _Outline_ConicTo(const FXFT_Vector* control, const FXFT_Vector* to, void* user)
    {
        OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
        if (!param->m_bCount) {
            param->m_pPoints[param->m_PointCount].m_PointX = (param->m_CurX + (control->x - param->m_CurX) * 2 / 3) / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_PointY = (param->m_CurY + (control->y - param->m_CurY) * 2 / 3) / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_Flag = FXPT_BEZIERTO;
            param->m_pPoints[param->m_PointCount + 1].m_PointX = (control->x + (to->x - control->x) / 3) / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 1].m_PointY = (control->y + (to->y - control->y) / 3) / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 1].m_Flag = FXPT_BEZIERTO;
            param->m_pPoints[param->m_PointCount + 2].m_PointX = to->x / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 2].m_PointY = to->y / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 2].m_Flag = FXPT_BEZIERTO;
            param->m_CurX = to->x;
            param->m_CurY = to->y;
        }
        param->m_PointCount += 3;
        return 0;
    }
};
extern "C" {
    static int _Outline_CubicTo(const FXFT_Vector* control1, const FXFT_Vector* control2, const FXFT_Vector* to, void* user)
    {
        OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
        if (!param->m_bCount) {
            param->m_pPoints[param->m_PointCount].m_PointX = control1->x / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_PointY = control1->y / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount].m_Flag = FXPT_BEZIERTO;
            param->m_pPoints[param->m_PointCount + 1].m_PointX = control2->x / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 1].m_PointY = control2->y / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 1].m_Flag = FXPT_BEZIERTO;
            param->m_pPoints[param->m_PointCount + 2].m_PointX = to->x / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 2].m_PointY = to->y / param->m_CoordUnit;
            param->m_pPoints[param->m_PointCount + 2].m_Flag = FXPT_BEZIERTO;
            param->m_CurX = to->x;
            param->m_CurY = to->y;
        }
        param->m_PointCount += 3;
        return 0;
    }
};
CFX_PathData* CFX_Font::LoadGlyphPath(FX_DWORD glyph_index, int dest_width)
{
    if (m_Face == NULL) {
        return NULL;
    }
    FXFT_Set_Pixel_Sizes(m_Face, 0, 64);
    FXFT_Matrix  ft_matrix = {65536, 0, 0, 65536};
    if (m_pSubstFont) {
        if (m_pSubstFont->m_ItalicAngle) {
            int skew = m_pSubstFont->m_ItalicAngle;
            skew = skew <= -ANGLESKEW_ARRAY_SIZE ? -58 : -g_AngleSkew[-skew];
            if (m_bVertical) {
                ft_matrix.yx += ft_matrix.yy * skew / 100;
            } else {
                ft_matrix.xy += -ft_matrix.xx * skew / 100;
            }
        }
        if (m_pSubstFont->m_SubstFlags & FXFONT_SUBST_MM) {
            AdjustMMParams(glyph_index, dest_width, m_pSubstFont->m_Weight);
        }
    }
    FXFT_Set_Transform(m_Face, &ft_matrix, 0);
    int load_flags = (m_Face->face_flags & FT_FACE_FLAG_SFNT) ? FXFT_LOAD_NO_BITMAP : FXFT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING;
    int error = FXFT_Load_Glyph(m_Face, glyph_index, load_flags);
    if (error) {
        return NULL;
    }
    if (m_pSubstFont && !(m_pSubstFont->m_SubstFlags & FXFONT_SUBST_MM) && m_pSubstFont->m_Weight > 400) {
        int index = (m_pSubstFont->m_Weight - 400) / 10;
        if (index >= WEIGHTPOW_ARRAY_SIZE)
            index = WEIGHTPOW_ARRAY_SIZE - 1;
        int level = 0;
        if (m_pSubstFont->m_Charset == FXFONT_SHIFTJIS_CHARSET) {
            level = g_WeightPow_SHIFTJIS[index] * 2 * 65536 / 36655;
        } else {
            level = g_WeightPow[index] * 2;
        }
        FXFT_Outline_Embolden(FXFT_Get_Glyph_Outline(m_Face), level);
    }
    FXFT_Outline_Funcs funcs;
    funcs.move_to = _Outline_MoveTo;
    funcs.line_to = _Outline_LineTo;
    funcs.conic_to = _Outline_ConicTo;
    funcs.cubic_to = _Outline_CubicTo;
    funcs.shift = 0;
    funcs.delta = 0;
    OUTLINE_PARAMS params;
    params.m_bCount = TRUE;
    params.m_PointCount = 0;
    FXFT_Outline_Decompose(FXFT_Get_Glyph_Outline(m_Face), &funcs, &params);
    if (params.m_PointCount == 0) {
        return NULL;
    }
    CFX_PathData* pPath = new CFX_PathData;
    pPath->SetPointCount(params.m_PointCount);
    params.m_bCount = FALSE;
    params.m_PointCount = 0;
    params.m_pPoints = pPath->GetPoints();
    params.m_CurX = params.m_CurY = 0;
    params.m_CoordUnit = 64 * 64.0;
    FXFT_Outline_Decompose(FXFT_Get_Glyph_Outline(m_Face), &funcs, &params);
    _Outline_CheckEmptyContour(&params);
    pPath->TrimPoints(params.m_PointCount);
    if (params.m_PointCount) {
        pPath->GetPoints()[params.m_PointCount - 1].m_Flag |= FXPT_CLOSEFIGURE;
    }
    return pPath;
}
void _CFX_UniqueKeyGen::Generate(int count, ...)
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
