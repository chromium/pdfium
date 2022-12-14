// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/skia/fx_skia_device.h"

#include <limits.h>
#include <math.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_expintfunc.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/page/cpdf_meshstream.h"
#include "core/fpdfapi/page/cpdf_sampledfunc.h"
#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fpdfapi/page/cpdf_stitchfunc.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/cfx_bitstream.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/notreached.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/span.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkClipOp.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPathEffect.h"
#include "third_party/skia/include/core/SkPathUtils.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkSamplingOptions.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/core/SkTextBlob.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "third_party/skia/include/effects/SkDashPathEffect.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/pathops/SkPathOps.h"

namespace {

#define SHOW_SKIA_PATH 0  // set to 1 to print the path contents
#if SHOW_SKIA_PATH
#define SHOW_SKIA_PATH_SHORTHAND 0  // set to 1 for abbreviated path contents
#endif

#if SHOW_SKIA_PATH
void DebugShowSkiaPaint(const SkPaint& paint) {
  if (SkPaint::kFill_Style == paint.getStyle()) {
    printf("fill 0x%08x\n", paint.getColor());
  } else {
    printf("stroke 0x%08x width %g\n", paint.getColor(),
           paint.getStrokeWidth());
  }
}
#endif  // SHOW_SKIA_PATH

void DebugShowSkiaPath(const SkPath& path) {
#if SHOW_SKIA_PATH
#if SHOW_SKIA_PATH_SHORTHAND
  printf(" **\n");
#else
  SkDynamicMemoryWStream stream;
  path.dump(&stream, false);
  DataVector<char> storage(stream.bytesWritten());
  stream.copyTo(storage.data());
  printf("%.*s", static_cast<int>(storage.size()), storage.data());
#endif  // SHOW_SKIA_PATH_SHORTHAND
#endif  // SHOW_SKIA_PATH
}

void DebugShowCanvasClip(CFX_SkiaDeviceDriver* driver, const SkCanvas* canvas) {
#if SHOW_SKIA_PATH
  SkMatrix matrix = canvas->getTotalMatrix();
  SkScalar m[9];
  matrix.get9(m);
  printf("matrix (%g,%g,%g) (%g,%g,%g) (%g,%g,%g)\n", m[0], m[1], m[2], m[3],
         m[4], m[5], m[6], m[7], m[8]);
  SkRect local = canvas->getLocalClipBounds();
  SkIRect device = canvas->getDeviceClipBounds();

  printf("local bounds %g %g %g %g\n", local.fLeft, local.fTop, local.fRight,
         local.fBottom);
  printf("device bounds %d %d %d %d\n", device.fLeft, device.fTop,
         device.fRight, device.fBottom);
  FX_RECT clipBox;
  driver->GetClipBox(&clipBox);
  printf("reported bounds %d %d %d %d\n", clipBox.left, clipBox.top,
         clipBox.right, clipBox.bottom);
#endif  // SHOW_SKIA_PATH
}

void DebugShowSkiaDrawPath(CFX_SkiaDeviceDriver* driver,
                           const SkCanvas* canvas,
                           const SkPaint& paint,
                           const SkPath& path) {
#if SHOW_SKIA_PATH
  DebugShowSkiaPaint(paint);
  DebugShowCanvasClip(driver, canvas);
  DebugShowSkiaPath(path);
  printf("\n");
#endif  // SHOW_SKIA_PATH
}

void DebugShowSkiaDrawRect(CFX_SkiaDeviceDriver* driver,
                           const SkCanvas* canvas,
                           const SkPaint& paint,
                           const SkRect& rect) {
#if SHOW_SKIA_PATH
  DebugShowSkiaPaint(paint);
  DebugShowCanvasClip(driver, canvas);
  printf("rect %g %g %g %g\n", rect.fLeft, rect.fTop, rect.fRight,
         rect.fBottom);
#endif  // SHOW_SKIA_PATH
}

bool IsRGBColorGrayScale(uint32_t color) {
  return FXARGB_R(color) == FXARGB_G(color) &&
         FXARGB_R(color) == FXARGB_B(color);
}

// Called by Upsample(), returns a 32 bit-per-pixel buffer filled with colors
// from `palette`.
DataVector<uint32_t> Fill32BppDestStorageWithPalette(
    const RetainPtr<CFX_DIBBase>& source,
    pdfium::span<const uint32_t> palette) {
  int width = source->GetWidth();
  int height = source->GetHeight();
  void* buffer = source->GetBuffer().data();
  DCHECK(buffer);
  DataVector<uint32_t> dst32_storage(Fx2DSizeOrDie(width, height));
  pdfium::span<SkPMColor> dst32_pixels(dst32_storage);

  for (int y = 0; y < height; ++y) {
    const uint8_t* src_row =
        static_cast<const uint8_t*>(buffer) + y * source->GetPitch();
    pdfium::span<uint32_t> dst_row = dst32_pixels.subspan(y * width);
    for (int x = 0; x < width; ++x) {
      unsigned index = src_row[x];
      if (index >= palette.size()) {
        index = 0;
      }
      dst_row[x] = palette[index];
    }
  }
  return dst32_storage;
}

static void DebugValidate(const RetainPtr<CFX_DIBitmap>& bitmap,
                          const RetainPtr<CFX_DIBitmap>& device) {
  if (bitmap) {
    DCHECK(bitmap->GetBPP() == 8 || bitmap->GetBPP() == 32);
    if (bitmap->GetBPP() == 32) {
      bitmap->DebugVerifyBitmapIsPreMultiplied();
    }
  }
  if (device) {
    DCHECK(device->GetBPP() == 8 || device->GetBPP() == 32);
    if (device->GetBPP() == 32) {
      device->DebugVerifyBitmapIsPreMultiplied();
    }
  }
}

SkColorType Get32BitSkColorType(bool is_rgb_byte_order) {
  return is_rgb_byte_order ? kRGBA_8888_SkColorType : kBGRA_8888_SkColorType;
}

SkPathFillType GetAlternateOrWindingFillType(
    const CFX_FillRenderOptions& fill_options) {
  // TODO(thestig): This function should be able to assert
  // fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill.
  return fill_options.fill_type == CFX_FillRenderOptions::FillType::kEvenOdd
             ? SkPathFillType::kEvenOdd
             : SkPathFillType::kWinding;
}

SkFont::Edging GetFontEdgingType(const CFX_TextRenderOptions& text_options) {
  if (text_options.aliasing_type == CFX_TextRenderOptions::kAliasing)
    return SkFont::Edging::kAlias;

  if (text_options.aliasing_type == CFX_TextRenderOptions::kAntiAliasing)
    return SkFont::Edging::kAntiAlias;

  DCHECK_EQ(text_options.aliasing_type, CFX_TextRenderOptions::kLcd);
  return SkFont::Edging::kSubpixelAntiAlias;
}

bool IsEvenOddFillType(SkPathFillType fill) {
  return fill == SkPathFillType::kEvenOdd ||
         fill == SkPathFillType::kInverseEvenOdd;
}

bool IsPathAPoint(const SkPath& path) {
  if (path.isEmpty())
    return false;

  if (path.countPoints() == 1)
    return true;

  for (int i = 0; i < path.countPoints() - 1; ++i) {
    if (path.getPoint(i) != path.getPoint(i + 1))
      return false;
  }
  return true;
}

SkPath BuildPath(const CFX_Path& path) {
  SkPath sk_path;
  pdfium::span<const CFX_Path::Point> points = path.GetPoints();
  for (size_t i = 0; i < points.size(); ++i) {
    const CFX_PointF& point = points[i].m_Point;
    CFX_Path::Point::Type point_type = points[i].m_Type;
    if (point_type == CFX_Path::Point::Type::kMove) {
      sk_path.moveTo(point.x, point.y);
    } else if (point_type == CFX_Path::Point::Type::kLine) {
      sk_path.lineTo(point.x, point.y);
    } else if (point_type == CFX_Path::Point::Type::kBezier) {
      const CFX_PointF& point2 = points[i + 1].m_Point;
      const CFX_PointF& point3 = points[i + 2].m_Point;
      sk_path.cubicTo(point.x, point.y, point2.x, point2.y, point3.x, point3.y);
      i += 2;
    }
    if (points[i].m_CloseFigure)
      sk_path.close();
  }
  return sk_path;
}

SkMatrix ToSkMatrix(const CFX_Matrix& m) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a, m.c, m.e, m.b, m.d, m.f, 0, 0, 1);
  return skMatrix;
}

// use when pdf's y-axis points up instead of down
SkMatrix ToFlippedSkMatrix(const CFX_Matrix& m, SkScalar flip) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a * flip, -m.c * flip, m.e, m.b * flip, -m.d * flip, m.f, 0,
                  0, 1);
  return skMatrix;
}

SkBlendMode GetSkiaBlendMode(BlendMode blend_type) {
  switch (blend_type) {
    case BlendMode::kMultiply:
      return SkBlendMode::kMultiply;
    case BlendMode::kScreen:
      return SkBlendMode::kScreen;
    case BlendMode::kOverlay:
      return SkBlendMode::kOverlay;
    case BlendMode::kDarken:
      return SkBlendMode::kDarken;
    case BlendMode::kLighten:
      return SkBlendMode::kLighten;
    case BlendMode::kColorDodge:
      return SkBlendMode::kColorDodge;
    case BlendMode::kColorBurn:
      return SkBlendMode::kColorBurn;
    case BlendMode::kHardLight:
      return SkBlendMode::kHardLight;
    case BlendMode::kSoftLight:
      return SkBlendMode::kSoftLight;
    case BlendMode::kDifference:
      return SkBlendMode::kDifference;
    case BlendMode::kExclusion:
      return SkBlendMode::kExclusion;
    case BlendMode::kHue:
      return SkBlendMode::kHue;
    case BlendMode::kSaturation:
      return SkBlendMode::kSaturation;
    case BlendMode::kColor:
      return SkBlendMode::kColor;
    case BlendMode::kLuminosity:
      return SkBlendMode::kLuminosity;
    case BlendMode::kNormal:
    default:
      return SkBlendMode::kSrcOver;
  }
}

