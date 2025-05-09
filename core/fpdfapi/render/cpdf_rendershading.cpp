// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_rendershading.h"

#include <math.h>

#include <algorithm>
#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/page/cpdf_meshstream.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfapi/render/cpdf_devicebuffer.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/numerics/clamped_math.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"

namespace {

constexpr int kShadingSteps = 256;

uint32_t CountOutputsFromFunctions(
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs) {
  FX_SAFE_UINT32 total = 0;
  for (const auto& func : funcs) {
    if (func) {
      total += func->OutputCount();
    }
  }
  return total.ValueOrDefault(0);
}

uint32_t GetValidatedOutputsCount(
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS) {
  uint32_t funcs_outputs = CountOutputsFromFunctions(funcs);
  return funcs_outputs ? std::max(funcs_outputs, pCS->ComponentCount()) : 0;
}

std::array<FX_ARGB, kShadingSteps> GetShadingSteps(
    float t_min,
    float t_max,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS,
    int alpha,
    size_t results_count) {
  CHECK_GE(results_count, CountOutputsFromFunctions(funcs));
  CHECK_GE(results_count, pCS->ComponentCount());
  std::array<FX_ARGB, kShadingSteps> shading_steps;
  std::vector<float> result_array(results_count);
  float diff = t_max - t_min;
  for (int i = 0; i < kShadingSteps; ++i) {
    float input = diff * i / kShadingSteps + t_min;
    pdfium::span<float> result_span = pdfium::span(result_array);
    for (const auto& func : funcs) {
      if (!func) {
        continue;
      }
      std::optional<uint32_t> nresults =
          func->Call(pdfium::span_from_ref(input), result_span);
      if (nresults.has_value()) {
        result_span = result_span.subspan(nresults.value());
      }
    }
    auto rgb = pCS->GetRGBOrZerosOnError(result_array);
    shading_steps[i] =
        ArgbEncode(alpha, FXSYS_roundf(rgb.red * 255),
                   FXSYS_roundf(rgb.green * 255), FXSYS_roundf(rgb.blue * 255));
  }
  return shading_steps;
}

void DrawAxialShading(const RetainPtr<CFX_DIBitmap>& pBitmap,
                      const CFX_Matrix& mtObject2Bitmap,
                      const CPDF_Dictionary* pDict,
                      const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                      const RetainPtr<CPDF_ColorSpace>& pCS,
                      int alpha) {
  DCHECK_EQ(pBitmap->GetFormat(), FXDIB_Format::kBgra);

  const uint32_t total_results = GetValidatedOutputsCount(funcs, pCS);
  if (total_results == 0) {
    return;
  }

  RetainPtr<const CPDF_Array> pCoords = pDict->GetArrayFor("Coords");
  if (!pCoords) {
    return;
  }

  float start_x = pCoords->GetFloatAt(0);
  float start_y = pCoords->GetFloatAt(1);
  float end_x = pCoords->GetFloatAt(2);
  float end_y = pCoords->GetFloatAt(3);
  float t_min = 0;
  float t_max = 1.0f;
  RetainPtr<const CPDF_Array> pArray = pDict->GetArrayFor("Domain");
  if (pArray) {
    t_min = pArray->GetFloatAt(0);
    t_max = pArray->GetFloatAt(1);
  }
  pArray = pDict->GetArrayFor("Extend");
  const bool bStartExtend = pArray && pArray->GetBooleanAt(0, false);
  const bool bEndExtend = pArray && pArray->GetBooleanAt(1, false);

  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  float x_span = end_x - start_x;
  float y_span = end_y - start_y;
  float axis_len_square = (x_span * x_span) + (y_span * y_span);

  std::array<FX_ARGB, kShadingSteps> shading_steps =
      GetShadingSteps(t_min, t_max, funcs, pCS, alpha, total_results);

  CFX_Matrix matrix = mtObject2Bitmap.GetInverse();
  for (int row = 0; row < height; row++) {
    auto dest_buf = pBitmap->GetWritableScanlineAs<uint32_t>(row).first(
        static_cast<size_t>(width));
    size_t column_counter = 0;
    for (auto& pix : dest_buf) {
      const float column = static_cast<float>(column_counter++);
      const CFX_PointF pos =
          matrix.Transform(CFX_PointF(column, static_cast<float>(row)));
      float scale =
          (((pos.x - start_x) * x_span) + ((pos.y - start_y) * y_span)) /
          axis_len_square;
      int index = static_cast<int32_t>(scale * (kShadingSteps - 1));
      if (index < 0) {
        if (!bStartExtend) {
          continue;
        }
        index = 0;
      } else if (index >= kShadingSteps) {
        if (!bEndExtend) {
          continue;
        }
        index = kShadingSteps - 1;
      }
      pix = shading_steps[index];
    }
  }
}

void DrawRadialShading(const RetainPtr<CFX_DIBitmap>& pBitmap,
                       const CFX_Matrix& mtObject2Bitmap,
                       const CPDF_Dictionary* pDict,
                       const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                       const RetainPtr<CPDF_ColorSpace>& pCS,
                       int alpha) {
  DCHECK_EQ(pBitmap->GetFormat(), FXDIB_Format::kBgra);

  const uint32_t total_results = GetValidatedOutputsCount(funcs, pCS);
  if (total_results == 0) {
    return;
  }

  RetainPtr<const CPDF_Array> pCoords = pDict->GetArrayFor("Coords");
  if (!pCoords) {
    return;
  }

  float start_x = pCoords->GetFloatAt(0);
  float start_y = pCoords->GetFloatAt(1);
  float start_r = pCoords->GetFloatAt(2);
  float end_x = pCoords->GetFloatAt(3);
  float end_y = pCoords->GetFloatAt(4);
  float end_r = pCoords->GetFloatAt(5);
  float t_min = 0;
  float t_max = 1.0f;
  RetainPtr<const CPDF_Array> pArray = pDict->GetArrayFor("Domain");
  if (pArray) {
    t_min = pArray->GetFloatAt(0);
    t_max = pArray->GetFloatAt(1);
  }
  pArray = pDict->GetArrayFor("Extend");
  const bool bStartExtend = pArray && pArray->GetBooleanAt(0, false);
  const bool bEndExtend = pArray && pArray->GetBooleanAt(1, false);

  std::array<FX_ARGB, kShadingSteps> shading_steps =
      GetShadingSteps(t_min, t_max, funcs, pCS, alpha, total_results);

  const float dx = end_x - start_x;
  const float dy = end_y - start_y;
  const float dr = end_r - start_r;
  const float a = dx * dx + dy * dy - dr * dr;
  const bool a_is_float_zero = FXSYS_IsFloatZero(a);

  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  bool bDecreasing = dr < 0 && static_cast<int>(hypotf(dx, dy)) < -dr;

  CFX_Matrix matrix = mtObject2Bitmap.GetInverse();
  for (int row = 0; row < height; row++) {
    auto dest_buf = pBitmap->GetWritableScanlineAs<uint32_t>(row).first(
        static_cast<size_t>(width));
    size_t column_counter = 0;
    for (auto& pix : dest_buf) {
      const float column = static_cast<float>(column_counter++);
      const CFX_PointF pos =
          matrix.Transform(CFX_PointF(column, static_cast<float>(row)));
      float pos_dx = pos.x - start_x;
      float pos_dy = pos.y - start_y;
      float b = -2 * (pos_dx * dx + pos_dy * dy + start_r * dr);
      float c = pos_dx * pos_dx + pos_dy * pos_dy - start_r * start_r;
      float s;
      if (FXSYS_IsFloatZero(b)) {
        s = sqrt(-c / a);
      } else if (a_is_float_zero) {
        s = -c / b;
      } else {
        float b2_4ac = (b * b) - 4 * (a * c);
        if (b2_4ac < 0) {
          continue;
        }
        float root = sqrt(b2_4ac);
        float s1 = (-b - root) / (2 * a);
        float s2 = (-b + root) / (2 * a);
        if (a <= 0) {
          std::swap(s1, s2);
        }
        if (bDecreasing) {
          s = (s1 >= 0 || bStartExtend) ? s1 : s2;
        } else {
          s = (s2 <= 1.0f || bEndExtend) ? s2 : s1;
        }
        if (start_r + s * dr < 0) {
          continue;
        }
      }
      int index = static_cast<int32_t>(s * (kShadingSteps - 1));
      if (index < 0) {
        if (!bStartExtend) {
          continue;
        }
        index = 0;
      } else if (index >= kShadingSteps) {
        if (!bEndExtend) {
          continue;
        }
        index = kShadingSteps - 1;
      }
      pix = shading_steps[index];
    }
  }
}

void DrawFuncShading(const RetainPtr<CFX_DIBitmap>& pBitmap,
                     const CFX_Matrix& mtObject2Bitmap,
                     const CPDF_Dictionary* pDict,
                     const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                     const RetainPtr<CPDF_ColorSpace>& pCS,
                     int alpha) {
  DCHECK_EQ(pBitmap->GetFormat(), FXDIB_Format::kBgra);

  const uint32_t total_results = GetValidatedOutputsCount(funcs, pCS);
  if (total_results == 0) {
    return;
  }

  RetainPtr<const CPDF_Array> pDomain = pDict->GetArrayFor("Domain");
  float xmin = 0.0f;
  float ymin = 0.0f;
  float xmax = 1.0f;
  float ymax = 1.0f;
  if (pDomain) {
    xmin = pDomain->GetFloatAt(0);
    xmax = pDomain->GetFloatAt(1);
    ymin = pDomain->GetFloatAt(2);
    ymax = pDomain->GetFloatAt(3);
  }
  CFX_Matrix mtDomain2Target = pDict->GetMatrixFor("Matrix");
  CFX_Matrix matrix =
      mtObject2Bitmap.GetInverse() * mtDomain2Target.GetInverse();
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();

  CHECK_GE(total_results, CountOutputsFromFunctions(funcs));
  CHECK_GE(total_results, pCS->ComponentCount());
  std::vector<float> result_array(total_results);
  for (int row = 0; row < height; ++row) {
    auto dib_buf = pBitmap->GetWritableScanlineAs<uint32_t>(row);
    for (int column = 0; column < width; column++) {
      CFX_PointF pos = matrix.Transform(
          CFX_PointF(static_cast<float>(column), static_cast<float>(row)));
      if (pos.x < xmin || pos.x > xmax || pos.y < ymin || pos.y > ymax) {
        continue;
      }

      float input[2] = {pos.x, pos.y};
      pdfium::span<float> result_span = pdfium::span(result_array);
      for (const auto& func : funcs) {
        if (!func) {
          continue;
        }
        std::optional<uint32_t> nresults = func->Call(input, result_span);
        if (nresults.has_value()) {
          result_span = result_span.subspan(nresults.value());
        }
      }
      auto rgb = pCS->GetRGBOrZerosOnError(result_array);
      dib_buf[column] = ArgbEncode(alpha, static_cast<int32_t>(rgb.red * 255),
                                   static_cast<int32_t>(rgb.green * 255),
                                   static_cast<int32_t>(rgb.blue * 255));
    }
  }
}

bool GetScanlineIntersect(int y,
                          const CFX_PointF& first,
                          const CFX_PointF& second,
                          float* x) {
  if (first.y == second.y) {
    return false;
  }

  if (first.y < second.y) {
    if (y < first.y || y > second.y) {
      return false;
    }
  } else if (y < second.y || y > first.y) {
    return false;
  }
  *x = first.x + ((second.x - first.x) * (y - first.y) / (second.y - first.y));
  return true;
}

void DrawGouraud(const RetainPtr<CFX_DIBitmap>& pBitmap,
                 int alpha,
                 pdfium::span<CPDF_MeshVertex, 3> triangle) {
  float min_y = triangle[0].position.y;
  float max_y = triangle[0].position.y;
  for (int i = 1; i < 3; i++) {
    min_y = std::min(min_y, triangle[i].position.y);
    max_y = std::max(max_y, triangle[i].position.y);
  }
  if (min_y == max_y) {
    return;
  }

  int min_yi = std::max(static_cast<int>(floorf(min_y)), 0);
  int max_yi = static_cast<int>(ceilf(max_y));
  if (max_yi >= pBitmap->GetHeight()) {
    max_yi = pBitmap->GetHeight() - 1;
  }

  for (int y = min_yi; y <= max_yi; y++) {
    int nIntersects = 0;
    std::array<float, 3> inter_x;
    std::array<float, 3> r;
    std::array<float, 3> g;
    std::array<float, 3> b;
    for (int i = 0; i < 3; i++) {
      const CPDF_MeshVertex& vertex1 = triangle[i];
      const CPDF_MeshVertex& vertex2 = triangle[(i + 1) % 3];
      const CFX_PointF& position1 = vertex1.position;
      const CFX_PointF& position2 = vertex2.position;
      bool bIntersect =
          GetScanlineIntersect(y, position1, position2, &inter_x[nIntersects]);
      if (!bIntersect) {
        continue;
      }

      float y_dist = (y - position1.y) / (position2.y - position1.y);
      r[nIntersects] =
          vertex1.rgb.red + ((vertex2.rgb.red - vertex1.rgb.red) * y_dist);
      g[nIntersects] = vertex1.rgb.green +
                       ((vertex2.rgb.green - vertex1.rgb.green) * y_dist);
      b[nIntersects] =
          vertex1.rgb.blue + ((vertex2.rgb.blue - vertex1.rgb.blue) * y_dist);
      nIntersects++;
    }
    if (nIntersects != 2) {
      continue;
    }

    int min_x;
    int max_x;
    int start_index;
    int end_index;
    if (inter_x[0] < inter_x[1]) {
      min_x = static_cast<int>(floorf(inter_x[0]));
      max_x = static_cast<int>(ceilf(inter_x[1]));
      start_index = 0;
      end_index = 1;
    } else {
      min_x = static_cast<int>(floorf(inter_x[1]));
      max_x = static_cast<int>(ceilf(inter_x[0]));
      start_index = 1;
      end_index = 0;
    }

    int start_x = std::clamp(min_x, 0, pBitmap->GetWidth());
    int end_x = std::clamp(max_x, 0, pBitmap->GetWidth());
    const int range_x = pdfium::ClampSub(max_x, min_x);
    float r_unit = (r[end_index] - r[start_index]) / range_x;
    float g_unit = (g[end_index] - g[start_index]) / range_x;
    float b_unit = (b[end_index] - b[start_index]) / range_x;
    const int diff_x = pdfium::ClampSub(start_x, min_x);
    float r_result = r[start_index] + diff_x * r_unit;
    float g_result = g[start_index] + diff_x * g_unit;
    float b_result = b[start_index] + diff_x * b_unit;
    pdfium::span<uint8_t> dib_span = pBitmap->GetWritableScanline(y).subspan(
        static_cast<size_t>(start_x * 4));

    for (int x = start_x; x < end_x; x++) {
      r_result += r_unit;
      g_result += g_unit;
      b_result += b_unit;
      UNSAFE_TODO(FXARGB_SetDIB(
          dib_span.data(), ArgbEncode(alpha, static_cast<int>(r_result * 255),
                                      static_cast<int>(g_result * 255),
                                      static_cast<int>(b_result * 255))));
      dib_span = dib_span.subspan<4u>();
    }
  }
}

void DrawFreeGouraudShading(
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const CFX_Matrix& mtObject2Bitmap,
    RetainPtr<const CPDF_Stream> pShadingStream,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    RetainPtr<CPDF_ColorSpace> pCS,
    int alpha) {
  DCHECK_EQ(pBitmap->GetFormat(), FXDIB_Format::kBgra);

  CPDF_MeshStream stream(kFreeFormGouraudTriangleMeshShading, funcs,
                         std::move(pShadingStream), std::move(pCS));
  if (!stream.Load()) {
    return;
  }

  std::array<CPDF_MeshVertex, 3> triangle;
  while (!stream.IsEOF()) {
    CPDF_MeshVertex vertex;
    uint32_t flag;
    if (!stream.ReadVertex(mtObject2Bitmap, &vertex, &flag)) {
      return;
    }

    if (flag == 0) {
      triangle[0] = vertex;
      for (int i = 1; i < 3; ++i) {
        uint32_t dummy_flag;
        if (!stream.ReadVertex(mtObject2Bitmap, &triangle[i], &dummy_flag)) {
          return;
        }
      }
    } else {
      if (flag == 1) {
        triangle[0] = triangle[1];
      }

      triangle[1] = triangle[2];
      triangle[2] = vertex;
    }
    DrawGouraud(pBitmap, alpha, triangle);
  }
}

void DrawLatticeGouraudShading(
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const CFX_Matrix& mtObject2Bitmap,
    RetainPtr<const CPDF_Stream> pShadingStream,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    RetainPtr<CPDF_ColorSpace> pCS,
    int alpha) {
  DCHECK_EQ(pBitmap->GetFormat(), FXDIB_Format::kBgra);

  int row_verts = pShadingStream->GetDict()->GetIntegerFor("VerticesPerRow");
  if (row_verts < 2) {
    return;
  }

  CPDF_MeshStream stream(kLatticeFormGouraudTriangleMeshShading, funcs,
                         std::move(pShadingStream), std::move(pCS));
  if (!stream.Load()) {
    return;
  }

  std::array<std::vector<CPDF_MeshVertex>, 2> vertices;
  vertices[0] = stream.ReadVertexRow(mtObject2Bitmap, row_verts);
  if (vertices[0].empty()) {
    return;
  }

  int last_index = 0;
  while (true) {
    vertices[1 - last_index] = stream.ReadVertexRow(mtObject2Bitmap, row_verts);
    if (vertices[1 - last_index].empty()) {
      return;
    }

    CPDF_MeshVertex triangle[3];
    for (int i = 1; i < row_verts; ++i) {
      triangle[0] = vertices[last_index][i];
      triangle[1] = vertices[1 - last_index][i - 1];
      triangle[2] = vertices[last_index][i - 1];
      DrawGouraud(pBitmap, alpha, triangle);
      triangle[2] = vertices[1 - last_index][i];
      DrawGouraud(pBitmap, alpha, triangle);
    }
    last_index = 1 - last_index;
  }
}

struct CubicBezierPatch {
  bool IsSmall() const {
    CFX_FloatRect bbox = CFX_FloatRect::GetBBox(
        fxcrt::reinterpret_span<const CFX_PointF>(pdfium::span(points)));
    return bbox.Width() < 2 && bbox.Height() < 2;
  }

