// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/skia/fx_skia_device.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <array>
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
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/ptr_util.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/agg/cfx_agg_imagerenderer.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/skia/include/core/SkBlendMode.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkClipOp.h"
#include "third_party/skia/include/core/SkColorFilter.h"
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
  float m[9];
  matrix.get9(m);
  printf("matrix (%g,%g,%g) (%g,%g,%g) (%g,%g,%g)\n", m[0], m[1], m[2], m[3],
         m[4], m[5], m[6], m[7], m[8]);
  SkRect local = canvas->getLocalClipBounds();
  SkIRect device = canvas->getDeviceClipBounds();

  printf("local bounds %g %g %g %g\n", local.fLeft, local.fTop, local.fRight,
         local.fBottom);
  printf("device bounds %d %d %d %d\n", device.fLeft, device.fTop,
         device.fRight, device.fBottom);
  FX_RECT clip_box = driver->GetClipBox();
  printf("reported bounds %d %d %d %d\n", clip_box.left, clip_box.top,
         clip_box.right, clip_box.bottom);
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
  if (text_options.aliasing_type == CFX_TextRenderOptions::kAliasing) {
    return SkFont::Edging::kAlias;
  }

  if (text_options.aliasing_type == CFX_TextRenderOptions::kAntiAliasing) {
    return SkFont::Edging::kAntiAlias;
  }

  DCHECK_EQ(text_options.aliasing_type, CFX_TextRenderOptions::kLcd);
  return SkFont::Edging::kSubpixelAntiAlias;
}

bool IsPathAPoint(const SkPath& path) {
  if (path.isEmpty()) {
    return false;
  }

  if (path.countPoints() == 1) {
    return true;
  }

  for (int i = 0; i < path.countPoints() - 1; ++i) {
    if (path.getPoint(i) != path.getPoint(i + 1)) {
      return false;
    }
  }
  return true;
}

SkPath BuildPath(const CFX_Path& path) {
  SkPath sk_path;
  pdfium::span<const CFX_Path::Point> points = path.GetPoints();
  for (size_t i = 0; i < points.size(); ++i) {
    const CFX_PointF& point = points[i].point_;
    CFX_Path::Point::Type point_type = points[i].type_;
    if (point_type == CFX_Path::Point::Type::kMove) {
      sk_path.moveTo(point.x, point.y);
    } else if (point_type == CFX_Path::Point::Type::kLine) {
      sk_path.lineTo(point.x, point.y);
    } else if (point_type == CFX_Path::Point::Type::kBezier) {
      const CFX_PointF& point2 = points[i + 1].point_;
      const CFX_PointF& point3 = points[i + 2].point_;
      sk_path.cubicTo(point.x, point.y, point2.x, point2.y, point3.x, point3.y);
      i += 2;
    }
    if (points[i].close_figure_) {
      sk_path.close();
    }
  }
  return sk_path;
}

SkMatrix ToSkMatrix(const CFX_Matrix& m) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a, m.c, m.e, m.b, m.d, m.f, 0, 0, 1);
  return skMatrix;
}

