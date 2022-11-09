// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/apple/fx_quartz_device.h"

#include <CoreGraphics/CoreGraphics.h>

#include "core/fxcrt/fx_extension.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/freetype/fx_freetype.h"

#ifndef CGFLOAT_IS_DOUBLE
#error Expected CGFLOAT_IS_DOUBLE to be defined by CoreGraphics headers
#endif

void* CQuartz2D::CreateGraphics(const RetainPtr<CFX_DIBitmap>& pBitmap) {
  if (!pBitmap)
    return nullptr;
  CGBitmapInfo bmpInfo = kCGBitmapByteOrder32Little;
  switch (pBitmap->GetFormat()) {
    case FXDIB_Format::kRgb32:
      bmpInfo |= kCGImageAlphaNoneSkipFirst;
      break;
    case FXDIB_Format::kArgb:
    default:
      return nullptr;
  }
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(
      pBitmap->GetBuffer().data(), pBitmap->GetWidth(), pBitmap->GetHeight(), 8,
      pBitmap->GetPitch(), colorSpace, bmpInfo);
  CGColorSpaceRelease(colorSpace);
  return context;
}

void CQuartz2D::DestroyGraphics(void* graphics) {
  if (graphics)
    CGContextRelease((CGContextRef)graphics);
}

void* CQuartz2D::CreateFont(pdfium::span<const uint8_t> pFontData) {
  CGDataProviderRef pDataProvider = CGDataProviderCreateWithData(
      nullptr, pFontData.data(), pFontData.size(), nullptr);
  if (!pDataProvider)
    return nullptr;

  CGFontRef pCGFont = CGFontCreateWithDataProvider(pDataProvider);
  CGDataProviderRelease(pDataProvider);
  return pCGFont;
}

void CQuartz2D::DestroyFont(void* pFont) {
  CGFontRelease((CGFontRef)pFont);
}

void CQuartz2D::SetGraphicsTextMatrix(void* graphics,
                                      const CFX_Matrix& matrix) {
  if (!graphics)
    return;
  CGContextRef context = reinterpret_cast<CGContextRef>(graphics);
  CGFloat ty = CGBitmapContextGetHeight(context) - matrix.f;
  CGContextSetTextMatrix(
      context, CGAffineTransformMake(matrix.a, matrix.b, matrix.c, matrix.d,
                                     matrix.e, ty));
}

bool CQuartz2D::DrawGraphicsString(void* graphics,
                                   void* font,
                                   float fontSize,
                                   pdfium::span<uint16_t> glyphIndices,
                                   pdfium::span<CGPoint> glyphPositions,
                                   FX_ARGB argb) {
  if (!graphics)
    return false;

  CGContextRef context = (CGContextRef)graphics;
  CGContextSetFont(context, (CGFontRef)font);
  CGContextSetFontSize(context, fontSize);

  int32_t a;
  int32_t r;
  int32_t g;
  int32_t b;
  std::tie(a, r, g, b) = ArgbDecode(argb);
  CGContextSetRGBFillColor(context, r / 255.f, g / 255.f, b / 255.f, a / 255.f);
  CGContextSaveGState(context);
#if CGFLOAT_IS_DOUBLE
  CGPoint* glyphPositionsCG = new CGPoint[glyphPositions.size()];
  for (size_t index = 0; index < glyphPositions.size(); ++index) {
    glyphPositionsCG[index].x = glyphPositions[index].x;
    glyphPositionsCG[index].y = glyphPositions[index].y;
  }
#else
  CGPoint* glyphPositionsCG = glyphPositions.data();
#endif
  CGContextShowGlyphsAtPositions(
      context, reinterpret_cast<CGGlyph*>(glyphIndices.data()),
      glyphPositionsCG, glyphPositions.size());
#if CGFLOAT_IS_DOUBLE
  delete[] glyphPositionsCG;
#endif
  CGContextRestoreGState(context);
  return true;
}