  void GetBoundary(pdfium::span<CFX_Path::Point> boundary) {
    // Returns a cubic bezier path consisting of the outer control points.
    // Note that patch boundary does not always contain all patch points,
    // but for "small" patches it's reasonably close.
    // TODO(thakis): The outer control points don't always contain all
    // points in the patch, e.g. for a single-color tensor product patch
    // where the "inner" control points (points[1][1], points[2][1],
    // points[1][2], points[2][2]) are far outside the "outer" ones.
    // Make a bezier patch stress test and fix this by continuing to
    // subdivide if the inner points are outside.
    boundary[0].point_ = points[0][0];
    boundary[1].point_ = points[0][1];
    boundary[2].point_ = points[0][2];
    boundary[3].point_ = points[0][3];
    boundary[4].point_ = points[1][3];
    boundary[5].point_ = points[2][3];
    boundary[6].point_ = points[3][3];
    boundary[7].point_ = points[3][2];
    boundary[8].point_ = points[3][1];
    boundary[9].point_ = points[3][0];
    boundary[10].point_ = points[2][0];
    boundary[11].point_ = points[1][0];
    boundary[12].point_ = points[0][0];
  }

  void SubdivideVertical(CubicBezierPatch& top, CubicBezierPatch& bottom) {
    for (int x = 0; x < 4; ++x) {
      std::array<CFX_PointF, 3> level1 = {
          0.5f * (points[x][0] + points[x][1]),
          0.5f * (points[x][1] + points[x][2]),
          0.5f * (points[x][2] + points[x][3]),
      };
      std::array<CFX_PointF, 2> level2 = {
          0.5f * (level1[0] + level1[1]),
          0.5f * (level1[1] + level1[2]),
      };
      CFX_PointF level3 = 0.5f * (level2[0] + level2[1]);

      top.points[x][0] = points[x][0];
      top.points[x][1] = level1[0];
      top.points[x][2] = level2[0];
      top.points[x][3] = level3;

      bottom.points[x][0] = level3;
      bottom.points[x][1] = level2[1];
      bottom.points[x][2] = level1[2];
      bottom.points[x][3] = points[x][3];
    }
  }