// Add begin & end colors into |skColors| array for each gradient transition.
//
// |is_encode_reversed| must be set to true when the parent function of |pFunc|
// has an Encode array, and the matching pair of encode values for |pFunc| are
// in decreasing order.
bool AddColors(const CPDF_ExpIntFunc* pFunc,
               SkTDArray<SkColor>* skColors,
               bool is_encode_reversed) {
  if (pFunc->CountInputs() != 1)
    return false;
  if (pFunc->GetExponent() != 1)
    return false;
  if (pFunc->GetOrigOutputs() != 3)
    return false;

  pdfium::span<const float> begin_values = pFunc->GetBeginValues();
  pdfium::span<const float> end_values = pFunc->GetEndValues();
  if (is_encode_reversed)
    std::swap(begin_values, end_values);

  skColors->push_back(SkColorSetARGB(0xFF,
                                     SkUnitScalarClampToByte(begin_values[0]),
                                     SkUnitScalarClampToByte(begin_values[1]),
                                     SkUnitScalarClampToByte(begin_values[2])));
  skColors->push_back(SkColorSetARGB(0xFF,
                                     SkUnitScalarClampToByte(end_values[0]),
                                     SkUnitScalarClampToByte(end_values[1]),
                                     SkUnitScalarClampToByte(end_values[2])));
  return true;
}

uint8_t FloatToByte(float f) {
  DCHECK(f >= 0);
  DCHECK(f <= 1);
  return (uint8_t)(f * 255.99f);
}

bool AddSamples(const CPDF_SampledFunc* pFunc,
                SkTDArray<SkColor>* skColors,
                SkTDArray<SkScalar>* skPos) {
  if (pFunc->CountInputs() != 1)
    return false;
  if (pFunc->CountOutputs() != 3)  // expect rgb
    return false;
  if (pFunc->GetEncodeInfo().empty())
    return false;
  const CPDF_SampledFunc::SampleEncodeInfo& encodeInfo =
      pFunc->GetEncodeInfo()[0];
  if (encodeInfo.encode_min != 0)
    return false;
  if (encodeInfo.encode_max != encodeInfo.sizes - 1)
    return false;
  uint32_t sampleSize = pFunc->GetBitsPerSample();
  uint32_t sampleCount = encodeInfo.sizes;
  if (sampleCount != 1U << sampleSize)
    return false;
  if (pFunc->GetSampleStream()->GetSize() < sampleCount * 3 * sampleSize / 8)
    return false;

  float colorsMin[3];
  float colorsMax[3];
  for (int i = 0; i < 3; ++i) {
    colorsMin[i] = pFunc->GetRange(i * 2);
    colorsMax[i] = pFunc->GetRange(i * 2 + 1);
  }
  pdfium::span<const uint8_t> pSampleData = pFunc->GetSampleStream()->GetSpan();
  CFX_BitStream bitstream(pSampleData);
  for (uint32_t i = 0; i < sampleCount; ++i) {
    float floatColors[3];
    for (uint32_t j = 0; j < 3; ++j) {
      float sample = static_cast<float>(bitstream.GetBits(sampleSize));
      float interp = sample / (sampleCount - 1);
      floatColors[j] = colorsMin[j] + (colorsMax[j] - colorsMin[j]) * interp;
    }
    SkColor color =
        SkPackARGB32(0xFF, FloatToByte(floatColors[0]),
                     FloatToByte(floatColors[1]), FloatToByte(floatColors[2]));
    skColors->push_back(color);
    skPos->push_back((float)i / (sampleCount - 1));
  }
  return true;
}

bool AddStitching(const CPDF_StitchFunc* pFunc,
                  SkTDArray<SkColor>* skColors,
                  SkTDArray<SkScalar>* skPos) {
  float boundsStart = pFunc->GetDomain(0);

  const auto& subFunctions = pFunc->GetSubFunctions();
  size_t subFunctionCount = subFunctions.size();
  for (size_t i = 0; i < subFunctionCount; ++i) {
    const CPDF_ExpIntFunc* pSubFunc = subFunctions[i]->ToExpIntFunc();
    if (!pSubFunc)
      return false;
    // Check if the matching encode values are reversed
    bool is_encode_reversed =
        pFunc->GetEncode(2 * i) > pFunc->GetEncode(2 * i + 1);
    if (!AddColors(pSubFunc, skColors, is_encode_reversed))
      return false;
    float boundsEnd =
        i < subFunctionCount - 1 ? pFunc->GetBound(i + 1) : pFunc->GetDomain(1);
    skPos->push_back(boundsStart);
    skPos->push_back(boundsEnd);
    boundsStart = boundsEnd;
  }
  return true;
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
      SkScalar smaller = std::min(sDist, eDist);
      if (minPerpDist > smaller) {
        minPerpDist = smaller;
        minPerpPtIndex = i;
      }
    } else {
      SkScalar larger = std::max(sDist, eDist);
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

  int noClipStartIndex = maxPerpPtIndex;
  int noClipEndIndex = minPerpPtIndex;
  if (beforeNeg)
    std::swap(noClipStartIndex, noClipEndIndex);
  if ((!clipStart && noClipStartIndex < 0) ||
      (!clipEnd && noClipEndIndex < 0)) {
    return;
  }

  const SkPoint& startEdgePt = clipStart ? pts[0] : rectPts[noClipStartIndex];
  const SkPoint& endEdgePt = clipEnd ? pts[1] : rectPts[noClipEndIndex];

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
  if (minBounds < 0 || maxBounds < 0)
    return;
  if (minBounds == maxBounds)
    return;
  // construct a clip parallel to the gradient that goes through
  // rectPts[minBounds] and rectPts[maxBounds] and perpendicular to the
  // gradient that goes through startEdgePt, endEdgePt.
  clip->moveTo(IntersectSides(rectPts[minBounds], slope, startEdgePt));
  clip->lineTo(IntersectSides(rectPts[minBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rectPts[maxBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rectPts[maxBounds], slope, startEdgePt));
}

void SetBitmapMatrix(const CFX_Matrix& m,
                     int width,
                     int height,
                     SkMatrix* skMatrix) {
  skMatrix->setAll(m.a / width, -m.c / height, m.c + m.e, m.b / width,
                   -m.d / height, m.d + m.f, 0, 0, 1);
}

void SetBitmapPaint(bool is_mask,
                    bool anti_alias,
                    int bitmap_alpha,
                    uint32_t argb,
                    BlendMode blend_type,
                    SkPaint* paint) {
  DCHECK_GE(bitmap_alpha, 0);
  DCHECK_LE(bitmap_alpha, 255);

  if (is_mask)
    paint->setColor(argb);
  else if (bitmap_alpha != 255)
    paint->setAlpha(bitmap_alpha);

  paint->setAntiAlias(anti_alias);
  paint->setBlendMode(GetSkiaBlendMode(blend_type));
}

void SetBitmapPaintForMerge(bool is_mask,
                            bool anti_alias,
                            uint32_t argb,
                            int bitmap_alpha,
                            BlendMode blend_type,
                            SkPaint* paint) {
  if (is_mask)
    paint->setColorFilter(SkColorFilters::Blend(argb, SkBlendMode::kSrc));

  paint->setAlpha(bitmap_alpha);
  paint->setAntiAlias(anti_alias);
  paint->setBlendMode(GetSkiaBlendMode(blend_type));
}

bool Upsample(const RetainPtr<CFX_DIBBase>& pSource,
              DataVector<uint8_t>& dst8_storage,
              DataVector<uint32_t>& dst32_storage,
              SkBitmap* skBitmap,
              int* widthPtr,
              int* heightPtr,
              bool forceAlpha,
              bool bRgbByteOrder) {
  void* buffer = pSource->GetBuffer().data();
  if (!buffer)
    return false;
  SkColorType colorType = forceAlpha || pSource->IsMaskFormat()
                              ? SkColorType::kAlpha_8_SkColorType
                              : SkColorType::kGray_8_SkColorType;
  SkAlphaType alphaType = kPremul_SkAlphaType;
  int width = pSource->GetWidth();
  int height = pSource->GetHeight();
  int rowBytes = pSource->GetPitch();
  switch (pSource->GetBPP()) {
    case 1: {
      // By default, the two colors for grayscale are 0xFF and 0x00 unless they
      // are specified in the palette.
      uint8_t color0 = 0x00;
      uint8_t color1 = 0xFF;

      if (pSource->GetFormat() == FXDIB_Format::k1bppRgb &&
          pSource->HasPalette()) {
        // When `pSource` has 1 bit per pixel, its palette will contain 2
        // colors, and the R,G,B channels of either color will have equal
        // values. However, there are cases that R,G,B are not all the same and
        // these 2 colors stand for the boundaries of a range of colors to be
        // used for bitmap rendering. We need to handle the color palette
        // differently depending on these conditions.
        uint32_t palette_color0 = pSource->GetPaletteArgb(0);
        uint32_t palette_color1 = pSource->GetPaletteArgb(1);
        bool use_two_colors = IsRGBColorGrayScale(palette_color0) &&
                              IsRGBColorGrayScale(palette_color1);
        if (!use_two_colors) {
          // If more than 2 colors are provided, calculate the range of colors
          // and store them in `new_palette`
          FX_ARGB new_palette[CFX_DIBBase::kPaletteSize];
          CFX_ImageStretcher::BuildPaletteFrom1BppSource(pSource, new_palette);
          dst32_storage = Fill32BppDestStorageWithPalette(pSource, new_palette);
          buffer = dst32_storage.data();
          rowBytes = width * sizeof(uint32_t);
          colorType = Get32BitSkColorType(bRgbByteOrder);
          break;
        }

        color0 = FXARGB_R(palette_color0);
        color1 = FXARGB_R(palette_color1);
      }

      dst8_storage = DataVector<uint8_t>(Fx2DSizeOrDie(width, height));
      pdfium::span<uint8_t> dst8_pixels(dst8_storage);
      for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow =
            static_cast<const uint8_t*>(buffer) + y * rowBytes;
        pdfium::span<uint8_t> dst_row = dst8_pixels.subspan(y * width);
        for (int x = 0; x < width; ++x)
          dst_row[x] = srcRow[x >> 3] & (1 << (~x & 0x07)) ? color1 : color0;
      }
      buffer = dst8_storage.data();
      rowBytes = width;
      break;
    }
    case 8:
      // we upscale ctables to 32bit.
      if (pSource->HasPalette()) {
        const size_t src_palette_size = pSource->GetRequiredPaletteSize();
        pdfium::span<const uint32_t> src_palette = pSource->GetPaletteSpan();
        CHECK_LE(src_palette_size, src_palette.size());
        if (src_palette_size < src_palette.size())
          src_palette = src_palette.first(src_palette_size);

        dst32_storage = Fill32BppDestStorageWithPalette(pSource, src_palette);
        buffer = dst32_storage.data();
        rowBytes = width * sizeof(uint32_t);
        colorType = Get32BitSkColorType(bRgbByteOrder);
      }
      break;
    case 24: {
      dst32_storage = DataVector<uint32_t>(Fx2DSizeOrDie(width, height));
      pdfium::span<uint32_t> dst32_pixels(dst32_storage);
      for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow =
            static_cast<const uint8_t*>(buffer) + y * rowBytes;
        pdfium::span<uint32_t> dst_row = dst32_pixels.subspan(y * width);
        for (int x = 0; x < width; ++x) {
          dst_row[x] = SkPackARGB32(0xFF, srcRow[x * 3 + 2], srcRow[x * 3 + 1],
                                    srcRow[x * 3 + 0]);
        }
      }
      buffer = dst32_storage.data();
      rowBytes = width * sizeof(uint32_t);
      colorType = Get32BitSkColorType(bRgbByteOrder);
      alphaType = kOpaque_SkAlphaType;
      break;
    }
    case 32:
      colorType = Get32BitSkColorType(bRgbByteOrder);
      pSource->DebugVerifyBufferIsPreMultiplied(buffer);
      break;
    default:
      NOTREACHED();  // TODO(bug_11) ensure that all cases are covered
      colorType = SkColorType::kUnknown_SkColorType;
  }
  SkImageInfo imageInfo =
      SkImageInfo::Make(width, height, colorType, alphaType);
  skBitmap->installPixels(imageInfo, buffer, rowBytes);
  *widthPtr = width;
  *heightPtr = height;
  return true;
}

