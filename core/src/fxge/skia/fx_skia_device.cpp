// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fxge/fx_ge.h"

#if defined(_SKIA_SUPPORT_)
#include "core/include/fxcodec/fx_codec.h"

#include "core/src/fxge/agg/fx_agg_driver.h"
#include "core/src/fxge/skia/fx_skia_device.h"

#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkPaint.h"
#include "SkPath.h"

static SkPath BuildPath(const CFX_PathData* pPathData,
                        const CFX_Matrix* pObject2Device) {
  SkPath skPath;
  const CFX_PathData* pFPath = pPathData;
  int nPoints = pFPath->GetPointCount();
  FX_PATHPOINT* pPoints = pFPath->GetPoints();
  for (int i = 0; i < nPoints; i++) {
    FX_FLOAT x = pPoints[i].m_PointX;
    FX_FLOAT y = pPoints[i].m_PointY;
    if (pObject2Device)
      pObject2Device->Transform(x, y);
    int point_type = pPoints[i].m_Flag & FXPT_TYPE;
    if (point_type == FXPT_MOVETO) {
      skPath.moveTo(x, y);
    } else if (point_type == FXPT_LINETO) {
      skPath.lineTo(x, y);
    } else if (point_type == FXPT_BEZIERTO) {
      FX_FLOAT x2 = pPoints[i + 1].m_PointX, y2 = pPoints[i + 1].m_PointY;
      FX_FLOAT x3 = pPoints[i + 2].m_PointX, y3 = pPoints[i + 2].m_PointY;
      if (pObject2Device) {
        pObject2Device->Transform(x2, y2);
        pObject2Device->Transform(x3, y3);
      }
      skPath.cubicTo(x, y, x2, y2, x3, y3);
      i += 2;
    }
    if (pPoints[i].m_Flag & FXPT_CLOSEFIGURE)
      skPath.close();
  }
  return skPath;
}

// convert a stroking path to scanlines
void CFX_SkiaDeviceDriver::PaintStroke(SkPaint* spaint,
                                       const CFX_GraphStateData* pGraphState) {
  SkPaint::Cap cap;
  switch (pGraphState->m_LineCap) {
    case CFX_GraphStateData::LineCapRound:
      cap = SkPaint::kRound_Cap;
      break;
    case CFX_GraphStateData::LineCapSquare:
      cap = SkPaint::kSquare_Cap;
      break;
    default:
      cap = SkPaint::kButt_Cap;
      break;
  }
  SkPaint::Join join;
  switch (pGraphState->m_LineJoin) {
    case CFX_GraphStateData::LineJoinRound:
      join = SkPaint::kRound_Join;
      break;
    case CFX_GraphStateData::LineJoinBevel:
      join = SkPaint::kBevel_Join;
      break;
    default:
      join = SkPaint::kMiter_Join;
      break;
  }
  FX_FLOAT width = pGraphState->m_LineWidth;

  if (pGraphState->m_DashArray) {
    int count = (pGraphState->m_DashCount + 1) / 2;
    SkScalar* intervals = FX_Alloc2D(SkScalar, count, sizeof(SkScalar));
    // Set dash pattern
    for (int i = 0; i < count; i++) {
      FX_FLOAT on = pGraphState->m_DashArray[i * 2];
      if (on <= 0.000001f)
        on = 1.f / 10;
      FX_FLOAT off = i * 2 + 1 == pGraphState->m_DashCount
                         ? on
                         : pGraphState->m_DashArray[i * 2 + 1];
      if (off < 0)
        off = 0;
      intervals[i * 2] = on;
      intervals[i * 2 + 1] = off;
    }
    spaint
        ->setPathEffect(SkDashPathEffect::Create(intervals, count * 2,
                                                 pGraphState->m_DashPhase))
        ->unref();
  }
  spaint->setStyle(SkPaint::kStroke_Style);
  spaint->setAntiAlias(TRUE);
  spaint->setStrokeWidth(width);
  spaint->setStrokeMiter(pGraphState->m_MiterLimit);
  spaint->setStrokeCap(cap);
  spaint->setStrokeJoin(join);
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(CFX_DIBitmap* pBitmap,
                                           int dither_bits,
                                           FX_BOOL bRgbByteOrder,
                                           CFX_DIBitmap* pOriDevice,
                                           FX_BOOL bGroupKnockout) {
  m_pAggDriver = new CFX_AggDeviceDriver(pBitmap, dither_bits, bRgbByteOrder,
                                         pOriDevice, bGroupKnockout);
  SkBitmap skBitmap;
  const CFX_DIBitmap* bitmap = m_pAggDriver->m_pBitmap;
  SkImageInfo imageInfo =
      SkImageInfo::Make(bitmap->GetWidth(), bitmap->GetHeight(),
                        kN32_SkColorType, kOpaque_SkAlphaType);
  skBitmap.installPixels(imageInfo, bitmap->GetBuffer(), bitmap->GetPitch(),
                         nullptr, /* to do : set color table */
                         nullptr, nullptr);
  m_canvas = new SkCanvas(skBitmap);
}

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
#if 0  // TODO(caryclark) : mismatch on allocator ?
  delete m_canvas;
#endif
  delete m_pAggDriver;
}