  void SubdivideHorizontal(CubicBezierPatch& left, CubicBezierPatch& right) {
    for (int y = 0; y < 4; ++y) {
      std::array<CFX_PointF, 3> level1 = {
          0.5f * (points[0][y] + points[1][y]),
          0.5f * (points[1][y] + points[2][y]),
          0.5f * (points[2][y] + points[3][y]),
      };
      std::array<CFX_PointF, 2> level2 = {
          0.5f * (level1[0] + level1[1]),
          (1.0f / 2.0f) * (level1[1] + level1[2]),
      };
      CFX_PointF level3 = 0.5f * (level2[0] + level2[1]);

      left.points[0][y] = points[0][y];
      left.points[1][y] = level1[0];
      left.points[2][y] = level2[0];
      left.points[3][y] = level3;

      right.points[0][y] = level3;
      right.points[1][y] = level2[1];
      right.points[2][y] = level1[2];
      right.points[3][y] = points[3][y];
    }
  }

  void Subdivide(CubicBezierPatch& top_left,
                 CubicBezierPatch& bottom_left,
                 CubicBezierPatch& top_right,
                 CubicBezierPatch& bottom_right) {
    CubicBezierPatch top;
    CubicBezierPatch bottom;
    SubdivideVertical(top, bottom);
    top.SubdivideHorizontal(top_left, top_right);
    bottom.SubdivideHorizontal(bottom_left, bottom_right);
  }

