// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_renderdevice.h"

#include <math.h>

#include <algorithm>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_color.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/renderdevicedriver_iface.h"
#include "core/fxge/text_char_pos.h"
#include "core/fxge/text_glyph_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/notreached.h"
#include "third_party/base/span.h"

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#include "third_party/skia/include/core/SkTypes.h"  // nogncheck
#endif

namespace {

void AdjustGlyphSpace(std::vector<TextGlyphPos>* pGlyphAndPos) {
  DCHECK_GT(pGlyphAndPos->size(), 1u);
  std::vector<TextGlyphPos>& glyphs = *pGlyphAndPos;
  bool bVertical = glyphs.back().m_Origin.x == glyphs.front().m_Origin.x;
  if (!bVertical && (glyphs.back().m_Origin.y != glyphs.front().m_Origin.y))
    return;

  for (size_t i = glyphs.size() - 1; i > 1; --i) {
    const TextGlyphPos& next = glyphs[i];
    int next_origin = bVertical ? next.m_Origin.y : next.m_Origin.x;
    float next_origin_f =
        bVertical ? next.m_fDeviceOrigin.y : next.m_fDeviceOrigin.x;

    TextGlyphPos& current = glyphs[i - 1];
    int& current_origin = bVertical ? current.m_Origin.y : current.m_Origin.x;
    float current_origin_f =
        bVertical ? current.m_fDeviceOrigin.y : current.m_fDeviceOrigin.x;

    FX_SAFE_INT32 safe_space = next_origin;
    safe_space -= current_origin;
    if (!safe_space.IsValid())
      continue;

    int space = safe_space.ValueOrDie();
    float space_f = next_origin_f - current_origin_f;
    float error = fabs(space_f) - fabs(static_cast<float>(space));
    if (error <= 0.5f)
      continue;

    FX_SAFE_INT32 safe_origin = current_origin;
    safe_origin += space > 0 ? -1 : 1;
    if (!safe_origin.IsValid())
      continue;

    current_origin = safe_origin.ValueOrDie();
  }
}

constexpr uint8_t kTextGammaAdjust[256] = {
    0,   2,   3,   4,   6,   7,   8,   10,  11,  12,  13,  15,  16,  17,  18,
    19,  21,  22,  23,  24,  25,  26,  27,  29,  30,  31,  32,  33,  34,  35,
    36,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  51,  52,
    53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,
    68,  69,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
    84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,
    99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
    114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
    129, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
    143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 156,
    157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171,
    172, 173, 174, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
    186, 187, 188, 189, 190, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213,
    214, 215, 216, 217, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227,
    228, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 239, 240,
    241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 250, 251, 252, 253, 254,
    255,
};

int TextGammaAdjust(int value) {
  DCHECK_GE(value, 0);
  DCHECK_LE(value, 255);
  return kTextGammaAdjust[value];
}

int CalcAlpha(int src, int alpha) {
  return src * alpha / 255;
}

void MergeGammaAdjust(uint8_t src, int channel, int alpha, uint8_t* dest) {
  *dest =
      FXDIB_ALPHA_MERGE(*dest, channel, CalcAlpha(TextGammaAdjust(src), alpha));
}

void MergeGammaAdjustRgb(const uint8_t* src,
                         int r,
                         int g,
                         int b,
                         int a,
                         uint8_t* dest) {
  MergeGammaAdjust(src[2], b, a, &dest[0]);
  MergeGammaAdjust(src[1], g, a, &dest[1]);
  MergeGammaAdjust(src[0], r, a, &dest[2]);
}

int AverageRgb(const uint8_t* src) {
  return (src[0] + src[1] + src[2]) / 3;
}

uint8_t CalculateDestAlpha(uint8_t back_alpha, int src_alpha) {
  return back_alpha + src_alpha - back_alpha * src_alpha / 255;
}

void ApplyAlpha(uint8_t* dest, int b, int g, int r, int alpha) {
  dest[0] = FXDIB_ALPHA_MERGE(dest[0], b, alpha);
  dest[1] = FXDIB_ALPHA_MERGE(dest[1], g, alpha);
  dest[2] = FXDIB_ALPHA_MERGE(dest[2], r, alpha);
}

void ApplyDestAlpha(uint8_t back_alpha,
                    int src_alpha,
                    int r,
                    int g,
                    int b,
                    uint8_t* dest) {
  uint8_t dest_alpha = CalculateDestAlpha(back_alpha, src_alpha);
  ApplyAlpha(dest, b, g, r, src_alpha * 255 / dest_alpha);
  dest[3] = dest_alpha;
}

void NormalizeArgb(int src_value,
                   int r,
                   int g,
                   int b,
                   int a,
                   uint8_t* dest,
                   int src_alpha) {
  uint8_t back_alpha = dest[3];
  if (back_alpha == 0)
    FXARGB_SETDIB(dest, ArgbEncode(src_alpha, r, g, b));
  else if (src_alpha != 0)
    ApplyDestAlpha(back_alpha, src_alpha, r, g, b, dest);
}

void NormalizeDest(bool has_alpha,
                   int src_value,
                   int r,
                   int g,
                   int b,
                   int a,
                   uint8_t* dest) {
  if (has_alpha) {
    NormalizeArgb(src_value, r, g, b, a, dest,
                  CalcAlpha(TextGammaAdjust(src_value), a));
    return;
  }
  int src_alpha = CalcAlpha(TextGammaAdjust(src_value), a);
  if (src_alpha == 0)
    return;

  ApplyAlpha(dest, b, g, r, src_alpha);
}

void NormalizeSrc(bool has_alpha,
                  int src_value,
                  int r,
                  int g,
                  int b,
                  int a,
                  uint8_t* dest) {
  if (!has_alpha) {
    ApplyAlpha(dest, b, g, r, CalcAlpha(TextGammaAdjust(src_value), a));
    return;
  }
  int src_alpha = CalcAlpha(TextGammaAdjust(src_value), a);
  if (src_alpha != 0)
    NormalizeArgb(src_value, r, g, b, a, dest, src_alpha);
}

void NextPixel(uint8_t** src_scan, uint8_t** dst_scan, int bpp) {
  *src_scan += 3;
  *dst_scan += bpp;
}

void SetAlpha(bool has_alpha, uint8_t* alpha) {
  if (has_alpha)
    alpha[3] = 255;
}

void DrawNormalTextHelper(const RetainPtr<CFX_DIBitmap>& bitmap,
                          const RetainPtr<CFX_DIBitmap>& pGlyph,
                          int nrows,
                          int left,
                          int top,
                          int start_col,
                          int end_col,
                          bool normalize,
                          int x_subpixel,
                          int a,
                          int r,
                          int g,
                          int b) {
  const bool has_alpha = bitmap->GetFormat() == FXDIB_Format::kArgb;
  uint8_t* src_buf = pGlyph->GetBuffer();
  int src_pitch = pGlyph->GetPitch();
  uint8_t* dest_buf = bitmap->GetBuffer();
  int dest_pitch = bitmap->GetPitch();
  const int Bpp = has_alpha ? 4 : bitmap->GetBPP() / 8;
  for (int row = 0; row < nrows; ++row) {
    int dest_row = row + top;
    if (dest_row < 0 || dest_row >= bitmap->GetHeight())
      continue;

    uint8_t* src_scan = src_buf + row * src_pitch + (start_col - left) * 3;
    uint8_t* dest_scan = dest_buf + dest_row * dest_pitch + start_col * Bpp;
    if (x_subpixel == 0) {
      for (int col = start_col; col < end_col; ++col) {
        if (normalize) {
          int src_value = AverageRgb(&src_scan[0]);
          NormalizeDest(has_alpha, src_value, r, g, b, a, dest_scan);
        } else {
          MergeGammaAdjustRgb(&src_scan[0], r, g, b, a, &dest_scan[0]);
          SetAlpha(has_alpha, dest_scan);
        }
        NextPixel(&src_scan, &dest_scan, Bpp);
      }
      continue;
    }
    if (x_subpixel == 1) {
      if (normalize) {
        int src_value = start_col > left ? AverageRgb(&src_scan[-1])
                                         : (src_scan[0] + src_scan[1]) / 3;
        NormalizeSrc(has_alpha, src_value, r, g, b, a, dest_scan);
      } else {
        if (start_col > left)
          MergeGammaAdjust(src_scan[-1], r, a, &dest_scan[2]);
        MergeGammaAdjust(src_scan[0], g, a, &dest_scan[1]);
        MergeGammaAdjust(src_scan[1], b, a, &dest_scan[0]);
        SetAlpha(has_alpha, dest_scan);
      }
      NextPixel(&src_scan, &dest_scan, Bpp);
      for (int col = start_col + 1; col < end_col; ++col) {
        if (normalize) {
          int src_value = AverageRgb(&src_scan[-1]);
          NormalizeDest(has_alpha, src_value, r, g, b, a, dest_scan);
        } else {
          MergeGammaAdjustRgb(&src_scan[-1], r, g, b, a, &dest_scan[0]);
          SetAlpha(has_alpha, dest_scan);
        }
        NextPixel(&src_scan, &dest_scan, Bpp);
      }
      continue;
    }
    if (normalize) {
      int src_value =
          start_col > left ? AverageRgb(&src_scan[-2]) : src_scan[0] / 3;
      NormalizeSrc(has_alpha, src_value, r, g, b, a, dest_scan);
    } else {
      if (start_col > left) {
        MergeGammaAdjust(src_scan[-2], r, a, &dest_scan[2]);
        MergeGammaAdjust(src_scan[-1], g, a, &dest_scan[1]);
      }
      MergeGammaAdjust(src_scan[0], b, a, &dest_scan[0]);
      SetAlpha(has_alpha, dest_scan);
    }
    NextPixel(&src_scan, &dest_scan, Bpp);
    for (int col = start_col + 1; col < end_col; ++col) {
      if (normalize) {
        int src_value = AverageRgb(&src_scan[-2]);
        NormalizeDest(has_alpha, src_value, r, g, b, a, dest_scan);
      } else {
        MergeGammaAdjustRgb(&src_scan[-2], r, g, b, a, &dest_scan[0]);
        SetAlpha(has_alpha, dest_scan);
      }
      NextPixel(&src_scan, &dest_scan, Bpp);
    }
  }
}

bool ShouldDrawDeviceText(const CFX_Font* pFont,
                          const CFX_TextRenderOptions& options) {
#if defined(OS_APPLE)
  if (options.font_is_cid)
    return false;

  const ByteString bsPsName = pFont->GetPsName();
  if (bsPsName.Contains("+ZJHL"))
    return false;

  if (bsPsName == "CNAAJI+cmex10")
    return false;
#endif
  return true;
}

// Returns true if the path is a 3-point path that draws A->B->A and forms a
// zero area, or a 2-point path which draws A->B.
bool CheckSimpleLinePath(pdfium::span<const CFX_Path::Point> points,
                         const CFX_Matrix* matrix,
                         bool adjust,
                         CFX_Path* new_path,
                         bool* thin,
                         bool* set_identity) {
  if (points.size() != 2 && points.size() != 3)
    return false;

  if (points[0].m_Type != CFX_Path::Point::Type::kMove ||
      points[1].m_Type != CFX_Path::Point::Type::kLine ||
      (points.size() == 3 &&
       (points[2].m_Type != CFX_Path::Point::Type::kLine ||
        points[0].m_Point != points[2].m_Point))) {
    return false;
  }

  // A special case that all points are identical, zero area is formed and no
  // thin line needs to be drawn.
  if (points[0].m_Point == points[1].m_Point)
    return true;

  for (size_t i = 0; i < 2; i++) {
    CFX_PointF point = points[i].m_Point;
    if (adjust) {
      if (matrix)
        point = matrix->Transform(point);

      point = CFX_PointF(static_cast<int>(point.x) + 0.5f,
                         static_cast<int>(point.y) + 0.5f);
    }
    new_path->AppendPoint(point, points[i].m_Type);
  }
  if (adjust && matrix)
    *set_identity = true;

  *thin = true;
  return true;
}

// Returns true if `points` is palindromic and forms zero area. Otherwise,
// returns false.
bool CheckPalindromicPath(pdfium::span<const CFX_Path::Point> points,
                          CFX_Path* new_path,
                          bool* thin) {
  if (points.size() <= 3 || !(points.size() % 2))
    return false;

  const int mid = points.size() / 2;
  CFX_Path temp_path;
  for (int i = 0; i < mid; i++) {
    const CFX_Path::Point& left = points[mid - i - 1];
    const CFX_Path::Point& right = points[mid + i + 1];
    bool zero_area = left.m_Point == right.m_Point &&
                     left.m_Type != CFX_Path::Point::Type::kBezier &&
                     right.m_Type != CFX_Path::Point::Type::kBezier;
    if (!zero_area)
      return false;

    temp_path.AppendPoint(points[mid - i].m_Point,
                          CFX_Path::Point::Type::kMove);
    temp_path.AppendPoint(left.m_Point, CFX_Path::Point::Type::kLine);
  }

  new_path->Append(temp_path, nullptr);
  *thin = true;
  return true;
}

bool IsFoldingVerticalLine(const CFX_PointF& a,
                           const CFX_PointF& b,
                           const CFX_PointF& c) {
  return a.x == b.x && b.x == c.x && (b.y - a.y) * (b.y - c.y) > 0;
}

bool IsFoldingHorizontalLine(const CFX_PointF& a,
                             const CFX_PointF& b,
                             const CFX_PointF& c) {
  return a.y == b.y && b.y == c.y && (b.x - a.x) * (b.x - c.x) > 0;
}

bool IsFoldingDiagonalLine(const CFX_PointF& a,
                           const CFX_PointF& b,
                           const CFX_PointF& c) {
  return a.x != b.x && c.x != b.x && a.y != b.y && c.y != b.y &&
         (a.y - b.y) * (c.x - b.x) == (c.y - b.y) * (a.x - b.x);
}

bool GetZeroAreaPath(pdfium::span<const CFX_Path::Point> points,
                     const CFX_Matrix* matrix,
                     bool adjust,
                     CFX_Path* new_path,
                     bool* thin,
                     bool* set_identity) {
  *set_identity = false;

  if (points.size() < 2)
    return false;

  if (CheckSimpleLinePath(points, matrix, adjust, new_path, thin,
                          set_identity)) {
    return true;
  }

  if (CheckPalindromicPath(points, new_path, thin))
    return true;

  for (size_t i = 0; i < points.size(); i++) {
    CFX_Path::Point::Type point_type = points[i].m_Type;
    if (point_type == CFX_Path::Point::Type::kMove) {
      DCHECK_EQ(0, i);
      continue;
    }

    if (point_type == CFX_Path::Point::Type::kBezier) {
      i += 2;
      DCHECK_LT(i, points.size());
      continue;
    }

    DCHECK_EQ(point_type, CFX_Path::Point::Type::kLine);
    size_t next_index = (i + 1) % (points.size());
    const CFX_Path::Point& next = points[next_index];
    if (next.m_Type != CFX_Path::Point::Type::kLine)
      continue;

    const CFX_Path::Point& prev = points[i - 1];
    const CFX_Path::Point& cur = points[i];
    if (IsFoldingVerticalLine(prev.m_Point, cur.m_Point, next.m_Point)) {
      bool use_prev = fabs(cur.m_Point.y - prev.m_Point.y) <
                      fabs(cur.m_Point.y - next.m_Point.y);
      const CFX_Path::Point& start = use_prev ? prev : cur;
      const CFX_Path::Point& end = use_prev ? cur : next;
      new_path->AppendPoint(start.m_Point, CFX_Path::Point::Type::kMove);
      new_path->AppendPoint(end.m_Point, CFX_Path::Point::Type::kLine);
      continue;
    }

    if (IsFoldingHorizontalLine(prev.m_Point, cur.m_Point, next.m_Point) ||
        IsFoldingDiagonalLine(prev.m_Point, cur.m_Point, next.m_Point)) {
      bool use_prev = fabs(cur.m_Point.x - prev.m_Point.x) <
                      fabs(cur.m_Point.x - next.m_Point.x);
      const CFX_Path::Point& start = use_prev ? prev : cur;
      const CFX_Path::Point& end = use_prev ? cur : next;
      new_path->AppendPoint(start.m_Point, CFX_Path::Point::Type::kMove);
      new_path->AppendPoint(end.m_Point, CFX_Path::Point::Type::kLine);
      continue;
    }
  }

  size_t new_path_size = new_path->GetPoints().size();
  if (points.size() > 3 && new_path_size > 0)
    *thin = true;
  return new_path_size != 0;
}

}  // namespace