FX_BOOL CFX_SkiaDeviceDriver::DrawDeviceText(int nChars,
                                             const FXTEXT_CHARPOS* pCharPos,
                                             CFX_Font* pFont,
                                             CFX_FontCache* pCache,
                                             const CFX_Matrix* pObject2Device,
                                             FX_FLOAT font_size,
                                             FX_DWORD color,
                                             int alpha_flag,
                                             void* pIccTransform) {
  return m_pAggDriver->DrawDeviceText(nChars, pCharPos, pFont, pCache,
                                      pObject2Device, font_size, color,
                                      alpha_flag, pIccTransform);
}

int CFX_SkiaDeviceDriver::GetDeviceCaps(int caps_id) {
  return m_pAggDriver->GetDeviceCaps(caps_id);
}

void CFX_SkiaDeviceDriver::SaveState() {
  m_canvas->save();
  m_pAggDriver->SaveState();
}

void CFX_SkiaDeviceDriver::RestoreState(FX_BOOL bKeepSaved) {
  m_pAggDriver->RestoreState(bKeepSaved);
  m_canvas->restore();
}

void CFX_SkiaDeviceDriver::SetClipMask(
    agg::rasterizer_scanline_aa& rasterizer) {
  m_pAggDriver->SetClipMask(rasterizer);
}