// Makes a bitmap filled with a solid color for debugging with `SkPicture`.
RetainPtr<CFX_DIBitmap> MakeDebugBitmap(int width, int height, uint32_t color) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!bitmap->Create(width, height, FXDIB_Format::kArgb))
    return nullptr;

  bitmap->Clear(color);
  return bitmap;
}

}  // namespace

// Encapsulate the state used for successive text and path draws so that
// they can be combined.
class SkiaState {
 public:
  enum class Clip {
    kSave,
    kPath,
  };

  enum class Accumulator {
    kNone,
    kPath,
    kText,
    kOther,
  };

  // mark all cached state as uninitialized
  explicit SkiaState(CFX_SkiaDeviceDriver* pDriver) : m_pDriver(pDriver) {}

  void DrawPath(const CFX_Path& path,
                const CFX_Matrix* pMatrix,
                const CFX_GraphStateData* pDrawState,
                uint32_t fill_color,
                uint32_t stroke_color,
                const CFX_FillRenderOptions& fill_options,
                BlendMode blend_type) {
    int drawIndex = std::min(m_drawIndex, m_commands.size());
    if (Accumulator::kText == m_type || drawIndex != m_commandIndex ||
        (Accumulator::kPath == m_type &&
         DrawChanged(pMatrix, pDrawState, fill_color, stroke_color,
                     fill_options.fill_type, blend_type,
                     m_pDriver->GetGroupKnockout()))) {
      Flush();
    }
    if (Accumulator::kPath != m_type) {
      m_skPath.reset();
      m_fillOptions = fill_options;
      m_fillPath =
          fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill &&
          fill_color;
      m_skPath.setFillType(GetAlternateOrWindingFillType(fill_options));
      if (pDrawState)
        m_drawState = *pDrawState;
      m_fillColor = fill_color;
      m_strokeColor = stroke_color;
      m_blendType = blend_type;
      m_groupKnockout = m_pDriver->GetGroupKnockout();
      if (pMatrix)
        m_drawMatrix = *pMatrix;
      m_drawIndex = m_commandIndex;
      m_type = Accumulator::kPath;
    }
    SkPath skPath = BuildPath(path);
    SkPoint delta;
    if (MatrixOffset(pMatrix, &delta))
      skPath.offset(delta.fX, delta.fY);
    m_skPath.addPath(skPath);
  }

  void FlushPath() {
    SkMatrix skMatrix = ToSkMatrix(m_drawMatrix);
    SkPaint skPaint;
    skPaint.setAntiAlias(!m_fillOptions.aliased_path);
    if (m_fillOptions.full_cover)
      skPaint.setBlendMode(SkBlendMode::kPlus);
    int stroke_alpha = FXARGB_A(m_strokeColor);
    if (stroke_alpha)
      m_pDriver->PaintStroke(&skPaint, &m_drawState, skMatrix);
    SkCanvas* skCanvas = m_pDriver->SkiaCanvas();
    SkAutoCanvasRestore scoped_save_restore(skCanvas, /*doSave=*/true);
    skCanvas->concat(skMatrix);
    bool do_stroke = true;
    if (m_fillPath) {
      SkPath strokePath;
      const SkPath* fillPath = &m_skPath;
      if (stroke_alpha) {
        if (m_groupKnockout) {
          FillPathWithPaint(m_skPath, skPaint, &strokePath);
          if (m_strokeColor == m_fillColor &&
              Op(m_skPath, strokePath, SkPathOp::kUnion_SkPathOp,
                 &strokePath)) {
            fillPath = &strokePath;
            do_stroke = false;
          } else if (Op(m_skPath, strokePath, SkPathOp::kDifference_SkPathOp,
                        &strokePath)) {
            fillPath = &strokePath;
          }
        }
      }
      skPaint.setStyle(SkPaint::kFill_Style);
      skPaint.setColor(m_fillColor);
      DebugShowSkiaDrawPath(m_pDriver, skCanvas, skPaint, *fillPath);
      skCanvas->drawPath(*fillPath, skPaint);
    }
    if (stroke_alpha && do_stroke) {
      skPaint.setStyle(SkPaint::kStroke_Style);
      skPaint.setColor(m_strokeColor);
      if (!m_skPath.isLastContourClosed() && IsPathAPoint(m_skPath)) {
        DCHECK_GE(m_skPath.countPoints(), 1);
        skCanvas->drawPoint(m_skPath.getPoint(0), skPaint);
      } else {
        DebugShowSkiaDrawPath(m_pDriver, skCanvas, skPaint, m_skPath);
        skCanvas->drawPath(m_skPath, skPaint);
      }
    }
    m_drawIndex = INT_MAX;
    m_type = Accumulator::kNone;
    m_drawMatrix = CFX_Matrix();
  }

  bool HasRSX(int nChars,
              const TextCharPos* pCharPos,
              float* scaleXPtr,
              bool* oneAtATimePtr) const {
    bool useRSXform = false;
    bool oneAtATime = false;
    float scaleX = 1;
    for (int index = 0; index < nChars; ++index) {
      const TextCharPos& cp = pCharPos[index];
      if (!cp.m_bGlyphAdjust)
        continue;
      bool upright = 0 == cp.m_AdjustMatrix[1] && 0 == cp.m_AdjustMatrix[2];
      if (cp.m_AdjustMatrix[0] != cp.m_AdjustMatrix[3]) {
        if (upright && 1 == cp.m_AdjustMatrix[3]) {
          if (1 == scaleX)
            scaleX = cp.m_AdjustMatrix[0];
          else if (scaleX != cp.m_AdjustMatrix[0])
            oneAtATime = true;
        } else {
          oneAtATime = true;
        }
      } else if (cp.m_AdjustMatrix[1] != -cp.m_AdjustMatrix[2]) {
        oneAtATime = true;
      } else {
        useRSXform = true;
      }
    }
    *oneAtATimePtr = oneAtATime;
    *scaleXPtr = oneAtATime ? 1 : scaleX;
    return oneAtATime ? false : useRSXform;
  }

  bool DrawText(int nChars,
                const TextCharPos* pCharPos,
                CFX_Font* pFont,
                const CFX_Matrix& matrix,
                float font_size,
                uint32_t color,
                const CFX_TextRenderOptions& options) {
    float scaleX = 1;
    bool oneAtATime = false;
    bool hasRSX = HasRSX(nChars, pCharPos, &scaleX, &oneAtATime);
    if (oneAtATime) {
      Flush();
      return false;
    }
    int drawIndex = std::min(m_drawIndex, m_commands.size());
    if (Accumulator::kPath == m_type || drawIndex != m_commandIndex ||
        (Accumulator::kText == m_type &&
         (FontChanged(pFont, matrix, font_size, scaleX, color, options) ||
          hasRSX == m_rsxform.empty()))) {
      Flush();
    }
    if (Accumulator::kText != m_type) {
      m_italicAngle = pFont->GetSubstFontItalicAngle();
      m_isSubstFontBold = pFont->IsSubstFontBold();
      m_charDetails.SetCount(0);
      m_rsxform.resize(0);
      if (pFont->GetFaceRec())
        m_pTypeFace.reset(SkSafeRef(pFont->GetDeviceCache()));
      else
        m_pTypeFace.reset();
      m_fontSize = font_size;
      m_scaleX = scaleX;
      m_fillColor = color;
      m_drawMatrix = matrix;
      m_drawIndex = m_commandIndex;
      m_type = Accumulator::kText;
      m_pFont = pFont;
      m_textOptions = options;
    }
    if (!hasRSX && !m_rsxform.empty())
      FlushText();

    int count = m_charDetails.Count();
    m_charDetails.SetCount(nChars + count);
    if (hasRSX)
      m_rsxform.resize(nChars + count);

    const SkScalar flip = m_fontSize < 0 ? -1 : 1;
    const SkScalar vFlip = pFont->IsVertical() ? -1 : 1;
    for (int index = 0; index < nChars; ++index) {
      const TextCharPos& cp = pCharPos[index];
      int cur_index = index + count;
      m_charDetails.SetPositionAt(
          cur_index, {cp.m_Origin.x * flip, cp.m_Origin.y * vFlip});
      m_charDetails.SetGlyphAt(cur_index,
                               static_cast<uint16_t>(cp.m_GlyphIndex));
      m_charDetails.SetFontCharWidthAt(cur_index, cp.m_FontCharWidth);
#if BUILDFLAG(IS_APPLE)
      if (cp.m_ExtGID) {
        m_charDetails.SetGlyphAt(cur_index, static_cast<uint16_t>(cp.m_ExtGID));
      }
#endif
    }
    SkPoint delta;
    if (MatrixOffset(&matrix, &delta)) {
      for (int index = 0; index < nChars; ++index) {
        m_charDetails.OffsetPositionAt(index + count, delta.fX * flip,
                                       -delta.fY * flip);
      }
    }
    if (hasRSX) {
      const SkTDArray<SkPoint>& positions = m_charDetails.GetPositions();
      for (int index = 0; index < nChars; ++index) {
        const TextCharPos& cp = pCharPos[index];
        SkRSXform* rsxform = &m_rsxform[index + count];
        if (cp.m_bGlyphAdjust) {
          rsxform->fSCos = cp.m_AdjustMatrix[0];
          rsxform->fSSin = cp.m_AdjustMatrix[1];
          rsxform->fTx = cp.m_AdjustMatrix[0] * positions[index].fX;
          rsxform->fTy = -cp.m_AdjustMatrix[3] * positions[index].fY;
        } else {
          rsxform->fSCos = 1;
          rsxform->fSSin = 0;
          rsxform->fTx = positions[index].fX;
          rsxform->fTy = positions[index].fY;
        }
      }
    }
    return true;
  }

