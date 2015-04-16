// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcrt/fx_ext.h"
#include "../../../include/fxge/fx_ge.h"
#include "../agg/include/fxfx_agg_clip_liang_barsky.h"
#include "../ge/text_int.h"
#include "../dib/dib_int.h"
#include "../agg/include/fx_agg_driver.h"
#include "../../../include/fxge/fx_freetype.h"
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
#include "apple_int.h"
#include "../../../include/fxge/fx_ge_apple.h"
#ifndef CGFLOAT_IS_DOUBLE
#error Expected CGFLOAT_IS_DOUBLE to be defined by CoreGraphics headers
#endif
void* CQuartz2D::createGraphics(CFX_DIBitmap* pBitmap)
{
    if (!pBitmap) {
        return NULL;
    }
    CGBitmapInfo bmpInfo = kCGBitmapByteOrder32Little;
    switch (pBitmap->GetFormat()) {
        case FXDIB_Rgb32:
            bmpInfo |= kCGImageAlphaNoneSkipFirst;
            break;
        case FXDIB_Argb:
        default:
            return NULL;
    }
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pBitmap->GetBuffer(),
                           pBitmap->GetWidth(),
                           pBitmap->GetHeight(),
                           8,
                           pBitmap->GetPitch(),
                           colorSpace,
                           bmpInfo);
    CGColorSpaceRelease(colorSpace);
    return context;
}
void CQuartz2D::destroyGraphics(void* graphics)
{
    if (graphics) {
        CGContextRelease((CGContextRef) graphics);
    }
}
void* CQuartz2D::CreateFont(FX_LPCBYTE pFontData, FX_DWORD dwFontSize)
{
    CGDataProviderRef pDataProvider = CGDataProviderCreateWithData(NULL, pFontData, (size_t)dwFontSize, NULL);
    if (NULL == pDataProvider) {
        return NULL;
    }
    CGFontRef pCGFont = CGFontCreateWithDataProvider(pDataProvider);
    CGDataProviderRelease(pDataProvider);
    return pCGFont;
}
void CQuartz2D::DestroyFont(void* pFont)
{
    CGFontRelease((CGFontRef)pFont);
}
void CQuartz2D::setGraphicsTextMatrix(void* graphics, CFX_AffineMatrix* matrix)
{
    if (!graphics || !matrix) {
        return;
    }
    CGContextRef context = (CGContextRef) graphics;
    CGFloat ty = CGBitmapContextGetHeight(context) - matrix->f;
    CGContextSetTextMatrix(context, CGAffineTransformMake(matrix->a,
                           matrix->b,
                           matrix->c,
                           matrix->d,
                           matrix->e,
                           ty));
}
FX_BOOL CQuartz2D::drawGraphicsString(void*                 graphics,
                                      void*                 font,
                                      FX_FLOAT              fontSize,
                                      FX_WORD*              glyphIndices,
                                      CGPoint*           glyphPositions,
                                      FX_INT32              charsCount,
                                      FX_ARGB               argb,
                                      CFX_AffineMatrix*     matrix )
{
    if (!graphics) {
        return FALSE;
    }
    CGContextRef context = (CGContextRef) graphics;
    CGContextSetFont(context, (CGFontRef)font);
    CGContextSetFontSize(context, fontSize);
    if (matrix) {
        CGAffineTransform m = CGContextGetTextMatrix(context);
        m = CGAffineTransformConcat(m,
                                    CGAffineTransformMake(matrix->a,
                                            matrix->b,
                                            matrix->c,
                                            matrix->d,
                                            matrix->e,
                                            matrix->f));
        CGContextSetTextMatrix(context, m);
    }
    FX_INT32 a, r, g, b;
    ArgbDecode(argb, a, r, g, b);
    CGContextSetRGBFillColor(context,
                             r / 255.f,
                             g / 255.f,
                             b / 255.f,
                             a / 255.f);
    CGContextSaveGState(context);
#if CGFLOAT_IS_DOUBLE
    CGPoint* glyphPositionsCG = new CGPoint[charsCount];
    if (!glyphPositionsCG) {
        return FALSE;
    }
    for (int index = 0; index < charsCount; ++index) {
        glyphPositionsCG[index].x = glyphPositions[index].x;
        glyphPositionsCG[index].y = glyphPositions[index].y;
    }
#else
    CGPoint* glyphPositionsCG = (CGPoint*)glyphPositions;
#endif
    CGContextShowGlyphsAtPositions(context,
                                   (CGGlyph *) glyphIndices,
                                   glyphPositionsCG,
                                   charsCount);
#if CGFLOAT_IS_DOUBLE
    delete[] glyphPositionsCG;
#endif
    CGContextRestoreGState(context);
    return TRUE;
}
void CQuartz2D::saveGraphicsState(void * graphics)
{
    if (graphics) {
        CGContextSaveGState((CGContextRef) graphics);
    }
}
void CQuartz2D::restoreGraphicsState(void * graphics)
{
    if (graphics) {
        CGContextRestoreGState((CGContextRef) graphics);
    }
}
static CGContextRef createContextWithBitmap(CFX_DIBitmap* pBitmap)
{
    if (!pBitmap || pBitmap->IsCmykImage() || pBitmap->GetBPP() < 32) {
        return NULL;
    }
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little;
    if (pBitmap->HasAlpha()) {
        bitmapInfo |= kCGImageAlphaPremultipliedFirst;
    } else {
        bitmapInfo |= kCGImageAlphaNoneSkipFirst;
    }
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pBitmap->GetBuffer(),
                           pBitmap->GetWidth(),
                           pBitmap->GetHeight(),
                           8,
                           pBitmap->GetPitch(),
                           colorSpace,
                           bitmapInfo);
    CGColorSpaceRelease(colorSpace);
    return context;
}
CFX_QuartzDeviceDriver::CFX_QuartzDeviceDriver(CGContextRef context, FX_INT32 deviceClass)
{
    m_saveCount = 0;
    _context		= context;
    _deviceClass	= deviceClass;
    CGContextRetain(_context);
    CGRect r = CGContextGetClipBoundingBox(context);
    _width	= FXSYS_round(r.size.width);
    _height	= FXSYS_round(r.size.height);
    _renderCaps = FXRC_SOFT_CLIP | FXRC_BLEND_MODE |
                  FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE |
                  FXRC_BIT_MASK | FXRC_ALPHA_MASK;
    if (_deviceClass != FXDC_DISPLAY) {
    } else {
        CGImageRef image = CGBitmapContextCreateImage(_context);
        if (image) {
            _renderCaps |= FXRC_GET_BITS;
            _width = CGImageGetWidth(image);
            _height = CGImageGetHeight(image);
            CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(image);
            if (kCGImageAlphaPremultipliedFirst == alphaInfo ||
                    kCGImageAlphaPremultipliedLast == alphaInfo ||
                    kCGImageAlphaOnly == alphaInfo) {
                _renderCaps |= FXRC_ALPHA_OUTPUT;
            }
        }
        CGImageRelease(image);
    }
    CGAffineTransform ctm = CGContextGetCTM(_context);
    CGContextSaveGState(_context);
    m_saveCount++;
    if (ctm.d >= 0) {
        CGFloat offset_x, offset_y;
        offset_x = ctm.tx;
        offset_y = ctm.ty;
        CGContextTranslateCTM(_context, -offset_x, -offset_y);
        CGContextConcatCTM(_context, CGAffineTransformMake(1, 0, 0, -1, offset_x, _height + offset_y));
    }
    _foxitDevice2User = CGAffineTransformIdentity;
    _user2FoxitDevice = CGAffineTransformInvert(_foxitDevice2User);
}
CFX_QuartzDeviceDriver::~CFX_QuartzDeviceDriver()
{
    CGContextRestoreGState(_context);
    m_saveCount--;
    for (int i = 0; i < m_saveCount; ++i) {
        CGContextRestoreGState(_context);
    }
    if (_context) {
        CGContextRelease(_context);
    }
}
int CFX_QuartzDeviceDriver::GetDeviceCaps(int capsID)
{
    switch (capsID) {
        case FXDC_DEVICE_CLASS: {
                return _deviceClass;
            }
        case FXDC_PIXEL_WIDTH: {
                return _width;
            }
        case FXDC_PIXEL_HEIGHT: {
                return _height;
            }
        case FXDC_BITS_PIXEL: {
                return 32;
            }
        case FXDC_RENDER_CAPS: {
                return _renderCaps;
            }
        default: {
                return 0;
            }
    }
}
CFX_Matrix CFX_QuartzDeviceDriver::GetCTM() const
{
    CGAffineTransform ctm = CGContextGetCTM(_context);
    return CFX_Matrix(ctm.a, ctm.b, ctm.c, ctm.d, ctm.tx, ctm.ty);
}
void CFX_QuartzDeviceDriver::SaveState()
{
    CGContextSaveGState(_context);
    m_saveCount++;
}
void CFX_QuartzDeviceDriver::RestoreState(FX_BOOL isKeepSaved )
{
    CGContextRestoreGState(_context);
    if (isKeepSaved) {
        CGContextSaveGState(_context);
    } else {
        m_saveCount--;
    }
}
FX_BOOL CFX_QuartzDeviceDriver::SetClip_PathFill(const CFX_PathData*    pathData,
        const CFX_AffineMatrix*   matrix,
        int                       fillMode )
{
    SaveState();
    CGAffineTransform m = CGAffineTransformIdentity;
    if (matrix) {
        m = CGAffineTransformMake(matrix->GetA(), matrix->GetB(), matrix->GetC(), matrix->GetD(), matrix->GetE(), matrix->GetF());
    }
    m = CGAffineTransformConcat(m, _foxitDevice2User);
    CGContextConcatCTM(_context, m);
    setPathToContext(pathData);
    RestoreState(FALSE);
    if ((fillMode & 3) == FXFILL_WINDING) {
        CGContextClip(_context);
    } else {
        CGContextEOClip(_context);
    }
    return TRUE;
}
FX_FLOAT CFX_QuartzDeviceDriver::getLineWidth(const CFX_GraphStateData * graphState, CGAffineTransform ctm)
{
    FX_FLOAT lineWidth = graphState->m_LineWidth;
    if (graphState->m_LineWidth <= 0.f) {
        CGSize size;
        size.width = 1;
        size.height = 1;
        CGSize temp = CGSizeApplyAffineTransform(size, ctm);
        CGFloat x = 1 / temp.width;
        CGFloat y = 1 / temp.height;
        lineWidth = x > y ? x : y;
    }
    return lineWidth;
}
FX_BOOL CFX_QuartzDeviceDriver::SetClip_PathStroke(const CFX_PathData*      pathData,
        const CFX_AffineMatrix*     matrix,
        const CFX_GraphStateData*   graphState )
{
    SaveState();
    CGAffineTransform m = CGAffineTransformIdentity;
    if (matrix) {
        m = CGAffineTransformMake(matrix->GetA(), matrix->GetB(), matrix->GetC(), matrix->GetD(), matrix->GetE(), matrix->GetF());
    }
    m = CGAffineTransformConcat(m, _foxitDevice2User);
    CGContextConcatCTM(_context, m);
    FX_FLOAT lineWidth = getLineWidth(graphState, m);
    setStrokeInfo(graphState, 0xFF000000, lineWidth);
    setPathToContext(pathData);
    CGContextReplacePathWithStrokedPath(_context);
    RestoreState(FALSE);
    CGContextClip(_context);
    return TRUE;
}
static CGBlendMode GetCGBlendMode(int blend_type)
{
    CGBlendMode mode = kCGBlendModeNormal;
    switch (blend_type) {
        case FXDIB_BLEND_NORMAL:
            mode = kCGBlendModeNormal;
            break;
        case FXDIB_BLEND_MULTIPLY:
            mode = kCGBlendModeMultiply;
            break;
        case FXDIB_BLEND_SCREEN:
            mode = kCGBlendModeScreen;
            break;
        case FXDIB_BLEND_OVERLAY:
            mode = kCGBlendModeOverlay;
            break;
        case FXDIB_BLEND_DARKEN:
            mode = kCGBlendModeDarken;
            break;
        case FXDIB_BLEND_LIGHTEN:
            mode = kCGBlendModeLighten;
            break;
        case FXDIB_BLEND_COLORDODGE:
            mode = kCGBlendModeColorDodge;
            break;
        case FXDIB_BLEND_COLORBURN:
            mode = kCGBlendModeColorBurn;
            break;
        case FXDIB_BLEND_HARDLIGHT:
            mode = kCGBlendModeHardLight;
            break;
        case FXDIB_BLEND_SOFTLIGHT:
            mode = kCGBlendModeSoftLight;
            break;
        case FXDIB_BLEND_DIFFERENCE:
            mode = kCGBlendModeDifference;
            break;
        case FXDIB_BLEND_EXCLUSION:
            mode = kCGBlendModeExclusion;
            break;
        case FXDIB_BLEND_HUE:
            mode = kCGBlendModeHue;
            break;
        case FXDIB_BLEND_SATURATION:
            mode = kCGBlendModeSaturation;
            break;
        case FXDIB_BLEND_COLOR:
            mode = kCGBlendModeColor;
            break;
        case FXDIB_BLEND_LUMINOSITY:
            mode = kCGBlendModeLuminosity;
            break;
        default:
            mode = kCGBlendModeNormal;
            break;
    }
    return mode;
}
FX_BOOL CFX_QuartzDeviceDriver::DrawPath(const CFX_PathData*        pathData,
        const CFX_AffineMatrix*       matrix,
        const CFX_GraphStateData*     graphState,
        FX_DWORD                      fillArgb,
        FX_DWORD                      strokeArgb,
        int                           fillMode,
        int                           alpha_flag,
        void*                         pIccTransform,
        int							blend_type
                                        )
{
    SaveState();
    CGBlendMode mode = GetCGBlendMode(blend_type);
    if (mode != kCGBlendModeNormal) {
        CGContextSetBlendMode(_context, mode);
    }
    CGAffineTransform m = CGAffineTransformIdentity;
    if (matrix) {
        m = CGAffineTransformMake(matrix->GetA(), matrix->GetB(), matrix->GetC(), matrix->GetD(), matrix->GetE(), matrix->GetF());
    }
    m = CGAffineTransformConcat(m, _foxitDevice2User);
    CGContextConcatCTM(_context, m);
    int pathMode = 0;
    if (graphState && strokeArgb) {
        CGContextSetMiterLimit(_context, graphState->m_MiterLimit);
        FX_FLOAT lineWidth = getLineWidth(graphState, m);
        setStrokeInfo(graphState, strokeArgb, lineWidth);
        pathMode |= 4;
    }
    if (fillMode && fillArgb) {
        setFillInfo(fillArgb);
        if ((fillMode & 3) == FXFILL_WINDING) {
            pathMode |= 1;
        } else if ((fillMode & 3) == FXFILL_ALTERNATE) {
            pathMode |= 2;
        }
    }
    setPathToContext(pathData);
    if (fillMode & FXFILL_FULLCOVER) {
        CGContextSetShouldAntialias(_context, false);
    }
    if (pathMode == 4) {
        CGContextStrokePath(_context);
    } else if (pathMode == 1) {
        CGContextFillPath(_context);
    } else if (pathMode == 2) {
        CGContextEOFillPath(_context);
    } else if (pathMode == 5) {
        CGContextDrawPath(_context, kCGPathFillStroke);
    } else if (pathMode == 6) {
        CGContextDrawPath(_context, kCGPathEOFillStroke);
    }
    RestoreState(FALSE);
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::FillRect(const FX_RECT*         rect,
        FX_ARGB                   fillArgb,
        int                       alphaFlag	   ,
        void*                     iccTransform ,
        int						blend_type )
{
    CGBlendMode mode = GetCGBlendMode(blend_type);
    if (mode != kCGBlendModeNormal) {
        CGContextSetBlendMode(_context, mode);
    }
    CGRect rect_fx = CGRectMake(rect->left, rect->top, rect->Width(), rect->Height());
    CGRect rect_usr = CGRectApplyAffineTransform(rect_fx, _foxitDevice2User);
    FX_INT32 a, r, g, b;
    ArgbDecode(fillArgb, a, r, g, b);
    CGContextSetRGBFillColor(_context,
                             r / 255.f,
                             g / 255.f,
                             b / 255.f,
                             a / 255.f);
    CGContextFillRect(_context, rect_usr);
    if (mode != kCGBlendModeNormal) {
        CGContextSetBlendMode(_context, kCGBlendModeNormal);
    }
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::DrawCosmeticLine(FX_FLOAT           x1,
        FX_FLOAT              y1,
        FX_FLOAT              x2,
        FX_FLOAT              y2,
        FX_DWORD              argb,
        int                   alphaFlag       ,
        void*                 iccTransform    ,
        int					blend_type )
{
    CGBlendMode mode = GetCGBlendMode(blend_type);
    if (mode != kCGBlendModeNormal) {
        CGContextSetBlendMode(_context, mode);
    }
    CGPoint pt = CGPointApplyAffineTransform(CGPointMake(x1, y1), _foxitDevice2User);
    x1 = pt.x;
    y1 = pt.y;
    pt = CGPointApplyAffineTransform(CGPointMake(x2, y2), _foxitDevice2User);
    x2 = pt.x;
    y2 = pt.y;
    FX_INT32 a, r, g, b;
    ArgbDecode(argb, a, r, g, b);
    CGContextSetRGBStrokeColor(_context,
                               r / 255.f,
                               g / 255.f,
                               b / 255.f,
                               a / 255.f);
    CGContextMoveToPoint(_context, x1, y1);
    CGContextAddLineToPoint(_context, x2, y2);
    CGContextStrokePath(_context);
    if (mode != kCGBlendModeNormal) {
        CGContextSetBlendMode(_context, kCGBlendModeNormal);
    }
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::GetClipBox(FX_RECT* rect)
{
    CGRect r = CGContextGetClipBoundingBox(_context);
    r = CGRectApplyAffineTransform(r, _user2FoxitDevice);
    rect->left		= FXSYS_floor(r.origin.x);
    rect->top		= FXSYS_floor(r.origin.y);
    rect->right		= FXSYS_ceil(r.origin.x + r.size.width);
    rect->bottom	= FXSYS_ceil(r.origin.y + r.size.height);
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::GetDIBits(CFX_DIBitmap*     bitmap,
        FX_INT32            left,
        FX_INT32            top,
        void* pIccTransform,
        FX_BOOL bDEdge)
{
    if (FXDC_PRINTER == _deviceClass) {
        return FALSE;
    }
    if (bitmap->GetBPP() < 32) {
        return FALSE;
    }
    if (!(_renderCaps | FXRC_GET_BITS)) {
        return FALSE;
    }
    CGPoint pt = CGPointMake(left, top);
    pt = CGPointApplyAffineTransform(pt, _foxitDevice2User);
    CGAffineTransform ctm = CGContextGetCTM(_context);
    pt.x *= FXSYS_fabs(ctm.a);
    pt.y *= FXSYS_fabs(ctm.d);
    CGImageRef image = CGBitmapContextCreateImage(_context);
    if (NULL == image) {
        return FALSE;
    }
    CGFloat width	= (CGFloat) bitmap->GetWidth();
    CGFloat height	= (CGFloat) bitmap->GetHeight();
    if (width + pt.x > _width) {
        width -= (width + pt.x - _width);
    }
    if (height + pt.y > _height) {
        height -= (height + pt.y - _height);
    }
    CGImageRef subImage = CGImageCreateWithImageInRect(image,
                          CGRectMake(pt.x,
                                     pt.y,
                                     width,
                                     height));
    CGContextRef context = createContextWithBitmap(bitmap);
    CGRect rect = CGContextGetClipBoundingBox(context);
    CGContextClearRect(context, rect);
    CGContextDrawImage(context, rect, subImage);
    CGContextRelease(context);
    CGImageRelease(subImage);
    CGImageRelease(image);
    if (bitmap->HasAlpha()) {
        for (int row = 0; row < bitmap->GetHeight(); row ++) {
            FX_LPBYTE pScanline = (FX_LPBYTE)bitmap->GetScanline(row);
            for (int col = 0; col < bitmap->GetWidth(); col ++) {
                if (pScanline[3] > 0) {
                    pScanline[0] = (pScanline[0] * 255.f / pScanline[3] + .5f);
                    pScanline[1] = (pScanline[1] * 255.f / pScanline[3] + .5f);
                    pScanline[2] = (pScanline[2] * 255.f / pScanline[3] + .5f);
                }
                pScanline += 4;
            }
        }
    }
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::SetDIBits(const CFX_DIBSource*      pBitmap,
        FX_ARGB                     argb,
        const FX_RECT*              srcRect,
        int                         dest_left,
        int                         dest_top,
        int                         blendType,
        int                         alphaFlag       ,
        void*                       iccTransform    )
{
    SaveState();
    CGFloat src_left, src_top, src_width, src_height;
    if (srcRect) {
        src_left = srcRect->left;
        src_top = srcRect->top;
        src_width = srcRect->Width();
        src_height = srcRect->Height();
    } else {
        src_left = src_top = 0;
        src_width = pBitmap->GetWidth();
        src_height = pBitmap->GetHeight();
    }
    CGAffineTransform ctm = CGContextGetCTM(_context);
    CGFloat scale_x = FXSYS_fabs(ctm.a);
    CGFloat scale_y = FXSYS_fabs(ctm.d);
    src_left /= scale_x;
    src_top /= scale_y;
    src_width /= scale_x;
    src_height /= scale_y;
    CGRect rect_fx = CGRectMake(dest_left, dest_top, src_width, src_height);
    CGRect rect_usr = CGRectApplyAffineTransform(rect_fx, _foxitDevice2User);
    CGContextBeginPath(_context);
    CGContextAddRect(_context, rect_usr);
    CGContextClip(_context);
    rect_usr.size = CGSizeMake(pBitmap->GetWidth() / scale_x, pBitmap->GetHeight() / scale_y);
    rect_usr = CGRectOffset(rect_usr, -src_left, -src_top);
    CG_SetImageTransform(dest_left, dest_top, src_width, src_height, &rect_usr);
    CFX_DIBitmap* pBitmap1 = NULL;
    if (pBitmap->IsAlphaMask()) {
        if (pBitmap->GetBuffer()) {
            pBitmap1 = (CFX_DIBitmap*)pBitmap;
        } else {
            pBitmap1 = pBitmap->Clone();
        }
        if (NULL == pBitmap1) {
            RestoreState(FALSE);
            return FALSE;
        }
        CGDataProviderRef pBitmapProvider = CGDataProviderCreateWithData(NULL,
                                            pBitmap1->GetBuffer(),
                                            pBitmap1->GetPitch() * pBitmap1->GetHeight(),
                                            NULL);
        CGColorSpaceRef pColorSpace = CGColorSpaceCreateDeviceGray();
        CGBitmapInfo bitmapInfo = kCGImageAlphaNone | kCGBitmapByteOrderDefault;
        CGImageRef pImage = CGImageCreate(pBitmap1->GetWidth(),
                                          pBitmap1->GetHeight(),
                                          pBitmap1->GetBPP(),
                                          pBitmap1->GetBPP(),
                                          pBitmap1->GetPitch(),
                                          pColorSpace,
                                          bitmapInfo,
                                          pBitmapProvider, NULL, true,
                                          kCGRenderingIntentDefault);
        CGContextClipToMask(_context, rect_usr, pImage);
        CGContextSetRGBFillColor(_context,
                                 FXARGB_R(argb) / 255.f,
                                 FXARGB_G(argb) / 255.f,
                                 FXARGB_B(argb) / 255.f,
                                 FXARGB_A(argb) / 255.f);
        CGContextFillRect(_context, rect_usr);
        CGImageRelease(pImage);
        CGColorSpaceRelease(pColorSpace);
        CGDataProviderRelease(pBitmapProvider);
        if (pBitmap1 != pBitmap) {
            delete pBitmap1;
        }
        RestoreState(FALSE);
        return TRUE;
    }
    if (pBitmap->GetBPP() < 32) {
        pBitmap1 = pBitmap->CloneConvert(FXDIB_Rgb32);
    } else {
        if (pBitmap->GetBuffer()) {
            pBitmap1 = (CFX_DIBitmap*)pBitmap;
        } else {
            pBitmap1 = pBitmap->Clone();
        }
    }
    if (NULL == pBitmap1) {
        RestoreState(FALSE);
        return FALSE;
    }
    if (pBitmap1->HasAlpha()) {
        if (pBitmap1 == pBitmap) {
            pBitmap1 = pBitmap->Clone();
            if (!pBitmap1) {
                RestoreState(FALSE);
                return FALSE;
            }
        }
        for (int row = 0; row < pBitmap1->GetHeight(); row ++) {
            FX_LPBYTE pScanline = (FX_LPBYTE)pBitmap1->GetScanline(row);
            for (int col = 0; col < pBitmap1->GetWidth(); col ++) {
                pScanline[0] = (FX_BYTE)(pScanline[0] * pScanline[3] / 255.f + .5f);
                pScanline[1] = (FX_BYTE)(pScanline[1] * pScanline[3] / 255.f + .5f);
                pScanline[2] = (FX_BYTE)(pScanline[2] * pScanline[3] / 255.f + .5f);
                pScanline += 4;
            }
        }
    }
    CGContextRef ctx = createContextWithBitmap(pBitmap1);
    CGImageRef image = CGBitmapContextCreateImage(ctx);
    int blend_mode = blendType;
    if (FXDIB_BLEND_HARDLIGHT == blendType) {
        blend_mode = kCGBlendModeSoftLight;
    } else if (FXDIB_BLEND_SOFTLIGHT == blendType) {
        blend_mode = kCGBlendModeHardLight;
    } else if (blendType >= FXDIB_BLEND_NONSEPARABLE && blendType <= FXDIB_BLEND_LUMINOSITY) {
        blend_mode = blendType - 9;
    } else if (blendType > FXDIB_BLEND_LUMINOSITY || blendType < 0) {
        blend_mode = kCGBlendModeNormal;
    }
    CGContextSetBlendMode(_context, (CGBlendMode)blend_mode);
    CGContextDrawImage(_context, rect_usr, image);
    CGImageRelease(image);
    CGContextRelease(ctx);
    if (pBitmap1 != pBitmap) {
        delete pBitmap1;
    }
    RestoreState(FALSE);
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::StretchDIBits(const CFX_DIBSource*      pBitmap,
        FX_ARGB                     argb,
        int                         dest_left,
        int                         dest_top,
        int                         dest_width,
        int                         dest_height,
        const FX_RECT*              clipRect,
        FX_DWORD                    flags,
        int                         alphaFlag	   ,
        void*                       iccTransform ,
        int							blend_type)
{
    SaveState();
    if (clipRect) {
        CGContextBeginPath(_context);
        CGRect rect_clip = CGRectMake(clipRect->left, clipRect->top, clipRect->Width(), clipRect->Height());
        rect_clip = CGRectApplyAffineTransform(rect_clip, _foxitDevice2User);
        CGContextAddRect(_context, rect_clip);
        CGContextClip(_context);
    }
    CGRect rect = CGRectMake(dest_left, dest_top, dest_width, dest_height);
    rect = CGRectApplyAffineTransform(rect, _foxitDevice2User);
    if (FXDIB_BICUBIC_INTERPOL == flags) {
        CGContextSetInterpolationQuality(_context, kCGInterpolationHigh);
    } else if (FXDIB_DOWNSAMPLE == flags) {
        CGContextSetInterpolationQuality(_context, kCGInterpolationNone);
    } else {
        CGContextSetInterpolationQuality(_context, kCGInterpolationMedium);
    }
    CG_SetImageTransform(dest_left, dest_top, dest_width, dest_height);
    CFX_DIBitmap* pBitmap1 = NULL;
    if (pBitmap->IsAlphaMask()) {
        if (pBitmap->GetBuffer()) {
            pBitmap1 = (CFX_DIBitmap*)pBitmap;
        } else {
            pBitmap1 = pBitmap->Clone();
        }
        if (NULL == pBitmap1) {
            RestoreState(FALSE);
            return FALSE;
        }
        CGDataProviderRef pBitmapProvider = CGDataProviderCreateWithData(NULL,
                                            pBitmap1->GetBuffer(),
                                            pBitmap1->GetPitch() * pBitmap1->GetHeight(),
                                            NULL);
        CGColorSpaceRef pColorSpace = CGColorSpaceCreateDeviceGray();
        CGBitmapInfo bitmapInfo = kCGImageAlphaNone | kCGBitmapByteOrderDefault;
        CGImageRef pImage = CGImageCreate(pBitmap1->GetWidth(),
                                          pBitmap1->GetHeight(),
                                          pBitmap1->GetBPP(),
                                          pBitmap1->GetBPP(),
                                          pBitmap1->GetPitch(),
                                          pColorSpace,
                                          bitmapInfo,
                                          pBitmapProvider, NULL, true,
                                          kCGRenderingIntentDefault);
        CGContextClipToMask(_context, rect, pImage);
        CGContextSetRGBFillColor(_context,
                                 FXARGB_R(argb) / 255.f,
                                 FXARGB_G(argb) / 255.f,
                                 FXARGB_B(argb) / 255.f,
                                 FXARGB_A(argb) / 255.f);
        CGContextFillRect(_context, rect);
        CGImageRelease(pImage);
        CGColorSpaceRelease(pColorSpace);
        CGDataProviderRelease(pBitmapProvider);
        if (pBitmap1 != pBitmap) {
            delete pBitmap1;
        }
        RestoreState(FALSE);
        return TRUE;
    }
    if (pBitmap->GetBPP() < 32) {
        pBitmap1 = pBitmap->CloneConvert(FXDIB_Rgb32);
    } else {
        if (pBitmap->GetBuffer()) {
            pBitmap1 = (CFX_DIBitmap*)pBitmap;
        } else {
            pBitmap1 = pBitmap->Clone();
        }
    }
    if (NULL == pBitmap1) {
        RestoreState(FALSE);
        return FALSE;
    }
    if (pBitmap1->HasAlpha()) {
        if (pBitmap1 == pBitmap) {
            pBitmap1 = pBitmap->Clone();
            if (!pBitmap1) {
                RestoreState(FALSE);
                return FALSE;
            }
        }
        for (int row = 0; row < pBitmap1->GetHeight(); row ++) {
            FX_LPBYTE pScanline = (FX_LPBYTE)pBitmap1->GetScanline(row);
            for (int col = 0; col < pBitmap1->GetWidth(); col ++) {
                pScanline[0] = (FX_BYTE)(pScanline[0] * pScanline[3] / 255.f + .5f);
                pScanline[1] = (FX_BYTE)(pScanline[1] * pScanline[3] / 255.f + .5f);
                pScanline[2] = (FX_BYTE)(pScanline[2] * pScanline[3] / 255.f + .5f);
                pScanline += 4;
            }
        }
    }
    CGContextRef ctx = createContextWithBitmap(pBitmap1);
    CGImageRef image = CGBitmapContextCreateImage(ctx);
    CGContextDrawImage(_context, rect, image);
    CGImageRelease(image);
    CGContextRelease(ctx);
    if (pBitmap1 != pBitmap) {
        delete pBitmap1;
    }
    RestoreState(FALSE);
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::CG_DrawGlypRun(int                        nChars,
        const FXTEXT_CHARPOS*      pCharPos,
        CFX_Font*                  pFont,
        CFX_FontCache*             pCache,
        const CFX_AffineMatrix*    pGlyphMatrix,
        const CFX_AffineMatrix*    pObject2Device,
        FX_FLOAT                   font_size,
        FX_DWORD                   argb,
        int                        alpha_flag,
        void*                      pIccTransform)
{
    if (nChars == 0) {
        return TRUE;
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
        glyph_positions[i].x = pCharPos[i].m_OriginX;
        glyph_positions[i].y = pCharPos[i].m_OriginY;
    }
    CFX_AffineMatrix text_matrix;
    if (pObject2Device) {
        text_matrix.Concat(*pObject2Device);
    }
    CGAffineTransform matrix_cg = CGAffineTransformMake(text_matrix.a,
                                  text_matrix.b,
                                  text_matrix.c,
                                  text_matrix.d,
                                  text_matrix.e,
                                  text_matrix.f);
    matrix_cg = CGAffineTransformConcat(matrix_cg, _foxitDevice2User);
    CGContextSetTextMatrix(_context, matrix_cg);
    CGContextSetFont(_context, (CGFontRef)pFont->m_pPlatformFont);
    CGContextSetFontSize(_context, FXSYS_fabs(font_size));
    FX_INT32 a, r, g, b;
    ArgbDecode(argb, a, r, g, b);
    CGContextSetRGBFillColor(_context,
                             r / 255.f,
                             g / 255.f,
                             b / 255.f,
                             a / 255.f);
    SaveState();
    if (pGlyphMatrix) {
        CGPoint origin = CGPointMake( glyph_positions[0].x,  glyph_positions[0].y);
        origin = CGPointApplyAffineTransform(origin, matrix_cg);
        CGContextTranslateCTM(_context, origin.x, origin.y);
        CGAffineTransform glyph_matrix = CGAffineTransformMake(pGlyphMatrix->a,
                                         pGlyphMatrix->b,
                                         pGlyphMatrix->c,
                                         pGlyphMatrix->d,
                                         pGlyphMatrix->e,
                                         pGlyphMatrix->f);
        if (_foxitDevice2User.d < 0) {
            glyph_matrix = CGAffineTransformInvert(glyph_matrix);
        }
        CGContextConcatCTM(_context, glyph_matrix);
        CGContextTranslateCTM(_context, -origin.x, -origin.y);
    }
    CGContextShowGlyphsAtPositions(_context,
                                   (CGGlyph*)glyph_indices,
                                   glyph_positions,
                                   nChars);
    RestoreState(FALSE);
    return TRUE;
}
FX_BOOL CFX_QuartzDeviceDriver::DrawDeviceText(int                      nChars,
        const FXTEXT_CHARPOS*    pCharPos,
        CFX_Font*                pFont,
        CFX_FontCache*           pCache,
        const CFX_AffineMatrix*  pObject2Device,
        FX_FLOAT                 font_size,
        FX_DWORD                 color,
        int                      alpha_flag       ,
        void*                    pIccTransform)
{
    if (NULL == pFont || NULL == _context) {
        return FALSE;
    }
    FX_BOOL bBold = pFont->IsBold();
    if (!bBold && pFont->GetSubstFont() &&
            pFont->GetSubstFont()->m_Weight >= 500 &&
            pFont->GetSubstFont()->m_Weight <= 600) {
        return FALSE;
    }
    SaveState();
    CGContextSetTextDrawingMode(_context, kCGTextFillClip);
    FX_BOOL ret = FALSE;
    FX_INT32 i = 0;
    while (i < nChars) {
        if (pCharPos[i].m_bGlyphAdjust || font_size < 0) {
            if (i > 0) {
                ret = CG_DrawGlypRun(i, pCharPos, pFont, pCache, NULL, pObject2Device, font_size, color, alpha_flag, pIccTransform);
                if (!ret) {
                    RestoreState(FALSE);
                    return ret;
                }
            }
            const FXTEXT_CHARPOS* char_pos = pCharPos + i;
            CFX_AffineMatrix glphy_matrix;
            if (font_size < 0) {
                glphy_matrix.Concat(-1, 0, 0, -1, 0, 0);
            }
            if (char_pos->m_bGlyphAdjust) {
                glphy_matrix.Concat(char_pos->m_AdjustMatrix[0],
                                    char_pos->m_AdjustMatrix[1],
                                    char_pos->m_AdjustMatrix[2],
                                    char_pos->m_AdjustMatrix[3], 0, 0);
            }
            ret = CG_DrawGlypRun(1, char_pos, pFont, pCache, &glphy_matrix, pObject2Device, font_size, color, alpha_flag, pIccTransform);
            if (!ret) {
                RestoreState(FALSE);
                return ret;
            }
            i ++;
            pCharPos += i;
            nChars -= i;
            i = 0;
        } else {
            i ++;
        }
    }
    if (i > 0) {
        ret = CG_DrawGlypRun(i, pCharPos, pFont, pCache, NULL, pObject2Device, font_size, color, alpha_flag, pIccTransform);
    }
    RestoreState(FALSE);
    return ret;
}
void CFX_QuartzDeviceDriver::setStrokeInfo(const CFX_GraphStateData* graphState, FX_ARGB argb, FX_FLOAT lineWidth)
{
    if (NULL == graphState) {
        return;
    }
    CGContextSetLineWidth(_context, lineWidth);
    CGLineCap cap;
    switch (graphState->m_LineCap) {
        case CFX_GraphStateData::LineCapRound: {
                cap = kCGLineCapRound;
                break;
            }
        case CFX_GraphStateData::LineCapSquare: {
                cap = kCGLineCapSquare;
                break;
            }
        case CFX_GraphStateData::LineCapButt:
        default: {
                cap = kCGLineCapButt;
            }
    }
    CGContextSetLineCap(_context, cap);
    CGLineJoin join;
    switch (graphState->m_LineJoin) {
        case CFX_GraphStateData::LineJoinRound: {
                join = kCGLineJoinRound;
                break;
            }
        case CFX_GraphStateData::LineJoinBevel: {
                join = kCGLineJoinBevel;
                break;
            }
        case CFX_GraphStateData::LineJoinMiter:
        default: {
                join = kCGLineJoinMiter;
            }
    }
    CGContextSetLineJoin(_context, join);
    if (graphState->m_DashCount) {
#if CGFLOAT_IS_DOUBLE
        CGFloat* dashArray = new CGFloat[graphState->m_DashCount];
        if (!dashArray) {
            return;
        }
        for (int index = 0; index < graphState->m_DashCount; ++index) {
            dashArray[index] = graphState->m_DashArray[index];
        }
#else
        CGFloat* dashArray = (CGFloat*)graphState->m_DashArray;
#endif
        CGContextSetLineDash(_context, graphState->m_DashPhase, dashArray, graphState->m_DashCount);
#if CGFLOAT_IS_DOUBLE
        delete[] dashArray;
#endif
    }
    FX_INT32 a, r, g, b;
    ArgbDecode(argb, a, r, g, b);
    CGContextSetRGBStrokeColor(_context,
                               r / 255.f,
                               g / 255.f,
                               b / 255.f,
                               a / 255.f);
}
void CFX_QuartzDeviceDriver::setFillInfo(FX_ARGB argb)
{
    FX_INT32 a, r, g, b;
    ArgbDecode(argb, a, r, g, b);
    CGContextSetRGBFillColor(_context,
                             r / 255.f,
                             g / 255.f,
                             b / 255.f,
                             a / 255.f);
}
void CFX_QuartzDeviceDriver::setPathToContext(const CFX_PathData* pathData)
{
    FX_INT32 count = pathData->GetPointCount();
    FX_PATHPOINT* points = pathData->GetPoints();
    CGContextBeginPath(_context);
    for (FX_INT32 i = 0; i < count; i ++) {
        switch (points[i].m_Flag & FXPT_TYPE) {
            case FXPT_MOVETO:
                CGContextMoveToPoint(_context, points[i].m_PointX, points[i].m_PointY);
                break;
            case FXPT_LINETO:
                CGContextAddLineToPoint(_context, points[i].m_PointX, points[i].m_PointY);
                break;
            case FXPT_BEZIERTO: {
                    CGContextAddCurveToPoint(_context,
                                             points[i].m_PointX, points[i].m_PointY,
                                             points[i + 1].m_PointX, points[i + 1].m_PointY,
                                             points[i + 2].m_PointX, points[i + 2].m_PointY);
                    i += 2;
                }
        }
        if (points[i].m_Flag & FXPT_CLOSEFIGURE) {
            CGContextClosePath(_context);
        }
    }
}
void CFX_QuartzDeviceDriver::CG_SetImageTransform(int dest_left, int dest_top, int dest_width, int dest_height,
        CGRect* rect )
{
    int flip_y = _foxitDevice2User.d * dest_height < 0 ? 1 : -1;
    int flip_x = _foxitDevice2User.a * dest_width > 0 ? 1 : -1;
    if (flip_y < 0 || flip_x < 0) {
        if (dest_height < 0) {
            dest_height = -dest_height;
            dest_top -= dest_height;
        }
        CGRect rt = CGRectApplyAffineTransform(CGRectMake(dest_left, dest_top, dest_width, dest_height), _foxitDevice2User);
        CGFloat offset_x = (rt.origin.x) + rt.size.width / 2.f,
                offset_y = (rt.origin.y) + rt.size.height / 2.f;
        CGAffineTransform transform = CGAffineTransformIdentity;
        transform = CGAffineTransformConcat(transform, CGAffineTransformMake(1, 0, 0, 1, -offset_x, -offset_y));
        transform = CGAffineTransformConcat(transform, CGAffineTransformMake(flip_x, 0, 0, flip_y, 0, 0));
        transform = CGAffineTransformConcat(transform, CGAffineTransformMake(1, 0, 0, 1, offset_x, offset_y));
        CGContextConcatCTM(_context, transform);
        if (rect) {
            *rect = CGRectApplyAffineTransform(*rect, transform);
        }
    }
}
void CFX_QuartzDeviceDriver::ClearDriver()
{
    if (NULL == _context) {
        return;
    }
    for (int i = 0; i < m_saveCount; ++i) {
        CGContextRestoreGState(_context);
    }
    m_saveCount = 0;
    if (_context) {
        CGContextRelease(_context);
    }
}
CFX_QuartzDevice::CFX_QuartzDevice()
{
    m_bOwnedBitmap = FALSE;
    m_pContext = NULL;
}
CFX_QuartzDevice::~CFX_QuartzDevice()
{
    if (m_pContext) {
        CGContextRelease(m_pContext);
    }
    if (GetBitmap() && m_bOwnedBitmap) {
        delete GetBitmap();
    }
}
CGContextRef CFX_QuartzDevice::GetContext()
{
    return m_pContext;
}
FX_BOOL CFX_QuartzDevice::Attach(CGContextRef context, FX_INT32 nDeviceClass)
{
    if (m_pContext) {
        CGContextRelease(m_pContext);
    }
    m_pContext = context;
    CGContextRetain(m_pContext);
    IFX_RenderDeviceDriver* pDriver = new CFX_QuartzDeviceDriver(m_pContext, nDeviceClass);
    SetDeviceDriver(pDriver);
    return TRUE;
}
FX_BOOL CFX_QuartzDevice::Attach(CFX_DIBitmap* pBitmap)
{
    SetBitmap(pBitmap);
    m_pContext = createContextWithBitmap(pBitmap);
    if (NULL == m_pContext) {
        return FALSE;
    }
    IFX_RenderDeviceDriver* pDriver = new CFX_QuartzDeviceDriver(m_pContext, FXDC_DISPLAY);
    SetDeviceDriver(pDriver);
    return TRUE;
}
FX_BOOL CFX_QuartzDevice::Create(FX_INT32 width, FX_INT32 height, FXDIB_Format format)
{
    if ((FX_BYTE)format < 32) {
        return FALSE;
    }
    CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
    if (!pBitmap->Create(width, height, format)) {
        delete pBitmap;
        return FALSE;
    }
    m_bOwnedBitmap = TRUE;
    return Attach(pBitmap);
}
#endif