CFX_RenderDevice::CFX_RenderDevice() = default;

CFX_RenderDevice::~CFX_RenderDevice() {
  RestoreState(false);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  Flush(true);
#endif
}

// static
CFX_Matrix CFX_RenderDevice::GetFlipMatrix(float width,
                                           float height,
                                           float left,
                                           float top) {
  return CFX_Matrix(width, 0, 0, -height, left, top + height);
}

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
void CFX_RenderDevice::Flush(bool release) {
  if (release)
    m_pDeviceDriver.reset();
  else
    m_pDeviceDriver->Flush();
}
#endif

void CFX_RenderDevice::SetDeviceDriver(
    std::unique_ptr<RenderDeviceDriverIface> pDriver) {
  DCHECK(pDriver);
  DCHECK(!m_pDeviceDriver);
  m_pDeviceDriver = std::move(pDriver);
  InitDeviceInfo();
}

void CFX_RenderDevice::InitDeviceInfo() {
  m_Width = m_pDeviceDriver->GetDeviceCaps(FXDC_PIXEL_WIDTH);
  m_Height = m_pDeviceDriver->GetDeviceCaps(FXDC_PIXEL_HEIGHT);
  m_bpp = m_pDeviceDriver->GetDeviceCaps(FXDC_BITS_PIXEL);
  m_RenderCaps = m_pDeviceDriver->GetDeviceCaps(FXDC_RENDER_CAPS);
  m_DeviceType = m_pDeviceDriver->GetDeviceType();
  if (!m_pDeviceDriver->GetClipBox(&m_ClipBox)) {
    m_ClipBox.left = 0;
    m_ClipBox.top = 0;
    m_ClipBox.right = m_Width;
    m_ClipBox.bottom = m_Height;
  }
}