  void FlushText() {
    SkPaint skPaint;
    skPaint.setAntiAlias(true);
    skPaint.setColor(m_fillColor);

    SkFont font;
    if (m_pTypeFace) {  // exclude placeholder test fonts
      font.setTypeface(m_pTypeFace);
    }
    font.setEmbolden(m_isSubstFontBold);
    font.setHinting(SkFontHinting::kNone);
    font.setScaleX(m_scaleX);
    font.setSkewX(tanf(m_italicAngle * FXSYS_PI / 180.0));
    font.setSize(SkTAbs(m_fontSize));
    font.setSubpixel(true);
    font.setEdging(GetFontEdgingType(m_textOptions));

    SkCanvas* skCanvas = m_pDriver->SkiaCanvas();
    SkAutoCanvasRestore scoped_save_restore(skCanvas, /*doSave=*/true);
    SkScalar flip = m_fontSize < 0 ? -1 : 1;
    SkMatrix skMatrix = ToFlippedSkMatrix(m_drawMatrix, flip);
    skCanvas->concat(skMatrix);
    const SkTDArray<uint16_t>& glyphs = m_charDetails.GetGlyphs();
    if (m_rsxform.size()) {
      sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromRSXform(
          glyphs.begin(), glyphs.size_bytes(), m_rsxform.begin(), font,
          SkTextEncoding::kGlyphID);
      skCanvas->drawTextBlob(blob, 0, 0, skPaint);
    } else {
      const SkTDArray<SkPoint>& positions = m_charDetails.GetPositions();
      for (int i = 0; i < m_charDetails.Count(); ++i) {
        sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText(
            &glyphs[i], sizeof(glyphs[i]), font, SkTextEncoding::kGlyphID);
        skCanvas->drawTextBlob(blob, positions[i].fX, positions[i].fY, skPaint);
      }
    }

    m_drawIndex = INT_MAX;
    m_type = Accumulator::kNone;
    m_drawMatrix = CFX_Matrix();
    m_pFont = nullptr;
    m_italicAngle = 0;
    m_isSubstFontBold = false;
    m_textOptions = CFX_TextRenderOptions();
  }

  bool IsEmpty() const { return m_commands.empty(); }

  void SetClipFill(const CFX_Path& path,
                   const CFX_Matrix* pMatrix,
                   const CFX_FillRenderOptions& fill_options) {
    SkPath skClipPath;
    if (path.GetPoints().size() == 5 || path.GetPoints().size() == 4) {
      absl::optional<CFX_FloatRect> maybe_rectf = path.GetRect(pMatrix);
      if (maybe_rectf.has_value()) {
        CFX_FloatRect& rectf = maybe_rectf.value();
        rectf.Intersect(CFX_FloatRect(
            0, 0,
            static_cast<float>(m_pDriver->GetDeviceCaps(FXDC_PIXEL_WIDTH)),
            static_cast<float>(m_pDriver->GetDeviceCaps(FXDC_PIXEL_HEIGHT))));
        FX_RECT outer = rectf.GetOuterRect();
        // note that PDF's y-axis goes up; Skia's y-axis goes down
        skClipPath.addRect({(float)outer.left, (float)outer.bottom,
                            (float)outer.right, (float)outer.top});
      }
    }
    if (skClipPath.isEmpty()) {
      skClipPath = BuildPath(path);
      skClipPath.setFillType(GetAlternateOrWindingFillType(fill_options));
      SkMatrix skMatrix = ToSkMatrix(*pMatrix);
      skClipPath.transform(skMatrix);
    }
    SetClip(skClipPath);
  }

  void SetClip(const SkPath& skClipPath) {
    // if a pending draw depends on clip state that is cached, flush it and draw
    if (m_commandIndex < m_commands.size()) {
      if (m_commands[m_commandIndex] == Clip::kPath &&
          m_clips[m_commandIndex] == skClipPath) {
        ++m_commandIndex;
        return;
      }
      Flush();
    }
    while (m_clipIndex > m_commandIndex) {
      do {
        --m_clipIndex;
        DCHECK(m_clipIndex >= 0);
      } while (m_commands[m_clipIndex] != Clip::kSave);
      m_pDriver->SkiaCanvas()->restore();
    }
    if (m_commandIndex < m_commands.size()) {
      m_commands[m_commandIndex] = Clip::kPath;
      m_clips[m_commandIndex] = skClipPath;
    } else {
      m_commands.push_back(Clip::kPath);
      m_clips.push_back(skClipPath);
    }
    ++m_commandIndex;
  }

  void SetClipStroke(const CFX_Path& path,
                     const CFX_Matrix* pMatrix,
                     const CFX_GraphStateData* pGraphState) {
    SkPath skPath = BuildPath(path);
    SkMatrix skMatrix = ToSkMatrix(*pMatrix);
    SkPaint skPaint;
    m_pDriver->PaintStroke(&skPaint, pGraphState, skMatrix);
    SkPath dst_path;
    FillPathWithPaint(skPath, skPaint, &dst_path);
    dst_path.transform(skMatrix);
    SetClip(dst_path);
  }

  bool MatrixOffset(const CFX_Matrix* pMatrix, SkPoint* delta) {
    CFX_Matrix identityMatrix;
    if (!pMatrix)
      pMatrix = &identityMatrix;
    delta->set(pMatrix->e - m_drawMatrix.e, pMatrix->f - m_drawMatrix.f);
    if (!delta->fX && !delta->fY)
      return true;
    SkMatrix drawMatrix = ToSkMatrix(m_drawMatrix);
    if (!(drawMatrix.getType() & ~SkMatrix::kTranslate_Mask))
      return true;
    SkMatrix invDrawMatrix;
    if (!drawMatrix.invert(&invDrawMatrix))
      return false;
    SkMatrix invNewMatrix;
    SkMatrix newMatrix = ToSkMatrix(*pMatrix);
    if (!newMatrix.invert(&invNewMatrix))
      return false;
    delta->set(invDrawMatrix.getTranslateX() - invNewMatrix.getTranslateX(),
               invDrawMatrix.getTranslateY() - invNewMatrix.getTranslateY());
    return true;
  }

  void ClipSave() {
    int count = m_commands.size();
    if (m_commandIndex < count) {
      if (Clip::kSave == m_commands[m_commandIndex]) {
        ++m_commandIndex;
        return;
      }
      Flush();
      AdjustClip(m_commandIndex);
      m_commands[m_commandIndex] = Clip::kSave;
      m_clips[m_commandIndex] = m_skEmptyPath;
    } else {
      AdjustClip(m_commandIndex);
      m_commands.push_back(Clip::kSave);
      m_clips.push_back(m_skEmptyPath);
    }
    ++m_commandIndex;
  }

  void ClipRestore() {
    for (int i = m_commandIndex - 1; i > 0; --i) {
      if (m_commands[i] == Clip::kSave) {
        m_commandIndex = i;
        break;
      }
    }
  }

  bool DrawChanged(const CFX_Matrix* pMatrix,
                   const CFX_GraphStateData* pState,
                   uint32_t fill_color,
                   uint32_t stroke_color,
                   CFX_FillRenderOptions::FillType fill_type,
                   BlendMode blend_type,
                   bool group_knockout) const {
    return MatrixChanged(pMatrix) || StateChanged(pState, m_drawState) ||
           fill_color != m_fillColor || stroke_color != m_strokeColor ||
           IsEvenOddFillType(m_skPath.getFillType()) ||
           fill_type == CFX_FillRenderOptions::FillType::kEvenOdd ||
           blend_type != m_blendType || group_knockout != m_groupKnockout;
  }

  bool FontChanged(CFX_Font* pFont,
                   const CFX_Matrix& matrix,
                   float font_size,
                   float scaleX,
                   uint32_t color,
                   const CFX_TextRenderOptions& options) const {
    CFX_TypeFace* typeface =
        pFont->GetFaceRec() ? pFont->GetDeviceCache() : nullptr;
    return typeface != m_pTypeFace.get() || MatrixChanged(&matrix) ||
           font_size != m_fontSize || scaleX != m_scaleX ||
           color != m_fillColor ||
           pFont->GetSubstFontItalicAngle() != m_italicAngle ||
           pFont->IsSubstFontBold() != m_isSubstFontBold ||
           options != m_textOptions;
  }

  bool MatrixChanged(const CFX_Matrix* pMatrix) const {
    return pMatrix ? *pMatrix != m_drawMatrix : m_drawMatrix.IsIdentity();
  }

  bool StateChanged(const CFX_GraphStateData* pState,
                    const CFX_GraphStateData& refState) const {
    CFX_GraphStateData identityState;
    if (!pState)
      pState = &identityState;
    return pState->m_LineWidth != refState.m_LineWidth ||
           pState->m_LineCap != refState.m_LineCap ||
           pState->m_LineJoin != refState.m_LineJoin ||
           pState->m_MiterLimit != refState.m_MiterLimit ||
           DashChanged(pState, refState);
  }

  bool DashChanged(const CFX_GraphStateData* pState,
                   const CFX_GraphStateData& refState) const {
    bool dashArray = pState && !pState->m_DashArray.empty();
    if (!dashArray && refState.m_DashArray.empty())
      return false;
    if (!dashArray || refState.m_DashArray.empty())
      return true;
    return pState->m_DashPhase != refState.m_DashPhase ||
           pState->m_DashArray != refState.m_DashArray;
  }

  void AdjustClip(int limit) {
    while (m_clipIndex > limit) {
      do {
        --m_clipIndex;
      } while (m_clipIndex >= 0 && m_commands[m_clipIndex] != Clip::kSave);
      if (m_clipIndex >= 0)
        m_pDriver->SkiaCanvas()->restore();
      else
        m_clipIndex = 0;
    }
    while (m_clipIndex < limit) {
      if (Clip::kSave == m_commands[m_clipIndex]) {
        m_pDriver->SkiaCanvas()->save();
      } else {
        DCHECK_EQ(Clip::kPath, m_commands[m_clipIndex]);
        m_pDriver->SkiaCanvas()->clipPath(m_clips[m_clipIndex],
                                          SkClipOp::kIntersect, true);
      }
      ++m_clipIndex;
    }
  }

  void Flush() {
    if (Accumulator::kPath == m_type || Accumulator::kText == m_type) {
      AdjustClip(std::min(m_drawIndex, m_commands.size()));
      Accumulator::kPath == m_type ? FlushPath() : FlushText();
    }
  }

  void FlushForDraw() {
    Flush();                     // draw any pending text or path
    AdjustClip(m_commandIndex);  // set up clip stack with any pending state
  }

 private:
  class CharDetail {
   public:
    CharDetail() = default;
    ~CharDetail() = default;

