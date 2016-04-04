// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fxge/fx_ge.h"

#if defined(_SKIA_SUPPORT_)
#include "core/include/fxcodec/fx_codec.h"

#include "core/fpdfapi/fpdf_page/cpdf_shadingpattern.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fxge/agg/fx_agg_driver.h"
#include "core/fxge/skia/fx_skia_device.h"

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "third_party/skia/include/effects/SkDashPathEffect.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/pathops/SkPathOps.h"

namespace {

#define SHOW_SKIA_PATH 0  // set to 1 to print the path contents
#define DRAW_SKIA_CLIP 0  // set to 1 to draw a green rectangle around the clip

void DebugShowSkiaPath(const SkPath& path) {
#if SHOW_SKIA_PATH
  char buffer[4096];
  sk_bzero(buffer, sizeof(buffer));
  SkMemoryWStream stream(buffer, sizeof(buffer));
  path.dump(&stream, false, false);
  printf("%s\n", buffer);
#endif  // SHOW_SKIA_PATH
}

void DebugShowCanvasMatrix(const SkCanvas* canvas) {
#if SHOW_SKIA_PATH
  SkMatrix matrix = canvas->getTotalMatrix();
  SkScalar m[9];
  matrix.get9(m);
  printf("(%g,%g,%g) (%g,%g,%g) (%g,%g,%g)\n", m[0], m[1], m[2], m[3], m[4],
         m[5], m[6], m[7], m[8]);
#endif  // SHOW_SKIA_PATH
}

#if DRAW_SKIA_CLIP

SkPaint DebugClipPaint() {
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(SK_ColorGREEN);
  paint.setStyle(SkPaint::kStroke_Style);
  return paint;
}

void DebugDrawSkiaClipRect(SkCanvas* canvas, const SkRect& rect) {
  SkPaint paint = DebugClipPaint();
  canvas->drawRect(rect, paint);
}

void DebugDrawSkiaClipPath(SkCanvas* canvas, const SkPath& path) {
  SkPaint paint = DebugClipPaint();
  canvas->drawPath(path, paint);
}

#else  // DRAW_SKIA_CLIP

void DebugDrawSkiaClipRect(SkCanvas* canvas, const SkRect& rect) {}
void DebugDrawSkiaClipPath(SkCanvas* canvas, const SkPath& path) {}

#endif  // DRAW_SKIA_CLIP

#undef SHOW_SKIA_PATH
#undef DRAW_SKIA_CLIP

SkPath BuildPath(const CFX_PathData* pPathData) {
  SkPath skPath;
  const CFX_PathData* pFPath = pPathData;
  int nPoints = pFPath->GetPointCount();
  FX_PATHPOINT* pPoints = pFPath->GetPoints();
  for (int i = 0; i < nPoints; i++) {
    FX_FLOAT x = pPoints[i].m_PointX;
    FX_FLOAT y = pPoints[i].m_PointY;
    int point_type = pPoints[i].m_Flag & FXPT_TYPE;
    if (point_type == FXPT_MOVETO) {
      skPath.moveTo(x, y);
    } else if (point_type == FXPT_LINETO) {
      skPath.lineTo(x, y);
    } else if (point_type == FXPT_BEZIERTO) {
      FX_FLOAT x2 = pPoints[i + 1].m_PointX, y2 = pPoints[i + 1].m_PointY;
      FX_FLOAT x3 = pPoints[i + 2].m_PointX, y3 = pPoints[i + 2].m_PointY;
      skPath.cubicTo(x, y, x2, y2, x3, y3);
      i += 2;
    }
    if (pPoints[i].m_Flag & FXPT_CLOSEFIGURE)
      skPath.close();
  }
  return skPath;
}

SkMatrix ToSkMatrix(const CFX_Matrix& m) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a, m.b, m.e, m.c, m.d, m.f, 0, 0, 1);
  return skMatrix;
}

// use when pdf's y-axis points up insead of down
SkMatrix ToFlippedSkMatrix(const CFX_Matrix& m) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a, m.b, m.e, -m.c, -m.d, m.f, 0, 0, 1);
  return skMatrix;
}

