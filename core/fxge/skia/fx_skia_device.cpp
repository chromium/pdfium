// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/skia/fx_skia_device.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
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
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/span.h"
#include "third_party/base/memory/ptr_util.h"
#include "third_party/base/notreached.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/skia/include/core/SkBlendMode.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkClipOp.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPathEffect.h"
#include "third_party/skia/include/core/SkPathUtils.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSamplingOptions.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkTextBlob.h"
#include "third_party/skia/include/core/SkTileMode.h"
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

void DebugValidate(const RetainPtr<CFX_DIBitmap>& bitmap) {
#if DCHECK_IS_ON()
  DCHECK(bitmap);
  DCHECK(bitmap->GetBPP() == 8 || bitmap->GetBPP() == 32);
#endif
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
      return SkBlendMode::kSrcOver;
  }
}

// Add begin & end colors into `colors` array for each gradient transition.
//
// `is_encode_reversed` must be set to true when the parent function of `func`
// has an Encode array, and the matching pair of encode values for `func` are
// in decreasing order.
bool AddColors(const CPDF_ExpIntFunc* func,
               DataVector<SkColor>& colors,
               bool is_encode_reversed) {
  if (func->CountInputs() != 1) {
    return false;
  }
  if (func->GetExponent() != 1) {
    return false;
  }
  if (func->GetOrigOutputs() != 3) {
    return false;
  }

  pdfium::span<const float> begin_values = func->GetBeginValues();
  pdfium::span<const float> end_values = func->GetEndValues();
  if (is_encode_reversed)
    std::swap(begin_values, end_values);

  colors.push_back(SkColorSetARGB(0xFF,
                                  SkUnitScalarClampToByte(begin_values[0]),
                                  SkUnitScalarClampToByte(begin_values[1]),
                                  SkUnitScalarClampToByte(begin_values[2])));
  colors.push_back(SkColorSetARGB(0xFF, SkUnitScalarClampToByte(end_values[0]),
                                  SkUnitScalarClampToByte(end_values[1]),
                                  SkUnitScalarClampToByte(end_values[2])));
  return true;
}

uint8_t FloatToByte(float f) {
  DCHECK(f >= 0);
  DCHECK(f <= 1);
  return (uint8_t)(f * 255.99f);
}

bool AddSamples(const CPDF_SampledFunc* func,
                DataVector<SkColor>& colors,
                DataVector<SkScalar>& pos) {
  if (func->CountInputs() != 1) {
    return false;
  }
  if (func->CountOutputs() != 3) {  // expect rgb
    return false;
  }
  if (func->GetEncodeInfo().empty()) {
    return false;
  }
  const CPDF_SampledFunc::SampleEncodeInfo& encode_info =
      func->GetEncodeInfo()[0];
  if (encode_info.encode_min != 0) {
    return false;
  }
  if (encode_info.encode_max != encode_info.sizes - 1) {
    return false;
  }
  uint32_t sample_size = func->GetBitsPerSample();
  uint32_t sample_count = encode_info.sizes;
  if (sample_count != 1U << sample_size) {
    return false;
  }
  if (func->GetSampleStream()->GetSize() < sample_count * 3 * sample_size / 8) {
    return false;
  }

  float colors_min[3];
  float colors_max[3];
  for (int i = 0; i < 3; ++i) {
    colors_min[i] = func->GetRange(i * 2);
    colors_max[i] = func->GetRange(i * 2 + 1);
  }
  pdfium::span<const uint8_t> sample_data = func->GetSampleStream()->GetSpan();
  CFX_BitStream bitstream(sample_data);
  for (uint32_t i = 0; i < sample_count; ++i) {
    float float_colors[3];
    for (uint32_t j = 0; j < 3; ++j) {
      float sample = static_cast<float>(bitstream.GetBits(sample_size));
      float interp = sample / (sample_count - 1);
      float_colors[j] =
          colors_min[j] + (colors_max[j] - colors_min[j]) * interp;
    }
    colors.push_back(SkPackARGB32NoCheck(0xFF, FloatToByte(float_colors[0]),
                                         FloatToByte(float_colors[1]),
                                         FloatToByte(float_colors[2])));
    pos.push_back(static_cast<float>(i) / (sample_count - 1));
  }
  return true;
}