void CFX_RenderDevice::SaveState() {
  m_pDeviceDriver->SaveState();
}

void CFX_RenderDevice::RestoreState(bool bKeepSaved) {
  if (m_pDeviceDriver) {
    m_pDeviceDriver->RestoreState(bKeepSaved);
    UpdateClipBox();
  }
}

int CFX_RenderDevice::GetDeviceCaps(int caps_id) const {
  return m_pDeviceDriver->GetDeviceCaps(caps_id);
}

RetainPtr<CFX_DIBitmap> CFX_RenderDevice::GetBitmap() const {
  return m_pBitmap;
}

void CFX_RenderDevice::SetBitmap(const RetainPtr<CFX_DIBitmap>& pBitmap) {
  m_pBitmap = pBitmap;
}

bool CFX_RenderDevice::CreateCompatibleBitmap(
    const RetainPtr<CFX_DIBitmap>& pDIB,
    int width,
    int height) const {
  if (m_RenderCaps & FXRC_BYTEMASK_OUTPUT)
    return pDIB->Create(width, height, FXDIB_Format::k8bppMask);
#if defined(_SKIA_SUPPORT_PATHS_)
  constexpr FXDIB_Format kFormat = FXDIB_Format::kRgb32;
#else
  constexpr FXDIB_Format kFormat = CFX_DIBBase::kPlatformRGBFormat;
#endif
  return pDIB->Create(
      width, height,
      m_RenderCaps & FXRC_ALPHA_OUTPUT ? FXDIB_Format::kArgb : kFormat);
}

void CFX_RenderDevice::SetBaseClip(const FX_RECT& rect) {
  m_pDeviceDriver->SetBaseClip(rect);
}

bool CFX_RenderDevice::SetClip_PathFill(
    const CFX_Path* pPath,
    const CFX_Matrix* pObject2Device,
    const CFX_FillRenderOptions& fill_options) {
  if (!m_pDeviceDriver->SetClip_PathFill(pPath, pObject2Device, fill_options)) {
    return false;
  }
  UpdateClipBox();
  return true;
}

bool CFX_RenderDevice::SetClip_PathStroke(
    const CFX_Path* pPath,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState) {
  if (!m_pDeviceDriver->SetClip_PathStroke(pPath, pObject2Device,
                                           pGraphState)) {
    return false;
  }
  UpdateClipBox();
  return true;
}

bool CFX_RenderDevice::SetClip_Rect(const FX_RECT& rect) {
  CFX_Path path;
  path.AppendRect(rect.left, rect.bottom, rect.right, rect.top);
  if (!SetClip_PathFill(&path, nullptr,
                        CFX_FillRenderOptions::WindingOptions()))
    return false;

  UpdateClipBox();
  return true;
}

void CFX_RenderDevice::UpdateClipBox() {
  if (m_pDeviceDriver->GetClipBox(&m_ClipBox))
    return;
  m_ClipBox.left = 0;
  m_ClipBox.top = 0;
  m_ClipBox.right = m_Width;
  m_ClipBox.bottom = m_Height;
}

bool CFX_RenderDevice::DrawPath(const CFX_Path* pPath,
                                const CFX_Matrix* pObject2Device,
                                const CFX_GraphStateData* pGraphState,
                                uint32_t fill_color,
                                uint32_t stroke_color,
                                const CFX_FillRenderOptions& fill_options) {
  return DrawPathWithBlend(pPath, pObject2Device, pGraphState, fill_color,
                           stroke_color, fill_options, BlendMode::kNormal);
}