// use when pdf's y-axis points up instead of down
SkMatrix ToFlippedSkMatrix(const CFX_Matrix& m, float flip) {
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

// Clamps and scales a float in range [0.0, 1.0] to 0-255.
uint8_t ClampFloatToByte(float f) {
  return static_cast<uint8_t>(std::clamp(f, 0.0f, 1.0f) * 255.f + 0.5f);
}

// Add begin & end colors into `colors` array for each gradient transition.
//
// `is_encode_reversed` must be set to true when the parent function of `func`
// has an Encode array, and the matching pair of encode values for `func` are
// in decreasing order.
bool AddColors(const CPDF_ExpIntFunc* func,
               DataVector<SkColor>& colors,
               bool is_encode_reversed) {
  if (func->InputCount() != 1) {
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
  if (is_encode_reversed) {
    std::swap(begin_values, end_values);
  }

  colors.push_back(SkColorSetRGB(ClampFloatToByte(begin_values[0]),
                                 ClampFloatToByte(begin_values[1]),
                                 ClampFloatToByte(begin_values[2])));
  colors.push_back(SkColorSetRGB(ClampFloatToByte(end_values[0]),
                                 ClampFloatToByte(end_values[1]),
                                 ClampFloatToByte(end_values[2])));
  return true;
}

// Scale a float in range [0.0, 1.0] to 0-255.
uint8_t FloatToByte(float f) {
  DCHECK(f >= 0);
  DCHECK(f <= 1);
  return static_cast<uint8_t>(f * 255.99f);
}

bool AddSamples(const CPDF_SampledFunc* func,
                DataVector<SkColor>& colors,
                DataVector<float>& pos) {
  if (func->InputCount() != 1) {
    return false;
  }
  if (func->OutputCount() != 3) {  // expect rgb
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

  std::array<float, 3> colors_min;
  std::array<float, 3> colors_max;
  for (int i = 0; i < 3; ++i) {
    colors_min[i] = func->GetRange(i * 2);
    colors_max[i] = func->GetRange(i * 2 + 1);
  }
  pdfium::span<const uint8_t> sample_data = func->GetSampleStream()->GetSpan();
  CFX_BitStream bitstream(sample_data);
  for (uint32_t i = 0; i < sample_count; ++i) {
    std::array<float, 3> float_colors;
    for (uint32_t j = 0; j < 3; ++j) {
      float sample = static_cast<float>(bitstream.GetBits(sample_size));
      float interp = sample / (sample_count - 1);
      float_colors[j] =
          colors_min[j] + (colors_max[j] - colors_min[j]) * interp;
    }
    colors.push_back(SkColorSetRGB(FloatToByte(float_colors[0]),
                                   FloatToByte(float_colors[1]),
                                   FloatToByte(float_colors[2])));
    pos.push_back(static_cast<float>(i) / (sample_count - 1));
  }
  return true;
}

bool AddStitching(const CPDF_StitchFunc* func,
                  DataVector<SkColor>& colors,
                  DataVector<float>& pos) {
  float bounds_start = func->GetDomain(0);

  const auto& sub_functions = func->GetSubFunctions();
  const size_t sub_function_count = sub_functions.size();
  for (size_t i = 0; i < sub_function_count; ++i) {
    const CPDF_ExpIntFunc* sub_func = sub_functions[i]->ToExpIntFunc();
    if (!sub_func) {
      return false;
    }
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
float LineSide(const SkPoint& line_start,
               const SkPoint& line_end,
               const SkPoint& pt) {
  return (line_end.fY - line_start.fY) * pt.fX -
         (line_end.fX - line_start.fX) * pt.fY + line_end.fX * line_start.fY -
         line_end.fY * line_start.fX;
}

SkPoint IntersectSides(const SkPoint& parallelPt,
                       const SkVector& paraRay,
                       const SkPoint& perpendicularPt) {
  SkVector perpRay = {paraRay.fY, -paraRay.fX};
  float denom = perpRay.fY * paraRay.fX - paraRay.fY * perpRay.fX;
  if (!denom) {
    SkPoint zeroPt = {0, 0};
    return zeroPt;
  }
  SkVector ab0 = parallelPt - perpendicularPt;
  float numerA = ab0.fY * perpRay.fX - perpRay.fY * ab0.fX;
  numerA /= denom;
  SkPoint result = {parallelPt.fX + paraRay.fX * numerA,
                    parallelPt.fY + paraRay.fY * numerA};
  return result;
}

void ClipAngledGradient(pdfium::span<const SkPoint, 2> pts,
                        pdfium::span<const SkPoint, 4> rect_pts,
                        bool clip_start,
                        bool clip_end,
                        SkPath* clip) {
  // find the corners furthest from the gradient perpendiculars
  float minPerpDist = std::numeric_limits<float>::max();
  float maxPerpDist = std::numeric_limits<float>::lowest();
  int minPerpPtIndex = -1;
  int maxPerpPtIndex = -1;
  SkVector slope = pts[1] - pts[0];
  const SkPoint start_perp[2] = {pts[0],
                                 {pts[0].fX + slope.fY, pts[0].fY - slope.fX}};
  const SkPoint end_perp[2] = {pts[1],
                               {pts[1].fX + slope.fY, pts[1].fY - slope.fX}};
  for (int i = 0; i < 4; ++i) {
    float sDist = LineSide(start_perp[0], start_perp[1], rect_pts[i]);
    float eDist = LineSide(end_perp[0], end_perp[1], rect_pts[i]);
    if (sDist * eDist <= 0) {  // if the signs are different,
      continue;                // the point is inside the gradient
    }
    if (sDist < 0) {
      float smaller = std::min(sDist, eDist);
      if (minPerpDist > smaller) {
        minPerpDist = smaller;
        minPerpPtIndex = i;
      }
    } else {
      float larger = std::max(sDist, eDist);
      if (maxPerpDist < larger) {
        maxPerpDist = larger;
        maxPerpPtIndex = i;
      }
    }
  }
  if (minPerpPtIndex < 0 && maxPerpPtIndex < 0) {  // nothing's outside
    return;
  }

  // determine if negative distances are before start or after end
  const SkPoint before_start = {pts[0].fX * 2 - pts[1].fX,
                                pts[0].fY * 2 - pts[1].fY};
  bool before_neg = LineSide(start_perp[0], start_perp[1], before_start) < 0;

  int noClipStartIndex = maxPerpPtIndex;
  int noClipEndIndex = minPerpPtIndex;
  if (before_neg) {
    std::swap(noClipStartIndex, noClipEndIndex);
  }
  if ((!clip_start && noClipStartIndex < 0) ||
      (!clip_end && noClipEndIndex < 0)) {
    return;
  }

  const SkPoint& startEdgePt = clip_start ? pts[0] : rect_pts[noClipStartIndex];
  const SkPoint& endEdgePt = clip_end ? pts[1] : rect_pts[noClipEndIndex];

  // find the corners that bound the gradient
  float minDist = std::numeric_limits<float>::max();
  float maxDist = std::numeric_limits<float>::lowest();
  int minBounds = -1;
  int maxBounds = -1;
  for (int i = 0; i < 4; ++i) {
    float dist = LineSide(pts[0], pts[1], rect_pts[i]);
    if (minDist > dist) {
      minDist = dist;
      minBounds = i;
    }
    if (maxDist < dist) {
      maxDist = dist;
      maxBounds = i;
    }
  }
  if (minBounds < 0 || maxBounds < 0) {
    return;
  }
  if (minBounds == maxBounds) {
    return;
  }
  // construct a clip parallel to the gradient that goes through
  // rect_pts[minBounds] and rect_pts[maxBounds] and perpendicular to the
  // gradient that goes through startEdgePt, endEdgePt.
  clip->moveTo(IntersectSides(rect_pts[minBounds], slope, startEdgePt));
  clip->lineTo(IntersectSides(rect_pts[minBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rect_pts[maxBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rect_pts[maxBounds], slope, startEdgePt));
}

// Converts a stroking path to scanlines
void PaintStroke(SkPaint* spaint,
                 const CFX_GraphStateData* graph_state,
                 const SkMatrix& matrix,
                 const CFX_FillRenderOptions& fill_options) {
  SkPaint::Cap cap;
  switch (graph_state->line_cap()) {
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
  switch (graph_state->line_join()) {
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
  inverse.mapPoints(deviceUnits);

  float width = fill_options.zero_area
                    ? 0.0f
                    : std::max(graph_state->line_width(),
                               std::min(deviceUnits[0].length(),
                                        deviceUnits[1].length()));
  const std::vector<float>& dash_array = graph_state->dash_array();
  if (!dash_array.empty()) {
    size_t count = (dash_array.size() + 1) / 2;
    DataVector<float> intervals(count * 2);
    // Set dash pattern
    for (size_t i = 0; i < count; i++) {
      float on = dash_array[i * 2];
      if (on <= 0.000001f) {
        on = 0.1f;
      }
      float off = i * 2 + 1 == dash_array.size() ? on : dash_array[i * 2 + 1];
      off = std::max(off, 0.0f);
      intervals[i * 2] = on;
      intervals[i * 2 + 1] = off;
    }
    spaint->setPathEffect(
        SkDashPathEffect::Make(intervals, graph_state->dash_phase()));
  }
  spaint->setStyle(SkPaint::kStroke_Style);
  spaint->setAntiAlias(!fill_options.aliased_path);
  spaint->setStrokeWidth(width);
  spaint->setStrokeMiter(graph_state->miter_limit());
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
                    float alpha,
                    uint32_t argb,
                    BlendMode blend_type,
                    SkPaint* paint) {
  DCHECK_GE(alpha, 0.0f);
  DCHECK_LE(alpha, 1.0f);

  if (is_mask) {
    paint->setColor(argb);
  } else if (alpha != 1.0f) {
    paint->setAlphaf(alpha);
  }

  paint->setAntiAlias(anti_alias);
  paint->setBlendMode(GetSkiaBlendMode(blend_type));
}

void SetBitmapPaintForMerge(bool is_mask,
                            bool anti_alias,
                            float alpha,
                            BlendMode blend_type,
                            SkPaint* paint) {
  if (is_mask) {
    paint->setColorFilter(SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrc));
  }

  paint->setAlphaf(alpha);
  paint->setAntiAlias(anti_alias);
  paint->setBlendMode(GetSkiaBlendMode(blend_type));
}

// Makes a bitmap filled with a solid color for debugging with `SkPicture`.
RetainPtr<CFX_DIBitmap> MakeDebugBitmap(int width, int height, uint32_t color) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  // TODO(crbug.com/42271020): Consider adding support for
  // `FXDIB_Format::kBgraPremul`
  if (!bitmap->Create(width, height, FXDIB_Format::kBgra)) {
    return nullptr;
  }

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
    if (!cp.glyph_adjust_) {
      continue;
    }
    bool upright = 0 == cp.adjust_matrix_[1] && 0 == cp.adjust_matrix_[2];
    if (cp.adjust_matrix_[0] != cp.adjust_matrix_[3]) {
      if (upright && 1 == cp.adjust_matrix_[3]) {
        if (1 == scaleX) {
          scaleX = cp.adjust_matrix_[0];
        } else if (scaleX != cp.adjust_matrix_[0]) {
          oneAtATime = true;
        }
      } else {
        oneAtATime = true;
      }
    } else if (cp.adjust_matrix_[1] != -cp.adjust_matrix_[2]) {
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
  if (!driver->canvas_) {
    return nullptr;
  }

  return driver;
}

// static
std::unique_ptr<CFX_SkiaDeviceDriver> CFX_SkiaDeviceDriver::Create(
    SkCanvas& canvas) {
  auto driver = pdfium::WrapUnique(new CFX_SkiaDeviceDriver(canvas));
  if (!driver->bitmap_ || !driver->backdrop_bitmap_) {
    return nullptr;
  }

  return driver;
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout)
    : bitmap_(std::move(pBitmap)),
      backdrop_bitmap_(pBackdropBitmap),
      rgb_byte_order_(bRgbByteOrder),
      group_knockout_(bGroupKnockout) {
  SkColorType color_type;
  const int bpp = bitmap_->GetBPP();
  SkAlphaType alpha_type = kPremul_SkAlphaType;
  if (bpp == 8) {
    color_type = bitmap_->IsAlphaFormat() || bitmap_->IsMaskFormat()
                     ? kAlpha_8_SkColorType
                     : kGray_8_SkColorType;
  } else if (bpp == 24) {
    DCHECK_EQ(bitmap_->GetFormat(), FXDIB_Format::kBgr);

    // Save the input bitmap as `original_bitmap_` and save its 32 bpp
    // equivalent at `bitmap_` for Skia's internal process.
    original_bitmap_ = std::move(bitmap_);
    const int width = original_bitmap_->GetWidth();
    const int height = original_bitmap_->GetHeight();

    bitmap_ = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!bitmap_->Create(width, height, FXDIB_Format::kBgraPremul) ||
        !bitmap_->TransferBitmap(width, height, original_bitmap_,
                                 /*src_left=*/0, /*src_top=*/0)) {
      // Skip creating SkCanvas if the 32-bpp bitmap creation fails.
      // CFX_SkiaDeviceDriver::Create() will check for the missing `canvas_`
      // and not use `this`.
      // Also reset `original_bitmap_` so the dtor does not try to transfer
      // `bitmap_` back to `original_bitmap_`.
      original_bitmap_.Reset();
      return;
    }

    color_type = Get32BitSkColorType(bRgbByteOrder);
  } else {
    DCHECK_EQ(bpp, 32);
    color_type = Get32BitSkColorType(bRgbByteOrder);
    FXDIB_Format format = bitmap_->GetFormat();
    if (format == FXDIB_Format::kBgrx) {
      alpha_type = kOpaque_SkAlphaType;
    } else if (format == FXDIB_Format::kBgra) {
      alpha_type = kUnpremul_SkAlphaType;
    }
  }

  SkImageInfo imageInfo = SkImageInfo::Make(
      bitmap_->GetWidth(), bitmap_->GetHeight(), color_type, alpha_type);
  surface_ = SkSurfaces::WrapPixels(
      imageInfo, bitmap_->GetWritableBuffer().data(), bitmap_->GetPitch());
  canvas_ = surface_->getCanvas();
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(SkCanvas& canvas)
    : canvas_(&canvas), rgb_byte_order_(false), group_knockout_(false) {
  int width = canvas_->imageInfo().width();
  int height = canvas_->imageInfo().height();
  DCHECK_EQ(kUnknown_SkColorType, canvas_->imageInfo().colorType());

  static constexpr uint32_t kMagenta = 0xffff00ff;
  static constexpr uint32_t kGreen = 0xff00ff00;
  bitmap_ = MakeDebugBitmap(width, height, kMagenta);
  backdrop_bitmap_ = MakeDebugBitmap(width, height, kGreen);
}

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
  // Convert and transfer the internal processed result to the original 24 bpp
  // bitmap provided by the render device if needed.
  SyncInternalBitmaps();
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
  if (pFont->GetFontSpan().empty()) {
    return false;
  }

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

  SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);
  const float horizontal_flip = font_size < 0 ? -1.f : 1.f;
  const float vertical_flip = pFont->IsVertical() ? -1.f : 1.f;
  SkMatrix skMatrix = ToFlippedSkMatrix(mtObject2Device, horizontal_flip);
  canvas_->concat(skMatrix);
  DataVector<SkPoint> positions(pCharPos.size());
  DataVector<uint16_t> glyphs(pCharPos.size());

  for (size_t index = 0; index < pCharPos.size(); ++index) {
    const TextCharPos& cp = pCharPos[index];
    positions[index] = {cp.origin_.x * horizontal_flip,
                        cp.origin_.y * vertical_flip};
    glyphs[index] = static_cast<uint16_t>(cp.glyph_index_);
#if BUILDFLAG(IS_APPLE)
    if (cp.ext_gid_) {
      glyphs[index] = static_cast<uint16_t>(cp.ext_gid_);
    }
#endif
  }

  for (size_t index = 0; index < pCharPos.size(); ++index) {
    const TextCharPos& cp = pCharPos[index];
    if (cp.glyph_adjust_) {
      if (0 == cp.adjust_matrix_[1] && 0 == cp.adjust_matrix_[2] &&
          1 == cp.adjust_matrix_[3]) {
        font.setScaleX(cp.adjust_matrix_[0]);
        auto blob =
            SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]),
                                     font, SkTextEncoding::kGlyphID);
        canvas_->drawTextBlob(blob, positions[index].fX, positions[index].fY,
                              paint);
        font.setScaleX(SkIntToScalar(1));
      } else {
        SkAutoCanvasRestore scoped_save_restore2(canvas_, /*doSave=*/true);
        SkMatrix adjust;
        adjust.preTranslate(positions[index].fX, -positions[index].fY);
        adjust.setScaleX(cp.adjust_matrix_[0]);
        adjust.setSkewX(cp.adjust_matrix_[1]);
        adjust.setSkewY(cp.adjust_matrix_[2]);
        adjust.setScaleY(cp.adjust_matrix_[3]);
        canvas_->concat(adjust);
        auto blob =
            SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]),
                                     font, SkTextEncoding::kGlyphID);
        canvas_->drawTextBlob(blob, 0, 0, paint);
      }
    } else {
      auto blob =
          SkTextBlob::MakeFromText(&glyphs[index], sizeof(glyphs[index]), font,
                                   SkTextEncoding::kGlyphID);
      canvas_->drawTextBlob(blob, positions[index].fX, positions[index].fY,
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

  char_details_.SetCount(char_pos.size());
  if (hasRSX) {
    rsxform_.resize(char_pos.size());
  } else {
    rsxform_.resize(0);
  }

  const float horizontal_flip = font_size < 0 ? -1.f : 1.f;
  const float vertical_flip = pFont->IsVertical() ? -1.f : 1.f;
  for (size_t index = 0; index < char_pos.size(); ++index) {
    const TextCharPos& cp = char_pos[index];
    char_details_.SetPositionAt(
        index, {cp.origin_.x * horizontal_flip, cp.origin_.y * vertical_flip});
    char_details_.SetGlyphAt(index, static_cast<uint16_t>(cp.glyph_index_));
    char_details_.SetFontCharWidthAt(index, cp.font_char_width_);
#if BUILDFLAG(IS_APPLE)
    if (cp.ext_gid_) {
      char_details_.SetGlyphAt(index, static_cast<uint16_t>(cp.ext_gid_));
    }
#endif
  }
  if (hasRSX) {
    const DataVector<SkPoint>& positions = char_details_.GetPositions();
    for (size_t index = 0; index < char_pos.size(); ++index) {
      const TextCharPos& cp = char_pos[index];
      SkRSXform& rsxform = rsxform_[index];
      if (cp.glyph_adjust_) {
        rsxform.fSCos = cp.adjust_matrix_[0];
        rsxform.fSSin = cp.adjust_matrix_[1];
        rsxform.fTx = cp.adjust_matrix_[0] * positions[index].fX;
        rsxform.fTy = -cp.adjust_matrix_[3] * positions[index].fY;
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

  SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);
  canvas_->concat(ToFlippedSkMatrix(matrix, horizontal_flip));

  const DataVector<uint16_t>& glyphs = char_details_.GetGlyphs();
  if (!rsxform_.empty()) {
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromRSXform(
        glyphs.data(), glyphs.size() * sizeof(uint16_t), rsxform_, font,
        SkTextEncoding::kGlyphID);
    canvas_->drawTextBlob(blob, 0, 0, skPaint);
    return true;
  }
  const DataVector<SkPoint>& positions = char_details_.GetPositions();
  const DataVector<uint32_t>& widths = char_details_.GetFontCharWidths();
  for (size_t i = 0; i < char_details_.Count(); ++i) {
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
    pdfium::span positions_span(positions);
    auto blob = SkTextBlob::MakeFromPosText(&glyphs[i], sizeof(uint16_t),
                                            positions_span.subspan(i), font,
                                            SkTextEncoding::kGlyphID);
    canvas_->drawTextBlob(blob, 0, 0, skPaint);
  }
  return true;
}

int CFX_SkiaDeviceDriver::GetDriverType() const {
  return 1;
}

bool CFX_SkiaDeviceDriver::MultiplyAlpha(float alpha) {
  CHECK_GE(alpha, 0.0f);
  CHECK_LE(alpha, 1.0f);

  if (alpha != 1.0f) {
    SkPaint paint;
    paint.setAlphaf(alpha);
    paint.setBlendMode(SkBlendMode::kDstIn);
    canvas_->drawPaint(paint);
  }
  return true;
}

bool CFX_SkiaDeviceDriver::MultiplyAlphaMask(
    RetainPtr<const CFX_DIBitmap> mask) {
  CHECK_EQ(FXDIB_Format::k8bppMask, mask->GetFormat());

  sk_sp<SkImage> skia_mask = mask->RealizeSkImage();
  if (!skia_mask) {
    return false;
  }
  DCHECK_EQ(skia_mask->colorType(), kAlpha_8_SkColorType);

  SkPaint paint;
  paint.setBlendMode(SkBlendMode::kDstIn);
  canvas_->drawImageRect(skia_mask, SkRect::Make(canvas_->imageInfo().bounds()),
                         SkSamplingOptions(), &paint);
  return true;
}

DeviceType CFX_SkiaDeviceDriver::GetDeviceType() const {
  return DeviceType::kDisplay;
}

int CFX_SkiaDeviceDriver::GetDeviceCaps(int caps_id) const {
  switch (caps_id) {
    case FXDC_PIXEL_WIDTH:
      return canvas_->imageInfo().width();
    case FXDC_PIXEL_HEIGHT:
      return canvas_->imageInfo().height();
    case FXDC_BITS_PIXEL:
      return 32;
    case FXDC_HORZ_SIZE:
    case FXDC_VERT_SIZE:
      return 0;
    case FXDC_RENDER_CAPS:
      return FXRC_GET_BITS | FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE |
             FXRC_BLEND_MODE | FXRC_SOFT_CLIP | FXRC_ALPHA_OUTPUT |
             FXRC_FILLSTROKE_PATH | FXRC_SHADING | FXRC_PREMULTIPLIED_ALPHA;
    default:
      NOTREACHED();
  }
}

void CFX_SkiaDeviceDriver::SaveState() {
  canvas_->save();
}

void CFX_SkiaDeviceDriver::RestoreState(bool bKeepSaved) {
  canvas_->restore();
  if (bKeepSaved) {
    canvas_->save();
  }
}

bool CFX_SkiaDeviceDriver::SetClip_PathFill(
    const CFX_Path& path,              // path info
    const CFX_Matrix* pObject2Device,  // flips object's y-axis
    const CFX_FillRenderOptions& fill_options) {
  fill_options_ = fill_options;
  const CFX_Matrix& deviceMatrix =
      pObject2Device ? *pObject2Device : CFX_Matrix();

  SkPath skClipPath;
  if (path.GetPoints().size() == 5 || path.GetPoints().size() == 4) {
    std::optional<CFX_FloatRect> maybe_rectf = path.GetRect(&deviceMatrix);
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
  canvas_->clipPath(skClipPath, SkClipOp::kIntersect, true);
  DebugShowCanvasClip(this, canvas_);
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
  canvas_->clipPath(dst_path, SkClipOp::kIntersect, true);
  DebugShowCanvasClip(this, canvas_);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawPath(const CFX_Path& path,
                                    const CFX_Matrix* pObject2Device,
                                    const CFX_GraphStateData* pGraphState,
                                    uint32_t fill_color,
                                    uint32_t stroke_color,
                                    const CFX_FillRenderOptions& fill_options) {
  fill_options_ = fill_options;

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

  SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);
  canvas_->concat(skMatrix);
  bool do_stroke = true;
  if (fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill &&
      fill_color) {
    SkPath strokePath;
    const SkPath* fillPath = &skia_path;
    if (stroke_alpha) {
      if (group_knockout_) {
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
    DrawPathImpl(*fillPath, skPaint);
  }
  if (stroke_alpha && do_stroke) {
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setColor(stroke_color);
    if (!skia_path.isLastContourClosed() && IsPathAPoint(skia_path)) {
      DCHECK_GE(skia_path.countPoints(), 1);
      canvas_->drawPoint(skia_path.getPoint(0), skPaint);
    } else if (IsPathAPoint(skia_path) &&
               skPaint.getStrokeCap() != SkPaint::kRound_Cap) {
      // Do nothing. A closed 0-length closed path can be rendered only if
      // its line cap type is round.
    } else {
      DrawPathImpl(skia_path, skPaint);
    }
  }
  return true;
}

bool CFX_SkiaDeviceDriver::FillRect(const FX_RECT& rect, uint32_t fill_color) {
  SkPaint spaint;
  spaint.setAntiAlias(true);
  spaint.setColor(fill_color);
  SkRect srect = SkRect::MakeLTRB(rect.left, std::min(rect.top, rect.bottom),
                                  rect.right, std::max(rect.bottom, rect.top));
  DebugShowSkiaDrawRect(this, canvas_, spaint, srect);
  canvas_->drawRect(srect, spaint);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawShading(const CPDF_ShadingPattern& pattern,
                                       const CFX_Matrix& matrix,
                                       const FX_RECT& clip_rect,
                                       int alpha) {
  const ShadingType shading_type = pattern.GetShadingType();
  if (shading_type != kAxialShading && shading_type != kRadialShading) {
    // TODO(caryclark) more types
    return false;
  }
  CPDF_ColorSpace::Family cs_family = pattern.GetCS()->GetFamily();
  if (CPDF_ColorSpace::Family::kDeviceRGB != cs_family &&
      CPDF_ColorSpace::Family::kDeviceGray != cs_family) {
    return false;
  }
  const std::vector<std::unique_ptr<CPDF_Function>>& pFuncs =
      pattern.GetFuncs();
  size_t nFuncs = pFuncs.size();
  if (nFuncs > 1) {  // TODO(caryclark) remove this restriction
    return false;
  }
  RetainPtr<const CPDF_Dictionary> dict = pattern.GetShadingObject()->GetDict();
  RetainPtr<const CPDF_Array> pCoords = dict->GetArrayFor("Coords");
  if (!pCoords) {
    return false;
  }
  // TODO(caryclark) Respect Domain[0], Domain[1]. (Don't know what they do
  // yet.)
  DataVector<SkColor> sk_colors;
  DataVector<float> sk_pos;
  for (size_t j = 0; j < nFuncs; j++) {
    if (!pFuncs[j]) {
      continue;
    }

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
  RetainPtr<const CPDF_Array> pArray = dict->GetArrayFor("Extend");
  bool clipStart = !pArray || !pArray->GetIntegerAt(0);
  bool clipEnd = !pArray || !pArray->GetIntegerAt(1);
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setAlpha(alpha);
  SkMatrix skMatrix = ToSkMatrix(matrix);
  SkRect skRect = SkRect::MakeLTRB(clip_rect.left, clip_rect.top,
                                   clip_rect.right, clip_rect.bottom);
  SkPath skClip;
  SkPath skPath;
  if (shading_type == kAxialShading) {
    float start_x = pCoords->GetFloatAt(0);
    float start_y = pCoords->GetFloatAt(1);
    float end_x = pCoords->GetFloatAt(2);
    float end_y = pCoords->GetFloatAt(3);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};
    skMatrix.mapPoints(pts);
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
        if (clipStart) {
          skRect.fTop = std::max(skRect.fTop, pts[0].fY);
        }
        if (clipEnd) {
          skRect.fBottom = std::min(skRect.fBottom, pts[1].fY);
        }
      } else if (pts[0].fY == pts[1].fY) {  // horizontal
        if (pts[0].fX > pts[1].fX) {
          std::swap(pts[0].fX, pts[1].fX);
          std::swap(clipStart, clipEnd);
        }
        if (clipStart) {
          skRect.fLeft = std::max(skRect.fLeft, pts[0].fX);
        }
        if (clipEnd) {
          skRect.fRight = std::min(skRect.fRight, pts[1].fX);
        }
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
    CHECK_EQ(shading_type, kRadialShading);
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
      if (clipStart && start_r) {
        skClip.addCircle(pts[0].fX, pts[0].fY, start_r);
      }
      if (clipEnd) {
        skClip.addCircle(pts[1].fX, pts[1].fY, end_r, SkPathDirection::kCCW);
      } else {
        skClip.setFillType(SkPathFillType::kInverseWinding);
      }
      skClip.transform(skMatrix);
    }
    SkMatrix inverse;
    if (!skMatrix.invert(&inverse)) {
      return false;
    }
    skPath.addRect(skRect);
    skPath.transform(inverse);
  }
  SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);
  if (!skClip.isEmpty()) {
    canvas_->clipPath(skClip, SkClipOp::kIntersect, true);
  }
  canvas_->concat(skMatrix);
  DrawPathImpl(skPath, paint);
  return true;
}

FX_RECT CFX_SkiaDeviceDriver::GetClipBox() const {
  SkIRect clip = canvas_->getDeviceClipBounds();
  return FX_RECT(clip.fLeft, clip.fTop, clip.fRight, clip.fBottom);
}

bool CFX_SkiaDeviceDriver::GetDIBits(RetainPtr<CFX_DIBitmap> bitmap,
                                     int left,
                                     int top) const {
  const uint8_t* input_buffer = bitmap_->GetBuffer().data();
  if (!input_buffer) {
    return true;
  }

  uint8_t* output_buffer = bitmap->GetWritableBuffer().data();
  DCHECK(output_buffer);

  SkImageInfo input_info = canvas_->imageInfo();
  sk_sp<SkImage> input = SkImages::RasterFromPixmap(
      SkPixmap(input_info, input_buffer, bitmap_->GetPitch()),
      /*rasterReleaseProc=*/nullptr, /*releaseContext=*/nullptr);

  CHECK_EQ(32, bitmap->GetBPP());
  SkImageInfo output_info = SkImageInfo::Make(
      bitmap->GetWidth(), bitmap->GetHeight(),
      Get32BitSkColorType(rgb_byte_order_),
      bitmap->IsPremultiplied() ? kPremul_SkAlphaType : kUnpremul_SkAlphaType);
  sk_sp<SkSurface> output =
      SkSurfaces::WrapPixels(output_info, output_buffer, bitmap->GetPitch());

  output->getCanvas()->drawImage(input, left, top, SkSamplingOptions());
  return true;
}

RetainPtr<const CFX_DIBitmap> CFX_SkiaDeviceDriver::GetBackDrop() const {
  return backdrop_bitmap_;
}

bool CFX_SkiaDeviceDriver::SetDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                                     uint32_t color,
                                     const FX_RECT& src_rect,
                                     int left,
                                     int top,
                                     BlendMode blend_type) {
  if (bitmap_->GetBuffer().empty()) {
    return true;
  }

  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(
      bitmap->GetWidth(), bitmap->GetHeight(), left, top);

  // `bNoSmoothing` prevents linear sampling when rendering bitmaps.
  FXDIB_ResampleOptions sampling_options;
  sampling_options.bNoSmoothing = true;

  return StartDIBitsSkia(std::move(bitmap), src_rect, /*alpha=*/1.0f, color,
                         matrix, sampling_options, blend_type);
}

bool CFX_SkiaDeviceDriver::StretchDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                                         uint32_t color,
                                         int dest_left,
                                         int dest_top,
                                         int dest_width,
                                         int dest_height,
                                         const FX_RECT* pClipRect,
                                         const FXDIB_ResampleOptions& options,
                                         BlendMode blend_type) {
  if (bitmap_->GetBuffer().empty()) {
    return true;
  }

  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(dest_width, dest_height,
                                                      dest_left, dest_top);
  SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);
  SkRect skClipRect = SkRect::MakeLTRB(pClipRect->left, pClipRect->bottom,
                                       pClipRect->right, pClipRect->top);
  canvas_->clipRect(skClipRect, SkClipOp::kIntersect, true);

  // `bNoSmoothing` prevents linear sampling when rendering bitmaps.
  FXDIB_ResampleOptions sampling_options;
  sampling_options.bNoSmoothing = true;

  FX_RECT rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
  return StartDIBitsSkia(std::move(bitmap), rect, /*alpha=*/1.0f, color, matrix,
                         sampling_options, blend_type);
}

RenderDeviceDriverIface::StartResult CFX_SkiaDeviceDriver::StartDIBits(
    RetainPtr<const CFX_DIBBase> bitmap,
    float alpha,
    uint32_t color,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    BlendMode blend_type) {
  FX_RECT rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
  bool success = StartDIBitsSkia(std::move(bitmap), rect, alpha, color, matrix,
                                 options, blend_type);
  return {success ? Result::kSuccess : Result::kFailure, nullptr};
}

void CFX_DIBitmap::PreMultiply() {
  CHECK(CFX_DefaultRenderDevice::UseSkiaRenderer());
  if (GetFormat() != FXDIB_Format::kBgra) {
    return;
  }

  void* buffer = GetWritableBuffer().data();
  if (!buffer) {
    return;
  }

  SetFormat(FXDIB_Format::kBgraPremul);
  int height = GetHeight();
  int width = GetWidth();
  int row_bytes = GetPitch();
  SkImageInfo premultiplied_info =
      SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
  SkPixmap premultiplied(premultiplied_info, buffer, row_bytes);
  SkImageInfo unpremultiplied_info =
      SkImageInfo::Make(width, height, kN32_SkColorType, kUnpremul_SkAlphaType);
  SkPixmap unpremultiplied(unpremultiplied_info, buffer, row_bytes);
  unpremultiplied.readPixels(premultiplied);
}

void CFX_DIBitmap::UnPreMultiply() {
  CHECK(CFX_DefaultRenderDevice::UseSkiaRenderer());
  if (GetFormat() != FXDIB_Format::kBgraPremul) {
    return;
  }

  void* buffer = GetWritableBuffer().data();
  if (!buffer) {
    return;
  }

  SetFormat(FXDIB_Format::kBgra);
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

bool CFX_SkiaDeviceDriver::DrawBitsWithMask(RetainPtr<const CFX_DIBBase> bitmap,
                                            RetainPtr<const CFX_DIBBase> mask,
                                            float alpha,
                                            const CFX_Matrix& matrix,
                                            BlendMode blend_type) {
  DebugValidate(bitmap_);

  sk_sp<SkImage> skia_source = bitmap->RealizeSkImage();
  if (!skia_source) {
    return false;
  }

  DCHECK(mask->IsMaskFormat());
  sk_sp<SkImage> skia_mask = mask->RealizeSkImage();
  if (!skia_mask) {
    return false;
  }
  DCHECK_EQ(skia_mask->colorType(), kAlpha_8_SkColorType);

  {
    SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);

    const int src_width = bitmap->GetWidth();
    const int src_height = bitmap->GetHeight();
    SkMatrix skMatrix;
    SetBitmapMatrix(matrix, src_width, src_height, &skMatrix);
    canvas_->concat(skMatrix);
    SkPaint paint;
    SetBitmapPaintForMerge(bitmap->IsMaskFormat(), !fill_options_.aliased_path,
                           alpha, blend_type, &paint);
    sk_sp<SkShader> source_shader = skia_source->makeShader(
        SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
    sk_sp<SkShader> mask_shader = skia_mask->makeShader(
        SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
    paint.setShader(SkShaders::Blend(
        SkBlendMode::kSrcIn, std::move(mask_shader), std::move(source_shader)));
    canvas_->drawRect(
        SkRect::MakeWH(SkIntToScalar(src_width), SkIntToScalar(src_height)),
        paint);
  }

  DebugValidate(bitmap_);
  return true;
}

bool CFX_SkiaDeviceDriver::SetBitsWithMask(RetainPtr<const CFX_DIBBase> bitmap,
                                           RetainPtr<const CFX_DIBBase> mask,
                                           int dest_left,
                                           int dest_top,
                                           float alpha,
                                           BlendMode blend_type) {
  if (bitmap_->GetBuffer().empty()) {
    return true;
  }

  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(
      bitmap->GetWidth(), bitmap->GetHeight(), dest_left, dest_top);
  return DrawBitsWithMask(std::move(bitmap), std::move(mask), alpha, matrix,
                          blend_type);
}

void CFX_SkiaDeviceDriver::SetGroupKnockout(bool group_knockout) {
  group_knockout_ = group_knockout;
}

void CFX_SkiaDeviceDriver::SyncInternalBitmaps() {
  if (!original_bitmap_) {
    return;
  }

  int width = original_bitmap_->GetWidth();
  int height = original_bitmap_->GetHeight();
  DCHECK_EQ(width, bitmap_->GetWidth());
  DCHECK_EQ(height, bitmap_->GetHeight());
  DCHECK_EQ(FXDIB_Format::kBgr, original_bitmap_->GetFormat());
  original_bitmap_->TransferBitmap(width, height, bitmap_, /*src_left=*/0,
                                   /*src_top=*/0);
}

void CFX_SkiaDeviceDriver::Clear(uint32_t color) {
  canvas_->clear(color);
}

bool CFX_SkiaDeviceDriver::StartDIBitsSkia(RetainPtr<const CFX_DIBBase> bitmap,
                                           const FX_RECT& src_rect,
                                           float alpha,
                                           uint32_t color,
                                           const CFX_Matrix& matrix,
                                           const FXDIB_ResampleOptions& options,
                                           BlendMode blend_type) {
  DebugValidate(bitmap_);

  sk_sp<SkImage> skia_source = bitmap->RealizeSkImage();
  if (!skia_source) {
    return false;
  }

  {
    SkAutoCanvasRestore scoped_save_restore(canvas_, /*doSave=*/true);

    const int width = bitmap->GetWidth();
    const int height = bitmap->GetHeight();
    SkMatrix skMatrix;
    SetBitmapMatrix(matrix, width, height, &skMatrix);
    canvas_->concat(skMatrix);
    SkPaint paint;
    SetBitmapPaint(bitmap->IsMaskFormat(), !fill_options_.aliased_path, alpha,
                   color, blend_type, &paint);

    bool use_interpolate_bilinear = options.bInterpolateBilinear;
    if (!use_interpolate_bilinear) {
      float dest_width = ceilf(matrix.GetXUnit());
      float dest_height = ceilf(matrix.GetYUnit());
      if (pdfium::IsValueInRangeForNumericType<int>(dest_width) &&
          pdfium::IsValueInRangeForNumericType<int>(dest_height)) {
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

    canvas_->drawImageRect(skia_source,
                           SkRect::MakeLTRB(src_rect.left, src_rect.top,
                                            src_rect.right, src_rect.bottom),
                           SkRect::MakeWH(src_rect.Width(), src_rect.Height()),
                           sampling_options, &paint,
                           SkCanvas::kFast_SrcRectConstraint);
  }

  DebugValidate(bitmap_);
  return true;
}

void CFX_SkiaDeviceDriver::DrawPathImpl(const SkPath& path,
                                        const SkPaint& paint) {
  DebugShowSkiaDrawPath(this, canvas_, paint, path);
  canvas_->drawPath(path, paint);
}

CFX_SkiaDeviceDriver::CharDetail::CharDetail() = default;
CFX_SkiaDeviceDriver::CharDetail::~CharDetail() = default;

bool CFX_DefaultRenderDevice::AttachSkiaImpl(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout) {
  // FPDF_FFLDrawSkia() ends up calling this method with a deliberately null
  // `pBitmap`.
  if (!pBitmap) {
    return false;
  }
  SetBitmap(pBitmap);
  auto driver =
      CFX_SkiaDeviceDriver::Create(std::move(pBitmap), bRgbByteOrder,
                                   std::move(pBackdropBitmap), bGroupKnockout);
  if (!driver) {
    return false;
  }

  SetDeviceDriver(std::move(driver));
  return true;
}

bool CFX_DefaultRenderDevice::AttachCanvas(SkCanvas& canvas) {
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
  if (!pBitmap->Create(width, height, format)) {
    return false;
  }

  SetBitmap(pBitmap);
  auto driver = CFX_SkiaDeviceDriver::Create(std::move(pBitmap), false,
                                             std::move(pBackdropBitmap), false);
  if (!driver) {
    return false;
  }

  SetDeviceDriver(std::move(driver));
  return true;
}