    const SkTDArray<SkPoint>& GetPositions() const { return m_positions; }
    void SetPositionAt(int index, const SkPoint& position) {
      m_positions[index] = position;
    }
    void OffsetPositionAt(int index, SkScalar dx, SkScalar dy) {
      m_positions[index].offset(dx, dy);
    }
    const SkTDArray<uint16_t>& GetGlyphs() const { return m_glyphs; }
    void SetGlyphAt(int index, uint16_t glyph) { m_glyphs[index] = glyph; }
    const SkTDArray<uint32_t>& GetFontCharWidths() const {
      return m_fontCharWidths;
    }
    void SetFontCharWidthAt(int index, uint32_t width) {
      m_fontCharWidths[index] = width;
    }
    int Count() const {
      DCHECK_EQ(m_positions.size(), m_glyphs.size());
      return m_glyphs.size();
    }
    void SetCount(int count) {
      DCHECK(count >= 0);
      m_positions.resize(count);
      m_glyphs.resize(count);
      m_fontCharWidths.resize(count);
    }

   private:
    SkTDArray<SkPoint> m_positions;  // accumulator for text positions
    SkTDArray<uint16_t> m_glyphs;    // accumulator for text glyphs
    // accumulator for glyphs' width defined in pdf
    SkTDArray<uint32_t> m_fontCharWidths;
  };

  SkTArray<SkPath> m_clips;        // stack of clips that may be reused
  SkTDArray<Clip> m_commands;      // stack of clip-related commands
  CharDetail m_charDetails;
  SkTDArray<SkRSXform> m_rsxform;  // accumulator for txt rotate/scale/translate
  SkPath m_skPath;                 // accumulator for path contours
  SkPath m_skEmptyPath;            // used as placehold in the clips array
  UnownedPtr<CFX_Font> m_pFont;
  CFX_Matrix m_drawMatrix;
  CFX_GraphStateData m_clipState;
  CFX_GraphStateData m_drawState;
  CFX_Matrix m_clipMatrix;
  CFX_FillRenderOptions m_fillOptions;
  CFX_TextRenderOptions m_textOptions;
  UnownedPtr<CFX_SkiaDeviceDriver> const m_pDriver;
  sk_sp<CFX_TypeFace> m_pTypeFace;
  float m_fontSize = 0;
  float m_scaleX = 0;
  uint32_t m_fillColor = 0;
  uint32_t m_strokeColor = 0;
  BlendMode m_blendType = BlendMode::kNormal;
  int m_commandIndex = 0;     // active position in clip command stack
  int m_drawIndex = INT_MAX;  // position of the pending path or text draw
  int m_clipIndex = 0;        // position reflecting depth of canvas clip stacck
  int m_italicAngle = 0;
  Accumulator m_type = Accumulator::kNone;  // type of pending draw
  bool m_fillPath = false;
  bool m_groupKnockout = false;
  bool m_isSubstFontBold = false;
};

// convert a stroking path to scanlines
void CFX_SkiaDeviceDriver::PaintStroke(SkPaint* spaint,
                                       const CFX_GraphStateData* pGraphState,
                                       const SkMatrix& matrix) {
  SkPaint::Cap cap;
  switch (pGraphState->m_LineCap) {
    case CFX_GraphStateData::LineCap::kRound:
      cap = SkPaint::kRound_Cap;
      break;
    case CFX_GraphStateData::LineCap::kSquare:
      cap = SkPaint::kSquare_Cap;
      break;
    default:
      cap = SkPaint::kButt_Cap;
      break;
  }
  SkPaint::Join join;
  switch (pGraphState->m_LineJoin) {
    case CFX_GraphStateData::LineJoin::kRound:
      join = SkPaint::kRound_Join;
      break;
    case CFX_GraphStateData::LineJoin::kBevel:
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
  float width =
      std::max(pGraphState->m_LineWidth,
               std::min(deviceUnits[0].length(), deviceUnits[1].length()));
  if (!pGraphState->m_DashArray.empty()) {
    size_t count = (pGraphState->m_DashArray.size() + 1) / 2;
    DataVector<SkScalar> intervals(count * 2);
    // Set dash pattern
    for (size_t i = 0; i < count; i++) {
      float on = pGraphState->m_DashArray[i * 2];
      if (on <= 0.000001f)
        on = 0.1f;
      float off = i * 2 + 1 == pGraphState->m_DashArray.size()
                      ? on
                      : pGraphState->m_DashArray[i * 2 + 1];
      off = std::max(off, 0.0f);
      intervals[i * 2] = on;
      intervals[i * 2 + 1] = off;
    }
    spaint->setPathEffect(SkDashPathEffect::Make(
        intervals.data(), pdfium::base::checked_cast<int>(intervals.size()),
        pGraphState->m_DashPhase));
  }
  spaint->setStyle(SkPaint::kStroke_Style);
  spaint->setAntiAlias(!m_FillOptions.aliased_path);
  spaint->setStrokeWidth(width);
  spaint->setStrokeMiter(pGraphState->m_MiterLimit);
  spaint->setStrokeCap(cap);
  spaint->setStrokeJoin(join);
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout)
    : m_pBitmap(pBitmap),
      m_pBackdropBitmap(pBackdropBitmap),
      m_pRecorder(nullptr),
      m_pCache(std::make_unique<SkiaState>(this)),
      m_bRgbByteOrder(bRgbByteOrder),
      m_bGroupKnockout(bGroupKnockout) {
  SkBitmap skBitmap;
  SkColorType color_type;
  const int bpp = pBitmap->GetBPP();
  if (bpp == 8) {
    color_type = pBitmap->IsAlphaFormat() || pBitmap->IsMaskFormat()
                     ? kAlpha_8_SkColorType
                     : kGray_8_SkColorType;
  } else {
    DCHECK_EQ(bpp, 32);
    color_type = Get32BitSkColorType(bRgbByteOrder);
  }

  SkImageInfo imageInfo =
      SkImageInfo::Make(pBitmap->GetWidth(), pBitmap->GetHeight(), color_type,
                        kPremul_SkAlphaType);
  skBitmap.installPixels(imageInfo, pBitmap->GetBuffer().data(),
                         pBitmap->GetPitch());
  m_pCanvas = new SkCanvas(skBitmap);
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(SkPictureRecorder* recorder)
    : m_pRecorder(recorder),
      m_pCache(std::make_unique<SkiaState>(this)),
      m_bGroupKnockout(false) {
  m_pCanvas = m_pRecorder->getRecordingCanvas();
  int width = m_pCanvas->imageInfo().width();
  int height = m_pCanvas->imageInfo().height();
  DCHECK_EQ(kUnknown_SkColorType, m_pCanvas->imageInfo().colorType());

  constexpr uint32_t kMagenta = 0xffff00ff;
  constexpr uint32_t kGreen = 0xff00ff00;
  m_pBitmap = MakeDebugBitmap(width, height, kMagenta);
  m_pBackdropBitmap = MakeDebugBitmap(width, height, kGreen);
}

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
  Flush();
  if (!m_pRecorder)
    delete m_pCanvas;
}

void CFX_SkiaDeviceDriver::Flush() {
  m_pCache->Flush();
}

void CFX_SkiaDeviceDriver::PreMultiply() {
  m_pBitmap->PreMultiply();
}

bool CFX_SkiaDeviceDriver::DrawDeviceText(
    pdfium::span<const TextCharPos> pCharPos,
    CFX_Font* pFont,
    const CFX_Matrix& mtObject2Device,
    float font_size,
    uint32_t color,
    const CFX_TextRenderOptions& options) {
  // `SkTextBlob` is built from `pFont`'s font data. If `pFont` doesn't contain
  // any font data, each text blob will have zero area to be drawn and the
  // drawing command will be rejected. In this case, we fall back to drawing
  // characters by their glyph bitmaps.
  if (pFont->GetFontSpan().empty())
    return false;

  // If a glyph's default width is no less than its width defined in the PDF,
  // draw the glyph with path since it can be scaled to avoid overlapping with
  // the adjacent glyphs (if there are any). Otherwise, use the device driver
  // to render the glyph without any adjustments.
  const CFX_SubstFont* subst_font = pFont->GetSubstFont();
  const int subst_font_weight =
      (subst_font && subst_font->IsBuiltInGenericFont()) ? subst_font->m_Weight
                                                         : 0;
  for (const TextCharPos& cp : pCharPos) {
    const int glyph_width = pFont->GetGlyphWidth(
        cp.m_GlyphIndex, cp.m_FontCharWidth, subst_font_weight);
    if (cp.m_FontCharWidth <= glyph_width)
      return false;
  }

  int nChars = fxcrt::CollectionSize<int>(pCharPos);
  if (m_pCache->DrawText(nChars, pCharPos.data(), pFont, mtObject2Device,
                         font_size, color, options)) {
    return true;
  }
  sk_sp<SkTypeface> typeface(SkSafeRef(pFont->GetDeviceCache()));
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(color);

  SkFont font;
  font.setTypeface(typeface);
  font.setEmbolden(pFont->IsSubstFontBold());
  font.setHinting(SkFontHinting::kNone);
  font.setSize(SkTAbs(font_size));
  font.setSubpixel(true);
  font.setSkewX(tanf(pFont->GetSubstFontItalicAngle() * FXSYS_PI / 180.0));
  font.setEdging(GetFontEdgingType(options));

  SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
  const SkScalar flip = font_size < 0 ? -1 : 1;
  const SkScalar vFlip = pFont->IsVertical() ? -1 : 1;
  SkMatrix skMatrix = ToFlippedSkMatrix(mtObject2Device, flip);
  m_pCanvas->concat(skMatrix);
  SkTDArray<SkPoint> positions;
  positions.resize(nChars);
  SkTDArray<uint16_t> glyphs;
  glyphs.resize(nChars);
  bool useRSXform = false;
  bool oneAtATime = false;
  for (int index = 0; index < nChars; ++index) {
    const TextCharPos& cp = pCharPos[index];
    positions[index] = {cp.m_Origin.x * flip, cp.m_Origin.y * vFlip};
    if (cp.m_bGlyphAdjust) {
      useRSXform = true;
      if (cp.m_AdjustMatrix[0] != cp.m_AdjustMatrix[3] ||
          cp.m_AdjustMatrix[1] != -cp.m_AdjustMatrix[2]) {
        oneAtATime = true;
      }
    }
    glyphs[index] = static_cast<uint16_t>(cp.m_GlyphIndex);
#if BUILDFLAG(IS_APPLE)
    if (cp.m_ExtGID)
      glyphs[index] = static_cast<uint16_t>(cp.m_ExtGID);
#endif
  }
  if (oneAtATime)
    useRSXform = false;
  if (useRSXform) {
    SkTDArray<SkRSXform> xforms;
    xforms.resize(nChars);
    for (int index = 0; index < nChars; ++index) {
      const TextCharPos& cp = pCharPos[index];
      SkRSXform* rsxform = &xforms[index];
      if (cp.m_bGlyphAdjust) {
        rsxform->fSCos = cp.m_AdjustMatrix[0];
        rsxform->fSSin = cp.m_AdjustMatrix[1];
        rsxform->fTx = cp.m_AdjustMatrix[0] * positions[index].fX;
        rsxform->fTy = -cp.m_AdjustMatrix[3] * positions[index].fY;
      } else {
        rsxform->fSCos = 1;
        rsxform->fSSin = 0;
        rsxform->fTx = positions[index].fX;
        rsxform->fTy = positions[index].fY;
      }
    }
    m_pCanvas->drawTextBlob(
        SkTextBlob::MakeFromRSXform(glyphs.begin(), nChars * 2, xforms.begin(),
                                    font, SkTextEncoding::kGlyphID),
        0, 0, paint);
  } else if (oneAtATime) {
    for (int index = 0; index < nChars; ++index) {
      const TextCharPos& cp = pCharPos[index];
      if (cp.m_bGlyphAdjust) {
        if (0 == cp.m_AdjustMatrix[1] && 0 == cp.m_AdjustMatrix[2] &&
            1 == cp.m_AdjustMatrix[3]) {
          font.setScaleX(cp.m_AdjustMatrix[0]);
          auto blob =
              SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]),
                                       font, SkTextEncoding::kGlyphID);
          m_pCanvas->drawTextBlob(blob, positions[index].fX,
                                  positions[index].fY, paint);
          font.setScaleX(SkIntToScalar(1));
        } else {
          SkAutoCanvasRestore scoped_save_restore2(m_pCanvas, /*doSave=*/true);
          SkMatrix adjust;
          adjust.preTranslate(positions[index].fX, -positions[index].fY);
          adjust.setScaleX(cp.m_AdjustMatrix[0]);
          adjust.setSkewX(cp.m_AdjustMatrix[1]);
          adjust.setSkewY(cp.m_AdjustMatrix[2]);
          adjust.setScaleY(cp.m_AdjustMatrix[3]);
          m_pCanvas->concat(adjust);
          auto blob =
              SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]),
                                       font, SkTextEncoding::kGlyphID);
          m_pCanvas->drawTextBlob(blob, 0, 0, paint);
        }
      } else {
        auto blob =
            SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]),
                                     font, SkTextEncoding::kGlyphID);
        m_pCanvas->drawTextBlob(blob, positions[index].fX, positions[index].fY,
                                paint);
      }
    }
  } else {
    for (int index = 0; index < nChars; ++index) {
      auto blob =
          SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]), font,
                                   SkTextEncoding::kGlyphID);
      m_pCanvas->drawTextBlob(blob, positions[index].fX, positions[index].fY,
                              paint);
    }
  }

  return true;
}