bool CFX_RenderDevice::DrawPathWithBlend(
    const CFX_Path* pPath,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState,
    uint32_t fill_color,
    uint32_t stroke_color,
    const CFX_FillRenderOptions& fill_options,
    BlendMode blend_type) {
  const bool fill =
      fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill;
  uint8_t fill_alpha = fill ? FXARGB_A(fill_color) : 0;
  uint8_t stroke_alpha = pGraphState ? FXARGB_A(stroke_color) : 0;
  pdfium::span<const CFX_Path::Point> points = pPath->GetPoints();
  if (stroke_alpha == 0 && points.size() == 2) {
    CFX_PointF pos1 = points[0].m_Point;
    CFX_PointF pos2 = points[1].m_Point;
    if (pObject2Device) {
      pos1 = pObject2Device->Transform(pos1);
      pos2 = pObject2Device->Transform(pos2);
    }
    DrawCosmeticLine(pos1, pos2, fill_color, fill_options, blend_type);
    return true;
  }

  if (stroke_alpha == 0 && !fill_options.rect_aa) {
    Optional<CFX_FloatRect> maybe_rect_f = pPath->GetRect(pObject2Device);
    if (maybe_rect_f.has_value()) {
      const CFX_FloatRect& rect_f = maybe_rect_f.value();
      FX_RECT rect_i = rect_f.GetOuterRect();

      // Depending on the top/bottom, left/right values of the rect it's
      // possible to overflow the Width() and Height() calculations. Check that
      // the rect will have valid dimension before continuing.
      if (!rect_i.Valid())
        return false;

      int width = static_cast<int>(ceil(rect_f.right - rect_f.left));
      if (width < 1) {
        width = 1;
        if (rect_i.left == rect_i.right)
          ++rect_i.right;
      }
      int height = static_cast<int>(ceil(rect_f.top - rect_f.bottom));
      if (height < 1) {
        height = 1;
        if (rect_i.bottom == rect_i.top)
          ++rect_i.bottom;
      }
      if (rect_i.Width() >= width + 1) {
        if (rect_f.left - static_cast<float>(rect_i.left) >
            static_cast<float>(rect_i.right) - rect_f.right) {
          ++rect_i.left;
        } else {
          --rect_i.right;
        }
      }
      if (rect_i.Height() >= height + 1) {
        if (rect_f.top - static_cast<float>(rect_i.top) >
            static_cast<float>(rect_i.bottom) - rect_f.bottom) {
          ++rect_i.top;
        } else {
          --rect_i.bottom;
        }
      }
      if (FillRectWithBlend(rect_i, fill_color, blend_type))
        return true;
    }
  }

  if (fill && stroke_alpha == 0 && !fill_options.stroke &&
      !fill_options.text_mode) {
    bool adjust = !!m_pDeviceDriver->GetDriverType();
    std::vector<CFX_Path::Point> sub_path;
    for (size_t i = 0; i < points.size(); i++) {
      CFX_Path::Point::Type point_type = points[i].m_Type;
      if (point_type == CFX_Path::Point::Type::kMove) {
        // Process the existing sub path.
        DrawZeroAreaPath(sub_path, pObject2Device, adjust,
                         fill_options.aliased_path, fill_color, fill_alpha,
                         blend_type);
        sub_path.clear();

        // Start forming the next sub path.
        sub_path.push_back(points[i]);
        continue;
      }

      if (point_type == CFX_Path::Point::Type::kBezier) {
        sub_path.push_back(points[i]);
        sub_path.push_back(points[i + 1]);
        sub_path.push_back(points[i + 2]);
        i += 2;
        continue;
      }

      DCHECK_EQ(point_type, CFX_Path::Point::Type::kLine);
      sub_path.push_back(points[i]);
      continue;
    }
    // Process the last sub paths.
    DrawZeroAreaPath(sub_path, pObject2Device, adjust,
                     fill_options.aliased_path, fill_color, fill_alpha,
                     blend_type);
  }

  if (fill && fill_alpha && stroke_alpha < 0xff && fill_options.stroke) {
    if (m_RenderCaps & FXRC_FILLSTROKE_PATH) {
      return m_pDeviceDriver->DrawPath(pPath, pObject2Device, pGraphState,
                                       fill_color, stroke_color, fill_options,
                                       blend_type);
    }
    return DrawFillStrokePath(pPath, pObject2Device, pGraphState, fill_color,
                              stroke_color, fill_options, blend_type);
  }
  return m_pDeviceDriver->DrawPath(pPath, pObject2Device, pGraphState,
                                   fill_color, stroke_color, fill_options,
                                   blend_type);
}

// This can be removed once PDFium entirely relies on Skia
bool CFX_RenderDevice::DrawFillStrokePath(
    const CFX_Path* pPath,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState,
    uint32_t fill_color,
    uint32_t stroke_color,
    const CFX_FillRenderOptions& fill_options,
    BlendMode blend_type) {
  if (!(m_RenderCaps & FXRC_GET_BITS))
    return false;
  CFX_FloatRect bbox;
  if (pGraphState) {
    bbox = pPath->GetBoundingBoxForStrokePath(pGraphState->m_LineWidth,
                                              pGraphState->m_MiterLimit);
  } else {
    bbox = pPath->GetBoundingBox();
  }
  if (pObject2Device)
    bbox = pObject2Device->TransformRect(bbox);

  FX_RECT rect = bbox.GetOuterRect();
  if (!rect.Valid())
    return false;

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  auto backdrop = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!CreateCompatibleBitmap(bitmap, rect.Width(), rect.Height()))
    return false;

  if (bitmap->IsAlphaFormat()) {
    bitmap->Clear(0);
    backdrop->Copy(bitmap);
  } else {
    if (!m_pDeviceDriver->GetDIBits(bitmap, rect.left, rect.top))
      return false;
    backdrop->Copy(bitmap);
  }
  CFX_DefaultRenderDevice bitmap_device;
  bitmap_device.Attach(bitmap, false, backdrop, true);

  CFX_Matrix matrix;
  if (pObject2Device)
    matrix = *pObject2Device;
  matrix.Translate(-rect.left, -rect.top);
  if (!bitmap_device.GetDeviceDriver()->DrawPath(pPath, &matrix, pGraphState,
                                                 fill_color, stroke_color,
                                                 fill_options, blend_type)) {
    return false;
  }
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  bitmap_device.GetDeviceDriver()->Flush();
#endif
  FX_RECT src_rect(0, 0, rect.Width(), rect.Height());
  return m_pDeviceDriver->SetDIBits(bitmap, 0, src_rect, rect.left, rect.top,
                                    BlendMode::kNormal);
}

bool CFX_RenderDevice::FillRectWithBlend(const FX_RECT& rect,
                                         uint32_t fill_color,
                                         BlendMode blend_type) {
  if (m_pDeviceDriver->FillRectWithBlend(rect, fill_color, blend_type))
    return true;

  if (!(m_RenderCaps & FXRC_GET_BITS))
    return false;

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!CreateCompatibleBitmap(bitmap, rect.Width(), rect.Height()))
    return false;

  if (!m_pDeviceDriver->GetDIBits(bitmap, rect.left, rect.top))
    return false;

  if (!bitmap->CompositeRect(0, 0, rect.Width(), rect.Height(), fill_color))
    return false;

  FX_RECT src_rect(0, 0, rect.Width(), rect.Height());
  m_pDeviceDriver->SetDIBits(bitmap, 0, src_rect, rect.left, rect.top,
                             BlendMode::kNormal);
  return true;
}