SkXfermode::Mode GetSkiaBlendMode(int blend_type) {
  switch (blend_type) {
    case FXDIB_BLEND_MULTIPLY:
      return SkXfermode::kMultiply_Mode;
    case FXDIB_BLEND_SCREEN:
      return SkXfermode::kScreen_Mode;
    case FXDIB_BLEND_OVERLAY:
      return SkXfermode::kOverlay_Mode;
    case FXDIB_BLEND_DARKEN:
      return SkXfermode::kDarken_Mode;
    case FXDIB_BLEND_LIGHTEN:
      return SkXfermode::kLighten_Mode;
    case FXDIB_BLEND_COLORDODGE:
      return SkXfermode::kColorDodge_Mode;
    case FXDIB_BLEND_COLORBURN:
      return SkXfermode::kColorBurn_Mode;
    case FXDIB_BLEND_HARDLIGHT:
      return SkXfermode::kHardLight_Mode;
    case FXDIB_BLEND_SOFTLIGHT:
      return SkXfermode::kSoftLight_Mode;
    case FXDIB_BLEND_DIFFERENCE:
      return SkXfermode::kDifference_Mode;
    case FXDIB_BLEND_EXCLUSION:
      return SkXfermode::kExclusion_Mode;
    case FXDIB_BLEND_HUE:
      return SkXfermode::kHue_Mode;
    case FXDIB_BLEND_SATURATION:
      return SkXfermode::kSaturation_Mode;
    case FXDIB_BLEND_COLOR:
      return SkXfermode::kColor_Mode;
    case FXDIB_BLEND_LUMINOSITY:
      return SkXfermode::kLuminosity_Mode;
    case FXDIB_BLEND_NORMAL:
    default:
      return SkXfermode::kSrcOver_Mode;
  }
}

bool AddColors(const CPDF_Function* pFunc, SkTDArray<SkColor>* skColors) {
  if (pFunc->CountInputs() != 1)
    return false;
  ASSERT(CPDF_Function::Type::kType2ExpotentialInterpolation ==
         pFunc->GetType());
  const CPDF_ExpIntFunc* expIntFunc =
      static_cast<const CPDF_ExpIntFunc*>(pFunc);
  if (expIntFunc->m_Exponent != 1)
    return false;
  if (expIntFunc->m_nOrigOutputs != 3)
    return false;
  skColors->push(SkColorSetARGB(
      0xFF, SkUnitScalarClampToByte(expIntFunc->m_pBeginValues[0]),
      SkUnitScalarClampToByte(expIntFunc->m_pBeginValues[1]),
      SkUnitScalarClampToByte(expIntFunc->m_pBeginValues[2])));
  skColors->push(
      SkColorSetARGB(0xFF, SkUnitScalarClampToByte(expIntFunc->m_pEndValues[0]),
                     SkUnitScalarClampToByte(expIntFunc->m_pEndValues[1]),
                     SkUnitScalarClampToByte(expIntFunc->m_pEndValues[2])));
  return true;
}

bool AddStitching(const CPDF_Function* pFunc,
                  SkTDArray<SkColor>* skColors,
                  SkTDArray<SkScalar>* skPos) {
  int inputs = pFunc->CountInputs();
  ASSERT(CPDF_Function::Type::kType3Stitching == pFunc->GetType());
  const CPDF_StitchFunc* stitchFunc =
      static_cast<const CPDF_StitchFunc*>(pFunc);
  FX_FLOAT boundsStart = stitchFunc->GetDomain(0);

  for (int i = 0; i < inputs; ++i) {
    const CPDF_Function* pSubFunc = stitchFunc->m_pSubFunctions[i];
    if (pSubFunc->GetType() !=
        CPDF_Function::Type::kType2ExpotentialInterpolation)
      return false;
    if (!AddColors(pSubFunc, skColors))
      return false;
    FX_FLOAT boundsEnd =
        i < inputs - 1 ? stitchFunc->m_pBounds[i] : stitchFunc->GetDomain(1);
    skPos->push(boundsStart);
    skPos->push(boundsEnd);
    boundsStart = boundsEnd;
  }
  return true;
}

}  // namespace

