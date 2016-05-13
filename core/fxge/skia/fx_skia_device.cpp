// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/include/fx_ge.h"

#if defined(_SKIA_SUPPORT_)
#include "core/fxcodec/include/fx_codec.h"

#include "core/fpdfapi/fpdf_page/cpdf_shadingpattern.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream_acc.h"
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

uint32_t GetBits32(const uint8_t* pData, int bitpos, int nbits) {
  ASSERT(0 < nbits && nbits <= 32);
  const uint8_t* dataPtr = &pData[bitpos / 8];
  int bitShift;
  int bitMask;
  int dstShift;
  int bitCount = bitpos & 0x07;
  if (nbits < 8 && nbits + bitCount <= 8) {
    bitShift = 8 - nbits - bitCount;
    bitMask = (1 << nbits) - 1;
    dstShift = 0;
  } else {
    bitShift = 0;
    int bitOffset = 8 - bitCount;
    bitMask = (1 << SkTMin(bitOffset, nbits)) - 1;
    dstShift = nbits - bitOffset;
  }
  uint32_t result = (uint32_t)(*dataPtr++ >> bitShift & bitMask) << dstShift;
  while (dstShift >= 8) {
    dstShift -= 8;
    result |= *dataPtr++ << dstShift;
  }
  if (dstShift > 0) {
    bitShift = 8 - dstShift;
    bitMask = (1 << dstShift) - 1;
    result |= *dataPtr++ >> bitShift & bitMask;
  }
  return result;
}

uint8_t FloatToByte(FX_FLOAT f) {
  ASSERT(0 <= f && f <= 1);
  return (uint8_t)(f * 255.99f);
}