bool CFX_RenderDevice::DrawCosmeticLine(
    const CFX_PointF& ptMoveTo,
    const CFX_PointF& ptLineTo,
    uint32_t color,
    const CFX_FillRenderOptions& fill_options,
    BlendMode blend_type) {
  if ((color >= 0xff000000) && m_pDeviceDriver->DrawCosmeticLine(
                                   ptMoveTo, ptLineTo, color, blend_type)) {
    return true;
  }
  CFX_GraphStateData graph_state;
  CFX_Path path;
  path.AppendPoint(ptMoveTo, CFX_Path::Point::Type::kMove);
  path.AppendPoint(ptLineTo, CFX_Path::Point::Type::kLine);
  return m_pDeviceDriver->DrawPath(&path, nullptr, &graph_state, 0, color,
                                   fill_options, blend_type);
}

void CFX_RenderDevice::DrawZeroAreaPath(
    const std::vector<CFX_Path::Point>& path,
    const CFX_Matrix* matrix,
    bool adjust,
    bool aliased_path,
    uint32_t fill_color,
    uint8_t fill_alpha,
    BlendMode blend_type) {
  if (path.empty())
    return;

  CFX_Path new_path;
  bool thin = false;
  bool set_identity = false;

  if (!GetZeroAreaPath(path, matrix, adjust, &new_path, &thin, &set_identity))
    return;

  CFX_GraphStateData graph_state;
  graph_state.m_LineWidth = 0.0f;

  uint32_t stroke_color = fill_color;
  if (thin)
    stroke_color = (((fill_alpha >> 2) << 24) | (stroke_color & 0x00ffffff));

  const CFX_Matrix* new_matrix = nullptr;
  if (matrix && !matrix->IsIdentity() && !set_identity)
    new_matrix = matrix;

  CFX_FillRenderOptions path_options;
  path_options.zero_area = true;
  path_options.aliased_path = aliased_path;

  m_pDeviceDriver->DrawPath(&new_path, new_matrix, &graph_state, 0,
                            stroke_color, path_options, blend_type);
}

bool CFX_RenderDevice::GetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                                 int left,
                                 int top) {
  return (m_RenderCaps & FXRC_GET_BITS) &&
         m_pDeviceDriver->GetDIBits(pBitmap, left, top);
}

RetainPtr<CFX_DIBitmap> CFX_RenderDevice::GetBackDrop() {
  return m_pDeviceDriver->GetBackDrop();
}

bool CFX_RenderDevice::SetDIBitsWithBlend(const RetainPtr<CFX_DIBBase>& pBitmap,
                                          int left,
                                          int top,
                                          BlendMode blend_mode) {
  DCHECK(!pBitmap->IsMaskFormat());
  FX_RECT dest_rect(left, top, left + pBitmap->GetWidth(),
                    top + pBitmap->GetHeight());
  dest_rect.Intersect(m_ClipBox);
  if (dest_rect.IsEmpty())
    return true;

  FX_RECT src_rect(dest_rect.left - left, dest_rect.top - top,
                   dest_rect.left - left + dest_rect.Width(),
                   dest_rect.top - top + dest_rect.Height());
  if ((blend_mode == BlendMode::kNormal || (m_RenderCaps & FXRC_BLEND_MODE)) &&
      (!pBitmap->IsAlphaFormat() || (m_RenderCaps & FXRC_ALPHA_IMAGE))) {
    return m_pDeviceDriver->SetDIBits(pBitmap, 0, src_rect, dest_rect.left,
                                      dest_rect.top, blend_mode);
  }
  if (!(m_RenderCaps & FXRC_GET_BITS))
    return false;

  int bg_pixel_width = dest_rect.Width();
  int bg_pixel_height = dest_rect.Height();
  auto background = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!background->Create(bg_pixel_width, bg_pixel_height,
                          FXDIB_Format::kRgb32)) {
    return false;
  }
  if (!m_pDeviceDriver->GetDIBits(background, dest_rect.left, dest_rect.top))
    return false;

  if (!background->CompositeBitmap(0, 0, bg_pixel_width, bg_pixel_height,
                                   pBitmap, src_rect.left, src_rect.top,
                                   blend_mode, nullptr, false)) {
    return false;
  }
  FX_RECT rect(0, 0, bg_pixel_width, bg_pixel_height);
  return m_pDeviceDriver->SetDIBits(background, 0, rect, dest_rect.left,
                                    dest_rect.top, BlendMode::kNormal);
}

bool CFX_RenderDevice::StretchDIBitsWithFlagsAndBlend(
    const RetainPtr<CFX_DIBBase>& pBitmap,
    int left,
    int top,
    int dest_width,
    int dest_height,
    const FXDIB_ResampleOptions& options,
    BlendMode blend_mode) {
  FX_RECT dest_rect(left, top, left + dest_width, top + dest_height);
  FX_RECT clip_box = m_ClipBox;
  clip_box.Intersect(dest_rect);
  return clip_box.IsEmpty() || m_pDeviceDriver->StretchDIBits(
                                   pBitmap, 0, left, top, dest_width,
                                   dest_height, &clip_box, options, blend_mode);
}

bool CFX_RenderDevice::SetBitMask(const RetainPtr<CFX_DIBBase>& pBitmap,
                                  int left,
                                  int top,
                                  uint32_t argb) {
  FX_RECT src_rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
  return m_pDeviceDriver->SetDIBits(pBitmap, argb, src_rect, left, top,
                                    BlendMode::kNormal);
}

bool CFX_RenderDevice::StretchBitMask(const RetainPtr<CFX_DIBBase>& pBitmap,
                                      int left,
                                      int top,
                                      int dest_width,
                                      int dest_height,
                                      uint32_t color) {
  return StretchBitMaskWithFlags(pBitmap, left, top, dest_width, dest_height,
                                 color, FXDIB_ResampleOptions());
}

bool CFX_RenderDevice::StretchBitMaskWithFlags(
    const RetainPtr<CFX_DIBBase>& pBitmap,
    int left,
    int top,
    int dest_width,
    int dest_height,
    uint32_t argb,
    const FXDIB_ResampleOptions& options) {
  FX_RECT dest_rect(left, top, left + dest_width, top + dest_height);
  FX_RECT clip_box = m_ClipBox;
  clip_box.Intersect(dest_rect);
  return m_pDeviceDriver->StretchDIBits(pBitmap, argb, left, top, dest_width,
                                        dest_height, &clip_box, options,
                                        BlendMode::kNormal);
}

bool CFX_RenderDevice::StartDIBitsWithBlend(
    const RetainPtr<CFX_DIBBase>& pBitmap,
    int bitmap_alpha,
    uint32_t argb,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    std::unique_ptr<CFX_ImageRenderer>* handle,
    BlendMode blend_mode) {
  return m_pDeviceDriver->StartDIBits(pBitmap, bitmap_alpha, argb, matrix,
                                      options, handle, blend_mode);
}