int CFX_SkiaDeviceDriver::GetDriverType() const {
  return 1;
}

DeviceType CFX_SkiaDeviceDriver::GetDeviceType() const {
  return DeviceType::kDisplay;
}

int CFX_SkiaDeviceDriver::GetDeviceCaps(int caps_id) const {
  switch (caps_id) {
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
    default:
      NOTREACHED();
      return 0;
  }
}

void CFX_SkiaDeviceDriver::SaveState() {
  m_pCache->ClipSave();
}

void CFX_SkiaDeviceDriver::RestoreState(bool bKeepSaved) {
  if (m_pCache->IsEmpty())
    return;
  m_pCache->ClipRestore();
  if (bKeepSaved)
    m_pCache->ClipSave();
}

bool CFX_SkiaDeviceDriver::SetClip_PathFill(
    const CFX_Path& path,              // path info
    const CFX_Matrix* pObject2Device,  // flips object's y-axis
    const CFX_FillRenderOptions& fill_options) {
  m_FillOptions = fill_options;
  CFX_Matrix identity;
  const CFX_Matrix* deviceMatrix = pObject2Device ? pObject2Device : &identity;
  m_pCache->SetClipFill(path, deviceMatrix, fill_options);
  if (path.GetPoints().size() == 5 || path.GetPoints().size() == 4) {
    absl::optional<CFX_FloatRect> maybe_rectf = path.GetRect(deviceMatrix);
    if (maybe_rectf.has_value()) {
      CFX_FloatRect& rectf = maybe_rectf.value();
      rectf.Intersect(CFX_FloatRect(0, 0,
                                    (float)GetDeviceCaps(FXDC_PIXEL_WIDTH),
                                    (float)GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
      DebugShowCanvasClip(this, m_pCanvas);
      return true;
    }
  }
  SkPath skClipPath = BuildPath(path);
  skClipPath.setFillType(GetAlternateOrWindingFillType(fill_options));
  SkMatrix skMatrix = ToSkMatrix(*deviceMatrix);
  skClipPath.transform(skMatrix);
  DebugShowSkiaPath(skClipPath);
  DebugShowCanvasClip(this, m_pCanvas);
  return true;
}

bool CFX_SkiaDeviceDriver::SetClip_PathStroke(
    const CFX_Path& path,                  // path info
    const CFX_Matrix* pObject2Device,      // required transformation
    const CFX_GraphStateData* pGraphState  // graphic state, for pen attributes
) {
  m_pCache->SetClipStroke(path, pObject2Device, pGraphState);

  // build path data
  SkPath skPath = BuildPath(path);
  SkMatrix skMatrix = ToSkMatrix(*pObject2Device);
  SkPaint skPaint;
  PaintStroke(&skPaint, pGraphState, skMatrix);
  SkPath dst_path;
  FillPathWithPaint(skPath, skPaint, &dst_path);
  dst_path.transform(skMatrix);
  DebugShowCanvasClip(this, m_pCanvas);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawPath(
    const CFX_Path& path,                   // path info
    const CFX_Matrix* pObject2Device,       // optional transformation
    const CFX_GraphStateData* pGraphState,  // graphic state, for pen attributes
    uint32_t fill_color,                    // fill color
    uint32_t stroke_color,                  // stroke color
    const CFX_FillRenderOptions& fill_options,
    BlendMode blend_type) {
  m_FillOptions = fill_options;
  m_pCache->DrawPath(path, pObject2Device, pGraphState, fill_color,
                     stroke_color, fill_options, blend_type);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawCosmeticLine(const CFX_PointF& ptMoveTo,
                                            const CFX_PointF& ptLineTo,
                                            uint32_t color,
                                            BlendMode blend_type) {
  return false;
}

bool CFX_SkiaDeviceDriver::FillRectWithBlend(const FX_RECT& rect,
                                             uint32_t fill_color,
                                             BlendMode blend_type) {
  m_pCache->FlushForDraw();
  SkPaint spaint;
  spaint.setAntiAlias(true);
  spaint.setColor(fill_color);
  spaint.setBlendMode(GetSkiaBlendMode(blend_type));
  SkRect srect = SkRect::MakeLTRB(rect.left, std::min(rect.top, rect.bottom),
                                  rect.right, std::max(rect.bottom, rect.top));
  DebugShowSkiaDrawRect(this, m_pCanvas, spaint, srect);
  m_pCanvas->drawRect(srect, spaint);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawShading(const CPDF_ShadingPattern* pPattern,
                                       const CFX_Matrix* pMatrix,
                                       const FX_RECT& clip_rect,
                                       int alpha,
                                       bool bAlphaMode) {
  m_pCache->FlushForDraw();
  ShadingType shadingType = pPattern->GetShadingType();
  if (kAxialShading != shadingType && kRadialShading != shadingType &&
      kCoonsPatchMeshShading != shadingType) {
    // TODO(caryclark) more types
    return false;
  }
  CPDF_ColorSpace::Family csFamily = pPattern->GetCS()->GetFamily();
  if (CPDF_ColorSpace::Family::kDeviceRGB != csFamily &&
      CPDF_ColorSpace::Family::kDeviceGray != csFamily) {
    return false;
  }
  const std::vector<std::unique_ptr<CPDF_Function>>& pFuncs =
      pPattern->GetFuncs();
  size_t nFuncs = pFuncs.size();
  if (nFuncs > 1)  // TODO(caryclark) remove this restriction
    return false;
  RetainPtr<const CPDF_Dictionary> pDict =
      pPattern->GetShadingObject()->GetDict();
  RetainPtr<const CPDF_Array> pCoords = pDict->GetArrayFor("Coords");
  if (!pCoords && kCoonsPatchMeshShading != shadingType)
    return false;
  // TODO(caryclark) Respect Domain[0], Domain[1]. (Don't know what they do
  // yet.)
  SkTDArray<SkColor> skColors;
  SkTDArray<SkScalar> skPos;
  for (size_t j = 0; j < nFuncs; j++) {
    if (!pFuncs[j])
      continue;

    if (const CPDF_SampledFunc* pSampledFunc = pFuncs[j]->ToSampledFunc()) {
      /* TODO(caryclark)
         Type 0 Sampled Functions in PostScript can also have an Order integer
         in the dictionary. PDFium doesn't appear to check for this anywhere.
       */
      if (!AddSamples(pSampledFunc, &skColors, &skPos))
        return false;
    } else if (const CPDF_ExpIntFunc* pExpIntFuc = pFuncs[j]->ToExpIntFunc()) {
      if (!AddColors(pExpIntFuc, &skColors, /*is_encode_reversed=*/false))
        return false;
      skPos.push_back(0);
      skPos.push_back(1);
    } else if (const CPDF_StitchFunc* pStitchFunc = pFuncs[j]->ToStitchFunc()) {
      if (!AddStitching(pStitchFunc, &skColors, &skPos))
        return false;
    } else {
      return false;
    }
  }
  RetainPtr<const CPDF_Array> pArray = pDict->GetArrayFor("Extend");
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
  if (kAxialShading == shadingType) {
    float start_x = pCoords->GetFloatAt(0);
    float start_y = pCoords->GetFloatAt(1);
    float end_x = pCoords->GetFloatAt(2);
    float end_y = pCoords->GetFloatAt(3);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};
    skMatrix.mapPoints(pts, SK_ARRAY_COUNT(pts));
    paint.setShader(SkGradientShader::MakeLinear(pts, skColors.begin(),
                                                 skPos.begin(), skColors.size(),
                                                 SkTileMode::kClamp));
    if (clipStart || clipEnd) {
      // if the gradient is horizontal or vertical, modify the draw rectangle
      if (pts[0].fX == pts[1].fX) {  // vertical
        if (pts[0].fY > pts[1].fY) {
          std::swap(pts[0].fY, pts[1].fY);
          std::swap(clipStart, clipEnd);
        }
        if (clipStart)
          skRect.fTop = std::max(skRect.fTop, pts[0].fY);
        if (clipEnd)
          skRect.fBottom = std::min(skRect.fBottom, pts[1].fY);
      } else if (pts[0].fY == pts[1].fY) {  // horizontal
        if (pts[0].fX > pts[1].fX) {
          std::swap(pts[0].fX, pts[1].fX);
          std::swap(clipStart, clipEnd);
        }
        if (clipStart)
          skRect.fLeft = std::max(skRect.fLeft, pts[0].fX);
        if (clipEnd)
          skRect.fRight = std::min(skRect.fRight, pts[1].fX);
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
  } else if (kRadialShading == shadingType) {
    float start_x = pCoords->GetFloatAt(0);
    float start_y = pCoords->GetFloatAt(1);
    float start_r = pCoords->GetFloatAt(2);
    float end_x = pCoords->GetFloatAt(3);
    float end_y = pCoords->GetFloatAt(4);
    float end_r = pCoords->GetFloatAt(5);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};

    paint.setShader(SkGradientShader::MakeTwoPointConical(
        pts[0], start_r, pts[1], end_r, skColors.begin(), skPos.begin(),
        skColors.size(), SkTileMode::kClamp));
    if (clipStart || clipEnd) {
      if (clipStart && start_r)
        skClip.addCircle(pts[0].fX, pts[0].fY, start_r);
      if (clipEnd)
        skClip.addCircle(pts[1].fX, pts[1].fY, end_r, SkPathDirection::kCCW);
      else
        skClip.setFillType(SkPathFillType::kInverseWinding);
      skClip.transform(skMatrix);
    }
    SkMatrix inverse;
    if (!skMatrix.invert(&inverse))
      return false;
    skPath.addRect(skRect);
    skPath.transform(inverse);
  } else {
    DCHECK_EQ(kCoonsPatchMeshShading, shadingType);
    RetainPtr<const CPDF_Stream> pStream =
        ToStream(pPattern->GetShadingObject());
    if (!pStream)
      return false;
    CPDF_MeshStream stream(shadingType, pPattern->GetFuncs(),
                           std::move(pStream), pPattern->GetCS());
    if (!stream.Load())
      return false;
    SkPoint cubics[12];
    SkColor colors[4];
    SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
    if (!skClip.isEmpty())
      m_pCanvas->clipPath(skClip, SkClipOp::kIntersect, true);
    m_pCanvas->concat(skMatrix);
    while (!stream.IsEOF()) {
      uint32_t flag = stream.ReadFlag();
      int iStartPoint = flag ? 4 : 0;
      int iStartColor = flag ? 2 : 0;
      if (flag) {
        SkPoint tempCubics[4];
        for (int i = 0; i < (int)SK_ARRAY_COUNT(tempCubics); i++)
          tempCubics[i] = cubics[(flag * 3 + i) % 12];
        memcpy(cubics, tempCubics, sizeof(tempCubics));
        SkColor tempColors[2];
        tempColors[0] = colors[flag];
        tempColors[1] = colors[(flag + 1) % 4];
        memcpy(colors, tempColors, sizeof(tempColors));
      }
      for (int i = iStartPoint; i < (int)SK_ARRAY_COUNT(cubics); i++) {
        CFX_PointF point = stream.ReadCoords();
        cubics[i].fX = point.x;
        cubics[i].fY = point.y;
      }
      for (int i = iStartColor; i < (int)SK_ARRAY_COUNT(colors); i++) {
        float r;
        float g;
        float b;
        std::tie(r, g, b) = stream.ReadColor();
        colors[i] = SkColorSetARGB(0xFF, (U8CPU)(r * 255), (U8CPU)(g * 255),
                                   (U8CPU)(b * 255));
      }
      m_pCanvas->drawPatch(cubics, colors, /*textCoords=*/nullptr,
                           SkBlendMode::kDst, paint);
    }
    return true;
  }
  SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
  if (!skClip.isEmpty())
    m_pCanvas->clipPath(skClip, SkClipOp::kIntersect, true);
  m_pCanvas->concat(skMatrix);
  m_pCanvas->drawPath(skPath, paint);
  return true;
}

uint8_t* CFX_SkiaDeviceDriver::GetBuffer() const {
  return m_pBitmap->GetBuffer().data();
}

bool CFX_SkiaDeviceDriver::GetClipBox(FX_RECT* pRect) {
  // TODO(caryclark) call m_canvas->getClipDeviceBounds() instead
  pRect->left = 0;
  pRect->top = 0;
  const SkImageInfo& canvasSize = m_pCanvas->imageInfo();
  pRect->right = canvasSize.width();
  pRect->bottom = canvasSize.height();
  return true;
}

bool CFX_SkiaDeviceDriver::GetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                                     int left,
                                     int top) {
  if (!m_pBitmap)
    return true;
  uint8_t* srcBuffer = m_pBitmap->GetBuffer().data();
  if (!srcBuffer)
    return true;
  m_pCache->FlushForDraw();
  int srcWidth = m_pBitmap->GetWidth();
  int srcHeight = m_pBitmap->GetHeight();
  size_t srcRowBytes = m_pBitmap->GetPitch();
  SkImageInfo srcImageInfo = SkImageInfo::Make(
      srcWidth, srcHeight, SkColorType::kN32_SkColorType, kPremul_SkAlphaType);
  SkBitmap skSrcBitmap;
  skSrcBitmap.installPixels(srcImageInfo, srcBuffer, srcRowBytes);
  uint8_t* dstBuffer = pBitmap->GetBuffer().data();
  DCHECK(dstBuffer);
  int dstWidth = pBitmap->GetWidth();
  int dstHeight = pBitmap->GetHeight();
  size_t dstRowBytes = pBitmap->GetPitch();
  SkImageInfo dstImageInfo = SkImageInfo::Make(
      dstWidth, dstHeight, Get32BitSkColorType(m_bRgbByteOrder),
      kPremul_SkAlphaType);
  SkBitmap skDstBitmap;
  skDstBitmap.installPixels(dstImageInfo, dstBuffer, dstRowBytes);
  SkCanvas canvas(skDstBitmap);
  canvas.drawImageRect(skSrcBitmap.asImage(),
                       SkRect::MakeXYWH(left, top, dstWidth, dstHeight),
                       SkSamplingOptions(), /*paint=*/nullptr);
  return true;
}

RetainPtr<CFX_DIBitmap> CFX_SkiaDeviceDriver::GetBackDrop() {
  return m_pBackdropBitmap;
}

bool CFX_SkiaDeviceDriver::SetDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                                     uint32_t argb,
                                     const FX_RECT& src_rect,
                                     int left,
                                     int top,
                                     BlendMode blend_type) {
  if (!m_pBitmap || m_pBitmap->GetBuffer().empty())
    return true;

  CFX_Matrix m = CFX_RenderDevice::GetFlipMatrix(
      pBitmap->GetWidth(), pBitmap->GetHeight(), left, top);

  // `bNoSmoothing` prevents linear sampling when rendering bitmaps.
  FXDIB_ResampleOptions sampling_options;
  sampling_options.bNoSmoothing = true;

  return StartDIBitsSkia(pBitmap, 0xFF, argb, m, sampling_options, blend_type);
}

bool CFX_SkiaDeviceDriver::StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                         uint32_t argb,
                                         int dest_left,
                                         int dest_top,
                                         int dest_width,
                                         int dest_height,
                                         const FX_RECT* pClipRect,
                                         const FXDIB_ResampleOptions& options,
                                         BlendMode blend_type) {
  m_pCache->FlushForDraw();
  if (m_pBitmap->GetBuffer().empty())
    return true;

  CFX_Matrix m = CFX_RenderDevice::GetFlipMatrix(dest_width, dest_height,
                                                 dest_left, dest_top);
  SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
  SkRect skClipRect = SkRect::MakeLTRB(pClipRect->left, pClipRect->bottom,
                                       pClipRect->right, pClipRect->top);
  m_pCanvas->clipRect(skClipRect, SkClipOp::kIntersect, true);

  // `bNoSmoothing` prevents linear sampling when rendering bitmaps.
  FXDIB_ResampleOptions sampling_options;
  sampling_options.bNoSmoothing = true;

  return StartDIBitsSkia(pSource, 0xFF, argb, m, sampling_options, blend_type);
}

bool CFX_SkiaDeviceDriver::StartDIBits(
    const RetainPtr<CFX_DIBBase>& pSource,
    int bitmap_alpha,
    uint32_t argb,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    std::unique_ptr<CFX_ImageRenderer>* handle,
    BlendMode blend_type) {
  return StartDIBitsSkia(pSource, bitmap_alpha, argb, matrix, options,
                         blend_type);
}

bool CFX_SkiaDeviceDriver::ContinueDIBits(CFX_ImageRenderer* handle,
                                          PauseIndicatorIface* pPause) {
  m_pCache->FlushForDraw();
  return false;
}

void CFX_SkiaDeviceDriver::PreMultiply(
    const RetainPtr<CFX_DIBitmap>& pDIBitmap) {
  pDIBitmap->PreMultiply();
}

void CFX_DIBitmap::PreMultiply() {
  if (this->GetBPP() != 32)
    return;

  void* buffer = this->GetBuffer().data();
  if (!buffer)
    return;

  Format priorFormat = m_nFormat;
  m_nFormat = Format::kPreMultiplied;
  if (priorFormat == Format::kPreMultiplied)
    return;

  int height = this->GetHeight();
  int width = this->GetWidth();
  int rowBytes = this->GetPitch();
  SkImageInfo unpremultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kUnpremul_SkAlphaType);
  SkPixmap unpremultiplied(unpremultipliedInfo, buffer, rowBytes);
  SkImageInfo premultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
  SkPixmap premultiplied(premultipliedInfo, buffer, rowBytes);
  unpremultiplied.readPixels(premultiplied);
  this->DebugVerifyBitmapIsPreMultiplied();
}

void CFX_DIBitmap::UnPreMultiply() {
  if (this->GetBPP() != 32)
    return;

  void* buffer = this->GetBuffer().data();
  if (!buffer)
    return;

  Format priorFormat = m_nFormat;
  m_nFormat = Format::kUnPreMultiplied;

  if (priorFormat == Format::kUnPreMultiplied)
    return;

  this->DebugVerifyBitmapIsPreMultiplied();
  int height = this->GetHeight();
  int width = this->GetWidth();
  int rowBytes = this->GetPitch();
  SkImageInfo premultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
  SkPixmap premultiplied(premultipliedInfo, buffer, rowBytes);
  SkImageInfo unpremultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kUnpremul_SkAlphaType);
  SkPixmap unpremultiplied(unpremultipliedInfo, buffer, rowBytes);
  premultiplied.readPixels(unpremultiplied);
}