  std::array<std::array<CFX_PointF, 4>, 4> points;
};

int Interpolate(int p1, int p2, int delta1, int delta2, bool* overflow) {
  FX_SAFE_INT32 p = p2;
  p -= p1;
  p *= delta1;
  p /= delta2;
  p += p1;
  if (!p.IsValid()) {
    *overflow = true;
  }
  return p.ValueOrDefault(0);
}

int BiInterpolImpl(int c0,
                   int c1,
                   int c2,
                   int c3,
                   int x,
                   int y,
                   int x_scale,
                   int y_scale,
                   bool* overflow) {
  int x1 = Interpolate(c0, c3, x, x_scale, overflow);
  int x2 = Interpolate(c1, c2, x, x_scale, overflow);
  return Interpolate(x1, x2, y, y_scale, overflow);
}

struct CoonColor {
  CoonColor() = default;

  // Returns true if successful, false if overflow detected.
  bool BiInterpol(pdfium::span<CoonColor, 4> colors,
                  int x,
                  int y,
                  int x_scale,
                  int y_scale) {
    bool overflow = false;
    for (int i = 0; i < 3; i++) {
      comp[i] = BiInterpolImpl(colors[0].comp[i], colors[1].comp[i],
                               colors[2].comp[i], colors[3].comp[i], x, y,
                               x_scale, y_scale, &overflow);
    }
    return !overflow;
  }