bool CFX_RenderDevice::ContinueDIBits(CFX_ImageRenderer* handle,
                                      PauseIndicatorIface* pPause) {
  return m_pDeviceDriver->ContinueDIBits(handle, pPause);
}

#if defined(_SKIA_SUPPORT_)
void CFX_RenderDevice::DebugVerifyBitmapIsPreMultiplied() const {
  NOTREACHED();
}

bool CFX_RenderDevice::SetBitsWithMask(const RetainPtr<CFX_DIBBase>& pBitmap,
                                       const RetainPtr<CFX_DIBBase>& pMask,
                                       int left,
                                       int top,
                                       int bitmap_alpha,
                                       BlendMode blend_type) {
  return m_pDeviceDriver->SetBitsWithMask(pBitmap, pMask, left, top,
                                          bitmap_alpha, blend_type);
}
#endif

bool CFX_RenderDevice::DrawNormalText(int nChars,
                                      const TextCharPos* pCharPos,
                                      CFX_Font* pFont,
                                      float font_size,
                                      const CFX_Matrix& mtText2Device,
                                      uint32_t fill_color,
                                      const CFX_TextRenderOptions& options) {
  // |anti_alias| and |normalize| don't affect Skia/SkiaPaths rendering.
  int anti_alias = FT_RENDER_MODE_MONO;
  bool normalize = false;
  const bool is_text_smooth = options.IsSmooth();
  // |text_options| has the potential to affect all derived classes of
  // RenderDeviceDriverIface. But now it only affects Skia rendering.
  CFX_TextRenderOptions text_options(options);
  if (is_text_smooth) {
    if (GetDeviceType() == DeviceType::kDisplay && m_bpp > 1) {
      if (!CFX_GEModule::Get()->GetFontMgr()->FTLibrarySupportsHinting()) {
        // Some Freetype implementations (like the one packaged with Fedora) do
        // not support hinting due to patents 6219025, 6239783, 6307566,
        // 6225973, 6243070, 6393145, 6421054, 6282327, and 6624828; the latest
        // one expires 10/7/19.  This makes LCD anti-aliasing very ugly, so we
        // instead fall back on NORMAL anti-aliasing.
        anti_alias = FT_RENDER_MODE_NORMAL;
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
        // Since |anti_alias| doesn't affect Skia rendering, and Skia only
        // follows strictly to the options provided by |text_options|, we need
        // to update |text_options| so that Skia falls back on normal
        // anti-aliasing as well.
        text_options.aliasing_type = CFX_TextRenderOptions::kAntiAliasing;
#endif
      } else if ((m_RenderCaps & FXRC_ALPHA_OUTPUT)) {
        // Whether Skia uses LCD optimization should strictly follow the
        // rendering options provided by |text_options|. No change needs to be
        // done for |text_options| here.
        anti_alias = FT_RENDER_MODE_LCD;
        normalize = true;
      } else if (m_bpp < 16) {
        // This case doesn't apply to Skia since Skia always have |m_bpp| = 32.
        anti_alias = FT_RENDER_MODE_NORMAL;
      } else {
        // Whether Skia uses LCD optimization should strictly follow the
        // rendering options provided by |text_options|. No change needs to be
        // done for |text_options| here.
        anti_alias = FT_RENDER_MODE_LCD;
        normalize = !pFont->GetFaceRec() ||
                    options.aliasing_type != CFX_TextRenderOptions::kLcd;
      }
    }
  }

  if (GetDeviceType() != DeviceType::kDisplay) {
    if (ShouldDrawDeviceText(pFont, options) &&
        m_pDeviceDriver->DrawDeviceText(nChars, pCharPos, pFont, mtText2Device,
                                        font_size, fill_color, text_options)) {
      return true;
    }
    if (FXARGB_A(fill_color) < 255)
      return false;
  } else if (options.native_text) {
    if (ShouldDrawDeviceText(pFont, options) &&
        m_pDeviceDriver->DrawDeviceText(nChars, pCharPos, pFont, mtText2Device,
                                        font_size, fill_color, text_options)) {
      return true;
    }
  }

  CFX_Matrix char2device = mtText2Device;
  CFX_Matrix text2Device = mtText2Device;
  char2device.Scale(font_size, -font_size);
  if (fabs(char2device.a) + fabs(char2device.b) > 50 * 1.0f ||
      GetDeviceType() == DeviceType::kPrinter) {
    if (pFont->GetFaceRec()) {
      CFX_FillRenderOptions path_options;
      path_options.aliased_path = !is_text_smooth;
      return DrawTextPath(nChars, pCharPos, pFont, font_size, mtText2Device,
                          nullptr, nullptr, fill_color, 0, nullptr,
                          path_options);
    }
  }
  std::vector<TextGlyphPos> glyphs(nChars);
  CFX_Matrix deviceCtm = char2device;

  for (size_t i = 0; i < glyphs.size(); ++i) {
    TextGlyphPos& glyph = glyphs[i];
    const TextCharPos& charpos = pCharPos[i];

    glyph.m_fDeviceOrigin = text2Device.Transform(charpos.m_Origin);
    if (anti_alias < FT_RENDER_MODE_LCD)
      glyph.m_Origin.x = FXSYS_roundf(glyph.m_fDeviceOrigin.x);
    else
      glyph.m_Origin.x = static_cast<int>(floor(glyph.m_fDeviceOrigin.x));
    glyph.m_Origin.y = FXSYS_roundf(glyph.m_fDeviceOrigin.y);

    if (charpos.m_bGlyphAdjust) {
      CFX_Matrix new_matrix(
          charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
          charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3], 0, 0);
      new_matrix.Concat(deviceCtm);
      glyph.m_pGlyph = pFont->LoadGlyphBitmap(
          charpos.m_GlyphIndex, charpos.m_bFontStyle, new_matrix,
          charpos.m_FontCharWidth, anti_alias, &text_options);
    } else {
      glyph.m_pGlyph = pFont->LoadGlyphBitmap(
          charpos.m_GlyphIndex, charpos.m_bFontStyle, deviceCtm,
          charpos.m_FontCharWidth, anti_alias, &text_options);
    }
  }
  if (anti_alias < FT_RENDER_MODE_LCD && glyphs.size() > 1)
    AdjustGlyphSpace(&glyphs);

  FX_RECT bmp_rect = GetGlyphsBBox(glyphs, anti_alias);
  bmp_rect.Intersect(m_ClipBox);
  if (bmp_rect.IsEmpty())
    return true;

  int pixel_width = bmp_rect.Width();
  int pixel_height = bmp_rect.Height();
  int pixel_left = bmp_rect.left;
  int pixel_top = bmp_rect.top;
  if (anti_alias == FT_RENDER_MODE_MONO) {
    auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!bitmap->Create(pixel_width, pixel_height, FXDIB_Format::k1bppMask))
      return false;
    bitmap->Clear(0);
    for (const TextGlyphPos& glyph : glyphs) {
      if (!glyph.m_pGlyph)
        continue;

      Optional<CFX_Point> point = glyph.GetOrigin({pixel_left, pixel_top});
      if (!point.has_value())
        continue;

      const RetainPtr<CFX_DIBitmap>& pGlyph = glyph.m_pGlyph->GetBitmap();
      bitmap->TransferBitmap(point.value().x, point.value().y,
                             pGlyph->GetWidth(), pGlyph->GetHeight(), pGlyph, 0,
                             0);
    }
    return SetBitMask(bitmap, bmp_rect.left, bmp_rect.top, fill_color);
  }
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (m_bpp == 8) {
    if (!bitmap->Create(pixel_width, pixel_height, FXDIB_Format::k8bppMask))
      return false;
  } else {
    if (!CreateCompatibleBitmap(bitmap, pixel_width, pixel_height))
      return false;
  }
  if (!bitmap->IsAlphaFormat() && !bitmap->IsMaskFormat()) {
    bitmap->Clear(0xFFFFFFFF);
    if (!GetDIBits(bitmap, bmp_rect.left, bmp_rect.top))
      return false;
  } else {
    bitmap->Clear(0);
    if (bitmap->HasAlphaMask())
      bitmap->GetAlphaMask()->Clear(0);
  }
  int dest_width = pixel_width;
  int a = 0;
  int r = 0;
  int g = 0;
  int b = 0;
  if (anti_alias == FT_RENDER_MODE_LCD)
    std::tie(a, r, g, b) = ArgbDecode(fill_color);

  for (const TextGlyphPos& glyph : glyphs) {
    if (!glyph.m_pGlyph)
      continue;

    Optional<CFX_Point> point = glyph.GetOrigin({pixel_left, pixel_top});
    if (!point.has_value())
      continue;

    const RetainPtr<CFX_DIBitmap>& pGlyph = glyph.m_pGlyph->GetBitmap();
    int ncols = pGlyph->GetWidth();
    int nrows = pGlyph->GetHeight();
    if (anti_alias == FT_RENDER_MODE_NORMAL) {
      if (!bitmap->CompositeMask(point.value().x, point.value().y, ncols, nrows,
                                 pGlyph, fill_color, 0, 0, BlendMode::kNormal,
                                 nullptr, false)) {
        return false;
      }
      continue;
    }
    ncols /= 3;
    int x_subpixel = static_cast<int>(glyph.m_fDeviceOrigin.x * 3) % 3;
    int start_col = std::max(point->x, 0);
    FX_SAFE_INT32 end_col_safe = point->x;
    end_col_safe += ncols;
    if (!end_col_safe.IsValid())
      continue;

    int end_col = std::min<int>(end_col_safe.ValueOrDie(), dest_width);
    if (start_col >= end_col)
      continue;

    DrawNormalTextHelper(bitmap, pGlyph, nrows, point->x, point->y, start_col,
                         end_col, normalize, x_subpixel, a, r, g, b);
  }
  if (bitmap->IsMaskFormat())
    SetBitMask(bitmap, bmp_rect.left, bmp_rect.top, fill_color);
  else
    SetDIBits(bitmap, bmp_rect.left, bmp_rect.top);
  return true;
}