FX_BOOL CFX_SkiaDeviceDriver::SetClip_PathFill(
    const CFX_PathData* pPathData,     // path info
    const CFX_Matrix* pObject2Device,  // optional transformation
    int fill_mode                      // fill mode, WINDING or ALTERNATE
    ) {
  if (!m_pAggDriver->m_pClipRgn) {
    m_pAggDriver->m_pClipRgn = new CFX_ClipRgn(
        GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  }

  if (pPathData->GetPointCount() == 5 || pPathData->GetPointCount() == 4) {
    CFX_FloatRect rectf;
    if (pPathData->IsRect(pObject2Device, &rectf)) {
      rectf.Intersect(
          CFX_FloatRect(0, 0, (FX_FLOAT)GetDeviceCaps(FXDC_PIXEL_WIDTH),
                        (FX_FLOAT)GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
      FX_RECT rect = rectf.GetOutterRect();
      m_pAggDriver->m_pClipRgn->IntersectRect(rect);
      return TRUE;
    }
  }
  SkPath clip = BuildPath(pPathData, pObject2Device);
  clip.setFillType((fill_mode & 3) == FXFILL_WINDING
                       ? SkPath::kWinding_FillType
                       : SkPath::kEvenOdd_FillType);
  const CFX_Matrix& m = *pObject2Device;
#if 0
  // TODO(caryclark) : don't clip quite yet
  // need to understand how to save/restore to balance the clip
  printf("m:(%g,%g,%g) (%g,%g,%g)\n", m.a, m.b, m.c, m.d, m.e, m.f);
  clip.dump();
  SkMatrix skMatrix;
  skMatrix.setAll(m.a, m.b, m.c, m.d, m.e, m.f, 0, 0, 1);
  m_canvas->setMatrix(skMatrix);
  m_canvas->clipPath(clip, SkRegion::kReplace_Op);
#endif

  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::SetClip_PathStroke(
    const CFX_PathData* pPathData,         // path info
    const CFX_Matrix* pObject2Device,      // optional transformation
    const CFX_GraphStateData* pGraphState  // graphic state, for pen attributes
    ) {
  if (!m_pAggDriver->m_pClipRgn) {
    m_pAggDriver->m_pClipRgn = new CFX_ClipRgn(
        GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  }

  // build path data
  SkPath skPath = BuildPath(pPathData, NULL);
  skPath.setFillType(SkPath::kWinding_FillType);

  SkPaint spaint;
  PaintStroke(&spaint, pGraphState);
  SkPath dst_path;
  spaint.getFillPath(skPath, &dst_path);
#if 01
  SkMatrix skMatrix;
  const CFX_Matrix& m = *pObject2Device;
  skMatrix.setAll(m.a, m.b, m.c, m.d, m.e, m.f, 0, 0, 1);
  m_canvas->setMatrix(skMatrix);
  // TODO(caryclark) : don't clip quite yet
  // need to understand how to save/restore so that clip is later undone
  m_canvas->clipPath(dst_path, SkRegion::kReplace_Op);
#endif
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::RenderRasterizer(
    agg::rasterizer_scanline_aa& rasterizer,
    FX_DWORD color,
    FX_BOOL bFullCover,
    FX_BOOL bGroupKnockout,
    int alpha_flag,
    void* pIccTransform) {
  return m_pAggDriver->RenderRasterizer(
      rasterizer, color, bFullCover, bGroupKnockout, alpha_flag, pIccTransform);
}

FX_BOOL CFX_SkiaDeviceDriver::DrawPath(
    const CFX_PathData* pPathData,          // path info
    const CFX_Matrix* pObject2Device,       // optional transformation
    const CFX_GraphStateData* pGraphState,  // graphic state, for pen attributes
    FX_DWORD fill_color,                    // fill color
    FX_DWORD stroke_color,                  // stroke color
    int fill_mode,  // fill mode, WINDING or ALTERNATE. 0 for not filled
    int alpha_flag,
    void* pIccTransform,
    int blend_type) {
  if (!GetBuffer())
    return TRUE;
  SkIRect rect;
  rect.set(0, 0, GetDeviceCaps(FXDC_PIXEL_WIDTH),
           GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  SkPath skPath = BuildPath(pPathData, pObject2Device);
  SkPaint spaint;
  spaint.setAntiAlias(TRUE);
  if ((fill_mode & 3) && fill_color) {
    skPath.setFillType((fill_mode & 3) == FXFILL_WINDING
                           ? SkPath::kWinding_FillType
                           : SkPath::kEvenOdd_FillType);

    spaint.setStyle(SkPaint::kFill_Style);
    spaint.setColor(fill_color);
    m_canvas->drawPath(skPath, spaint);
  }
  int stroke_alpha = FXGETFLAG_COLORTYPE(alpha_flag)
                         ? FXGETFLAG_ALPHA_STROKE(alpha_flag)
                         : FXARGB_A(stroke_color);

  if (pGraphState && stroke_alpha) {
    spaint.setColor(stroke_color);
    PaintStroke(&spaint, pGraphState);
    m_canvas->drawPath(skPath, spaint);
  }

  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::SetPixel(int x,
                                       int y,
                                       FX_DWORD color,
                                       int alpha_flag,
                                       void* pIccTransform) {
  return m_pAggDriver->SetPixel(x, y, color, alpha_flag, pIccTransform);
}

FX_BOOL CFX_SkiaDeviceDriver::FillRect(const FX_RECT* pRect,
                                       FX_DWORD fill_color,
                                       int alpha_flag,
                                       void* pIccTransform,
                                       int blend_type) {
  SkPaint spaint;
  spaint.setAntiAlias(true);
  spaint.setColor(fill_color);

  m_canvas->drawRect(
      SkRect::MakeLTRB(pRect->left, pRect->top, pRect->right, pRect->bottom),
      spaint);
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::GetClipBox(FX_RECT* pRect) {
  return m_pAggDriver->GetClipBox(pRect);
}

FX_BOOL CFX_SkiaDeviceDriver::GetDIBits(CFX_DIBitmap* pBitmap,
                                        int left,
                                        int top,
                                        void* pIccTransform,
                                        FX_BOOL bDEdge) {
  return m_pAggDriver->GetDIBits(pBitmap, left, top, pIccTransform, bDEdge);
}

FX_BOOL CFX_SkiaDeviceDriver::SetDIBits(const CFX_DIBSource* pBitmap,
                                        FX_DWORD argb,
                                        const FX_RECT* pSrcRect,
                                        int left,
                                        int top,
                                        int blend_type,
                                        int alpha_flag,
                                        void* pIccTransform) {
  return m_pAggDriver->SetDIBits(pBitmap, argb, pSrcRect, left, top, blend_type,
                                 alpha_flag, pIccTransform);
}

FX_BOOL CFX_SkiaDeviceDriver::StretchDIBits(const CFX_DIBSource* pSource,
                                            FX_DWORD argb,
                                            int dest_left,
                                            int dest_top,
                                            int dest_width,
                                            int dest_height,
                                            const FX_RECT* pClipRect,
                                            FX_DWORD flags,
                                            int alpha_flag,
                                            void* pIccTransform,
                                            int blend_type) {
  return m_pAggDriver->StretchDIBits(pSource, argb, dest_left, dest_top,
                                     dest_width, dest_height, pClipRect, flags,
                                     alpha_flag, pIccTransform, blend_type);
}

FX_BOOL CFX_SkiaDeviceDriver::StartDIBits(const CFX_DIBSource* pSource,
                                          int bitmap_alpha,
                                          FX_DWORD argb,
                                          const CFX_Matrix* pMatrix,
                                          FX_DWORD render_flags,
                                          void*& handle,
                                          int alpha_flag,
                                          void* pIccTransform,
                                          int blend_type) {
  return m_pAggDriver->StartDIBits(pSource, bitmap_alpha, argb, pMatrix,
                                   render_flags, handle, alpha_flag,
                                   pIccTransform, blend_type);
}

FX_BOOL CFX_SkiaDeviceDriver::ContinueDIBits(void* pHandle, IFX_Pause* pPause) {
  return m_pAggDriver->ContinueDIBits(pHandle, pPause);
}

void CFX_SkiaDeviceDriver::CancelDIBits(void* pHandle) {
  m_pAggDriver->CancelDIBits(pHandle);
}

CFX_SkiaDevice::CFX_SkiaDevice() {
  m_bOwnedBitmap = FALSE;
}

FX_BOOL CFX_SkiaDevice::Attach(CFX_DIBitmap* pBitmap,
                               int dither_bits,
                               FX_BOOL bRgbByteOrder,
                               CFX_DIBitmap* pOriDevice,
                               FX_BOOL bGroupKnockout) {
  if (!pBitmap)
    return FALSE;
  SetBitmap(pBitmap);
  CFX_SkiaDeviceDriver* pDriver = new CFX_SkiaDeviceDriver(
      pBitmap, dither_bits, bRgbByteOrder, pOriDevice, bGroupKnockout);
  SetDeviceDriver(pDriver);
  return TRUE;
}

FX_BOOL CFX_SkiaDevice::Create(int width,
                               int height,
                               FXDIB_Format format,
                               int dither_bits,
                               CFX_DIBitmap* pOriDevice) {
  m_bOwnedBitmap = TRUE;
  CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
  if (!pBitmap->Create(width, height, format)) {
    delete pBitmap;
    return FALSE;
  }
  SetBitmap(pBitmap);
  CFX_SkiaDeviceDriver* pDriver =
      new CFX_SkiaDeviceDriver(pBitmap, dither_bits, FALSE, pOriDevice, FALSE);
  SetDeviceDriver(pDriver);
  return TRUE;
}

CFX_SkiaDevice::~CFX_SkiaDevice() {
  if (m_bOwnedBitmap && GetBitmap())
    delete GetBitmap();
}

#if 0
#include <stdarg.h>
#include <stdio.h>

void SkDebugf(const char format[], ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

#endif

#endif