bool AddSamples(const CPDF_Function* pFunc,
                SkTDArray<SkColor>* skColors,
                SkTDArray<SkScalar>* skPos) {
  if (pFunc->CountInputs() != 1)
    return false;
  if (pFunc->CountOutputs() != 3)  // expect rgb
    return false;
  ASSERT(CPDF_Function::Type::kType0Sampled == pFunc->GetType());
  const CPDF_SampledFunc* sampledFunc =
      static_cast<const CPDF_SampledFunc*>(pFunc);
  if (!sampledFunc->m_pEncodeInfo)
    return false;
  const CPDF_SampledFunc::SampleEncodeInfo& encodeInfo =
      sampledFunc->m_pEncodeInfo[0];
  if (encodeInfo.encode_min != 0)
    return false;
  if (encodeInfo.encode_max != encodeInfo.sizes - 1)
    return false;
  uint32_t sampleSize = sampledFunc->m_nBitsPerSample;
  uint32_t sampleCount = encodeInfo.sizes;
  if (sampleCount != 1 << sampleSize)
    return false;
  if (sampledFunc->m_pSampleStream->GetSize() <
      sampleCount * 3 * sampleSize / 8) {
    return false;
  }
  FX_FLOAT colorsMin[3];
  FX_FLOAT colorsMax[3];
  for (int i = 0; i < 3; ++i) {
    colorsMin[i] = sampledFunc->GetRange(i * 2);
    colorsMax[i] = sampledFunc->GetRange(i * 2 + 1);
  }
  const uint8_t* pSampleData = sampledFunc->m_pSampleStream->GetData();
  for (uint32_t i = 0; i < sampleCount; ++i) {
    FX_FLOAT floatColors[3];
    for (uint32_t j = 0; j < 3; ++j) {
      int sample = GetBits32(pSampleData, (i * 3 + j) * sampleSize, sampleSize);
      FX_FLOAT interp = (FX_FLOAT)sample / (sampleCount - 1);
      floatColors[j] = colorsMin[j] + (colorsMax[j] - colorsMin[j]) * interp;
    }
    SkColor color =
        SkPackARGB32(0xFF, FloatToByte(floatColors[0]),
                     FloatToByte(floatColors[1]), FloatToByte(floatColors[2]));
    skColors->push(color);
    skPos->push((FX_FLOAT)i / (sampleCount - 1));
  }
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

void RgbByteOrderTransferBitmap(CFX_DIBitmap* pBitmap,
                                int dest_left,
                                int dest_top,
                                int width,
                                int height,
                                const CFX_DIBSource* pSrcBitmap,
                                int src_left,
                                int src_top) {
  if (!pBitmap)
    return;
  pBitmap->GetOverlapRect(dest_left, dest_top, width, height,
                          pSrcBitmap->GetWidth(), pSrcBitmap->GetHeight(),
                          src_left, src_top, NULL);
  if (width == 0 || height == 0)
    return;
  int Bpp = pBitmap->GetBPP() / 8;
  FXDIB_Format dest_format = pBitmap->GetFormat();
  FXDIB_Format src_format = pSrcBitmap->GetFormat();
  int pitch = pBitmap->GetPitch();
  uint8_t* buffer = pBitmap->GetBuffer();
  if (dest_format == src_format) {
    for (int row = 0; row < height; row++) {
      uint8_t* dest_scan = buffer + (dest_top + row) * pitch + dest_left * Bpp;
      uint8_t* src_scan =
          (uint8_t*)pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
      if (Bpp == 4) {
        for (int col = 0; col < width; col++) {
          FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_scan[3], src_scan[0],
                                               src_scan[1], src_scan[2]));
          dest_scan += 4;
          src_scan += 4;
        }
      } else {
        for (int col = 0; col < width; col++) {
          *dest_scan++ = src_scan[2];
          *dest_scan++ = src_scan[1];
          *dest_scan++ = src_scan[0];
          src_scan += 3;
        }
      }
    }
    return;
  }
  uint8_t* dest_buf = buffer + dest_top * pitch + dest_left * Bpp;
  if (dest_format == FXDIB_Rgb) {
    if (src_format == FXDIB_Rgb32) {
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = dest_buf + row * pitch;
        uint8_t* src_scan =
            (uint8_t*)pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
        for (int col = 0; col < width; col++) {
          *dest_scan++ = src_scan[2];
          *dest_scan++ = src_scan[1];
          *dest_scan++ = src_scan[0];
          src_scan += 4;
        }
      }
    } else {
      ASSERT(FALSE);
    }
  } else if (dest_format == FXDIB_Argb || dest_format == FXDIB_Rgb32) {
    if (src_format == FXDIB_Rgb) {
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = (uint8_t*)(dest_buf + row * pitch);
        uint8_t* src_scan =
            (uint8_t*)pSrcBitmap->GetScanline(src_top + row) + src_left * 3;
        if (src_format == FXDIB_Argb) {
          for (int col = 0; col < width; col++) {
            FXARGB_SETDIB(dest_scan, FXARGB_MAKE(0xff, FX_GAMMA(src_scan[0]),
                                                 FX_GAMMA(src_scan[1]),
                                                 FX_GAMMA(src_scan[2])));
            dest_scan += 4;
            src_scan += 3;
          }
        } else {
          for (int col = 0; col < width; col++) {
            FXARGB_SETDIB(dest_scan, FXARGB_MAKE(0xff, src_scan[0], src_scan[1],
                                                 src_scan[2]));
            dest_scan += 4;
            src_scan += 3;
          }
        }
      }
    } else if (src_format == FXDIB_Rgb32) {
      ASSERT(dest_format == FXDIB_Argb);
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = dest_buf + row * pitch;
        uint8_t* src_scan =
            (uint8_t*)(pSrcBitmap->GetScanline(src_top + row) + src_left * 4);
        for (int col = 0; col < width; col++) {
          FXARGB_SETDIB(dest_scan, FXARGB_MAKE(0xff, src_scan[0], src_scan[1],
                                               src_scan[2]));
          src_scan += 4;
          dest_scan += 4;
        }
      }
    }
  } else {
    ASSERT(FALSE);
  }
}

// see https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
SkScalar LineSide(const SkPoint line[2], const SkPoint& pt) {
  return (line[1].fY - line[0].fY) * pt.fX - (line[1].fX - line[0].fX) * pt.fY +
         line[1].fX * line[0].fY - line[1].fY * line[0].fX;
}

SkPoint IntersectSides(const SkPoint& parallelPt,
                       const SkVector& paraRay,
                       const SkPoint& perpendicularPt) {
  SkVector perpRay = {paraRay.fY, -paraRay.fX};
  SkScalar denom = perpRay.fY * paraRay.fX - paraRay.fY * perpRay.fX;
  if (!denom) {
    SkPoint zeroPt = {0, 0};
    return zeroPt;
  }
  SkVector ab0 = parallelPt - perpendicularPt;
  SkScalar numerA = ab0.fY * perpRay.fX - perpRay.fY * ab0.fX;
  numerA /= denom;
  SkPoint result = {parallelPt.fX + paraRay.fX * numerA,
                    parallelPt.fY + paraRay.fY * numerA};
  return result;
}