bool AddStitching(const CPDF_StitchFunc* func,
                  DataVector<SkColor>& colors,
                  DataVector<SkScalar>& pos) {
  float bounds_start = func->GetDomain(0);

  const auto& sub_functions = func->GetSubFunctions();
  const size_t sub_function_count = sub_functions.size();
  for (size_t i = 0; i < sub_function_count; ++i) {
    const CPDF_ExpIntFunc* sub_func = sub_functions[i]->ToExpIntFunc();
    if (!sub_func)
      return false;
    // Check if the matching encode values are reversed
    bool is_encode_reversed =
        func->GetEncode(2 * i) > func->GetEncode(2 * i + 1);
    if (!AddColors(sub_func, colors, is_encode_reversed)) {
      return false;
    }
    float bounds_end =
        i < sub_function_count - 1 ? func->GetBound(i + 1) : func->GetDomain(1);
    pos.push_back(bounds_start);
    pos.push_back(bounds_end);
    bounds_start = bounds_end;
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

// Converts a stroking path to scanlines
void PaintStroke(SkPaint* spaint,
                 const CFX_GraphStateData* graph_state,
                 const SkMatrix& matrix,
                 const CFX_FillRenderOptions& fill_options) {
  SkPaint::Cap cap;
  switch (graph_state->m_LineCap) {
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
  switch (graph_state->m_LineJoin) {
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
  if (!matrix.invert(&inverse)) {
    return;  // give up if the matrix is degenerate, and not invertable
  }
  inverse.set(SkMatrix::kMTransX, 0);
  inverse.set(SkMatrix::kMTransY, 0);
  SkVector deviceUnits[2] = {{0, 1}, {1, 0}};
  inverse.mapPoints(deviceUnits, std::size(deviceUnits));

  float width = fill_options.zero_area
                    ? 0.0f
                    : std::max(graph_state->m_LineWidth,
                               std::min(deviceUnits[0].length(),
                                        deviceUnits[1].length()));
  if (!graph_state->m_DashArray.empty()) {
    size_t count = (graph_state->m_DashArray.size() + 1) / 2;
    DataVector<SkScalar> intervals(count * 2);
    // Set dash pattern
    for (size_t i = 0; i < count; i++) {
      float on = graph_state->m_DashArray[i * 2];
      if (on <= 0.000001f) {
        on = 0.1f;
      }
      float off = i * 2 + 1 == graph_state->m_DashArray.size()
                      ? on
                      : graph_state->m_DashArray[i * 2 + 1];
      off = std::max(off, 0.0f);
      intervals[i * 2] = on;
      intervals[i * 2 + 1] = off;
    }
    spaint->setPathEffect(SkDashPathEffect::Make(
        intervals.data(), pdfium::base::checked_cast<int>(intervals.size()),
        graph_state->m_DashPhase));
  }
  spaint->setStyle(SkPaint::kStroke_Style);
  spaint->setAntiAlias(!fill_options.aliased_path);
  spaint->setStrokeWidth(width);
  spaint->setStrokeMiter(graph_state->m_MiterLimit);
  spaint->setStrokeCap(cap);
  spaint->setStrokeJoin(join);
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

// Makes a bitmap filled with a solid color for debugging with `SkPicture`.
RetainPtr<CFX_DIBitmap> MakeDebugBitmap(int width, int height, uint32_t color) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!bitmap->Create(width, height, FXDIB_Format::kArgb))
    return nullptr;

  bitmap->Clear(color);
  return bitmap;
}

bool HasRSX(pdfium::span<const TextCharPos> char_pos,
            float* scaleXPtr,
            bool* oneAtATimePtr) {
  bool useRSXform = false;
  bool oneAtATime = false;
  float scaleX = 1;
  for (const TextCharPos& cp : char_pos) {
    if (!cp.m_bGlyphAdjust) {
      continue;
    }
    bool upright = 0 == cp.m_AdjustMatrix[1] && 0 == cp.m_AdjustMatrix[2];
    if (cp.m_AdjustMatrix[0] != cp.m_AdjustMatrix[3]) {
      if (upright && 1 == cp.m_AdjustMatrix[3]) {
        if (1 == scaleX) {
          scaleX = cp.m_AdjustMatrix[0];
        } else if (scaleX != cp.m_AdjustMatrix[0]) {
          oneAtATime = true;
        }
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

}  // namespace

// static
std::unique_ptr<CFX_SkiaDeviceDriver> CFX_SkiaDeviceDriver::Create(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout) {
  auto driver = pdfium::WrapUnique(
      new CFX_SkiaDeviceDriver(std::move(pBitmap), bRgbByteOrder,
                               std::move(pBackdropBitmap), bGroupKnockout));
  if (!driver->m_pCanvas) {
    return nullptr;
  }

  return driver;
}

// static
std::unique_ptr<CFX_SkiaDeviceDriver> CFX_SkiaDeviceDriver::Create(
    SkCanvas* canvas) {
  if (!canvas) {
    return nullptr;
  }

  auto driver = pdfium::WrapUnique(new CFX_SkiaDeviceDriver(canvas));
  if (!driver->m_pBitmap || !driver->m_pBackdropBitmap) {
    return nullptr;
  }

  return driver;
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout)
    : m_pBitmap(std::move(pBitmap)),
      m_pBackdropBitmap(pBackdropBitmap),
      m_bRgbByteOrder(bRgbByteOrder),
      m_bGroupKnockout(bGroupKnockout) {
  SkColorType color_type;
  const int bpp = m_pBitmap->GetBPP();
  if (bpp == 8) {
    color_type = m_pBitmap->IsAlphaFormat() || m_pBitmap->IsMaskFormat()
                     ? kAlpha_8_SkColorType
                     : kGray_8_SkColorType;
  } else if (bpp == 24) {
    DCHECK_EQ(m_pBitmap->GetFormat(), FXDIB_Format::kRgb);

    // Save the input bitmap as `m_pOriginalBitmap` and save its 32 bpp
    // equivalent at `m_pBitmap` for Skia's internal process.
    m_pOriginalBitmap = std::move(m_pBitmap);
    m_pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!m_pBitmap->Copy(m_pOriginalBitmap) ||
        !m_pBitmap->ConvertFormat(FXDIB_Format::kArgb)) {
      // Skip creating SkCanvas if the 32-bpp bitmap creation fails.
      // CFX_SkiaDeviceDriver::Create() will check for the missing `m_pCanvas`
      // and not use `this`.
      // Also reset `m_pOriginalBitmap` so the dtor does not try to transfer
      // `m_pBitmap` back to `m_pOriginalBitmap`.
      m_pOriginalBitmap.Reset();
      return;
    }

    color_type = Get32BitSkColorType(bRgbByteOrder);
  } else {
    DCHECK_EQ(bpp, 32);
    color_type = Get32BitSkColorType(bRgbByteOrder);
  }

  SkImageInfo imageInfo =
      SkImageInfo::Make(m_pBitmap->GetWidth(), m_pBitmap->GetHeight(),
                        color_type, kPremul_SkAlphaType);
  surface_ = SkSurfaces::WrapPixels(
      imageInfo, m_pBitmap->GetWritableBuffer().data(), m_pBitmap->GetPitch());
  m_pCanvas = surface_->getCanvas();
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(SkCanvas* canvas)
    : m_pCanvas(canvas), m_bGroupKnockout(false) {
  int width = m_pCanvas->imageInfo().width();
  int height = m_pCanvas->imageInfo().height();
  DCHECK_EQ(kUnknown_SkColorType, m_pCanvas->imageInfo().colorType());

  constexpr uint32_t kMagenta = 0xffff00ff;
  constexpr uint32_t kGreen = 0xff00ff00;
  m_pBitmap = MakeDebugBitmap(width, height, kMagenta);
  m_pBackdropBitmap = MakeDebugBitmap(width, height, kGreen);
}

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
  // Convert and transfer the internal processed result to the original 24 bpp
  // bitmap provided by the render device.
  if (m_pOriginalBitmap && m_pBitmap->ConvertFormat(FXDIB_Format::kRgb)) {
    CHECK(SyncInternalBitmaps());
  }
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

  if (TryDrawText(pCharPos, pFont, mtObject2Device, font_size, color,
                  options)) {
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
  const SkScalar horizontal_flip = font_size < 0 ? -1 : 1;
  const SkScalar vertical_flip = pFont->IsVertical() ? -1 : 1;
  SkMatrix skMatrix = ToFlippedSkMatrix(mtObject2Device, horizontal_flip);
  m_pCanvas->concat(skMatrix);
  DataVector<SkPoint> positions(pCharPos.size());
  DataVector<uint16_t> glyphs(pCharPos.size());

  for (size_t index = 0; index < pCharPos.size(); ++index) {
    const TextCharPos& cp = pCharPos[index];
    positions[index] = {cp.m_Origin.x * horizontal_flip,
                        cp.m_Origin.y * vertical_flip};
    glyphs[index] = static_cast<uint16_t>(cp.m_GlyphIndex);
#if BUILDFLAG(IS_APPLE)
    if (cp.m_ExtGID)
      glyphs[index] = static_cast<uint16_t>(cp.m_ExtGID);
#endif
  }

  for (size_t index = 0; index < pCharPos.size(); ++index) {
    const TextCharPos& cp = pCharPos[index];
    if (cp.m_bGlyphAdjust) {
      if (0 == cp.m_AdjustMatrix[1] && 0 == cp.m_AdjustMatrix[2] &&
          1 == cp.m_AdjustMatrix[3]) {
        font.setScaleX(cp.m_AdjustMatrix[0]);
        auto blob =
            SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]),
                                     font, SkTextEncoding::kGlyphID);
        m_pCanvas->drawTextBlob(blob, positions[index].fX, positions[index].fY,
                                paint);
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
          SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]), font,
                                   SkTextEncoding::kGlyphID);
      m_pCanvas->drawTextBlob(blob, positions[index].fX, positions[index].fY,
                              paint);
    }
  }
  return true;
}

// TODO(crbug.com/pdfium/1999): Merge with `DrawDeviceText()` and refactor
// common logic.
// TODO(crbug.com/pdfium/1774): Sometimes the thickness of the glyphs is not
// ideal. Improve text rendering results regarding different font weight.
bool CFX_SkiaDeviceDriver::TryDrawText(pdfium::span<const TextCharPos> char_pos,
                                       const CFX_Font* pFont,
                                       const CFX_Matrix& matrix,
                                       float font_size,
                                       uint32_t color,
                                       const CFX_TextRenderOptions& options) {
  float scaleX = 1;
  bool oneAtATime = false;
  bool hasRSX = HasRSX(char_pos, &scaleX, &oneAtATime);
  if (oneAtATime) {
    return false;
  }

  m_charDetails.SetCount(0);
  m_rsxform.resize(0);

  const size_t original_count = m_charDetails.Count();
  FX_SAFE_SIZE_T safe_count = original_count;
  safe_count += char_pos.size();
  const size_t total_count = safe_count.ValueOrDie();
  m_charDetails.SetCount(total_count);
  if (hasRSX) {
    m_rsxform.resize(total_count);
  }

  const SkScalar horizontal_flip = font_size < 0 ? -1 : 1;
  const SkScalar vertical_flip = pFont->IsVertical() ? -1 : 1;
  for (size_t index = 0; index < char_pos.size(); ++index) {
    const TextCharPos& cp = char_pos[index];
    size_t cur_index = index + original_count;
    m_charDetails.SetPositionAt(cur_index, {cp.m_Origin.x * horizontal_flip,
                                            cp.m_Origin.y * vertical_flip});
    m_charDetails.SetGlyphAt(cur_index, static_cast<uint16_t>(cp.m_GlyphIndex));
    m_charDetails.SetFontCharWidthAt(cur_index, cp.m_FontCharWidth);
#if BUILDFLAG(IS_APPLE)
    if (cp.m_ExtGID) {
      m_charDetails.SetGlyphAt(cur_index, static_cast<uint16_t>(cp.m_ExtGID));
    }
#endif
  }
  if (hasRSX) {
    const DataVector<SkPoint>& positions = m_charDetails.GetPositions();
    for (size_t index = 0; index < char_pos.size(); ++index) {
      const TextCharPos& cp = char_pos[index];
      SkRSXform& rsxform = m_rsxform[index + original_count];
      if (cp.m_bGlyphAdjust) {
        rsxform.fSCos = cp.m_AdjustMatrix[0];
        rsxform.fSSin = cp.m_AdjustMatrix[1];
        rsxform.fTx = cp.m_AdjustMatrix[0] * positions[index].fX;
        rsxform.fTy = -cp.m_AdjustMatrix[3] * positions[index].fY;
      } else {
        rsxform.fSCos = 1;
        rsxform.fSSin = 0;
        rsxform.fTx = positions[index].fX;
        rsxform.fTy = positions[index].fY;
      }
    }
  }

  SkPaint skPaint;
  skPaint.setAntiAlias(true);
  skPaint.setColor(color);

  SkFont font;
  if (pFont->GetFaceRec()) {  // exclude placeholder test fonts
    font.setTypeface(sk_ref_sp(pFont->GetDeviceCache()));
  }
  font.setEmbolden(pFont->IsSubstFontBold());
  font.setHinting(SkFontHinting::kNone);
  font.setScaleX(scaleX);
  font.setSkewX(tanf(pFont->GetSubstFontItalicAngle() * FXSYS_PI / 180.0));
  font.setSize(SkTAbs(font_size));
  font.setSubpixel(true);
  font.setEdging(GetFontEdgingType(options));

  SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
  m_pCanvas->concat(ToFlippedSkMatrix(matrix, horizontal_flip));

  const DataVector<uint16_t>& glyphs = m_charDetails.GetGlyphs();
  if (!m_rsxform.empty()) {
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromRSXform(
        glyphs.data(), glyphs.size() * sizeof(uint16_t), m_rsxform.data(), font,
        SkTextEncoding::kGlyphID);
    m_pCanvas->drawTextBlob(blob, 0, 0, skPaint);
    return true;
  }
  const DataVector<SkPoint>& positions = m_charDetails.GetPositions();
  const DataVector<uint32_t>& widths = m_charDetails.GetFontCharWidths();
  for (size_t i = 0; i < m_charDetails.Count(); ++i) {
    const uint32_t font_glyph_width = pFont->GetGlyphWidth(glyphs[i]);
    const uint32_t pdf_glyph_width = widths[i];
    if (pdf_glyph_width > 0 && font_glyph_width > 0) {
      // Scale the glyph from its default width `pdf_glyph_width` to the
      // targeted width `pdf_glyph_width`.
      font.setScaleX(scaleX * SkIntToScalar(pdf_glyph_width) /
                     font_glyph_width);
    } else {
      font.setScaleX(scaleX);
    }
    auto blob =
        SkTextBlob::MakeFromPosText(&glyphs[i], sizeof(uint16_t), &positions[i],
                                    font, SkTextEncoding::kGlyphID);
    m_pCanvas->drawTextBlob(blob, 0, 0, skPaint);
  }
  return true;
}

int CFX_SkiaDeviceDriver::GetDriverType() const {
  return 1;
}

bool CFX_SkiaDeviceDriver::MultiplyAlpha(float alpha) {
  SkPaint paint;
  paint.setAlphaf(alpha);
  paint.setBlendMode(SkBlendMode::kDstIn);
  m_pCanvas->drawPaint(paint);
  return true;
}

bool CFX_SkiaDeviceDriver::MultiplyAlpha(const RetainPtr<CFX_DIBBase>& mask) {
  CHECK(mask->IsMaskFormat());

  sk_sp<SkImage> skia_mask = mask->RealizeSkImage();
  if (!skia_mask) {
    return false;
  }
  DCHECK_EQ(skia_mask->colorType(), kAlpha_8_SkColorType);

  SkPaint paint;
  paint.setBlendMode(SkBlendMode::kDstIn);
  m_pCanvas->drawImageRect(skia_mask,
                           SkRect::Make(m_pCanvas->imageInfo().bounds()),
                           SkSamplingOptions(), &paint);
  return true;
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
      NOTREACHED_NORETURN();
  }
}