bool CFX_RenderDevice::DrawTextPath(int nChars,
                                    const TextCharPos* pCharPos,
                                    CFX_Font* pFont,
                                    float font_size,
                                    const CFX_Matrix& mtText2User,
                                    const CFX_Matrix* pUser2Device,
                                    const CFX_GraphStateData* pGraphState,
                                    uint32_t fill_color,
                                    FX_ARGB stroke_color,
                                    CFX_Path* pClippingPath,
                                    const CFX_FillRenderOptions& fill_options) {
  for (int iChar = 0; iChar < nChars; ++iChar) {
    const TextCharPos& charpos = pCharPos[iChar];
    CFX_Matrix matrix;
    if (charpos.m_bGlyphAdjust) {
      matrix = CFX_Matrix(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                          charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3],
                          0, 0);
    }
    matrix.Concat(CFX_Matrix(font_size, 0, 0, font_size, charpos.m_Origin.x,
                             charpos.m_Origin.y));
    const CFX_Path* pPath =
        pFont->LoadGlyphPath(charpos.m_GlyphIndex, charpos.m_FontCharWidth);
    if (!pPath)
      continue;

    matrix.Concat(mtText2User);

    CFX_Path TransformedPath(*pPath);
    TransformedPath.Transform(matrix);
    if (fill_color || stroke_color) {
      CFX_FillRenderOptions options(fill_options);
      if (fill_color)
        options.fill_type = CFX_FillRenderOptions::FillType::kWinding;
      options.text_mode = true;
      if (!DrawPathWithBlend(&TransformedPath, pUser2Device, pGraphState,
                             fill_color, stroke_color, options,
                             BlendMode::kNormal)) {
        return false;
      }
    }
    if (pClippingPath)
      pClippingPath->Append(TransformedPath, pUser2Device);
  }
  return true;
}

void CFX_RenderDevice::DrawFillRect(const CFX_Matrix* pUser2Device,
                                    const CFX_FloatRect& rect,
                                    const FX_COLORREF& color) {
  CFX_Path path;
  path.AppendFloatRect(rect);
  DrawPath(&path, pUser2Device, nullptr, color, 0,
           CFX_FillRenderOptions::WindingOptions());
}

void CFX_RenderDevice::DrawFillArea(const CFX_Matrix& mtUser2Device,
                                    const std::vector<CFX_PointF>& points,
                                    const FX_COLORREF& color) {
  DCHECK(!points.empty());
  CFX_Path path;
  path.AppendPoint(points[0], CFX_Path::Point::Type::kMove);
  for (size_t i = 1; i < points.size(); ++i)
    path.AppendPoint(points[i], CFX_Path::Point::Type::kLine);

  DrawPath(&path, &mtUser2Device, nullptr, color, 0,
           CFX_FillRenderOptions::EvenOddOptions());
}

void CFX_RenderDevice::DrawStrokeRect(const CFX_Matrix& mtUser2Device,
                                      const CFX_FloatRect& rect,
                                      const FX_COLORREF& color,
                                      float fWidth) {
  CFX_GraphStateData gsd;
  gsd.m_LineWidth = fWidth;

  CFX_Path path;
  path.AppendFloatRect(rect);
  DrawPath(&path, &mtUser2Device, &gsd, 0, color,
           CFX_FillRenderOptions::EvenOddOptions());
}

void CFX_RenderDevice::DrawStrokeLine(const CFX_Matrix* pUser2Device,
                                      const CFX_PointF& ptMoveTo,
                                      const CFX_PointF& ptLineTo,
                                      const FX_COLORREF& color,
                                      float fWidth) {
  CFX_Path path;
  path.AppendPoint(ptMoveTo, CFX_Path::Point::Type::kMove);
  path.AppendPoint(ptLineTo, CFX_Path::Point::Type::kLine);

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = fWidth;

  DrawPath(&path, pUser2Device, &gsd, 0, color,
           CFX_FillRenderOptions::EvenOddOptions());
}

void CFX_RenderDevice::DrawFillRect(const CFX_Matrix* pUser2Device,
                                    const CFX_FloatRect& rect,
                                    const CFX_Color& color,
                                    int32_t nTransparency) {
  DrawFillRect(pUser2Device, rect, color.ToFXColor(nTransparency));
}