// convert a stroking path to scanlines
void CFX_SkiaDeviceDriver::PaintStroke(SkPaint* spaint,
                                       const CFX_GraphStateData* pGraphState,
                                       const SkMatrix& matrix) {
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
  SkMatrix inverse;
  if (!matrix.invert(&inverse))
    return;  // give up if the matrix is degenerate, and not invertable
  inverse.set(SkMatrix::kMTransX, 0);
  inverse.set(SkMatrix::kMTransY, 0);
  SkVector deviceUnits[2] = {{0, 1}, {1, 0}};
  inverse.mapPoints(deviceUnits, SK_ARRAY_COUNT(deviceUnits));
  FX_FLOAT width =
      SkTMax(pGraphState->m_LineWidth,
             SkTMin(deviceUnits[0].length(), deviceUnits[1].length()));
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
    spaint->setPathEffect(
        SkDashPathEffect::Make(intervals, count * 2, pGraphState->m_DashPhase));
  }
  spaint->setStyle(SkPaint::kStroke_Style);
  spaint->setAntiAlias(true);
  spaint->setStrokeWidth(width);
  spaint->setStrokeMiter(pGraphState->m_MiterLimit);
  spaint->setStrokeCap(cap);
  spaint->setStrokeJoin(join);
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(CFX_DIBitmap* pBitmap,
                                           int dither_bits,
                                           FX_BOOL bRgbByteOrder,
                                           CFX_DIBitmap* pOriDevice,
                                           FX_BOOL bGroupKnockout)
    : m_pRecorder(nullptr) {
  m_pAggDriver = new CFX_AggDeviceDriver(pBitmap, dither_bits, bRgbByteOrder,
                                         pOriDevice, bGroupKnockout);
  SkBitmap skBitmap;
  const CFX_DIBitmap* bitmap = m_pAggDriver->GetBitmap();
  SkImageInfo imageInfo =
      SkImageInfo::Make(bitmap->GetWidth(), bitmap->GetHeight(),
                        kN32_SkColorType, kOpaque_SkAlphaType);
  skBitmap.installPixels(imageInfo, bitmap->GetBuffer(), bitmap->GetPitch(),
                         nullptr, /* to do : set color table */
                         nullptr, nullptr);
  m_pCanvas = new SkCanvas(skBitmap);
  m_ditherBits = dither_bits;
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(int size_x, int size_y)
    : m_pRecorder(new SkPictureRecorder) {
  m_pAggDriver = nullptr;
  m_pRecorder->beginRecording(SkIntToScalar(size_x), SkIntToScalar(size_y));
  m_pCanvas = m_pRecorder->getRecordingCanvas();
  m_ditherBits = 0;
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(SkPictureRecorder* recorder)
    : m_pRecorder(recorder) {
  m_pAggDriver = nullptr;
  m_pCanvas = m_pRecorder->getRecordingCanvas();
  m_ditherBits = 0;
}

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
  if (!m_pRecorder)
    delete m_pCanvas;
  delete m_pAggDriver;
}

FX_BOOL CFX_SkiaDeviceDriver::DrawDeviceText(int nChars,
                                             const FXTEXT_CHARPOS* pCharPos,
                                             CFX_Font* pFont,
                                             CFX_FontCache* pCache,
                                             const CFX_Matrix* pObject2Device,
                                             FX_FLOAT font_size,
                                             uint32_t color,
                                             int alpha_flag,
                                             void* pIccTransform) {
  CFX_TypeFace* typeface = pCache->GetDeviceCache(pFont);
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(color);
  paint.setTypeface(typeface);
  paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
  paint.setTextSize(font_size);
  paint.setSubpixelText(true);
  m_pCanvas->save();
  SkMatrix skMatrix = ToFlippedSkMatrix(*pObject2Device);
  m_pCanvas->concat(skMatrix);
  for (int index = 0; index < nChars; ++index) {
    const FXTEXT_CHARPOS& cp = pCharPos[index];
    uint16_t glyph = (uint16_t)cp.m_GlyphIndex;
    m_pCanvas->drawText(&glyph, 2, cp.m_OriginX, cp.m_OriginY, paint);
  }
  m_pCanvas->restore();
  return TRUE;
}

int CFX_SkiaDeviceDriver::GetDeviceCaps(int caps_id) {
  switch (caps_id) {
    case FXDC_DEVICE_CLASS:
      return FXDC_DISPLAY;
    case FXDC_PIXEL_WIDTH:
      return m_pCanvas->imageInfo().width();
    case FXDC_PIXEL_HEIGHT:
      return m_pCanvas->imageInfo().height();
    case FXDC_BITS_PIXEL:
      return 32;
    case FXDC_HORZ_SIZE:
    case FXDC_VERT_SIZE:
      return 0;
    case FXDC_RENDER_CAPS:
      return FXRC_GET_BITS | FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE |
             FXRC_BLEND_MODE | FXRC_SOFT_CLIP | FXRC_ALPHA_OUTPUT |
             FXRC_FILLSTROKE_PATH | FXRC_SHADING;
    case FXDC_DITHER_BITS:
      return m_ditherBits;
  }
  return 0;
}

void CFX_SkiaDeviceDriver::SaveState() {
  m_pCanvas->save();
}

void CFX_SkiaDeviceDriver::RestoreState(FX_BOOL bKeepSaved) {
  m_pCanvas->restore();
  if (bKeepSaved)
    m_pCanvas->save();
}

FX_BOOL CFX_SkiaDeviceDriver::SetClip_PathFill(
    const CFX_PathData* pPathData,     // path info
    const CFX_Matrix* pObject2Device,  // flips object's y-axis
    int fill_mode                      // fill mode, WINDING or ALTERNATE
    ) {
  if (pPathData->GetPointCount() == 5 || pPathData->GetPointCount() == 4) {
    CFX_FloatRect rectf;
    if (pPathData->IsRect(pObject2Device, &rectf)) {
      rectf.Intersect(
          CFX_FloatRect(0, 0, (FX_FLOAT)GetDeviceCaps(FXDC_PIXEL_WIDTH),
                        (FX_FLOAT)GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
      // note that PDF's y-axis goes up; Skia's y-axis goes down
      SkRect skClipRect =
          SkRect::MakeLTRB(rectf.left, rectf.bottom, rectf.right, rectf.top);
      DebugDrawSkiaClipRect(m_pCanvas, skClipRect);
      m_pCanvas->clipRect(skClipRect);
      return TRUE;
    }
  }
  SkPath skClipPath = BuildPath(pPathData);
  skClipPath.setFillType((fill_mode & 3) == FXFILL_WINDING
                             ? SkPath::kWinding_FillType
                             : SkPath::kEvenOdd_FillType);
  SkMatrix skMatrix = ToSkMatrix(*pObject2Device);
  skClipPath.transform(skMatrix);
  DebugShowSkiaPath(skClipPath);
  DebugDrawSkiaClipPath(m_pCanvas, skClipPath);
  m_pCanvas->clipPath(skClipPath);

  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::SetClip_PathStroke(
    const CFX_PathData* pPathData,         // path info
    const CFX_Matrix* pObject2Device,      // optional transformation
    const CFX_GraphStateData* pGraphState  // graphic state, for pen attributes
    ) {
  // build path data
  SkPath skPath = BuildPath(pPathData);
  skPath.setFillType(SkPath::kWinding_FillType);

  SkMatrix skMatrix = ToSkMatrix(*pObject2Device);
  SkPaint spaint;
  PaintStroke(&spaint, pGraphState, skMatrix);
  SkPath dst_path;
  spaint.getFillPath(skPath, &dst_path);
  dst_path.transform(skMatrix);
  DebugDrawSkiaClipPath(m_pCanvas, dst_path);
  m_pCanvas->clipPath(dst_path);
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::DrawPath(
    const CFX_PathData* pPathData,          // path info
    const CFX_Matrix* pObject2Device,       // optional transformation
    const CFX_GraphStateData* pGraphState,  // graphic state, for pen attributes
    uint32_t fill_color,                    // fill color
    uint32_t stroke_color,                  // stroke color
    int fill_mode,  // fill mode, WINDING or ALTERNATE. 0 for not filled
    int alpha_flag,
    void* pIccTransform,
    int blend_type) {
  SkIRect rect;
  rect.set(0, 0, GetDeviceCaps(FXDC_PIXEL_WIDTH),
           GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  SkMatrix skMatrix = ToSkMatrix(*pObject2Device);
  SkPaint skPaint;
  skPaint.setAntiAlias(true);
  int stroke_alpha = FXGETFLAG_COLORTYPE(alpha_flag)
                         ? FXGETFLAG_ALPHA_STROKE(alpha_flag)
                         : FXARGB_A(stroke_color);
  if (pGraphState && stroke_alpha)
    PaintStroke(&skPaint, pGraphState, skMatrix);
  SkPath skPath = BuildPath(pPathData);
  m_pCanvas->save();
  m_pCanvas->concat(skMatrix);
  if ((fill_mode & 3) && fill_color) {
    skPath.setFillType((fill_mode & 3) == FXFILL_WINDING
                           ? SkPath::kWinding_FillType
                           : SkPath::kEvenOdd_FillType);
    SkPath strokePath;
    const SkPath* fillPath = &skPath;
    if (pGraphState && stroke_alpha) {
      SkAlpha fillA = SkColorGetA(fill_color);
      SkAlpha strokeA = SkColorGetA(stroke_color);
      if (fillA && fillA < 0xFF && strokeA && strokeA < 0xFF) {
        skPaint.getFillPath(skPath, &strokePath);
        if (Op(skPath, strokePath, SkPathOp::kDifference_SkPathOp,
               &strokePath)) {
          fillPath = &strokePath;
        }
      }
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    skPaint.setColor(fill_color);
    m_pCanvas->drawPath(*fillPath, skPaint);
  }
  if (pGraphState && stroke_alpha) {
    DebugShowSkiaPath(skPath);
    DebugShowCanvasMatrix(m_pCanvas);
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setColor(stroke_color);
    m_pCanvas->drawPath(skPath, skPaint);
  }
  m_pCanvas->restore();
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::FillRect(const FX_RECT* pRect,
                                       uint32_t fill_color,
                                       int alpha_flag,
                                       void* pIccTransform,
                                       int blend_type) {
  SkPaint spaint;
  spaint.setAntiAlias(true);
  spaint.setColor(fill_color);
  spaint.setXfermodeMode(GetSkiaBlendMode(blend_type));

  m_pCanvas->drawRect(
      SkRect::MakeLTRB(pRect->left, pRect->top, pRect->right, pRect->bottom),
      spaint);
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::DrawShading(CPDF_ShadingPattern* pPattern,
                                          CFX_Matrix* pMatrix,
                                          int alpha,
                                          FX_BOOL bAlphaMode) {
  CPDF_Function** pFuncs = pPattern->m_pFunctions;
  int nFuncs = pPattern->m_nFuncs;
  if (nFuncs != 1)  // TODO(caryclark) remove this restriction
    return false;
  CPDF_Dictionary* pDict = pPattern->m_pShadingObj->GetDict();
  CPDF_Array* pCoords = pDict->GetArrayBy("Coords");
  if (!pCoords)
    return true;
  FX_FLOAT start_x = pCoords->GetNumberAt(0);
  FX_FLOAT start_y = pCoords->GetNumberAt(1);
  FX_FLOAT end_x = pCoords->GetNumberAt(2);
  FX_FLOAT end_y = pCoords->GetNumberAt(3);
  FX_FLOAT t_min = 0;
  FX_FLOAT t_max = 1;
  CPDF_Array* pArray = pDict->GetArrayBy("Domain");
  if (pArray) {
    t_min = pArray->GetNumberAt(0);
    t_max = pArray->GetNumberAt(1);
  }
  FX_BOOL bStartExtend = FALSE, bEndExtend = FALSE;
  pArray = pDict->GetArrayBy("Extend");
  if (pArray) {
    bStartExtend = pArray->GetIntegerAt(0);
    bEndExtend = pArray->GetIntegerAt(1);
  }
  SkTDArray<SkColor> skColors;
  SkTDArray<SkScalar> skPos;
  for (int j = 0; j < nFuncs; j++) {
    const CPDF_Function* pFunc = pFuncs[j];
    if (!pFunc)
      continue;
    switch (pFunc->GetType()) {
      case CPDF_Function::Type::kType2ExpotentialInterpolation:
        if (!AddColors(pFunc, &skColors))
          return false;
        skPos.push(0);
        skPos.push(1);
        break;
      case CPDF_Function::Type::kType3Stitching:
        if (!AddStitching(pFunc, &skColors, &skPos))
          return false;
        break;
      default:
        return false;
    }
  }
  SkMatrix skMatrix = ToSkMatrix(*pMatrix);
  SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setShader(SkGradientShader::MakeLinear(pts, skColors.begin(),
                                               skPos.begin(), skColors.count(),
                                               SkShader::kClamp_TileMode));
  paint.setAlpha(alpha);
  m_pCanvas->save();
  m_pCanvas->concat(skMatrix);
  m_pCanvas->drawRect(SkRect::MakeWH(1, 1), paint);
  m_pCanvas->restore();
  return true;
}

FX_BOOL CFX_SkiaDeviceDriver::GetClipBox(FX_RECT* pRect) {
  // TODO(caryclark) call m_canvas->getClipDeviceBounds() instead
  pRect->left = 0;
  pRect->top = 0;
  const SkImageInfo& canvasSize = m_pCanvas->imageInfo();
  pRect->right = canvasSize.width();
  pRect->bottom = canvasSize.height();
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::GetDIBits(CFX_DIBitmap* pBitmap,
                                        int left,
                                        int top,
                                        void* pIccTransform,
                                        FX_BOOL bDEdge) {
  return m_pAggDriver &&
         m_pAggDriver->GetDIBits(pBitmap, left, top, pIccTransform, bDEdge);
}

FX_BOOL CFX_SkiaDeviceDriver::SetDIBits(const CFX_DIBSource* pBitmap,
                                        uint32_t argb,
                                        const FX_RECT* pSrcRect,
                                        int left,
                                        int top,
                                        int blend_type,
                                        int alpha_flag,
                                        void* pIccTransform) {
  return m_pAggDriver &&
         m_pAggDriver->SetDIBits(pBitmap, argb, pSrcRect, left, top, blend_type,
                                 alpha_flag, pIccTransform);
}

FX_BOOL CFX_SkiaDeviceDriver::StretchDIBits(const CFX_DIBSource* pSource,
                                            uint32_t argb,
                                            int dest_left,
                                            int dest_top,
                                            int dest_width,
                                            int dest_height,
                                            const FX_RECT* pClipRect,
                                            uint32_t flags,
                                            int alpha_flag,
                                            void* pIccTransform,
                                            int blend_type) {
  return m_pAggDriver &&
         m_pAggDriver->StretchDIBits(pSource, argb, dest_left, dest_top,
                                     dest_width, dest_height, pClipRect, flags,
                                     alpha_flag, pIccTransform, blend_type);
}

FX_BOOL CFX_SkiaDeviceDriver::StartDIBits(const CFX_DIBSource* pSource,
                                          int bitmap_alpha,
                                          uint32_t argb,
                                          const CFX_Matrix* pMatrix,
                                          uint32_t render_flags,
                                          void*& handle,
                                          int alpha_flag,
                                          void* pIccTransform,
                                          int blend_type) {
  SkColorType colorType;
  void* buffer = pSource->GetBuffer();
  std::unique_ptr<uint8_t, FxFreeDeleter> dst8Storage;
  std::unique_ptr<uint32_t, FxFreeDeleter> dst32Storage;
  int width = pSource->GetWidth();
  int height = pSource->GetHeight();
  int rowBytes = pSource->GetPitch();
  switch (pSource->GetBPP()) {
    case 1: {
      uint8_t zero = pSource->IsAlphaMask() ? 0xFF : 0x00;
      uint8_t one = zero ^ 0xFF;
      dst8Storage.reset(FX_Alloc2D(uint8_t, width, height));
      uint8_t* dst8Pixels = dst8Storage.get();
      for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow =
            static_cast<const uint8_t*>(buffer) + y * rowBytes;
        uint8_t* dstRow = dst8Pixels + y * width;
        for (int x = 0; x < width; ++x)
          dstRow[x] = srcRow[x >> 3] & (1 << (~x & 0x07)) ? one : zero;
      }
      buffer = dst8Storage.get();
      rowBytes = width;
      colorType = SkColorType::kGray_8_SkColorType;
    } break;
    case 24: {
      dst32Storage.reset(FX_Alloc2D(uint32_t, width, height));
      uint32_t* dst32Pixels = dst32Storage.get();
      for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow =
            static_cast<const uint8_t*>(buffer) + y * rowBytes;
        uint32_t* dstRow = dst32Pixels + y * width;
        for (int x = 0; x < width; ++x)
          dstRow[x] = SkPackARGB32(0xFF, srcRow[x * 3 + 2], srcRow[x * 3 + 1],
                                   srcRow[x * 3 + 0]);
      }
      buffer = dst32Storage.get();
      rowBytes = width * sizeof(uint32_t);
      colorType = SkColorType::kN32_SkColorType;
    } break;
    case 32:
      colorType = SkColorType::kN32_SkColorType;
      break;
    default:
      colorType = SkColorType::kUnknown_SkColorType;
  }
  SkImageInfo imageInfo =
      SkImageInfo::Make(width, height, colorType, kOpaque_SkAlphaType);
  SkBitmap skBitmap;
  skBitmap.installPixels(imageInfo, buffer, rowBytes,
                         nullptr, /* TODO(caryclark) : set color table */
                         nullptr, nullptr);
  m_pCanvas->save();
  bool landscape = !pMatrix->a;
  if (landscape)
    m_pCanvas->translate(m_pCanvas->imageInfo().width(), 0);
  else
    m_pCanvas->translate(pMatrix->e, pMatrix->f + pMatrix->d);

  SkMatrix skMatrix = SkMatrix::MakeScale(1.f / width, 1.f / height);
  m_pCanvas->concat(skMatrix);
  const CFX_Matrix& m = *pMatrix;
  // note that PDF's y-axis goes up; Skia's y-axis goes down
  if (landscape)
    skMatrix.setAll(-m.a, -m.b, m.e, m.c, m.d, m.f, 0, 0, 1);
  else
    skMatrix.setAll(m.a, m.b, 0, -m.c, -m.d, 0, 0, 0, 1);
  m_pCanvas->concat(skMatrix);
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setFilterQuality(kHigh_SkFilterQuality);
  paint.setXfermodeMode(GetSkiaBlendMode(blend_type));
  paint.setAlpha(bitmap_alpha);
  m_pCanvas->drawBitmap(skBitmap, 0, 0, &paint);
  m_pCanvas->restore();
  return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::ContinueDIBits(void* pHandle, IFX_Pause* pPause) {
  return m_pAggDriver && m_pAggDriver->ContinueDIBits(pHandle, pPause);
}

void CFX_SkiaDeviceDriver::CancelDIBits(void* pHandle) {
  if (m_pAggDriver)
    m_pAggDriver->CancelDIBits(pHandle);
}

CFX_SkiaDevice::CFX_SkiaDevice() {
  m_bOwnedBitmap = FALSE;
}

SkPictureRecorder* CFX_SkiaDevice::CreateRecorder(int size_x, int size_y) {
  CFX_SkiaDeviceDriver* skDriver = new CFX_SkiaDeviceDriver(size_x, size_y);
  SetDeviceDriver(skDriver);
  return skDriver->GetRecorder();
}

FX_BOOL CFX_SkiaDevice::Attach(CFX_DIBitmap* pBitmap,
                               int dither_bits,
                               FX_BOOL bRgbByteOrder,
                               CFX_DIBitmap* pOriDevice,
                               FX_BOOL bGroupKnockout) {
  if (!pBitmap)
    return FALSE;
  SetBitmap(pBitmap);
  SetDeviceDriver(new CFX_SkiaDeviceDriver(pBitmap, dither_bits, bRgbByteOrder,
                                           pOriDevice, bGroupKnockout));
  return TRUE;
}

FX_BOOL CFX_SkiaDevice::AttachRecorder(SkPictureRecorder* recorder) {
  if (!recorder)
    return FALSE;
  SetDeviceDriver(new CFX_SkiaDeviceDriver(recorder));
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

#endif