void ClipAngledGradient(const SkPoint pts[2],
                        SkPoint rectPts[4],
                        bool clipStart,
                        bool clipEnd,
                        SkPath* clip) {
  // find the corners furthest from the gradient perpendiculars
  SkScalar minPerpDist = SK_ScalarMax;
  SkScalar maxPerpDist = SK_ScalarMin;
  int minPerpPtIndex = -1;
  int maxPerpPtIndex = -1;
  SkVector slope = pts[1] - pts[0];
  SkPoint startPerp[2] = {pts[0], {pts[0].fX + slope.fY, pts[0].fY - slope.fX}};
  SkPoint endPerp[2] = {pts[1], {pts[1].fX + slope.fY, pts[1].fY - slope.fX}};
  for (int i = 0; i < 4; ++i) {
    SkScalar sDist = LineSide(startPerp, rectPts[i]);
    SkScalar eDist = LineSide(endPerp, rectPts[i]);
    if (sDist * eDist <= 0)  // if the signs are different,
      continue;              // the point is inside the gradient
    if (sDist < 0) {
      SkScalar smaller = SkTMin(sDist, eDist);
      if (minPerpDist > smaller) {
        minPerpDist = smaller;
        minPerpPtIndex = i;
      }
    } else {
      SkScalar larger = SkTMax(sDist, eDist);
      if (maxPerpDist < larger) {
        maxPerpDist = larger;
        maxPerpPtIndex = i;
      }
    }
  }
  if (minPerpPtIndex < 0 && maxPerpPtIndex < 0)  // nothing's outside
    return;
  // determine if negative distances are before start or after end
  SkPoint beforeStart = {pts[0].fX * 2 - pts[1].fX, pts[0].fY * 2 - pts[1].fY};
  bool beforeNeg = LineSide(startPerp, beforeStart) < 0;
  const SkPoint& startEdgePt =
      clipStart ? pts[0] : beforeNeg ? rectPts[minPerpPtIndex]
                                     : rectPts[maxPerpPtIndex];
  const SkPoint& endEdgePt = clipEnd ? pts[1] : beforeNeg
                                                    ? rectPts[maxPerpPtIndex]
                                                    : rectPts[minPerpPtIndex];
  // find the corners that bound the gradient
  SkScalar minDist = SK_ScalarMax;
  SkScalar maxDist = SK_ScalarMin;
  int minBounds = -1;
  int maxBounds = -1;
  for (int i = 0; i < 4; ++i) {
    SkScalar dist = LineSide(pts, rectPts[i]);
    if (minDist > dist) {
      minDist = dist;
      minBounds = i;
    }
    if (maxDist < dist) {
      maxDist = dist;
      maxBounds = i;
    }
  }
  ASSERT(minBounds >= 0);
  ASSERT(maxBounds != minBounds && maxBounds >= 0);
  // construct a clip parallel to the gradient that goes through
  // rectPts[minBounds] and rectPts[maxBounds] and perpendicular to the
  // gradient that goes through startEdgePt, endEdgePt.
  clip->moveTo(IntersectSides(rectPts[minBounds], slope, startEdgePt));
  clip->lineTo(IntersectSides(rectPts[minBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rectPts[maxBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rectPts[maxBounds], slope, startEdgePt));
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
    : m_pBitmap(pBitmap),
      m_pOriDevice(pOriDevice),
      m_pRecorder(nullptr),
      m_ditherBits(dither_bits),
      m_bRgbByteOrder(bRgbByteOrder),
      m_bGroupKnockout(bGroupKnockout) {
  SkBitmap skBitmap;
  SkImageInfo imageInfo =
      SkImageInfo::Make(pBitmap->GetWidth(), pBitmap->GetHeight(),
                        kN32_SkColorType, kOpaque_SkAlphaType);
  skBitmap.installPixels(imageInfo, pBitmap->GetBuffer(), pBitmap->GetPitch(),
                         nullptr, /* to do : set color table */
                         nullptr, nullptr);
  m_pCanvas = new SkCanvas(skBitmap);
  if (m_bGroupKnockout)
    SkDebugf("");  // FIXME(caryclark) suppress 'm_bGroupKnockout is unused'
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(int size_x, int size_y)
    : m_pBitmap(nullptr),
      m_pOriDevice(nullptr),
      m_pRecorder(new SkPictureRecorder),
      m_ditherBits(0),
      m_bRgbByteOrder(FALSE),
      m_bGroupKnockout(FALSE) {
  m_pRecorder->beginRecording(SkIntToScalar(size_x), SkIntToScalar(size_y));
  m_pCanvas = m_pRecorder->getRecordingCanvas();
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(SkPictureRecorder* recorder)
    : m_pBitmap(nullptr),
      m_pOriDevice(nullptr),
      m_pRecorder(recorder),
      m_ditherBits(0),
      m_bRgbByteOrder(FALSE),
      m_bGroupKnockout(FALSE) {
  m_pCanvas = m_pRecorder->getRecordingCanvas();
}

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
  if (!m_pRecorder)
    delete m_pCanvas;
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
  SkMatrix skMatrix;
  if (pObject2Device)
    skMatrix = ToSkMatrix(*pObject2Device);
  else
    skMatrix.setIdentity();
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

FX_BOOL CFX_SkiaDeviceDriver::DrawShading(const CPDF_ShadingPattern* pPattern,
                                          const CFX_Matrix* pMatrix,
                                          const FX_RECT& clip_rect,
                                          int alpha,
                                          FX_BOOL bAlphaMode) {
  if (kAxialShading != pPattern->m_ShadingType &&
      kRadialShading != pPattern->m_ShadingType) {
    // TODO(caryclark) more types
    return false;
  }
  CPDF_Function* const* pFuncs = pPattern->m_pFunctions;
  int nFuncs = pPattern->m_nFuncs;
  if (nFuncs != 1)  // TODO(caryclark) remove this restriction
    return false;
  CPDF_Dictionary* pDict = pPattern->m_pShadingObj->GetDict();
  CPDF_Array* pCoords = pDict->GetArrayBy("Coords");
  if (!pCoords)
    return true;
  // TODO(caryclark) Respect Domain[0], Domain[1]. (Don't know what they do
  // yet.)
  SkTDArray<SkColor> skColors;
  SkTDArray<SkScalar> skPos;
  for (int j = 0; j < nFuncs; j++) {
    const CPDF_Function* pFunc = pFuncs[j];
    if (!pFunc)
      continue;
    switch (pFunc->GetType()) {
      case CPDF_Function::Type::kType0Sampled:
        /* TODO(caryclark)
           Type 0 Sampled Functions in PostScript can also have an Order integer
           in the dictionary. PDFium doesn't appear to check for this anywhere.
         */
        if (!AddSamples(pFunc, &skColors, &skPos))
          return false;
        break;
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
  CPDF_Array* pArray = pDict->GetArrayBy("Extend");
  bool clipStart = !pArray || !pArray->GetIntegerAt(0);
  bool clipEnd = !pArray || !pArray->GetIntegerAt(1);
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setAlpha(alpha);
  SkMatrix skMatrix = ToSkMatrix(*pMatrix);
  SkRect skRect = SkRect::MakeLTRB(clip_rect.left, clip_rect.top,
                                   clip_rect.right, clip_rect.bottom);
  SkPath skClip;
  SkPath skPath;
  if (kAxialShading == pPattern->m_ShadingType) {
    FX_FLOAT start_x = pCoords->GetNumberAt(0);
    FX_FLOAT start_y = pCoords->GetNumberAt(1);
    FX_FLOAT end_x = pCoords->GetNumberAt(2);
    FX_FLOAT end_y = pCoords->GetNumberAt(3);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};
    skMatrix.mapPoints(pts, SK_ARRAY_COUNT(pts));
    paint.setShader(SkGradientShader::MakeLinear(
        pts, skColors.begin(), skPos.begin(), skColors.count(),
        SkShader::kClamp_TileMode));
    if (clipStart || clipEnd) {
      // if the gradient is horizontal or vertical, modify the draw rectangle
      if (pts[0].fX == pts[1].fX) {  // vertical
        if (pts[0].fY > pts[1].fY) {
          SkTSwap(pts[0].fY, pts[1].fY);
          SkTSwap(clipStart, clipEnd);
        }
        if (clipStart)
          skRect.fTop = SkTMax(skRect.fTop, pts[0].fY);
        if (clipEnd)
          skRect.fBottom = SkTMin(skRect.fBottom, pts[1].fY);
      } else if (pts[0].fY == pts[1].fY) {  // horizontal
        if (pts[0].fX > pts[1].fX) {
          SkTSwap(pts[0].fX, pts[1].fX);
          SkTSwap(clipStart, clipEnd);
        }
        if (clipStart)
          skRect.fLeft = SkTMax(skRect.fLeft, pts[0].fX);
        if (clipEnd)
          skRect.fRight = SkTMin(skRect.fRight, pts[1].fX);
      } else {  // if the gradient is angled and contained by the rect, clip
        SkPoint rectPts[4] = {{skRect.fLeft, skRect.fTop},
                              {skRect.fRight, skRect.fTop},
                              {skRect.fRight, skRect.fBottom},
                              {skRect.fLeft, skRect.fBottom}};
        ClipAngledGradient(pts, rectPts, clipStart, clipEnd, &skClip);
      }
    }
    skPath.addRect(skRect);
    skMatrix.setIdentity();
  } else {
    ASSERT(kRadialShading == pPattern->m_ShadingType);
    FX_FLOAT start_x = pCoords->GetNumberAt(0);
    FX_FLOAT start_y = pCoords->GetNumberAt(1);
    FX_FLOAT start_r = pCoords->GetNumberAt(2);
    FX_FLOAT end_x = pCoords->GetNumberAt(3);
    FX_FLOAT end_y = pCoords->GetNumberAt(4);
    FX_FLOAT end_r = pCoords->GetNumberAt(5);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};

    paint.setShader(SkGradientShader::MakeTwoPointConical(
        pts[0], start_r, pts[1], end_r, skColors.begin(), skPos.begin(),
        skColors.count(), SkShader::kClamp_TileMode));
    if (clipStart || clipEnd) {
      if (clipStart && start_r)
        skClip.addCircle(pts[0].fX, pts[0].fY, start_r);
      if (clipEnd)
        skClip.addCircle(pts[1].fX, pts[1].fY, end_r, SkPath::kCCW_Direction);
      else
        skClip.setFillType(SkPath::kInverseWinding_FillType);
      skClip.transform(skMatrix);
    }
    SkMatrix inverse;
    if (!skMatrix.invert(&inverse))
      return false;
    skPath.addRect(skRect);
    skPath.transform(inverse);
  }
  m_pCanvas->save();
  if (!skClip.isEmpty())
    m_pCanvas->clipPath(skClip);
  m_pCanvas->concat(skMatrix);
  m_pCanvas->drawPath(skPath, paint);
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
  if (!m_pBitmap || !m_pBitmap->GetBuffer())
    return TRUE;
  if (bDEdge) {
    if (m_bRgbByteOrder) {
      RgbByteOrderTransferBitmap(pBitmap, 0, 0, pBitmap->GetWidth(),
                                 pBitmap->GetHeight(), m_pBitmap, left, top);
    } else {
      return pBitmap->TransferBitmap(0, 0, pBitmap->GetWidth(),
                                     pBitmap->GetHeight(), m_pBitmap, left, top,
                                     pIccTransform);
    }
    return TRUE;
  }
  FX_RECT rect(left, top, left + pBitmap->GetWidth(),
               top + pBitmap->GetHeight());
  CFX_DIBitmap* pBack;
  if (m_pOriDevice) {
    pBack = m_pOriDevice->Clone(&rect);
    if (!pBack)
      return TRUE;
    pBack->CompositeBitmap(0, 0, pBack->GetWidth(), pBack->GetHeight(),
                           m_pBitmap, 0, 0);
  } else {
    pBack = m_pBitmap->Clone(&rect);
    if (!pBack)
      return TRUE;
  }
  FX_BOOL bRet = TRUE;
  left = left >= 0 ? 0 : left;
  top = top >= 0 ? 0 : top;
  if (m_bRgbByteOrder) {
    RgbByteOrderTransferBitmap(pBitmap, 0, 0, rect.Width(), rect.Height(),
                               pBack, left, top);
  } else {
    bRet = pBitmap->TransferBitmap(0, 0, rect.Width(), rect.Height(), pBack,
                                   left, top, pIccTransform);
  }
  delete pBack;
  return bRet;
}

FX_BOOL CFX_SkiaDeviceDriver::SetDIBits(const CFX_DIBSource* pBitmap,
                                        uint32_t argb,
                                        const FX_RECT* pSrcRect,
                                        int left,
                                        int top,
                                        int blend_type,
                                        int alpha_flag,
                                        void* pIccTransform) {
  if (!m_pBitmap || !m_pBitmap->GetBuffer())
    return TRUE;
  if (pBitmap->IsAlphaMask()) {
    return m_pBitmap->CompositeMask(
        left, top, pSrcRect->Width(), pSrcRect->Height(), pBitmap, argb,
        pSrcRect->left, pSrcRect->top, blend_type, nullptr, m_bRgbByteOrder,
        alpha_flag, pIccTransform);
  }
  return m_pBitmap->CompositeBitmap(
      left, top, pSrcRect->Width(), pSrcRect->Height(), pBitmap, pSrcRect->left,
      pSrcRect->top, blend_type, nullptr, m_bRgbByteOrder, pIccTransform);
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
  if (!m_pBitmap->GetBuffer())
    return TRUE;
  if (dest_width == pSource->GetWidth() &&
      dest_height == pSource->GetHeight()) {
    FX_RECT rect(0, 0, dest_width, dest_height);
    return SetDIBits(pSource, argb, &rect, dest_left, dest_top, blend_type,
                     alpha_flag, pIccTransform);
  }
  FX_RECT dest_rect(dest_left, dest_top, dest_left + dest_width,
                    dest_top + dest_height);
  dest_rect.Normalize();
  FX_RECT dest_clip = dest_rect;
  dest_clip.Intersect(*pClipRect);
  CFX_BitmapComposer composer;
  composer.Compose(m_pBitmap, nullptr, 255, argb, dest_clip, FALSE, FALSE,
                   FALSE, m_bRgbByteOrder, alpha_flag, pIccTransform,
                   blend_type);
  dest_clip.Offset(-dest_rect.left, -dest_rect.top);
  CFX_ImageStretcher stretcher(&composer, pSource, dest_width, dest_height,
                               dest_clip, flags);
  if (stretcher.Start())
    stretcher.Continue(nullptr);
  return TRUE;
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
    case 8:
      colorType = SkColorType::kGray_8_SkColorType;
      break;
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
  if (!m_pBitmap->GetBuffer())
    return TRUE;
  return ((CFX_ImageRenderer*)pHandle)->Continue(pPause);
}

void CFX_SkiaDeviceDriver::CancelDIBits(void* pHandle) {
  if (!m_pBitmap->GetBuffer())
    return;
  delete (CFX_ImageRenderer*)pHandle;
}

CFX_FxgeDevice::CFX_FxgeDevice() {
  m_bOwnedBitmap = FALSE;
}

SkPictureRecorder* CFX_FxgeDevice::CreateRecorder(int size_x, int size_y) {
  CFX_SkiaDeviceDriver* skDriver = new CFX_SkiaDeviceDriver(size_x, size_y);
  SetDeviceDriver(skDriver);
  return skDriver->GetRecorder();
}

bool CFX_FxgeDevice::Attach(CFX_DIBitmap* pBitmap,
                            int dither_bits,
                            bool bRgbByteOrder,
                            CFX_DIBitmap* pOriDevice,
                            bool bGroupKnockout) {
  if (!pBitmap)
    return false;
  SetBitmap(pBitmap);
  SetDeviceDriver(new CFX_SkiaDeviceDriver(pBitmap, dither_bits, bRgbByteOrder,
                                           pOriDevice, bGroupKnockout));
  return true;
}

bool CFX_FxgeDevice::AttachRecorder(SkPictureRecorder* recorder) {
  if (!recorder)
    return false;
  SetDeviceDriver(new CFX_SkiaDeviceDriver(recorder));
  return true;
}

bool CFX_FxgeDevice::Create(int width,
                            int height,
                            FXDIB_Format format,
                            int dither_bits,
                            CFX_DIBitmap* pOriDevice) {
  m_bOwnedBitmap = TRUE;
  CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
  if (!pBitmap->Create(width, height, format)) {
    delete pBitmap;
    return false;
  }
  SetBitmap(pBitmap);
  CFX_SkiaDeviceDriver* pDriver =
      new CFX_SkiaDeviceDriver(pBitmap, dither_bits, FALSE, pOriDevice, FALSE);
  SetDeviceDriver(pDriver);
  return true;
}

CFX_FxgeDevice::~CFX_FxgeDevice() {
  if (m_bOwnedBitmap && GetBitmap())
    delete GetBitmap();
}

#endif