void CFX_RenderDevice::DrawShadow(const CFX_Matrix& mtUser2Device,
                                  bool bVertical,
                                  bool bHorizontal,
                                  const CFX_FloatRect& rect,
                                  int32_t nTransparency,
                                  int32_t nStartGray,
                                  int32_t nEndGray) {
  constexpr float kBorder = 0.5f;
  constexpr float kSegmentWidth = 1.0f;
  constexpr float kLineWidth = 1.5f;

  if (bVertical) {
    float fStepGray = (nEndGray - nStartGray) / rect.Height();
    CFX_PointF start(rect.left, 0);
    CFX_PointF end(rect.right, 0);

    for (float fy = rect.bottom + kBorder; fy <= rect.top - kBorder;
         fy += kSegmentWidth) {
      start.y = fy;
      end.y = fy;
      int nGray = nStartGray + static_cast<int>(fStepGray * (fy - rect.bottom));
      FX_ARGB color = ArgbEncode(nTransparency, nGray, nGray, nGray);
      DrawStrokeLine(&mtUser2Device, start, end, color, kLineWidth);
    }
  }

  if (bHorizontal) {
    float fStepGray = (nEndGray - nStartGray) / rect.Width();
    CFX_PointF start(0, rect.bottom);
    CFX_PointF end(0, rect.top);

    for (float fx = rect.left + kBorder; fx <= rect.right - kBorder;
         fx += kSegmentWidth) {
      start.x = fx;
      end.x = fx;
      int nGray = nStartGray + static_cast<int>(fStepGray * (fx - rect.left));
      FX_ARGB color = ArgbEncode(nTransparency, nGray, nGray, nGray);
      DrawStrokeLine(&mtUser2Device, start, end, color, kLineWidth);
    }
  }
}

void CFX_RenderDevice::DrawBorder(const CFX_Matrix* pUser2Device,
                                  const CFX_FloatRect& rect,
                                  float fWidth,
                                  const CFX_Color& color,
                                  const CFX_Color& crLeftTop,
                                  const CFX_Color& crRightBottom,
                                  BorderStyle nStyle,
                                  int32_t nTransparency) {
  if (fWidth <= 0.0f)
    return;

  const float fLeft = rect.left;
  const float fRight = rect.right;
  const float fTop = rect.top;
  const float fBottom = rect.bottom;
  const float fHalfWidth = fWidth / 2.0f;

  switch (nStyle) {
    default:
    case BorderStyle::kSolid: {
      CFX_Path path;
      path.AppendRect(fLeft, fBottom, fRight, fTop);
      path.AppendRect(fLeft + fWidth, fBottom + fWidth, fRight - fWidth,
                      fTop - fWidth);
      DrawPath(&path, pUser2Device, nullptr, color.ToFXColor(nTransparency), 0,
               CFX_FillRenderOptions::EvenOddOptions());
      break;
    }
    case BorderStyle::kDash: {
      CFX_GraphStateData gsd;
      gsd.m_DashArray = {3.0f, 3.0f};
      gsd.m_DashPhase = 0;
      gsd.m_LineWidth = fWidth;

      CFX_Path path;
      path.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
                       CFX_Path::Point::Type::kMove);
      path.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fTop - fHalfWidth),
                       CFX_Path::Point::Type::kLine);
      path.AppendPoint(CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
                       CFX_Path::Point::Type::kLine);
      path.AppendPoint(CFX_PointF(fRight - fHalfWidth, fBottom + fHalfWidth),
                       CFX_Path::Point::Type::kLine);
      path.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
                       CFX_Path::Point::Type::kLine);
      DrawPath(&path, pUser2Device, &gsd, 0, color.ToFXColor(nTransparency),
               CFX_FillRenderOptions::WindingOptions());
      break;
    }
    case BorderStyle::kBeveled:
    case BorderStyle::kInset: {
      CFX_GraphStateData gsd;
      gsd.m_LineWidth = fHalfWidth;

      CFX_Path path_left_top;
      path_left_top.AppendPoint(
          CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
          CFX_Path::Point::Type::kMove);
      path_left_top.AppendPoint(
          CFX_PointF(fLeft + fHalfWidth, fTop - fHalfWidth),
          CFX_Path::Point::Type::kLine);
      path_left_top.AppendPoint(
          CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
          CFX_Path::Point::Type::kLine);
      path_left_top.AppendPoint(CFX_PointF(fRight - fWidth, fTop - fWidth),
                                CFX_Path::Point::Type::kLine);
      path_left_top.AppendPoint(CFX_PointF(fLeft + fWidth, fTop - fWidth),
                                CFX_Path::Point::Type::kLine);
      path_left_top.AppendPoint(CFX_PointF(fLeft + fWidth, fBottom + fWidth),
                                CFX_Path::Point::Type::kLine);
      path_left_top.AppendPoint(
          CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
          CFX_Path::Point::Type::kLine);
      DrawPath(&path_left_top, pUser2Device, &gsd,
               crLeftTop.ToFXColor(nTransparency), 0,
               CFX_FillRenderOptions::EvenOddOptions());

      CFX_Path path_right_bottom;
      path_right_bottom.AppendPoint(
          CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
          CFX_Path::Point::Type::kMove);
      path_right_bottom.AppendPoint(
          CFX_PointF(fRight - fHalfWidth, fBottom + fHalfWidth),
          CFX_Path::Point::Type::kLine);
      path_right_bottom.AppendPoint(
          CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
          CFX_Path::Point::Type::kLine);
      path_right_bottom.AppendPoint(
          CFX_PointF(fLeft + fWidth, fBottom + fWidth),
          CFX_Path::Point::Type::kLine);
      path_right_bottom.AppendPoint(
          CFX_PointF(fRight - fWidth, fBottom + fWidth),
          CFX_Path::Point::Type::kLine);
      path_right_bottom.AppendPoint(CFX_PointF(fRight - fWidth, fTop - fWidth),
                                    CFX_Path::Point::Type::kLine);
      path_right_bottom.AppendPoint(
          CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
          CFX_Path::Point::Type::kLine);
      DrawPath(&path_right_bottom, pUser2Device, &gsd,
               crRightBottom.ToFXColor(nTransparency), 0,
               CFX_FillRenderOptions::EvenOddOptions());

      CFX_Path path;
      path.AppendRect(fLeft, fBottom, fRight, fTop);
      path.AppendRect(fLeft + fHalfWidth, fBottom + fHalfWidth,
                      fRight - fHalfWidth, fTop - fHalfWidth);
      DrawPath(&path, pUser2Device, &gsd, color.ToFXColor(nTransparency), 0,
               CFX_FillRenderOptions::EvenOddOptions());
      break;
    }
    case BorderStyle::kUnderline: {
      CFX_GraphStateData gsd;
      gsd.m_LineWidth = fWidth;

      CFX_Path path;
      path.AppendPoint(CFX_PointF(fLeft, fBottom + fHalfWidth),
                       CFX_Path::Point::Type::kMove);
      path.AppendPoint(CFX_PointF(fRight, fBottom + fHalfWidth),
                       CFX_Path::Point::Type::kLine);
      DrawPath(&path, pUser2Device, &gsd, 0, color.ToFXColor(nTransparency),
               CFX_FillRenderOptions::EvenOddOptions());
      break;
    }
  }
}

CFX_RenderDevice::StateRestorer::StateRestorer(CFX_RenderDevice* pDevice)
    : m_pDevice(pDevice) {
  m_pDevice->SaveState();
}

CFX_RenderDevice::StateRestorer::~StateRestorer() {
  m_pDevice->RestoreState(false);
}
