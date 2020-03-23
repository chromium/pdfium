// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_rendershading.h"

#include <algorithm>
#include <array>
#include <cmath>
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
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/span.h"

namespace {

constexpr int kShadingSteps = 256;

uint32_t CountOutputsFromFunctions(
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs) {
  FX_SAFE_UINT32 total = 0;
  for (const auto& func : funcs) {
    if (func)
      total += func->CountOutputs();
  }
  return total.ValueOrDefault(0);
}

uint32_t GetValidatedOutputsCount(
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS) {
  uint32_t funcs_outputs = CountOutputsFromFunctions(funcs);
  return funcs_outputs ? std::max(funcs_outputs, pCS->CountComponents()) : 0;
}

std::array<FX_ARGB, kShadingSteps> GetShadingSteps(
    float t_min,
    float t_max,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS,
    int alpha,
    size_t results_count) {
  ASSERT(results_count >= CountOutputsFromFunctions(funcs));
  ASSERT(results_count >= pCS->CountComponents());
  std::array<FX_ARGB, kShadingSteps> shading_steps;
  std::vector<float> result_array(results_count);
  float diff = t_max - t_min;
  for (int i = 0; i < kShadingSteps; ++i) {
    float input = diff * i / kShadingSteps + t_min;
    int offset = 0;
    for (const auto& func : funcs) {
      if (func) {
        int nresults = 0;
        if (func->Call(&input, 1, &result_array[offset], &nresults))
          offset += nresults;
      }
    }
    float R = 0.0f;
    float G = 0.0f;
    float B = 0.0f;
    pCS->GetRGB(result_array.data(), &R, &G, &B);
    shading_steps[i] =
        FXARGB_TODIB(ArgbEncode(alpha, FXSYS_roundf(R * 255),
                                FXSYS_roundf(G * 255), FXSYS_roundf(B * 255)));
  }
  return shading_steps;
}

void DrawAxialShading(const RetainPtr<CFX_DIBitmap>& pBitmap,
                      const CFX_Matrix& mtObject2Bitmap,
                      const CPDF_Dictionary* pDict,
                      const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                      const RetainPtr<CPDF_ColorSpace>& pCS,
                      int alpha) {
  ASSERT(pBitmap->GetFormat() == FXDIB_Argb);

  const uint32_t total_results = GetValidatedOutputsCount(funcs, pCS);
  if (total_results == 0)
    return;

  const CPDF_Array* pCoords = pDict->GetArrayFor("Coords");
  if (!pCoords)
    return;

  float start_x = pCoords->GetNumberAt(0);
  float start_y = pCoords->GetNumberAt(1);
  float end_x = pCoords->GetNumberAt(2);
  float end_y = pCoords->GetNumberAt(3);
  float t_min = 0;
  float t_max = 1.0f;
  const CPDF_Array* pArray = pDict->GetArrayFor("Domain");
  if (pArray) {
    t_min = pArray->GetNumberAt(0);
    t_max = pArray->GetNumberAt(1);
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

  int pitch = pBitmap->GetPitch();
  CFX_Matrix matrix = mtObject2Bitmap.GetInverse();
  for (int row = 0; row < height; row++) {
    uint32_t* dib_buf =
        reinterpret_cast<uint32_t*>(pBitmap->GetBuffer() + row * pitch);
    for (int column = 0; column < width; column++) {
      CFX_PointF pos = matrix.Transform(
          CFX_PointF(static_cast<float>(column), static_cast<float>(row)));
      float scale =
          (((pos.x - start_x) * x_span) + ((pos.y - start_y) * y_span)) /
          axis_len_square;
      int index = (int32_t)(scale * (kShadingSteps - 1));
      if (index < 0) {
        if (!bStartExtend)
          continue;

        index = 0;
      } else if (index >= kShadingSteps) {
        if (!bEndExtend)
          continue;

        index = kShadingSteps - 1;
      }
      dib_buf[column] = shading_steps[index];
    }
  }
}

void DrawRadialShading(const RetainPtr<CFX_DIBitmap>& pBitmap,
                       const CFX_Matrix& mtObject2Bitmap,
                       const CPDF_Dictionary* pDict,
                       const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                       const RetainPtr<CPDF_ColorSpace>& pCS,
                       int alpha) {
  ASSERT(pBitmap->GetFormat() == FXDIB_Argb);

  const uint32_t total_results = GetValidatedOutputsCount(funcs, pCS);
  if (total_results == 0)
    return;

  const CPDF_Array* pCoords = pDict->GetArrayFor("Coords");
  if (!pCoords)
    return;

  float start_x = pCoords->GetNumberAt(0);
  float start_y = pCoords->GetNumberAt(1);
  float start_r = pCoords->GetNumberAt(2);
  float end_x = pCoords->GetNumberAt(3);
  float end_y = pCoords->GetNumberAt(4);
  float end_r = pCoords->GetNumberAt(5);
  float t_min = 0;
  float t_max = 1.0f;
  const CPDF_Array* pArray = pDict->GetArrayFor("Domain");
  if (pArray) {
    t_min = pArray->GetNumberAt(0);
    t_max = pArray->GetNumberAt(1);
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
  const bool a_is_float_zero = IsFloatZero(a);

  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  int pitch = pBitmap->GetPitch();

  bool bDecreasing =
      (dr < 0 && static_cast<int>(sqrt(dx * dx + dy * dy)) < -dr);

  CFX_Matrix matrix = mtObject2Bitmap.GetInverse();
  for (int row = 0; row < height; row++) {
    uint32_t* dib_buf =
        reinterpret_cast<uint32_t*>(pBitmap->GetBuffer() + row * pitch);
    for (int column = 0; column < width; column++) {
      CFX_PointF pos = matrix.Transform(
          CFX_PointF(static_cast<float>(column), static_cast<float>(row)));
      float pos_dx = pos.x - start_x;
      float pos_dy = pos.y - start_y;
      float b = -2 * (pos_dx * dx + pos_dy * dy + start_r * dr);
      float c = pos_dx * pos_dx + pos_dy * pos_dy - start_r * start_r;
      float s;
      if (IsFloatZero(b)) {
        s = sqrt(-c / a);
      } else if (a_is_float_zero) {
        s = -c / b;
      } else {
        float b2_4ac = (b * b) - 4 * (a * c);
        if (b2_4ac < 0)
          continue;

        float root = sqrt(b2_4ac);
        float s1 = (-b - root) / (2 * a);
        float s2 = (-b + root) / (2 * a);
        if (a <= 0)
          std::swap(s1, s2);
        if (bDecreasing)
          s = (s1 >= 0 || bStartExtend) ? s1 : s2;
        else
          s = (s2 <= 1.0f || bEndExtend) ? s2 : s1;

        if (start_r + s * dr < 0)
          continue;
      }

      int index = static_cast<int32_t>(s * (kShadingSteps - 1));
      if (index < 0) {
        if (!bStartExtend)
          continue;
        index = 0;
      } else if (index >= kShadingSteps) {
        if (!bEndExtend)
          continue;
        index = kShadingSteps - 1;
      }
      dib_buf[column] = shading_steps[index];
    }
  }
}

void DrawFuncShading(const RetainPtr<CFX_DIBitmap>& pBitmap,
                     const CFX_Matrix& mtObject2Bitmap,
                     const CPDF_Dictionary* pDict,
                     const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                     const RetainPtr<CPDF_ColorSpace>& pCS,
                     int alpha) {
  ASSERT(pBitmap->GetFormat() == FXDIB_Argb);

  const uint32_t total_results = GetValidatedOutputsCount(funcs, pCS);
  if (total_results == 0)
    return;

  const CPDF_Array* pDomain = pDict->GetArrayFor("Domain");
  float xmin = 0.0f;
  float ymin = 0.0f;
  float xmax = 1.0f;
  float ymax = 1.0f;
  if (pDomain) {
    xmin = pDomain->GetNumberAt(0);
    xmax = pDomain->GetNumberAt(1);
    ymin = pDomain->GetNumberAt(2);
    ymax = pDomain->GetNumberAt(3);
  }
  CFX_Matrix mtDomain2Target = pDict->GetMatrixFor("Matrix");
  CFX_Matrix matrix =
      mtObject2Bitmap.GetInverse() * mtDomain2Target.GetInverse();
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  int pitch = pBitmap->GetPitch();

  ASSERT(total_results >= CountOutputsFromFunctions(funcs));
  ASSERT(total_results >= pCS->CountComponents());
  std::vector<float> result_array(total_results);
  for (int row = 0; row < height; ++row) {
    uint32_t* dib_buf = (uint32_t*)(pBitmap->GetBuffer() + row * pitch);
    for (int column = 0; column < width; column++) {
      CFX_PointF pos = matrix.Transform(
          CFX_PointF(static_cast<float>(column), static_cast<float>(row)));
      if (pos.x < xmin || pos.x > xmax || pos.y < ymin || pos.y > ymax)
        continue;

      float input[] = {pos.x, pos.y};
      int offset = 0;
      for (const auto& func : funcs) {
        if (func) {
          int nresults;
          if (func->Call(input, 2, &result_array[offset], &nresults))
            offset += nresults;
        }
      }

      float R = 0.0f;
      float G = 0.0f;
      float B = 0.0f;
      pCS->GetRGB(result_array.data(), &R, &G, &B);
      dib_buf[column] = FXARGB_TODIB(ArgbEncode(
          alpha, (int32_t)(R * 255), (int32_t)(G * 255), (int32_t)(B * 255)));
    }
  }
}

bool GetScanlineIntersect(int y,
                          const CFX_PointF& first,
                          const CFX_PointF& second,
                          float* x) {
  if (first.y == second.y)
    return false;

  if (first.y < second.y) {
    if (y < first.y || y > second.y)
      return false;
  } else if (y < second.y || y > first.y) {
    return false;
  }
  *x = first.x + ((second.x - first.x) * (y - first.y) / (second.y - first.y));
  return true;
}

void DrawGouraud(const RetainPtr<CFX_DIBitmap>& pBitmap,
                 int alpha,
                 CPDF_MeshVertex triangle[3]) {
  float min_y = triangle[0].position.y;
  float max_y = triangle[0].position.y;
  for (int i = 1; i < 3; i++) {
    min_y = std::min(min_y, triangle[i].position.y);
    max_y = std::max(max_y, triangle[i].position.y);
  }
  if (min_y == max_y)
    return;

  int min_yi = std::max(static_cast<int>(floor(min_y)), 0);
  int max_yi = static_cast<int>(ceil(max_y));

  if (max_yi >= pBitmap->GetHeight())
    max_yi = pBitmap->GetHeight() - 1;

  for (int y = min_yi; y <= max_yi; y++) {
    int nIntersects = 0;
    float inter_x[3];
    float r[3];
    float g[3];
    float b[3];
    for (int i = 0; i < 3; i++) {
      CPDF_MeshVertex& vertex1 = triangle[i];
      CPDF_MeshVertex& vertex2 = triangle[(i + 1) % 3];
      CFX_PointF& position1 = vertex1.position;
      CFX_PointF& position2 = vertex2.position;
      bool bIntersect =
          GetScanlineIntersect(y, position1, position2, &inter_x[nIntersects]);
      if (!bIntersect)
        continue;

      float y_dist = (y - position1.y) / (position2.y - position1.y);
      r[nIntersects] = vertex1.r + ((vertex2.r - vertex1.r) * y_dist);
      g[nIntersects] = vertex1.g + ((vertex2.g - vertex1.g) * y_dist);
      b[nIntersects] = vertex1.b + ((vertex2.b - vertex1.b) * y_dist);
      nIntersects++;
    }
    if (nIntersects != 2)
      continue;

    int min_x, max_x, start_index, end_index;
    if (inter_x[0] < inter_x[1]) {
      min_x = (int)floor(inter_x[0]);
      max_x = (int)ceil(inter_x[1]);
      start_index = 0;
      end_index = 1;
    } else {
      min_x = (int)floor(inter_x[1]);
      max_x = (int)ceil(inter_x[0]);
      start_index = 1;
      end_index = 0;
    }

    int start_x = std::max(min_x, 0);
    int end_x = max_x;
    if (end_x > pBitmap->GetWidth())
      end_x = pBitmap->GetWidth();

    uint8_t* dib_buf =
        pBitmap->GetBuffer() + y * pBitmap->GetPitch() + start_x * 4;
    float r_unit = (r[end_index] - r[start_index]) / (max_x - min_x);
    float g_unit = (g[end_index] - g[start_index]) / (max_x - min_x);
    float b_unit = (b[end_index] - b[start_index]) / (max_x - min_x);
    float R = r[start_index] + (start_x - min_x) * r_unit;
    float G = g[start_index] + (start_x - min_x) * g_unit;
    float B = b[start_index] + (start_x - min_x) * b_unit;
    for (int x = start_x; x < end_x; x++) {
      R += r_unit;
      G += g_unit;
      B += b_unit;
      FXARGB_SETDIB(dib_buf,
                    ArgbEncode(alpha, (int32_t)(R * 255), (int32_t)(G * 255),
                               (int32_t)(B * 255)));
      dib_buf += 4;
    }
  }
}

void DrawFreeGouraudShading(
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const CFX_Matrix& mtObject2Bitmap,
    const CPDF_Stream* pShadingStream,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS,
    int alpha) {
  ASSERT(pBitmap->GetFormat() == FXDIB_Argb);

  CPDF_MeshStream stream(kFreeFormGouraudTriangleMeshShading, funcs,
                         pShadingStream, pCS);
  if (!stream.Load())
    return;

  CPDF_MeshVertex triangle[3];
  memset(triangle, 0, sizeof(triangle));

  while (!stream.BitStream()->IsEOF()) {
    CPDF_MeshVertex vertex;
    uint32_t flag;
    if (!stream.ReadVertex(mtObject2Bitmap, &vertex, &flag))
      return;

    if (flag == 0) {
      triangle[0] = vertex;
      for (int j = 1; j < 3; j++) {
        uint32_t tflag;
        if (!stream.ReadVertex(mtObject2Bitmap, &triangle[j], &tflag))
          return;
      }
    } else {
      if (flag == 1)
        triangle[0] = triangle[1];

      triangle[1] = triangle[2];
      triangle[2] = vertex;
    }
    DrawGouraud(pBitmap, alpha, triangle);
  }
}

void DrawLatticeGouraudShading(
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const CFX_Matrix& mtObject2Bitmap,
    const CPDF_Stream* pShadingStream,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS,
    int alpha) {
  ASSERT(pBitmap->GetFormat() == FXDIB_Argb);

  int row_verts = pShadingStream->GetDict()->GetIntegerFor("VerticesPerRow");
  if (row_verts < 2)
    return;

  CPDF_MeshStream stream(kLatticeFormGouraudTriangleMeshShading, funcs,
                         pShadingStream, pCS);
  if (!stream.Load())
    return;

  std::vector<CPDF_MeshVertex> vertices[2];
  vertices[0] = stream.ReadVertexRow(mtObject2Bitmap, row_verts);
  if (vertices[0].empty())
    return;

  int last_index = 0;
  while (1) {
    vertices[1 - last_index] = stream.ReadVertexRow(mtObject2Bitmap, row_verts);
    if (vertices[1 - last_index].empty())
      return;

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

struct Coon_BezierCoeff {
  float a, b, c, d;
  void FromPoints(float p0, float p1, float p2, float p3) {
    a = -p0 + 3 * p1 - 3 * p2 + p3;
    b = 3 * p0 - 6 * p1 + 3 * p2;
    c = -3 * p0 + 3 * p1;
    d = p0;
  }
  Coon_BezierCoeff first_half() {
    Coon_BezierCoeff result;
    result.a = a / 8;
    result.b = b / 4;
    result.c = c / 2;
    result.d = d;
    return result;
  }
  Coon_BezierCoeff second_half() {
    Coon_BezierCoeff result;
    result.a = a / 8;
    result.b = 3 * a / 8 + b / 4;
    result.c = 3 * a / 8 + b / 2 + c / 2;
    result.d = a / 8 + b / 4 + c / 2 + d;
    return result;
  }
  void GetPoints(float p[4]) {
    p[0] = d;
    p[1] = c / 3 + p[0];
    p[2] = b / 3 - p[0] + 2 * p[1];
    p[3] = a + p[0] - 3 * p[1] + 3 * p[2];
  }
  void BezierInterpol(Coon_BezierCoeff& C1,
                      Coon_BezierCoeff& C2,
                      Coon_BezierCoeff& D1,
                      Coon_BezierCoeff& D2) {
    a = (D1.a + D2.a) / 2;
    b = (D1.b + D2.b) / 2;
    c = (D1.c + D2.c) / 2 - (C1.a / 8 + C1.b / 4 + C1.c / 2) +
        (C2.a / 8 + C2.b / 4) + (-C1.d + D2.d) / 2 - (C2.a + C2.b) / 2;
    d = C1.a / 8 + C1.b / 4 + C1.c / 2 + C1.d;
  }
  float Distance() {
    float dis = a + b + c;
    return dis < 0 ? -dis : dis;
  }
};

struct Coon_Bezier {
  Coon_BezierCoeff x, y;
  void FromPoints(float x0,
                  float y0,
                  float x1,
                  float y1,
                  float x2,
                  float y2,
                  float x3,
                  float y3) {
    x.FromPoints(x0, x1, x2, x3);
    y.FromPoints(y0, y1, y2, y3);
  }

  Coon_Bezier first_half() {
    Coon_Bezier result;
    result.x = x.first_half();
    result.y = y.first_half();
    return result;
  }

  Coon_Bezier second_half() {
    Coon_Bezier result;
    result.x = x.second_half();
    result.y = y.second_half();
    return result;
  }

  void BezierInterpol(Coon_Bezier& C1,
                      Coon_Bezier& C2,
                      Coon_Bezier& D1,
                      Coon_Bezier& D2) {
    x.BezierInterpol(C1.x, C2.x, D1.x, D2.x);
    y.BezierInterpol(C1.y, C2.y, D1.y, D2.y);
  }

  void GetPoints(pdfium::span<FX_PATHPOINT> path_points) {
    constexpr size_t kPointsCount = 4;
    float points_x[kPointsCount];
    float points_y[kPointsCount];
    x.GetPoints(points_x);
    y.GetPoints(points_y);
    for (size_t i = 0; i < kPointsCount; ++i)
      path_points[i].m_Point = {points_x[i], points_y[i]};
  }

  void GetPointsReverse(pdfium::span<FX_PATHPOINT> path_points) {
    constexpr size_t kPointsCount = 4;
    float points_x[kPointsCount];
    float points_y[kPointsCount];
    x.GetPoints(points_x);
    y.GetPoints(points_y);
    for (size_t i = 0; i < kPointsCount; ++i) {
      size_t reverse_index = kPointsCount - i - 1;
      path_points[i].m_Point = {points_x[reverse_index],
                                points_y[reverse_index]};
    }
  }

  float Distance() { return x.Distance() + y.Distance(); }
};

int Interpolate(int p1, int p2, int delta1, int delta2, bool* overflow) {
  pdfium::base::CheckedNumeric<int> p = p2;
  p -= p1;
  p *= delta1;
  p /= delta2;
  p += p1;
  if (!p.IsValid())
    *overflow = true;
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

struct Coon_Color {
  Coon_Color() { memset(comp, 0, sizeof(int) * 3); }

  // Returns true if successful, false if overflow detected.
  bool BiInterpol(Coon_Color colors[4],
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

  int Distance(Coon_Color& o) {
    return std::max({abs(comp[0] - o.comp[0]), abs(comp[1] - o.comp[1]),
                     abs(comp[2] - o.comp[2])});
  }

  int comp[3];
};

#define COONCOLOR_THRESHOLD 4
struct CPDF_PatchDrawer {
  void Draw(int x_scale,
            int y_scale,
            int left,
            int bottom,
            Coon_Bezier C1,
            Coon_Bezier C2,
            Coon_Bezier D1,
            Coon_Bezier D2) {
    bool bSmall = C1.Distance() < 2 && C2.Distance() < 2 && D1.Distance() < 2 &&
                  D2.Distance() < 2;
    Coon_Color div_colors[4];
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
        (d_bottom < COONCOLOR_THRESHOLD && d_left < COONCOLOR_THRESHOLD &&
         d_top < COONCOLOR_THRESHOLD && d_right < COONCOLOR_THRESHOLD)) {
      pdfium::span<FX_PATHPOINT> points = path.GetPoints();
      C1.GetPoints(points.subspan(0, 4));
      D2.GetPoints(points.subspan(3, 4));
      C2.GetPointsReverse(points.subspan(6, 4));
      D1.GetPointsReverse(points.subspan(9, 4));
      int fillFlags = FXFILL_WINDING | FXFILL_FULLCOVER;
      if (bNoPathSmooth)
        fillFlags |= FXFILL_NOPATHSMOOTH;
      pDevice->DrawPath(
          &path, nullptr, nullptr,
          ArgbEncode(alpha, div_colors[0].comp[0], div_colors[0].comp[1],
                     div_colors[0].comp[2]),
          0, fillFlags);
    } else {
      if (d_bottom < COONCOLOR_THRESHOLD && d_top < COONCOLOR_THRESHOLD) {
        Coon_Bezier m1;
        m1.BezierInterpol(D1, D2, C1, C2);
        y_scale *= 2;
        bottom *= 2;
        Draw(x_scale, y_scale, left, bottom, C1, m1, D1.first_half(),
             D2.first_half());
        Draw(x_scale, y_scale, left, bottom + 1, m1, C2, D1.second_half(),
             D2.second_half());
      } else if (d_left < COONCOLOR_THRESHOLD &&
                 d_right < COONCOLOR_THRESHOLD) {
        Coon_Bezier m2;
        m2.BezierInterpol(C1, C2, D1, D2);
        x_scale *= 2;
        left *= 2;
        Draw(x_scale, y_scale, left, bottom, C1.first_half(), C2.first_half(),
             D1, m2);
        Draw(x_scale, y_scale, left + 1, bottom, C1.second_half(),
             C2.second_half(), m2, D2);
      } else {
        Coon_Bezier m1, m2;
        m1.BezierInterpol(D1, D2, C1, C2);
        m2.BezierInterpol(C1, C2, D1, D2);
        Coon_Bezier m1f = m1.first_half();
        Coon_Bezier m1s = m1.second_half();
        Coon_Bezier m2f = m2.first_half();
        Coon_Bezier m2s = m2.second_half();
        x_scale *= 2;
        y_scale *= 2;
        left *= 2;
        bottom *= 2;
        Draw(x_scale, y_scale, left, bottom, C1.first_half(), m1f,
             D1.first_half(), m2f);
        Draw(x_scale, y_scale, left, bottom + 1, m1f, C2.first_half(),
             D1.second_half(), m2s);
        Draw(x_scale, y_scale, left + 1, bottom, C1.second_half(), m1s, m2f,
             D2.first_half());
        Draw(x_scale, y_scale, left + 1, bottom + 1, m1s, C2.second_half(), m2s,
             D2.second_half());
      }
    }
  }

  int max_delta;
  CFX_PathData path;
  CFX_RenderDevice* pDevice;
  int bNoPathSmooth;
  int alpha;
  Coon_Color patch_colors[4];
};

void DrawCoonPatchMeshes(
    ShadingType type,
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const CFX_Matrix& mtObject2Bitmap,
    const CPDF_Stream* pShadingStream,
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    const RetainPtr<CPDF_ColorSpace>& pCS,
    bool bNoPathSmooth,
    int alpha) {
  ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
  ASSERT(type == kCoonsPatchMeshShading ||
         type == kTensorProductPatchMeshShading);

  CFX_DefaultRenderDevice device;
  device.Attach(pBitmap, false, nullptr, false);
  CPDF_MeshStream stream(type, funcs, pShadingStream, pCS);
  if (!stream.Load())
    return;

  CPDF_PatchDrawer patch;
  patch.alpha = alpha;
  patch.pDevice = &device;
  patch.bNoPathSmooth = bNoPathSmooth;

  for (int i = 0; i < 13; i++) {
    patch.path.AppendPoint(
        CFX_PointF(), i == 0 ? FXPT_TYPE::MoveTo : FXPT_TYPE::BezierTo, false);
  }

  CFX_PointF coords[16];
  int point_count = type == kTensorProductPatchMeshShading ? 16 : 12;
  while (!stream.BitStream()->IsEOF()) {
    if (!stream.CanReadFlag())
      break;
    uint32_t flag = stream.ReadFlag();
    int iStartPoint = 0, iStartColor = 0, i = 0;
    if (flag) {
      iStartPoint = 4;
      iStartColor = 2;
      CFX_PointF tempCoords[4];
      for (i = 0; i < 4; i++) {
        tempCoords[i] = coords[(flag * 3 + i) % 12];
      }
      memcpy(coords, tempCoords, sizeof(tempCoords));
      Coon_Color tempColors[2];
      tempColors[0] = patch.patch_colors[flag];
      tempColors[1] = patch.patch_colors[(flag + 1) % 4];
      memcpy(patch.patch_colors, tempColors, sizeof(Coon_Color) * 2);
    }
    for (i = iStartPoint; i < point_count; i++) {
      if (!stream.CanReadCoords())
        break;
      coords[i] = mtObject2Bitmap.Transform(stream.ReadCoords());
    }

    for (i = iStartColor; i < 4; i++) {
      if (!stream.CanReadColor())
        break;

      float r;
      float g;
      float b;
      std::tie(r, g, b) = stream.ReadColor();

      patch.patch_colors[i].comp[0] = (int32_t)(r * 255);
      patch.patch_colors[i].comp[1] = (int32_t)(g * 255);
      patch.patch_colors[i].comp[2] = (int32_t)(b * 255);
    }
    CFX_FloatRect bbox = CFX_FloatRect::GetBBox(coords, point_count);
    if (bbox.right <= 0 || bbox.left >= (float)pBitmap->GetWidth() ||
        bbox.top <= 0 || bbox.bottom >= (float)pBitmap->GetHeight()) {
      continue;
    }
    Coon_Bezier C1, C2, D1, D2;
    C1.FromPoints(coords[0].x, coords[0].y, coords[11].x, coords[11].y,
                  coords[10].x, coords[10].y, coords[9].x, coords[9].y);
    C2.FromPoints(coords[3].x, coords[3].y, coords[4].x, coords[4].y,
                  coords[5].x, coords[5].y, coords[6].x, coords[6].y);
    D1.FromPoints(coords[0].x, coords[0].y, coords[1].x, coords[1].y,
                  coords[2].x, coords[2].y, coords[3].x, coords[3].y);
    D2.FromPoints(coords[9].x, coords[9].y, coords[8].x, coords[8].y,
                  coords[7].x, coords[7].y, coords[6].x, coords[6].y);
    patch.Draw(1, 1, 0, 0, C1, C2, D1, D2);
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
  const auto& funcs = pPattern->GetFuncs();
  const CPDF_Dictionary* pDict = pPattern->GetShadingObject()->GetDict();
  RetainPtr<CPDF_ColorSpace> pColorSpace = pPattern->GetCS();
  if (!pColorSpace)
    return;

  FX_ARGB background = 0;
  if (!pPattern->IsShadingObject() && pDict->KeyExist("Background")) {
    const CPDF_Array* pBackColor = pDict->GetArrayFor("Background");
    if (pBackColor && pBackColor->size() >= pColorSpace->CountComponents()) {
      std::vector<float> comps =
          ReadArrayElementsToVector(pBackColor, pColorSpace->CountComponents());

      float R = 0.0f;
      float G = 0.0f;
      float B = 0.0f;
      pColorSpace->GetRGB(comps.data(), &R, &G, &B);
      background = ArgbEncode(255, (int32_t)(R * 255), (int32_t)(G * 255),
                              (int32_t)(B * 255));
    }
  }
  FX_RECT clip_rect_bbox = clip_rect;
  if (pDict->KeyExist("BBox")) {
    clip_rect_bbox.Intersect(
        mtMatrix.TransformRect(pDict->GetRectFor("BBox")).GetOuterRect());
  }
  bool bAlphaMode = options.ColorModeIs(CPDF_RenderOptions::kAlpha);
  if (pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SHADING &&
      pDevice->GetDeviceDriver()->DrawShading(
          pPattern, &mtMatrix, clip_rect_bbox, alpha, bAlphaMode)) {
    return;
  }
  CPDF_DeviceBuffer buffer(pContext, pDevice, clip_rect_bbox, pCurObj, 150);
  if (!buffer.Initialize())
    return;

  CFX_Matrix FinalMatrix = mtMatrix * buffer.GetMatrix();
  RetainPtr<CFX_DIBitmap> pBitmap = buffer.GetBitmap();
  if (!pBitmap->GetBuffer())
    return;

  pBitmap->Clear(background);
  switch (pPattern->GetShadingType()) {
    case kInvalidShading:
    case kMaxShading:
      return;
    case kFunctionBasedShading:
      DrawFuncShading(pBitmap, FinalMatrix, pDict, funcs, pColorSpace, alpha);
      break;
    case kAxialShading:
      DrawAxialShading(pBitmap, FinalMatrix, pDict, funcs, pColorSpace, alpha);
      break;
    case kRadialShading:
      DrawRadialShading(pBitmap, FinalMatrix, pDict, funcs, pColorSpace, alpha);
      break;
    case kFreeFormGouraudTriangleMeshShading: {
      // The shading object can be a stream or a dictionary. We do not handle
      // the case of dictionary at the moment.
      if (const CPDF_Stream* pStream = ToStream(pPattern->GetShadingObject())) {
        DrawFreeGouraudShading(pBitmap, FinalMatrix, pStream, funcs,
                               pColorSpace, alpha);
      }
    } break;
    case kLatticeFormGouraudTriangleMeshShading: {
      // The shading object can be a stream or a dictionary. We do not handle
      // the case of dictionary at the moment.
      if (const CPDF_Stream* pStream = ToStream(pPattern->GetShadingObject())) {
        DrawLatticeGouraudShading(pBitmap, FinalMatrix, pStream, funcs,
                                  pColorSpace, alpha);
      }
    } break;
    case kCoonsPatchMeshShading:
    case kTensorProductPatchMeshShading: {
      // The shading object can be a stream or a dictionary. We do not handle
      // the case of dictionary at the moment.
      if (const CPDF_Stream* pStream = ToStream(pPattern->GetShadingObject())) {
        DrawCoonPatchMeshes(pPattern->GetShadingType(), pBitmap, FinalMatrix,
                            pStream, funcs, pColorSpace,
                            options.GetOptions().bNoPathSmooth, alpha);
      }
    } break;
  }
  if (bAlphaMode)
    pBitmap->LoadChannelFromAlpha(FXDIB_Red, pBitmap);

  if (options.ColorModeIs(CPDF_RenderOptions::kGray))
    pBitmap->ConvertColorScale(0, 0xffffff);
  buffer.OutputToDevice();
}