  int Distance(const CoonColor& o) const {
    return std::max({abs(comp[0] - o.comp[0]), abs(comp[1] - o.comp[1]),
                     abs(comp[2] - o.comp[2])});
  }

  std::array<int, 3> comp = {};
};

struct PatchDrawer {
  static constexpr int kCoonColorThreshold = 4;

  void Draw(int x_scale,
            int y_scale,
            int left,
            int bottom,
            CubicBezierPatch patch) {
    bool bSmall = patch.IsSmall();

    CoonColor div_colors[4];
    int d_bottom = 0;
    int d_left = 0;
    int d_top = 0;
    int d_right = 0;
    if (!div_colors[0].BiInterpol(patch_colors, left, bottom, x_scale,
                                  y_scale)) {
      return;
    }
    if (!bSmall) {
      if (!div_colors[1].BiInterpol(patch_colors, left, bottom + 1, x_scale,
                                    y_scale)) {
        return;
      }
      if (!div_colors[2].BiInterpol(patch_colors, left + 1, bottom + 1, x_scale,
                                    y_scale)) {
        return;
      }
      if (!div_colors[3].BiInterpol(patch_colors, left + 1, bottom, x_scale,
                                    y_scale)) {
        return;
      }
      d_bottom = div_colors[3].Distance(div_colors[0]);
      d_left = div_colors[1].Distance(div_colors[0]);
      d_top = div_colors[1].Distance(div_colors[2]);
      d_right = div_colors[2].Distance(div_colors[3]);
    }

    if (bSmall ||
        (d_bottom < kCoonColorThreshold && d_left < kCoonColorThreshold &&
         d_top < kCoonColorThreshold && d_right < kCoonColorThreshold)) {
      pdfium::span<CFX_Path::Point> points = path.GetPoints();
      patch.GetBoundary(points);
      CFX_FillRenderOptions fill_options(
          CFX_FillRenderOptions::WindingOptions());
      fill_options.full_cover = true;
      if (bNoPathSmooth) {
        fill_options.aliased_path = true;
      }
      pDevice->DrawPath(
          path, nullptr, nullptr,
          ArgbEncode(alpha, div_colors[0].comp[0], div_colors[0].comp[1],
                     div_colors[0].comp[2]),
          0, fill_options);
    } else {
      if (d_bottom < kCoonColorThreshold && d_top < kCoonColorThreshold) {
        CubicBezierPatch top_patch;
        CubicBezierPatch bottom_patch;
        patch.SubdivideVertical(top_patch, bottom_patch);
        y_scale *= 2;
        bottom *= 2;
        Draw(x_scale, y_scale, left, bottom, top_patch);
        Draw(x_scale, y_scale, left, bottom + 1, bottom_patch);
      } else if (d_left < kCoonColorThreshold &&
                 d_right < kCoonColorThreshold) {
        CubicBezierPatch left_patch;
        CubicBezierPatch right_patch;
        patch.SubdivideHorizontal(left_patch, right_patch);
        x_scale *= 2;
        left *= 2;
        Draw(x_scale, y_scale, left, bottom, left_patch);
        Draw(x_scale, y_scale, left + 1, bottom, right_patch);
      } else {
        CubicBezierPatch top_left;
        CubicBezierPatch bottom_left;
        CubicBezierPatch top_right;
        CubicBezierPatch bottom_right;
        patch.Subdivide(top_left, bottom_left, top_right, bottom_right);
        x_scale *= 2;
        y_scale *= 2;
        left *= 2;
        bottom *= 2;
        Draw(x_scale, y_scale, left, bottom, top_left);
        Draw(x_scale, y_scale, left, bottom + 1, bottom_left);
        Draw(x_scale, y_scale, left + 1, bottom, top_right);
        Draw(x_scale, y_scale, left + 1, bottom + 1, bottom_right);
      }
    }
  }