bool CFX_SkiaDeviceDriver::DrawBitsWithMask(
    const RetainPtr<CFX_DIBBase>& pSource,
    const RetainPtr<CFX_DIBBase>& pMask,
    int bitmap_alpha,
    const CFX_Matrix& matrix,
    BlendMode blend_type) {
  DebugValidate(m_pBitmap, m_pBackdropBitmap);
  // Storage vectors must outlive `skBitmap` and `skMask`.
  DataVector<uint8_t> src8_storage;
  DataVector<uint8_t> mask8_storage;
  DataVector<uint32_t> src32_storage;
  DataVector<uint32_t> mask32_storage;
  SkBitmap skBitmap;
  SkBitmap skMask;
  int srcWidth;
  int srcHeight;
  int maskWidth;
  int maskHeight;
  if (!Upsample(pSource, src8_storage, src32_storage, &skBitmap, &srcWidth,
                &srcHeight, /*forceAlpha=*/false, m_bRgbByteOrder)) {
    return false;
  }
  if (!Upsample(pMask, mask8_storage, mask32_storage, &skMask, &maskWidth,
                &maskHeight, /*forceAlpha=*/true, m_bRgbByteOrder)) {
    return false;
  }
  {
    m_pCache->FlushForDraw();

    SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
    SkMatrix skMatrix;
    SetBitmapMatrix(matrix, srcWidth, srcHeight, &skMatrix);
    m_pCanvas->concat(skMatrix);
    SkPaint paint;
    SetBitmapPaintForMerge(pSource->IsMaskFormat(), !m_FillOptions.aliased_path,
                           0xFFFFFFFF, bitmap_alpha, blend_type, &paint);
    sk_sp<SkImage> skSrc = SkImage::MakeFromBitmap(skBitmap);
    sk_sp<SkShader> skSrcShader = skSrc->makeShader(
        SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
    sk_sp<SkImage> skMaskImage = SkImage::MakeFromBitmap(skMask);
    sk_sp<SkShader> skMaskShader = skMaskImage->makeShader(
        SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
    paint.setShader(
        SkShaders::Blend(SkBlendMode::kSrcIn, skMaskShader, skSrcShader));
    SkRect r = {0, 0, SkIntToScalar(srcWidth), SkIntToScalar(srcHeight)};
    m_pCanvas->drawRect(r, paint);
  }
  DebugValidate(m_pBitmap, m_pBackdropBitmap);
  return true;
}

bool CFX_SkiaDeviceDriver::SetBitsWithMask(
    const RetainPtr<CFX_DIBBase>& pBitmap,
    const RetainPtr<CFX_DIBBase>& pMask,
    int dest_left,
    int dest_top,
    int bitmap_alpha,
    BlendMode blend_type) {
  if (!m_pBitmap || m_pBitmap->GetBuffer().empty())
    return true;

  CFX_Matrix m = CFX_RenderDevice::GetFlipMatrix(
      pBitmap->GetWidth(), pBitmap->GetHeight(), dest_left, dest_top);
  return DrawBitsWithMask(pBitmap, pMask, bitmap_alpha, m, blend_type);
}

void CFX_SkiaDeviceDriver::SetGroupKnockout(bool group_knockout) {
  if (group_knockout == m_bGroupKnockout)
    return;

  // Make sure to flush cached commands before changing `m_bGroupKnockout`
  // status.
  Flush();
  m_bGroupKnockout = group_knockout;
}

void CFX_SkiaDeviceDriver::Clear(uint32_t color) {
  m_pCanvas->clear(color);
}

void CFX_SkiaDeviceDriver::DebugVerifyBitmapIsPreMultiplied() const {
  if (m_pBackdropBitmap)
    m_pBackdropBitmap->DebugVerifyBitmapIsPreMultiplied();
}

bool CFX_SkiaDeviceDriver::StartDIBitsSkia(
    const RetainPtr<CFX_DIBBase>& pSource,
    int bitmap_alpha,
    uint32_t argb,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    BlendMode blend_type) {
  m_pCache->FlushForDraw();
  DebugValidate(m_pBitmap, m_pBackdropBitmap);
  // Storage vectors must outlive `skBitmap`.
  DataVector<uint8_t> dst8_storage;
  DataVector<uint32_t> dst32_storage;
  SkBitmap skBitmap;
  int width;
  int height;
  if (!Upsample(pSource, dst8_storage, dst32_storage, &skBitmap, &width,
                &height, /*forceAlpha=*/false, m_bRgbByteOrder)) {
    return false;
  }
  {
    SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
    SkMatrix skMatrix;
    SetBitmapMatrix(matrix, width, height, &skMatrix);
    m_pCanvas->concat(skMatrix);
    SkPaint paint;
    SetBitmapPaint(pSource->IsMaskFormat(), !m_FillOptions.aliased_path,
                   bitmap_alpha, argb, blend_type, &paint);
    // TODO(caryclark) Once Skia supports 8 bit src to 8 bit dst remove this
    if (m_pBitmap && m_pBitmap->GetBPP() == 8 && pSource->GetBPP() == 8) {
      SkMatrix inv;
      SkAssertResult(skMatrix.invert(&inv));
      for (int y = 0; y < m_pBitmap->GetHeight(); ++y) {
        for (int x = 0; x < m_pBitmap->GetWidth(); ++x) {
          SkPoint src = {x + 0.5f, y + 0.5f};
          inv.mapPoints(&src, 1);
          // SkMatrix::mapPoints() can sometimes output NaN values or values
          // outside the boundary of the `skBitmap`. Therefore clamping these
          // values is necessary before getting color information within the
          // `skBitmap`.
          src.fX =
              isnan(src.fX) ? 0.5f : pdfium::clamp(src.fX, 0.5f, width - 0.5f);
          src.fY =
              isnan(src.fY) ? 0.5f : pdfium::clamp(src.fY, 0.5f, height - 0.5f);

          m_pBitmap->SetPixel(x, y, skBitmap.getColor(src.fX, src.fY));
        }
      }
    } else {
      bool use_interpolate_bilinear = options.bInterpolateBilinear;
      if (!use_interpolate_bilinear) {
        float dest_width = ceilf(matrix.GetXUnit());
        float dest_height = ceilf(matrix.GetYUnit());
        if (pdfium::base::IsValueInRangeForNumericType<int>(dest_width) &&
            pdfium::base::IsValueInRangeForNumericType<int>(dest_height)) {
          use_interpolate_bilinear = CStretchEngine::UseInterpolateBilinear(
              options, static_cast<int>(dest_width),
              static_cast<int>(dest_height), width, height);
        }
      }
      SkSamplingOptions sampling_options;
      if (use_interpolate_bilinear) {
        sampling_options =
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
      }
      m_pCanvas->drawImageRect(skBitmap.asImage(),
                               SkRect::MakeWH(width, height), sampling_options,
                               &paint);
    }
  }
  DebugValidate(m_pBitmap, m_pBackdropBitmap);
  return true;
}

void CFX_DefaultRenderDevice::Clear(uint32_t color) {
  CFX_SkiaDeviceDriver* skDriver =
      static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver());
  skDriver->Clear(color);
}

std::unique_ptr<SkPictureRecorder> CFX_DefaultRenderDevice::CreateRecorder(
    const SkRect& bounds) {
  auto recorder = std::make_unique<SkPictureRecorder>();
  recorder->beginRecording(bounds);

  SetDeviceDriver(std::make_unique<CFX_SkiaDeviceDriver>(recorder.get()));
  return recorder;
}

bool CFX_DefaultRenderDevice::AttachSkiaImpl(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout) {
  if (!pBitmap)
    return false;
  SetBitmap(pBitmap);
  SetDeviceDriver(std::make_unique<CFX_SkiaDeviceDriver>(
      std::move(pBitmap), bRgbByteOrder, std::move(pBackdropBitmap),
      bGroupKnockout));
  return true;
}

bool CFX_DefaultRenderDevice::AttachRecorder(SkPictureRecorder* recorder) {
  if (!recorder)
    return false;
  SetDeviceDriver(std::make_unique<CFX_SkiaDeviceDriver>(recorder));
  return true;
}

bool CFX_DefaultRenderDevice::CreateSkia(
    int width,
    int height,
    FXDIB_Format format,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(width, height, format))
    return false;

  SetBitmap(pBitmap);
  SetDeviceDriver(std::make_unique<CFX_SkiaDeviceDriver>(
      std::move(pBitmap), false, std::move(pBackdropBitmap), false));
  return true;
}

