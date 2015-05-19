// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "text_int.h"
struct PSGlyph {
    CFX_Font*		m_pFont;
    FX_DWORD		m_GlyphIndex;
    FX_BOOL			m_bGlyphAdjust;
    FX_FLOAT		m_AdjustMatrix[4];
};
class CPSFont 
{
public:
    PSGlyph			m_Glyphs[256];
    int				m_nGlyphs;
};
CFX_PSRenderer::CFX_PSRenderer()
{
    m_pOutput = NULL;
    m_bColorSet = m_bGraphStateSet = FALSE;
    m_bInited = FALSE;
}
CFX_PSRenderer::~CFX_PSRenderer()
{
    for (int i = 0; i < (int)m_PSFontList.GetSize(); i ++) {
        CPSFont* pFont = m_PSFontList[i];
        delete pFont;
    }
}
#define OUTPUT_PS(str) m_pOutput->OutputPS(str, sizeof str-1)
void CFX_PSRenderer::Init(IFX_PSOutput* pOutput, int pslevel, int width, int height, FX_BOOL bCmykOutput)
{
    m_PSLevel = pslevel;
    m_pOutput = pOutput;
    m_ClipBox.left = m_ClipBox.top = 0;
    m_ClipBox.right = width;
    m_ClipBox.bottom = height;
    m_bCmykOutput = bCmykOutput;
}
FX_BOOL CFX_PSRenderer::StartRendering()
{
    if (m_bInited) {
        return TRUE;
    }
    static const char init_str[] = "\nsave\n/im/initmatrix load def\n"
                                   "/n/newpath load def/m/moveto load def/l/lineto load def/c/curveto load def/h/closepath load def\n"
                                   "/f/fill load def/F/eofill load def/s/stroke load def/W/clip load def/W*/eoclip load def\n"
                                   "/rg/setrgbcolor load def/k/setcmykcolor load def\n"
                                   "/J/setlinecap load def/j/setlinejoin load def/w/setlinewidth load def/M/setmiterlimit load def/d/setdash load def\n"
                                   "/q/gsave load def/Q/grestore load def/iM/imagemask load def\n"
                                   "/Tj/show load def/Ff/findfont load def/Fs/scalefont load def/Sf/setfont load def\n"
                                   "/cm/concat load def/Cm/currentmatrix load def/mx/matrix load def/sm/setmatrix load def\n"
                                   ;
    OUTPUT_PS(init_str);
    m_bInited = TRUE;
    return TRUE;
}
void CFX_PSRenderer::EndRendering()
{
    if (m_bInited) {
        OUTPUT_PS("\nrestore\n");
    }
    m_bInited = FALSE;
}
void CFX_PSRenderer::SaveState()
{
    StartRendering();
    OUTPUT_PS("q\n");
    m_ClipBoxStack.Add(m_ClipBox);
}
void CFX_PSRenderer::RestoreState(FX_BOOL bKeepSaved)
{
    StartRendering();
    if (bKeepSaved) {
        OUTPUT_PS("Q\nq\n");
    } else {
        OUTPUT_PS("Q\n");
    }
    m_bColorSet = m_bGraphStateSet = FALSE;
    m_ClipBox = m_ClipBoxStack.GetAt(m_ClipBoxStack.GetSize() - 1);
    if (!bKeepSaved) {
        m_ClipBoxStack.RemoveAt(m_ClipBoxStack.GetSize() - 1);
    }
}
void CFX_PSRenderer::OutputPath(const CFX_PathData* pPathData, const CFX_AffineMatrix* pObject2Device)
{
    int nPoints = pPathData->GetPointCount();
    CFX_ByteTextBuf buf;
    buf.EstimateSize(nPoints * 10);
    for (int i = 0; i < nPoints; i ++) {
        FX_BYTE flag = pPathData->GetFlag(i);
        FX_FLOAT x = pPathData->GetPointX(i);
        FX_FLOAT y = pPathData->GetPointY(i);
        if (pObject2Device) {
            pObject2Device->Transform(x, y);
        }
        buf << x << FX_BSTRC(" ") << y;
        switch (flag & FXPT_TYPE) {
            case FXPT_MOVETO:
                buf << FX_BSTRC(" m ");
                break;
            case FXPT_LINETO:
                if (flag & FXPT_CLOSEFIGURE) {
                    buf << FX_BSTRC(" l h ");
                } else {
                    buf << FX_BSTRC(" l ");
                }
                break;
            case FXPT_BEZIERTO: {
                    FX_FLOAT x1 = pPathData->GetPointX(i + 1);
                    FX_FLOAT x2 = pPathData->GetPointX(i + 2);
                    FX_FLOAT y1 = pPathData->GetPointY(i + 1);
                    FX_FLOAT y2 = pPathData->GetPointY(i + 2);
                    if (pObject2Device) {
                        pObject2Device->Transform(x1, y1);
                        pObject2Device->Transform(x2, y2);
                    }
                    buf << FX_BSTRC(" ") << x1 << FX_BSTRC(" ") << y1 << FX_BSTRC(" ") << x2 << FX_BSTRC(" ") << y2;
                    if (flag & FXPT_CLOSEFIGURE) {
                        buf << FX_BSTRC(" c h\n");
                    } else {
                        buf << FX_BSTRC(" c\n");
                    }
                    i += 2;
                    break;
                }
        }
    }
    m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
}
void CFX_PSRenderer::SetClip_PathFill(const CFX_PathData* pPathData,
                                      const CFX_AffineMatrix* pObject2Device,
                                      int fill_mode
                                     )
{
    StartRendering();
    OutputPath(pPathData, pObject2Device);
    CFX_FloatRect rect = pPathData->GetBoundingBox();
    if (pObject2Device) {
        rect.Transform(pObject2Device);
    }
    m_ClipBox.Intersect(rect.GetOutterRect());
    if ((fill_mode & 3) == FXFILL_WINDING) {
        OUTPUT_PS("W n\n");
    } else {
        OUTPUT_PS("W* n\n");
    }
}
void CFX_PSRenderer::SetClip_PathStroke(const CFX_PathData* pPathData,
                                        const CFX_AffineMatrix* pObject2Device,
                                        const CFX_GraphStateData* pGraphState
                                       )
{
    StartRendering();
    SetGraphState(pGraphState);
    if (pObject2Device) {
        CFX_ByteTextBuf buf;
        buf << FX_BSTRC("mx Cm [") << pObject2Device->a << FX_BSTRC(" ") << pObject2Device->b << FX_BSTRC(" ") <<
            pObject2Device->c << FX_BSTRC(" ") << pObject2Device->d << FX_BSTRC(" ") << pObject2Device->e <<
            FX_BSTRC(" ") << pObject2Device->f << FX_BSTRC("]cm ");
        m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
    }
    OutputPath(pPathData, NULL);
    CFX_FloatRect rect = pPathData->GetBoundingBox(pGraphState->m_LineWidth, pGraphState->m_MiterLimit);
    rect.Transform(pObject2Device);
    m_ClipBox.Intersect(rect.GetOutterRect());
    if (pObject2Device) {
        OUTPUT_PS("strokepath W n sm\n");
    } else {
        OUTPUT_PS("strokepath W n\n");
    }
}
FX_BOOL CFX_PSRenderer::DrawPath(const CFX_PathData* pPathData,
                                 const CFX_AffineMatrix* pObject2Device,
                                 const CFX_GraphStateData* pGraphState,
                                 FX_DWORD fill_color,
                                 FX_DWORD stroke_color,
                                 int fill_mode,
                                 int alpha_flag,
                                 void* pIccTransform
                                )
{
    StartRendering();
    int fill_alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXARGB_A(fill_color);
    int stroke_alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_STROKE(alpha_flag) : FXARGB_A(stroke_color);
    if (fill_alpha && fill_alpha < 255) {
        return FALSE;
    }
    if (stroke_alpha && stroke_alpha < 255) {
        return FALSE;
    }
    if (fill_alpha == 0 && stroke_alpha == 0) {
        return FALSE;
    }
    if (stroke_alpha) {
        SetGraphState(pGraphState);
        if (pObject2Device) {
            CFX_ByteTextBuf buf;
            buf << FX_BSTRC("mx Cm [") << pObject2Device->a << FX_BSTRC(" ") << pObject2Device->b << FX_BSTRC(" ") <<
                pObject2Device->c << FX_BSTRC(" ") << pObject2Device->d << FX_BSTRC(" ") << pObject2Device->e <<
                FX_BSTRC(" ") << pObject2Device->f << FX_BSTRC("]cm ");
            m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
        }
    }
    OutputPath(pPathData, stroke_alpha ? NULL : pObject2Device);
    if (fill_mode && fill_alpha) {
        SetColor(fill_color, alpha_flag, pIccTransform);
        if ((fill_mode & 3) == FXFILL_WINDING) {
            if (stroke_alpha) {
                OUTPUT_PS("q f Q ");
            } else {
                OUTPUT_PS("f");
            }
        } else if ((fill_mode & 3) == FXFILL_ALTERNATE) {
            if (stroke_alpha) {
                OUTPUT_PS("q F Q ");
            } else {
                OUTPUT_PS("F");
            }
        }
    }
    if (stroke_alpha) {
        SetColor(stroke_color, alpha_flag, pIccTransform);
        if (pObject2Device) {
            OUTPUT_PS("s sm");
        } else {
            OUTPUT_PS("s");
        }
    }
    OUTPUT_PS("\n");
    return TRUE;
}
void CFX_PSRenderer::SetGraphState(const CFX_GraphStateData* pGraphState)
{
    CFX_ByteTextBuf buf;
    if (!m_bGraphStateSet || m_CurGraphState.m_LineCap != pGraphState->m_LineCap) {
        buf << pGraphState->m_LineCap << FX_BSTRC(" J\n");
    }
    if (!m_bGraphStateSet || m_CurGraphState.m_DashCount != pGraphState->m_DashCount ||
            FXSYS_memcmp32(m_CurGraphState.m_DashArray, pGraphState->m_DashArray, sizeof(FX_FLOAT)*m_CurGraphState.m_DashCount)) {
        buf << FX_BSTRC("[");
        for (int i = 0; i < pGraphState->m_DashCount; i ++) {
            buf << pGraphState->m_DashArray[i] << FX_BSTRC(" ");
        }
        buf << FX_BSTRC("]") << pGraphState->m_DashPhase << FX_BSTRC(" d\n");
    }
    if (!m_bGraphStateSet || m_CurGraphState.m_LineJoin != pGraphState->m_LineJoin) {
        buf << pGraphState->m_LineJoin << FX_BSTRC(" j\n");
    }
    if (!m_bGraphStateSet || m_CurGraphState.m_LineWidth != pGraphState->m_LineWidth) {
        buf << pGraphState->m_LineWidth << FX_BSTRC(" w\n");
    }
    if (!m_bGraphStateSet || m_CurGraphState.m_MiterLimit != pGraphState->m_MiterLimit) {
        buf << pGraphState->m_MiterLimit << FX_BSTRC(" M\n");
    }
    m_CurGraphState.Copy(*pGraphState);
    m_bGraphStateSet = TRUE;
    if (buf.GetSize()) {
        m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
    }
}
static void FaxCompressData(FX_LPBYTE src_buf, int width, int height, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    CCodec_ModuleMgr* pEncoders = CFX_GEModule::Get()->GetCodecModule();
    if (width * height > 128 && pEncoders && pEncoders->GetFaxModule()->Encode(src_buf, width, height, (width + 7) / 8, dest_buf, dest_size)) {
        FX_Free(src_buf);
    } else {
        dest_buf = src_buf;
        dest_size = (width + 7) / 8 * height;
    }
}
static void PSCompressData(int PSLevel, FX_LPBYTE src_buf, FX_DWORD src_size,
                           FX_LPBYTE& output_buf, FX_DWORD& output_size, FX_LPCSTR& filter)
{
    output_buf = src_buf;
    output_size = src_size;
    filter = "";
    if (src_size < 1024) {
        return;
    }
    CCodec_ModuleMgr* pEncoders = CFX_GEModule::Get()->GetCodecModule();
    FX_LPBYTE dest_buf = NULL;
    FX_DWORD dest_size = src_size;
    if (PSLevel >= 3) {
        if (pEncoders && pEncoders->GetFlateModule()->Encode(src_buf, src_size, dest_buf, dest_size)) {
            filter = "/FlateDecode filter ";
        }
    } else {
        if (pEncoders && pEncoders->GetBasicModule()->RunLengthEncode(src_buf, src_size, dest_buf, dest_size)) {
            filter = "/RunLengthDecode filter ";
        }
    }
    if (dest_size < src_size) {
        output_buf = dest_buf;
        output_size = dest_size;
    } else {
        filter = NULL;
        if (dest_buf) {
            FX_Free(dest_buf);
        }
    }
}
FX_BOOL CFX_PSRenderer::SetDIBits(const CFX_DIBSource* pSource, FX_DWORD color, int left, int top,
                                  int alpha_flag, void* pIccTransform)
{
    StartRendering();
    CFX_AffineMatrix matrix((FX_FLOAT)(pSource->GetWidth()), 0.0f, 0.0f, -(FX_FLOAT)(pSource->GetHeight()),
                            (FX_FLOAT)(left), (FX_FLOAT)(top + pSource->GetHeight()));
    return DrawDIBits(pSource, color, &matrix, 0, alpha_flag, pIccTransform);
}
FX_BOOL CFX_PSRenderer::StretchDIBits(const CFX_DIBSource* pSource, FX_DWORD color, int dest_left, int dest_top,
                                      int dest_width, int dest_height, FX_DWORD flags,
                                      int alpha_flag, void* pIccTransform)
{
    StartRendering();
    CFX_AffineMatrix matrix((FX_FLOAT)(dest_width), 0.0f, 0.0f, (FX_FLOAT)(-dest_height),
                            (FX_FLOAT)(dest_left), (FX_FLOAT)(dest_top + dest_height));
    return DrawDIBits(pSource, color, &matrix, flags, alpha_flag, pIccTransform);
}
FX_BOOL CFX_PSRenderer::DrawDIBits(const CFX_DIBSource* pSource, FX_DWORD color,
                                   const CFX_AffineMatrix* pMatrix, FX_DWORD flags,
                                   int alpha_flag, void* pIccTransform)
{
    StartRendering();
    if ((pMatrix->a == 0 && pMatrix->b == 0) || (pMatrix->c == 0 && pMatrix->d == 0)) {
        return TRUE;
    }
    if (pSource->HasAlpha()) {
        return FALSE;
    }
    int alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_FILL(color) : FXARGB_A(color);
    if (pSource->IsAlphaMask() && (alpha < 255 || pSource->GetBPP() != 1)) {
        return FALSE;
    }
    OUTPUT_PS("q\n");
    CFX_ByteTextBuf buf;
    buf << FX_BSTRC("[") << pMatrix->a << FX_BSTRC(" ") << pMatrix->b << FX_BSTRC(" ") <<
        pMatrix->c << FX_BSTRC(" ") << pMatrix->d << FX_BSTRC(" ") << pMatrix->e <<
        FX_BSTRC(" ") << pMatrix->f << FX_BSTRC("]cm ");
    int width = pSource->GetWidth();
    int height = pSource->GetHeight();
    buf << width << FX_BSTRC(" ") << height;
    if (pSource->GetBPP() == 1 && pSource->GetPalette() == NULL) {
        int pitch = (width + 7) / 8;
        FX_DWORD src_size = height * pitch;
        FX_LPBYTE src_buf = FX_Alloc(FX_BYTE, src_size);
        for (int row = 0; row < height; row ++) {
            FX_LPCBYTE src_scan = pSource->GetScanline(row);
            FXSYS_memcpy32(src_buf + row * pitch, src_scan, pitch);
        }
        FX_LPBYTE output_buf;
        FX_DWORD output_size;
        FaxCompressData(src_buf, width, height, output_buf, output_size);
        if (pSource->IsAlphaMask()) {
            SetColor(color, alpha_flag, pIccTransform);
            m_bColorSet = FALSE;
            buf << FX_BSTRC(" true[");
        } else {
            buf << FX_BSTRC(" 1[");
        }
        buf << width << FX_BSTRC(" 0 0 -") << height << FX_BSTRC(" 0 ") << height <<
            FX_BSTRC("]currentfile/ASCII85Decode filter ");
        if (output_buf != src_buf)
            buf << FX_BSTRC("<</K -1/EndOfBlock false/Columns ") << width << FX_BSTRC("/Rows ") << height <<
                FX_BSTRC(">>/CCITTFaxDecode filter ");
        if (pSource->IsAlphaMask()) {
            buf << FX_BSTRC("iM\n");
        } else {
            buf << FX_BSTRC("false 1 colorimage\n");
        }
        m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
        WritePSBinary(output_buf, output_size);
        FX_Free(output_buf);
    } else {
        CFX_DIBSource* pConverted = (CFX_DIBSource*)pSource;
        if (pIccTransform) {
            FXDIB_Format format = m_bCmykOutput ? FXDIB_Cmyk : FXDIB_Rgb;
            pConverted = pSource->CloneConvert(format, NULL, pIccTransform);
        } else {
            switch (pSource->GetFormat()) {
                case FXDIB_1bppRgb:
                case FXDIB_Rgb32:
                    pConverted = pSource->CloneConvert(FXDIB_Rgb);
                    break;
                case FXDIB_8bppRgb:
                    if (pSource->GetPalette() != NULL) {
                        pConverted = pSource->CloneConvert(FXDIB_Rgb);
                    }
                    break;
                case FXDIB_1bppCmyk:
                    pConverted = pSource->CloneConvert(FXDIB_Cmyk);
                    break;
                case FXDIB_8bppCmyk:
                    if (pSource->GetPalette() != NULL) {
                        pConverted = pSource->CloneConvert(FXDIB_Cmyk);
                    }
                    break;
                default:
                    break;
            }
        }
        if (pConverted == NULL) {
            OUTPUT_PS("\nQ\n");
            return FALSE;
        }
        int Bpp = pConverted->GetBPP() / 8;
        FX_LPBYTE output_buf = NULL;
        FX_STRSIZE output_size = 0;
        FX_LPCSTR filter = NULL;
        if (flags & FXRENDER_IMAGE_LOSSY) {
            CCodec_ModuleMgr* pEncoders = CFX_GEModule::Get()->GetCodecModule();
            if (pEncoders && pEncoders->GetJpegModule()->Encode(pConverted, output_buf, output_size)) {
                filter = "/DCTDecode filter ";
            }
        }
        if (filter == NULL) {
            int src_pitch = width * Bpp;
            output_size = height * src_pitch;
            output_buf = FX_Alloc(FX_BYTE, output_size);
            for (int row = 0; row < height; row ++) {
                FX_LPCBYTE src_scan = pConverted->GetScanline(row);
                FX_LPBYTE dest_scan = output_buf + row * src_pitch;
                if (Bpp == 3) {
                    for (int col = 0; col < width; col ++) {
                        *dest_scan++ = src_scan[2];
                        *dest_scan++ = src_scan[1];
                        *dest_scan++ = *src_scan;
                        src_scan += 3;
                    }
                } else {
                    FXSYS_memcpy32(dest_scan, src_scan, src_pitch);
                }
            }
            FX_LPBYTE compressed_buf;
            FX_DWORD compressed_size;
            PSCompressData(m_PSLevel, output_buf, output_size, compressed_buf, compressed_size, filter);
            if (output_buf != compressed_buf) {
                FX_Free(output_buf);
            }
            output_buf = compressed_buf;
            output_size = compressed_size;
        }
        if (pConverted != pSource) {
            delete pConverted;
            pConverted = NULL;
        }
        buf << FX_BSTRC(" 8[");
        buf << width << FX_BSTRC(" 0 0 -") << height << FX_BSTRC(" 0 ") << height << FX_BSTRC("]");
        buf << FX_BSTRC("currentfile/ASCII85Decode filter ");
        if (filter) {
            buf << filter;
        }
        buf << FX_BSTRC("false ") << Bpp;
        buf << FX_BSTRC(" colorimage\n");
        m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
        WritePSBinary(output_buf, output_size);
        FX_Free(output_buf);
    }
    OUTPUT_PS("\nQ\n");
    return TRUE;
}
void CFX_PSRenderer::SetColor(FX_DWORD color, int alpha_flag, void* pIccTransform)
{
    if (!CFX_GEModule::Get()->GetCodecModule() || !CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
        pIccTransform = NULL;
    }
    FX_BOOL bCMYK = FALSE;
    if (pIccTransform) {
        ICodec_IccModule* pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        color = FXGETFLAG_COLORTYPE(alpha_flag) ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
        FX_LPBYTE pColor = (FX_LPBYTE)&color;
        pIccModule->TranslateScanline(pIccTransform, pColor, pColor, 1);
        color = m_bCmykOutput ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
        bCMYK = m_bCmykOutput;
    } else {
        bCMYK = FXGETFLAG_COLORTYPE(alpha_flag);
    }
    if (bCMYK != m_bCmykOutput || !m_bColorSet || m_LastColor != color) {
        CFX_ByteTextBuf buf;
        if (bCMYK) {
            buf << FXSYS_GetCValue(color) / 255.0 << FX_BSTRC(" ") << FXSYS_GetMValue(color) / 255.0 << FX_BSTRC(" ")
                << FXSYS_GetYValue(color) / 255.0 << FX_BSTRC(" ") << FXSYS_GetKValue(color) / 255.0 << FX_BSTRC(" k\n");
        } else {
            buf << FXARGB_R(color) / 255.0 << FX_BSTRC(" ") << FXARGB_G(color) / 255.0 << FX_BSTRC(" ")
                << FXARGB_B(color) / 255.0 << FX_BSTRC(" rg\n");
        }
        if (bCMYK == m_bCmykOutput) {
            m_bColorSet = TRUE;
            m_LastColor = color;
        }
        m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
    }
}
void CFX_PSRenderer::FindPSFontGlyph(CFX_FaceCache* pFaceCache, CFX_Font* pFont, const FXTEXT_CHARPOS& charpos,
                                     int& ps_fontnum, int &ps_glyphindex)
{
    for (int i = 0; i < (int)m_PSFontList.GetSize(); i ++) {
        CPSFont* pPSFont = m_PSFontList[i];
        for (int j = 0; j < pPSFont->m_nGlyphs; j ++)
            if (pPSFont->m_Glyphs[j].m_pFont == pFont && pPSFont->m_Glyphs[j].m_GlyphIndex == charpos.m_GlyphIndex) {
                if ((!pPSFont->m_Glyphs[j].m_bGlyphAdjust && !charpos.m_bGlyphAdjust) ||
                        (pPSFont->m_Glyphs[j].m_bGlyphAdjust && charpos.m_bGlyphAdjust &&
                         (FXSYS_fabs(pPSFont->m_Glyphs[j].m_AdjustMatrix[0] - charpos.m_AdjustMatrix[0]) < 0.01 &&
                          FXSYS_fabs(pPSFont->m_Glyphs[j].m_AdjustMatrix[1] - charpos.m_AdjustMatrix[1]) < 0.01 &&
                          FXSYS_fabs(pPSFont->m_Glyphs[j].m_AdjustMatrix[2] - charpos.m_AdjustMatrix[2]) < 0.01 &&
                          FXSYS_fabs(pPSFont->m_Glyphs[j].m_AdjustMatrix[3] - charpos.m_AdjustMatrix[3]) < 0.01))) {
                    ps_fontnum = i;
                    ps_glyphindex = j;
                    return;
                }
            }
    }
    if (m_PSFontList.GetSize() == 0 || m_PSFontList[m_PSFontList.GetSize() - 1]->m_nGlyphs == 256) {
        CPSFont* pPSFont = new CPSFont;
        pPSFont->m_nGlyphs = 0;
        m_PSFontList.Add(pPSFont);
        CFX_ByteTextBuf buf;
        buf << FX_BSTRC("8 dict begin/FontType 3 def/FontMatrix[1 0 0 1 0 0]def\n"
                        "/FontBBox[0 0 0 0]def/Encoding 256 array def 0 1 255{Encoding exch/.notdef put}for\n"
                        "/CharProcs 1 dict def CharProcs begin/.notdef {} def end\n"
                        "/BuildGlyph{1 0 -10 -10 10 10 setcachedevice exch/CharProcs get exch 2 copy known not{pop/.notdef}if get exec}bind def\n"
                        "/BuildChar{1 index/Encoding get exch get 1 index/BuildGlyph get exec}bind def\n"
                        "currentdict end\n");
        buf << FX_BSTRC("/X") << m_PSFontList.GetSize() - 1 << FX_BSTRC(" exch definefont pop\n");
        m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
        buf.Clear();
    }
    ps_fontnum = m_PSFontList.GetSize() - 1;
    CPSFont* pPSFont = m_PSFontList[ps_fontnum];
    ps_glyphindex = pPSFont->m_nGlyphs;
    pPSFont->m_Glyphs[ps_glyphindex].m_GlyphIndex = charpos.m_GlyphIndex;
    pPSFont->m_Glyphs[ps_glyphindex].m_pFont = pFont;
    pPSFont->m_Glyphs[ps_glyphindex].m_bGlyphAdjust = charpos.m_bGlyphAdjust;
    if (charpos.m_bGlyphAdjust) {
        pPSFont->m_Glyphs[ps_glyphindex].m_AdjustMatrix[0] = charpos.m_AdjustMatrix[0];
        pPSFont->m_Glyphs[ps_glyphindex].m_AdjustMatrix[1] = charpos.m_AdjustMatrix[1];
        pPSFont->m_Glyphs[ps_glyphindex].m_AdjustMatrix[2] = charpos.m_AdjustMatrix[2];
        pPSFont->m_Glyphs[ps_glyphindex].m_AdjustMatrix[3] = charpos.m_AdjustMatrix[3];
    }
    pPSFont->m_nGlyphs ++;
    CFX_AffineMatrix matrix;
    if (charpos.m_bGlyphAdjust)
        matrix.Set(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                   charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3], 0, 0);
    matrix.Concat(1.0f, 0, 0, 1.0f, 0, 0);
    const CFX_PathData* pPathData = pFaceCache->LoadGlyphPath(pFont, charpos.m_GlyphIndex, charpos.m_FontCharWidth);
    if (pPathData == NULL) {
        return;
    }
    CFX_PathData TransformedPath(*pPathData);
    if (charpos.m_bGlyphAdjust) {
        TransformedPath.Transform(&matrix);
    }
    CFX_ByteTextBuf buf;
    buf << FX_BSTRC("/X") << ps_fontnum << FX_BSTRC(" Ff/CharProcs get begin/")
        << ps_glyphindex << FX_BSTRC("{");
    buf << FX_BSTRC("n ");
    for (int p = 0; p < TransformedPath.GetPointCount(); p ++) {
        FX_FLOAT x = TransformedPath.GetPointX(p), y = TransformedPath.GetPointY(p);
        switch (TransformedPath.GetFlag(p) & FXPT_TYPE) {
            case FXPT_MOVETO: {
                    buf << x << FX_BSTRC(" ") << y << FX_BSTRC(" m\n");
                    break;
                }
            case FXPT_LINETO: {
                    buf << x << FX_BSTRC(" ") << y << FX_BSTRC(" l\n");
                    break;
                }
            case FXPT_BEZIERTO: {
                    buf << x << FX_BSTRC(" ") << y << FX_BSTRC(" ")
                        << TransformedPath.GetPointX(p + 1) << FX_BSTRC(" ")
                        << TransformedPath.GetPointY(p + 1) << FX_BSTRC(" ")
                        << TransformedPath.GetPointX(p + 2) << FX_BSTRC(" ")
                        << TransformedPath.GetPointY(p + 2) << FX_BSTRC(" c\n");
                    p += 2;
                    break;
                }
        }
    }
    buf << FX_BSTRC("f");
    buf << FX_BSTRC("}bind def end\n");
    buf << FX_BSTRC("/X") << ps_fontnum << FX_BSTRC(" Ff/Encoding get ") << ps_glyphindex
        << FX_BSTRC("/") << ps_glyphindex << FX_BSTRC(" put\n");
    m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
}
FX_BOOL CFX_PSRenderer::DrawText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                 CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device,
                                 FX_FLOAT font_size, FX_DWORD color,
                                 int alpha_flag, void* pIccTransform)
{
    StartRendering();
    int alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXARGB_A(color);
    if (alpha < 255) {
        return FALSE;
    }
    if ((pObject2Device->a == 0 && pObject2Device->b == 0) || (pObject2Device->c == 0 && pObject2Device->d == 0)) {
        return TRUE;
    }
    SetColor(color, alpha_flag, pIccTransform);
    CFX_ByteTextBuf buf;
    buf << FX_BSTRC("q[") << pObject2Device->a << FX_BSTRC(" ") << pObject2Device->b << FX_BSTRC(" ")
        << pObject2Device->c << FX_BSTRC(" ") << pObject2Device->d;
    buf << FX_BSTRC(" ") << pObject2Device->e << FX_BSTRC(" ") << pObject2Device->f << "]cm\n";
    if (pCache == NULL) {
        pCache = CFX_GEModule::Get()->GetFontCache();
    }
    CFX_FaceCache* pFaceCache = pCache->GetCachedFace(pFont);
    FX_FONTCACHE_DEFINE(pCache, pFont);
    int last_fontnum = -1;
    for (int i = 0; i < nChars; i ++) {
        int ps_fontnum, ps_glyphindex;
        FindPSFontGlyph(pFaceCache, pFont, pCharPos[i], ps_fontnum, ps_glyphindex);
        if (last_fontnum != ps_fontnum) {
            buf << FX_BSTRC("/X") << ps_fontnum << FX_BSTRC(" Ff ") << font_size
                << FX_BSTRC(" Fs Sf ");
            last_fontnum = ps_fontnum;
        }
        buf << pCharPos[i].m_OriginX << FX_BSTRC(" ")
            << pCharPos[i].m_OriginY << FX_BSTRC(" m");
        CFX_ByteString hex;
        hex.Format("<%02X>", ps_glyphindex);
        buf << hex << FX_BSTRC("Tj\n");
    }
    buf << FX_BSTRC("Q\n");
    m_pOutput->OutputPS((FX_LPCSTR)buf.GetBuffer(), buf.GetSize());
    return TRUE;
}
void CFX_PSRenderer::WritePSBinary(FX_LPCBYTE data, int len)
{
    FX_LPBYTE dest_buf;
    FX_DWORD dest_size;
    CCodec_ModuleMgr* pEncoders = CFX_GEModule::Get()->GetCodecModule();
    if (pEncoders && pEncoders->GetBasicModule()->A85Encode(data, len, dest_buf, dest_size)) {
        m_pOutput->OutputPS((FX_LPCSTR)dest_buf, dest_size);
        FX_Free(dest_buf);
    } else {
        m_pOutput->OutputPS((FX_LPCSTR)data, len);
    }
}