  int max_delta;
  CFX_Path path;
  UnownedPtr<CFX_RenderDevice> pDevice;
  bool bNoPathSmooth;
  int alpha;
  std::array<CoonColor, 4> patch_colors;
};

void DrawCoonPatchMeshes(
    ShadingType type,
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const CFX_Matrix& mtObject2Bitmap,
    RetainPtr<const CPDF_Stream> pShadingStream,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    RetainPtr<CPDF_ColorSpace> pCS,
    bool bNoPathSmooth,
    int alpha) {
  DCHECK_EQ(pBitmap->GetFormat(), FXDIB_Format::kBgra);
  DCHECK(type == kCoonsPatchMeshShading ||
         type == kTensorProductPatchMeshShading);

  CFX_DefaultRenderDevice device;
  device.Attach(pBitmap);

  CPDF_MeshStream stream(type, funcs, std::move(pShadingStream),
                         std::move(pCS));
  if (!stream.Load()) {
    return;
  }

  PatchDrawer patch_drawer;
  patch_drawer.alpha = alpha;
  patch_drawer.pDevice = &device;
  patch_drawer.bNoPathSmooth = bNoPathSmooth;

  for (int i = 0; i < 13; i++) {
    patch_drawer.path.AppendPoint(
        CFX_PointF(),
        i == 0 ? CFX_Path::Point::Type::kMove : CFX_Path::Point::Type::kBezier);
  }

  std::array<CFX_PointF, 16> coords;
  int point_count = type == kTensorProductPatchMeshShading ? 16 : 12;
  while (!stream.IsEOF()) {
    if (!stream.CanReadFlag()) {
      break;
    }
    uint32_t flag = stream.ReadFlag();
    int iStartPoint = 0;
    int iStartColor = 0;
    int i = 0;
    if (flag) {
      iStartPoint = 4;
      iStartColor = 2;
      std::array<CFX_PointF, 4> tempCoords;
      for (i = 0; i < 4; i++) {
        tempCoords[i] = coords[(flag * 3 + i) % 12];
      }
      fxcrt::Copy(tempCoords, coords);
      std::array<CoonColor, 2> tempColors = {{
          patch_drawer.patch_colors[flag],
          patch_drawer.patch_colors[(flag + 1) % 4],
      }};
      fxcrt::Copy(tempColors, patch_drawer.patch_colors);
    }
    for (i = iStartPoint; i < point_count; i++) {
      if (!stream.CanReadCoords()) {
        break;
      }
      coords[i] = mtObject2Bitmap.Transform(stream.ReadCoords());
    }

    for (i = iStartColor; i < 4; i++) {
      if (!stream.CanReadColor()) {
        break;
      }

      FX_RGB_STRUCT<float> rgb = stream.ReadColor();
      patch_drawer.patch_colors[i].comp[0] =
          static_cast<int32_t>(rgb.red * 255);
      patch_drawer.patch_colors[i].comp[1] =
          static_cast<int32_t>(rgb.green * 255);
      patch_drawer.patch_colors[i].comp[2] =
          static_cast<int32_t>(rgb.blue * 255);
    }

    CFX_FloatRect bbox = CFX_FloatRect::GetBBox(
        pdfium::span(coords).first(static_cast<size_t>(point_count)));
    if (bbox.right <= 0 || bbox.left >= (float)pBitmap->GetWidth() ||
        bbox.top <= 0 || bbox.bottom >= (float)pBitmap->GetHeight()) {
      continue;
    }

    CubicBezierPatch patch;
    patch.points[0][0] = coords[0];
    patch.points[0][1] = coords[1];
    patch.points[0][2] = coords[2];
    patch.points[0][3] = coords[3];
    patch.points[1][3] = coords[4];
    patch.points[2][3] = coords[5];
    patch.points[3][3] = coords[6];
    patch.points[3][2] = coords[7];
    patch.points[3][1] = coords[8];
    patch.points[3][0] = coords[9];
    patch.points[2][0] = coords[10];
    patch.points[1][0] = coords[11];
    if (type == kTensorProductPatchMeshShading) {
      patch.points[1][1] = coords[12];
      patch.points[1][2] = coords[13];
      patch.points[2][2] = coords[14];
      patch.points[2][1] = coords[15];
    } else {
      CHECK_EQ(type, kCoonsPatchMeshShading);
      // These equations are from ISO 32000-2:2020, page 267, in
      // 8.7.4.5.8 Type 7 (tensor-product patch mesh) shadings:
      patch.points[1][1] =
          (1.0f / 9.0f) * (-4.0f * patch.points[0][0] +
                           6.0f * (patch.points[0][1] + patch.points[1][0]) -
                           2.0f * (patch.points[0][3] + patch.points[3][0]) +
                           3.0f * (patch.points[3][1] + patch.points[1][3]) -
                           1.0f * patch.points[3][3]);
      patch.points[1][2] =
          (1.0f / 9.0f) * (-4.0f * patch.points[0][3] +
                           6.0f * (patch.points[0][2] + patch.points[1][3]) -
                           2.0f * (patch.points[0][0] + patch.points[3][3]) +
                           3.0f * (patch.points[3][2] + patch.points[1][0]) -
                           1.0f * patch.points[3][0]);
      patch.points[2][1] =
          (1.0f / 9.0f) * (-4.0f * patch.points[3][0] +
                           6.0f * (patch.points[3][1] + patch.points[2][0]) -
                           2.0f * (patch.points[3][3] + patch.points[0][0]) +
                           3.0f * (patch.points[0][1] + patch.points[2][3]) -
                           1.0f * patch.points[0][3]);
      patch.points[2][2] =
          (1.0f / 9.0f) * (-4.0f * patch.points[3][3] +
                           6.0f * (patch.points[3][2] + patch.points[2][3]) -
                           2.0f * (patch.points[3][0] + patch.points[0][3]) +
                           3.0f * (patch.points[0][2] + patch.points[2][0]) -
                           1.0f * patch.points[0][0]);
    }

    patch_drawer.Draw(1, 1, 0, 0, patch);
  }
}

}  // namespace