void CFX_DefaultRenderDevice::DebugVerifyBitmapIsPreMultiplied() const {
#ifdef SK_DEBUG
  CFX_SkiaDeviceDriver* skDriver =
      static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver());
  if (skDriver)
    skDriver->DebugVerifyBitmapIsPreMultiplied();
#endif  // SK_DEBUG
}

bool CFX_DefaultRenderDevice::SetBitsWithMask(
    const RetainPtr<CFX_DIBBase>& pBitmap,
    const RetainPtr<CFX_DIBBase>& pMask,
    int left,
    int top,
    int bitmap_alpha,
    BlendMode blend_type) {
  CFX_SkiaDeviceDriver* skDriver =
      static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver());
  if (skDriver) {
    // Finish painting before drawing masks.
    Flush(false);
    return skDriver->SetBitsWithMask(pBitmap, pMask, left, top, bitmap_alpha,
                                     blend_type);
  }
  return false;
}

void CFX_DIBBase::DebugVerifyBufferIsPreMultiplied(void* arg) const {
#ifdef SK_DEBUG
  DCHECK_EQ(GetBPP(), 32);
  const uint32_t* buffer = (const uint32_t*)arg;
  int width = GetWidth();
  int height = GetHeight();
  // verify that input is really premultiplied
  for (int y = 0; y < height; ++y) {
    const uint32_t* srcRow = buffer + y * width;
    for (int x = 0; x < width; ++x) {
      uint8_t a = SkGetPackedA32(srcRow[x]);
      uint8_t r = SkGetPackedR32(srcRow[x]);
      uint8_t g = SkGetPackedG32(srcRow[x]);
      uint8_t b = SkGetPackedB32(srcRow[x]);
      SkA32Assert(a);
      DCHECK(r <= a);
      DCHECK(g <= a);
      DCHECK(b <= a);
    }
  }
#endif  // SK_DEBUG
}

void CFX_DIBitmap::DebugVerifyBitmapIsPreMultiplied() const {
#ifdef SK_DEBUG
  DebugVerifyBufferIsPreMultiplied(GetBuffer().data());
#endif  // SK_DEBUG
}