void CFX_SkiaDeviceDriver::SaveState() {
  m_pCanvas->save();
}

void CFX_SkiaDeviceDriver::RestoreState(bool bKeepSaved) {
  m_pCanvas->restore();
  if (bKeepSaved) {
    m_pCanvas->save();
  }
}

bool CFX_SkiaDeviceDriver::SetClip_PathFill(
    const CFX_Path& path,              // path info
    const CFX_Matrix* pObject2Device,  // flips object's y-axis
    const CFX_FillRenderOptions& fill_options) {
  m_FillOptions = fill_options;
  const CFX_Matrix& deviceMatrix =
      pObject2Device ? *pObject2Device : CFX_Matrix();

  SkPath skClipPath;
  if (path.GetPoints().size() == 5 || path.GetPoints().size() == 4) {
    absl::optional<CFX_FloatRect> maybe_rectf = path.GetRect(&deviceMatrix);
    if (maybe_rectf.has_value()) {
      CFX_FloatRect& rectf = maybe_rectf.value();
      rectf.Intersect(CFX_FloatRect(0, 0,
                                    (float)GetDeviceCaps(FXDC_PIXEL_WIDTH),
                                    (float)GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
      FX_RECT outer = rectf.GetOuterRect();
      // note that PDF's y-axis goes up; Skia's y-axis goes down
      skClipPath.addRect({(float)outer.left, (float)outer.bottom,
                          (float)outer.right, (float)outer.top});
    }
  }
  if (skClipPath.isEmpty()) {
    skClipPath = BuildPath(path);
    skClipPath.setFillType(GetAlternateOrWindingFillType(fill_options));
    skClipPath.transform(ToSkMatrix(deviceMatrix));
    DebugShowSkiaPath(skClipPath);
  }
  m_pCanvas->clipPath(skClipPath, SkClipOp::kIntersect, true);
  DebugShowCanvasClip(this, m_pCanvas);
  return true;
}

bool CFX_SkiaDeviceDriver::SetClip_PathStroke(
    const CFX_Path& path,                  // path info
    const CFX_Matrix* pObject2Device,      // required transformation
    const CFX_GraphStateData* pGraphState  // graphic state, for pen attributes
) {
  SkPath skPath = BuildPath(path);
  SkMatrix skMatrix = ToSkMatrix(*pObject2Device);
  SkPaint skPaint;
  PaintStroke(&skPaint, pGraphState, skMatrix, CFX_FillRenderOptions());
  SkPath dst_path;
  skpathutils::FillPathWithPaint(skPath, skPaint, &dst_path);
  dst_path.transform(skMatrix);
  m_pCanvas->clipPath(dst_path, SkClipOp::kIntersect, true);
  DebugShowCanvasClip(this, m_pCanvas);
  return true;
}

// TODO(crbug.com/pdfium/1963): `blend_type` isn't used?
bool CFX_SkiaDeviceDriver::DrawPath(
    const CFX_Path& path,                   // path info
    const CFX_Matrix* pObject2Device,       // optional transformation
    const CFX_GraphStateData* pGraphState,  // graphic state, for pen attributes
    uint32_t fill_color,                    // fill color
    uint32_t stroke_color,                  // stroke color
    const CFX_FillRenderOptions& fill_options,
    BlendMode blend_type) {
  m_FillOptions = fill_options;

  SkPath skia_path = BuildPath(path);
  skia_path.setFillType(GetAlternateOrWindingFillType(fill_options));

  SkMatrix skMatrix = pObject2Device ? ToSkMatrix(*pObject2Device) : SkMatrix();
  SkPaint skPaint;
  skPaint.setAntiAlias(!fill_options.aliased_path);
  if (fill_options.full_cover) {
    skPaint.setBlendMode(SkBlendMode::kPlus);
  }
  int stroke_alpha = FXARGB_A(stroke_color);
  if (stroke_alpha) {
    const CFX_GraphStateData& graph_state =
        pGraphState ? *pGraphState : CFX_GraphStateData();
    PaintStroke(&skPaint, &graph_state, skMatrix, fill_options);
  }

  SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);
  m_pCanvas->concat(skMatrix);
  bool do_stroke = true;
  if (fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill &&
      fill_color) {
    SkPath strokePath;
    const SkPath* fillPath = &skia_path;
    if (stroke_alpha) {
      if (m_bGroupKnockout) {
        skpathutils::FillPathWithPaint(skia_path, skPaint, &strokePath);
        if (stroke_color == fill_color &&
            Op(skia_path, strokePath, SkPathOp::kUnion_SkPathOp, &strokePath)) {
          fillPath = &strokePath;
          do_stroke = false;
        } else if (Op(skia_path, strokePath, SkPathOp::kDifference_SkPathOp,
                      &strokePath)) {
          fillPath = &strokePath;
        }
      }
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    skPaint.setColor(fill_color);
    DebugShowSkiaDrawPath(this, m_pCanvas, skPaint, *fillPath);
    m_pCanvas->drawPath(*fillPath, skPaint);
  }
  if (stroke_alpha && do_stroke) {
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setColor(stroke_color);
    if (!skia_path.isLastContourClosed() && IsPathAPoint(skia_path)) {
      DCHECK_GE(skia_path.countPoints(), 1);
      m_pCanvas->drawPoint(skia_path.getPoint(0), skPaint);
    } else if (IsPathAPoint(skia_path) &&
               skPaint.getStrokeCap() != SkPaint::kRound_Cap) {
      // Do nothing. A closed 0-length closed path can be rendered only if
      // its line cap type is round.
    } else {
      DebugShowSkiaDrawPath(this, m_pCanvas, skPaint, skia_path);
      m_pCanvas->drawPath(skia_path, skPaint);
    }
  }
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
  DataVector<SkColor> sk_colors;
  DataVector<SkScalar> sk_pos;
  for (size_t j = 0; j < nFuncs; j++) {
    if (!pFuncs[j])
      continue;

    if (const CPDF_SampledFunc* pSampledFunc = pFuncs[j]->ToSampledFunc()) {
      /* TODO(caryclark)
         Type 0 Sampled Functions in PostScript can also have an Order integer
         in the dictionary. PDFium doesn't appear to check for this anywhere.
       */
      if (!AddSamples(pSampledFunc, sk_colors, sk_pos)) {
        return false;
      }
    } else if (const CPDF_ExpIntFunc* pExpIntFuc = pFuncs[j]->ToExpIntFunc()) {
      if (!AddColors(pExpIntFuc, sk_colors, /*is_encode_reversed=*/false)) {
        return false;
      }
      sk_pos.push_back(0);
      sk_pos.push_back(1);
    } else if (const CPDF_StitchFunc* pStitchFunc = pFuncs[j]->ToStitchFunc()) {
      if (!AddStitching(pStitchFunc, sk_colors, sk_pos)) {
        return false;
      }
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
    skMatrix.mapPoints(pts, std::size(pts));
    paint.setShader(SkGradientShader::MakeLinear(
        pts, sk_colors.data(), sk_pos.data(),
        fxcrt::CollectionSize<int>(sk_colors), SkTileMode::kClamp));
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
        pts[0], start_r, pts[1], end_r, sk_colors.data(), sk_pos.data(),
        fxcrt::CollectionSize<int>(sk_colors), SkTileMode::kClamp));
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
      size_t start_point = flag ? 4 : 0;
      size_t start_color = flag ? 2 : 0;
      if (flag) {
        SkPoint temp_cubics[4];
        for (size_t i = 0; i < std::size(temp_cubics); ++i) {
          temp_cubics[i] = cubics[(flag * 3 + i) % 12];
        }
        std::copy(std::begin(temp_cubics), std::end(temp_cubics),
                  std::begin(cubics));
        SkColor temp_colors[2] = {colors[flag % 4], colors[(flag + 1) % 4]};
        std::copy(std::begin(temp_colors), std::end(temp_colors),
                  std::begin(colors));
      }
      for (size_t i = start_point; i < std::size(cubics); ++i) {
        CFX_PointF point = stream.ReadCoords();
        cubics[i].fX = point.x;
        cubics[i].fY = point.y;
      }
      for (size_t i = start_color; i < std::size(colors); ++i) {
        float r;
        float g;
        float b;
        std::tie(r, g, b) = stream.ReadColor();
        colors[i] = SkColorSetARGB(0xFF, (U8CPU)(r * 255), (U8CPU)(g * 255),
                                   (U8CPU)(b * 255));
      }
      m_pCanvas->drawPatch(cubics, colors, /*texCoords=*/nullptr,
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

bool CFX_SkiaDeviceDriver::GetClipBox(FX_RECT* pRect) {
  SkIRect clip = m_pCanvas->getDeviceClipBounds();
  pRect->left = clip.fLeft;
  pRect->top = clip.fTop;
  pRect->right = clip.fRight;
  pRect->bottom = clip.fBottom;
  return true;
}

bool CFX_SkiaDeviceDriver::GetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                                     int left,
                                     int top) {
  const uint8_t* input_buffer = m_pBitmap->GetBuffer().data();
  if (!input_buffer) {
    return true;
  }

  uint8_t* output_buffer = pBitmap->GetWritableBuffer().data();
  DCHECK(output_buffer);

  SkImageInfo input_info =
      SkImageInfo::Make(m_pBitmap->GetWidth(), m_pBitmap->GetHeight(),
                        SkColorType::kN32_SkColorType, kPremul_SkAlphaType);
  sk_sp<SkImage> input = SkImages::RasterFromPixmap(
      SkPixmap(input_info, input_buffer, m_pBitmap->GetPitch()),
      /*rasterReleaseProc=*/nullptr, /*releaseContext=*/nullptr);

  SkImageInfo output_info = SkImageInfo::Make(
      pBitmap->GetWidth(), pBitmap->GetHeight(),
      Get32BitSkColorType(m_bRgbByteOrder), kPremul_SkAlphaType);
  sk_sp<SkSurface> output =
      SkSurfaces::WrapPixels(output_info, output_buffer, pBitmap->GetPitch());

  output->getCanvas()->drawImage(input, left, top, SkSamplingOptions());
  return true;
}

RetainPtr<CFX_DIBitmap> CFX_SkiaDeviceDriver::GetBackDrop() {
  return m_pBackdropBitmap;
}

bool CFX_SkiaDeviceDriver::SetDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                                     uint32_t color,
                                     const FX_RECT& src_rect,
                                     int left,
                                     int top,
                                     BlendMode blend_type) {
  if (m_pBitmap->GetBuffer().empty()) {
    return true;
  }

  CFX_Matrix m = CFX_RenderDevice::GetFlipMatrix(
      pBitmap->GetWidth(), pBitmap->GetHeight(), left, top);

  // `bNoSmoothing` prevents linear sampling when rendering bitmaps.
  FXDIB_ResampleOptions sampling_options;
  sampling_options.bNoSmoothing = true;

  return StartDIBitsSkia(pBitmap, src_rect, 0xFF, color, m, sampling_options,
                         blend_type);
}

bool CFX_SkiaDeviceDriver::StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                         uint32_t color,
                                         int dest_left,
                                         int dest_top,
                                         int dest_width,
                                         int dest_height,
                                         const FX_RECT* pClipRect,
                                         const FXDIB_ResampleOptions& options,
                                         BlendMode blend_type) {
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

  return StartDIBitsSkia(
      pSource, FX_RECT(0, 0, pSource->GetWidth(), pSource->GetHeight()), 0xFF,
      color, m, sampling_options, blend_type);
}

bool CFX_SkiaDeviceDriver::StartDIBits(
    const RetainPtr<CFX_DIBBase>& pSource,
    int bitmap_alpha,
    uint32_t color,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    std::unique_ptr<CFX_ImageRenderer>* handle,
    BlendMode blend_type) {
  return StartDIBitsSkia(
      pSource, FX_RECT(0, 0, pSource->GetWidth(), pSource->GetHeight()),
      bitmap_alpha, color, matrix, options, blend_type);
}

bool CFX_SkiaDeviceDriver::ContinueDIBits(CFX_ImageRenderer* handle,
                                          PauseIndicatorIface* pPause) {
  return false;
}

void CFX_DIBitmap::UnPreMultiply() {
  if (m_nFormat == Format::kUnPreMultiplied || GetBPP() != 32) {
    return;
  }

  void* buffer = GetWritableBuffer().data();
  if (!buffer) {
    return;
  }

  m_nFormat = Format::kUnPreMultiplied;
  int height = GetHeight();
  int width = GetWidth();
  int row_bytes = GetPitch();
  SkImageInfo premultiplied_info =
      SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
  SkPixmap premultiplied(premultiplied_info, buffer, row_bytes);
  SkImageInfo unpremultiplied_info =
      SkImageInfo::Make(width, height, kN32_SkColorType, kUnpremul_SkAlphaType);
  SkPixmap unpremultiplied(unpremultiplied_info, buffer, row_bytes);
  premultiplied.readPixels(unpremultiplied);
}

void CFX_DIBitmap::ForcePreMultiply() {
  m_nFormat = Format::kPreMultiplied;
}

bool CFX_DIBitmap::IsPremultiplied() const {
  return m_nFormat == Format::kPreMultiplied;
}

bool CFX_SkiaDeviceDriver::DrawBitsWithMask(
    const RetainPtr<CFX_DIBBase>& pSource,
    const RetainPtr<CFX_DIBBase>& pMask,
    int bitmap_alpha,
    const CFX_Matrix& matrix,
    BlendMode blend_type) {
  DebugValidate(m_pBitmap);

  sk_sp<SkImage> skia_source = pSource->RealizeSkImage();
  if (!skia_source) {
    return false;
  }

  DCHECK(pMask->IsMaskFormat());
  sk_sp<SkImage> skia_mask = pMask->RealizeSkImage();
  if (!skia_mask) {
    return false;
  }
  DCHECK_EQ(skia_mask->colorType(), kAlpha_8_SkColorType);

  {
    SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);

    const int src_width = pSource->GetWidth();
    const int src_height = pSource->GetHeight();
    SkMatrix skMatrix;
    SetBitmapMatrix(matrix, src_width, src_height, &skMatrix);
    m_pCanvas->concat(skMatrix);
    SkPaint paint;
    SetBitmapPaintForMerge(pSource->IsMaskFormat(), !m_FillOptions.aliased_path,
                           0xFFFFFFFF, bitmap_alpha, blend_type, &paint);
    sk_sp<SkShader> source_shader = skia_source->makeShader(
        SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
    sk_sp<SkShader> mask_shader = skia_mask->makeShader(
        SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
    paint.setShader(SkShaders::Blend(
        SkBlendMode::kSrcIn, std::move(mask_shader), std::move(source_shader)));
    m_pCanvas->drawRect(
        SkRect::MakeWH(SkIntToScalar(src_width), SkIntToScalar(src_height)),
        paint);
  }

  DebugValidate(m_pBitmap);
  return true;
}

bool CFX_SkiaDeviceDriver::SetBitsWithMask(
    const RetainPtr<CFX_DIBBase>& pBitmap,
    const RetainPtr<CFX_DIBBase>& pMask,
    int dest_left,
    int dest_top,
    int bitmap_alpha,
    BlendMode blend_type) {
  if (m_pBitmap->GetBuffer().empty()) {
    return true;
  }

  CFX_Matrix m = CFX_RenderDevice::GetFlipMatrix(
      pBitmap->GetWidth(), pBitmap->GetHeight(), dest_left, dest_top);
  return DrawBitsWithMask(pBitmap, pMask, bitmap_alpha, m, blend_type);
}

void CFX_SkiaDeviceDriver::SetGroupKnockout(bool group_knockout) {
  m_bGroupKnockout = group_knockout;
}

bool CFX_SkiaDeviceDriver::SyncInternalBitmaps() {
  if (!m_pOriginalBitmap) {
    return true;
  }

  int width = m_pOriginalBitmap->GetWidth();
  int height = m_pOriginalBitmap->GetHeight();
  DCHECK_EQ(width, m_pBitmap->GetWidth());
  DCHECK_EQ(height, m_pBitmap->GetHeight());
  DCHECK_EQ(FXDIB_Format::kRgb, m_pOriginalBitmap->GetFormat());
  m_pOriginalBitmap->TransferBitmap(/*dest_left=*/0, /*dest_top=*/0, width,
                                    height, m_pBitmap, /*src_left=*/0,
                                    /*src_top=*/0);
  return true;
}

void CFX_SkiaDeviceDriver::Clear(uint32_t color) {
  m_pCanvas->clear(color);
}

bool CFX_SkiaDeviceDriver::StartDIBitsSkia(
    const RetainPtr<CFX_DIBBase>& pSource,
    const FX_RECT& src_rect,
    int bitmap_alpha,
    uint32_t color,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    BlendMode blend_type) {
  DebugValidate(m_pBitmap);

  sk_sp<SkImage> skia_source = pSource->RealizeSkImage();
  if (!skia_source) {
    return false;
  }

  {
    SkAutoCanvasRestore scoped_save_restore(m_pCanvas, /*doSave=*/true);

    const int width = pSource->GetWidth();
    const int height = pSource->GetHeight();
    SkMatrix skMatrix;
    SetBitmapMatrix(matrix, width, height, &skMatrix);
    m_pCanvas->concat(skMatrix);
    SkPaint paint;
    SetBitmapPaint(pSource->IsMaskFormat(), !m_FillOptions.aliased_path,
                   bitmap_alpha, color, blend_type, &paint);

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

    m_pCanvas->drawImageRect(
        skia_source,
        SkRect::MakeLTRB(src_rect.left, src_rect.top, src_rect.right,
                         src_rect.bottom),
        SkRect::MakeWH(src_rect.Width(), src_rect.Height()), sampling_options,
        &paint, SkCanvas::kFast_SrcRectConstraint);
  }

  DebugValidate(m_pBitmap);
  return true;
}

CFX_SkiaDeviceDriver::CharDetail::CharDetail() = default;
CFX_SkiaDeviceDriver::CharDetail::~CharDetail() = default;

void CFX_DefaultRenderDevice::Clear(uint32_t color) {
  static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver())->Clear(color);
}

bool CFX_DefaultRenderDevice::AttachSkiaImpl(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout) {
  if (!pBitmap)
    return false;
  SetBitmap(pBitmap);
  auto driver =
      CFX_SkiaDeviceDriver::Create(std::move(pBitmap), bRgbByteOrder,
                                   std::move(pBackdropBitmap), bGroupKnockout);
  if (!driver)
    return false;

  SetDeviceDriver(std::move(driver));
  return true;
}

bool CFX_DefaultRenderDevice::AttachCanvas(SkCanvas* canvas) {
  auto driver = CFX_SkiaDeviceDriver::Create(canvas);
  if (!driver) {
    return false;
  }
  SetDeviceDriver(std::move(driver));
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
  auto driver = CFX_SkiaDeviceDriver::Create(std::move(pBitmap), false,
                                             std::move(pBackdropBitmap), false);
  if (!driver)
    return false;

  SetDeviceDriver(std::move(driver));
  return true;
}