// static
void CPDF_RenderShading::Draw(CFX_RenderDevice* pDevice,
                              CPDF_RenderContext* pContext,
                              const CPDF_PageObject* pCurObj,
                              const CPDF_ShadingPattern* pPattern,
                              const CFX_Matrix& mtMatrix,
                              const FX_RECT& clip_rect,
                              int alpha,
                              const CPDF_RenderOptions& options) {
  RetainPtr<CPDF_ColorSpace> pColorSpace = pPattern->GetCS();
  if (!pColorSpace) {
    return;
  }

  FX_ARGB background = 0;
  RetainPtr<const CPDF_Dictionary> pDict =
      pPattern->GetShadingObject()->GetDict();
  if (!pPattern->IsShadingObject() && pDict->KeyExist("Background")) {
    RetainPtr<const CPDF_Array> pBackColor = pDict->GetArrayFor("Background");
    if (pBackColor && pBackColor->size() >= pColorSpace->ComponentCount()) {
      std::vector<float> comps = ReadArrayElementsToVector(
          pBackColor.Get(), pColorSpace->ComponentCount());

      auto rgb = pColorSpace->GetRGBOrZerosOnError(comps);
      background = ArgbEncode(255, static_cast<int32_t>(rgb.red * 255),
                              static_cast<int32_t>(rgb.green * 255),
                              static_cast<int32_t>(rgb.blue * 255));
    }
  }
  FX_RECT clip_rect_bbox = clip_rect;
  if (pDict->KeyExist("BBox")) {
    clip_rect_bbox.Intersect(
        mtMatrix.TransformRect(pDict->GetRectFor("BBox")).GetOuterRect());
  }
#if defined(PDF_USE_SKIA)
  if ((pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SHADING) &&
      pDevice->DrawShading(*pPattern, mtMatrix, clip_rect_bbox, alpha)) {
    return;
  }
#endif  // defined(PDF_USE_SKIA)
  CPDF_DeviceBuffer buffer(pContext, pDevice, clip_rect_bbox, pCurObj, 150);
  RetainPtr<CFX_DIBitmap> pBitmap = buffer.Initialize();
  if (!pBitmap) {
    return;
  }

  if (background != 0) {
    pBitmap->Clear(background);
  }
  const CFX_Matrix final_matrix = mtMatrix * buffer.GetMatrix();
  const auto& funcs = pPattern->GetFuncs();
  switch (pPattern->GetShadingType()) {
    case kInvalidShading:
    case kMaxShading:
      return;
    case kFunctionBasedShading:
      DrawFuncShading(pBitmap, final_matrix, pDict.Get(), funcs, pColorSpace,
                      alpha);
      break;
    case kAxialShading:
      DrawAxialShading(pBitmap, final_matrix, pDict.Get(), funcs, pColorSpace,
                       alpha);
      break;
    case kRadialShading:
      DrawRadialShading(pBitmap, final_matrix, pDict.Get(), funcs, pColorSpace,
                        alpha);
      break;
    case kFreeFormGouraudTriangleMeshShading: {
      // The shading object can be a stream or a dictionary. We do not handle
      // the case of dictionary at the moment.
      RetainPtr<const CPDF_Stream> pStream =
          ToStream(pPattern->GetShadingObject());
      if (pStream) {
        DrawFreeGouraudShading(pBitmap, final_matrix, std::move(pStream), funcs,
                               pColorSpace, alpha);
      }
      break;
    }
    case kLatticeFormGouraudTriangleMeshShading: {
      // The shading object can be a stream or a dictionary. We do not handle
      // the case of dictionary at the moment.
      RetainPtr<const CPDF_Stream> pStream =
          ToStream(pPattern->GetShadingObject());
      if (pStream) {
        DrawLatticeGouraudShading(pBitmap, final_matrix, std::move(pStream),
                                  funcs, pColorSpace, alpha);
      }
      break;
    }
    case kCoonsPatchMeshShading:
    case kTensorProductPatchMeshShading: {
      // The shading object can be a stream or a dictionary. We do not handle
      // the case of dictionary at the moment.
      RetainPtr<const CPDF_Stream> pStream =
          ToStream(pPattern->GetShadingObject());
      if (pStream) {
        DrawCoonPatchMeshes(pPattern->GetShadingType(), pBitmap, final_matrix,
                            std::move(pStream), funcs, pColorSpace,
                            options.GetOptions().bNoPathSmooth, alpha);
      }
      break;
    }
  }

  if (options.ColorModeIs(CPDF_RenderOptions::kAlpha)) {
    pBitmap->SetRedFromAlpha();
  } else if (options.ColorModeIs(CPDF_RenderOptions::kGray)) {
    pBitmap->ConvertColorScale(0, 0xffffff);
  }

  buffer.OutputToDevice();
}
