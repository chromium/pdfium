// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cfx_graphics.h"

#include <memory>

#include "core/fxge/cfx_fxgedevice.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_unicodeencoding.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxgraphics/cfx_color.h"
#include "xfa/fxgraphics/cfx_path.h"
#include "xfa/fxgraphics/cfx_pattern.h"
#include "xfa/fxgraphics/cfx_shading.h"

namespace {

enum {
  FX_CONTEXT_None = 0,
  FX_CONTEXT_Device,
};

#define FX_HATCHSTYLE_Total 53

struct FX_HATCHDATA {
  int32_t width;
  int32_t height;
  uint8_t maskBits[64];
};

const FX_HATCHDATA hatchBitmapData[FX_HATCHSTYLE_Total] = {
    {16,  // Horizontal
     16,
     {
         0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
         0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     }},
    {16,  // Vertical
     16,
     {
         0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00,
         0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80,
         0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80,
         0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
         0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00,
         0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
     }},
    {16,  // ForwardDiagonal
     16,
     {
         0x80, 0x80, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x20, 0x20, 0x00,
         0x00, 0x10, 0x10, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x04, 0x04,
         0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x80,
         0x80, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00,
         0x10, 0x10, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x04, 0x04, 0x00,
         0x00, 0x02, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
     }},
    {16,  // BackwardDiagonal
     16,
     {
         0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x04, 0x04, 0x00,
         0x00, 0x08, 0x08, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x20, 0x20,
         0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x01,
         0x01, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00,
         0x08, 0x08, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x20, 0x20, 0x00,
         0x00, 0x40, 0x40, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
     }},
    {16,  // Cross
     16,
     {
         0xff, 0xff, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00,
         0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80,
         0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0xff,
         0xff, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
         0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00,
         0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
     }},
    {16,  // DiagonalCross
     16,
     {
         0x81, 0x81, 0x00, 0x00, 0x42, 0x42, 0x00, 0x00, 0x24, 0x24, 0x00,
         0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x24, 0x24,
         0x00, 0x00, 0x42, 0x42, 0x00, 0x00, 0x81, 0x81, 0x00, 0x00, 0x81,
         0x81, 0x00, 0x00, 0x42, 0x42, 0x00, 0x00, 0x24, 0x24, 0x00, 0x00,
         0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x24, 0x24, 0x00,
         0x00, 0x42, 0x42, 0x00, 0x00, 0x81, 0x81, 0x00, 0x00,
     }},
};

}  // namespace

CFX_Graphics::CFX_Graphics(CFX_RenderDevice* renderDevice)
    : m_type(FX_CONTEXT_None), m_renderDevice(renderDevice) {
  if (!renderDevice)
    return;
  m_type = FX_CONTEXT_Device;
}

CFX_Graphics::~CFX_Graphics() {}

void CFX_Graphics::SaveGraphState() {
  if (m_type != FX_CONTEXT_Device || !m_renderDevice)
    return;

  m_renderDevice->SaveState();
  m_infoStack.push_back(pdfium::MakeUnique<TInfo>(m_info));
}

void CFX_Graphics::RestoreGraphState() {
  if (m_type != FX_CONTEXT_Device || !m_renderDevice)
    return;

  m_renderDevice->RestoreState(false);
  if (m_infoStack.empty() || !m_infoStack.back())
    return;

  m_info = *m_infoStack.back();
  m_infoStack.pop_back();
  return;
}

void CFX_Graphics::SetLineCap(CFX_GraphStateData::LineCap lineCap) {
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    m_info.graphState.m_LineCap = lineCap;
  }
}

void CFX_Graphics::SetLineDash(FX_FLOAT dashPhase,
                               FX_FLOAT* dashArray,
                               int32_t dashCount) {
  if (dashCount > 0 && !dashArray)
    return;

  dashCount = dashCount < 0 ? 0 : dashCount;
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    FX_FLOAT scale = 1.0;
    if (m_info.isActOnDash) {
      scale = m_info.graphState.m_LineWidth;
    }
    m_info.graphState.m_DashPhase = dashPhase;
    m_info.graphState.SetDashCount(dashCount);
    for (int32_t i = 0; i < dashCount; i++) {
      m_info.graphState.m_DashArray[i] = dashArray[i] * scale;
    }
  }
}

void CFX_Graphics::SetLineDash(FX_DashStyle dashStyle) {
  if (m_type == FX_CONTEXT_Device && m_renderDevice)
    RenderDeviceSetLineDash(dashStyle);
}

void CFX_Graphics::SetLineWidth(FX_FLOAT lineWidth, bool isActOnDash) {
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    m_info.graphState.m_LineWidth = lineWidth;
    m_info.isActOnDash = isActOnDash;
  }
}

void CFX_Graphics::SetStrokeColor(CFX_Color* color) {
  if (!color)
    return;
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    m_info.strokeColor = color;
  }
}

void CFX_Graphics::SetFillColor(CFX_Color* color) {
  if (!color)
    return;
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    m_info.fillColor = color;
  }
}

void CFX_Graphics::StrokePath(CFX_Path* path, CFX_Matrix* matrix) {
  if (!path)
    return;
  if (m_type == FX_CONTEXT_Device && m_renderDevice)
    RenderDeviceStrokePath(path, matrix);
}

void CFX_Graphics::FillPath(CFX_Path* path,
                            FX_FillMode fillMode,
                            CFX_Matrix* matrix) {
  if (!path)
    return;
  if (m_type == FX_CONTEXT_Device && m_renderDevice)
    RenderDeviceFillPath(path, fillMode, matrix);
}

void CFX_Graphics::StretchImage(CFX_DIBSource* source,
                                const CFX_RectF& rect,
                                CFX_Matrix* matrix) {
  if (!source)
    return;
  if (m_type == FX_CONTEXT_Device && m_renderDevice)
    RenderDeviceStretchImage(source, rect, matrix);
}

void CFX_Graphics::ConcatMatrix(const CFX_Matrix* matrix) {
  if (!matrix)
    return;
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    m_info.CTM.Concat(*matrix);
  }
}

CFX_Matrix* CFX_Graphics::GetMatrix() {
  if (m_type == FX_CONTEXT_Device && m_renderDevice)
    return &m_info.CTM;
  return nullptr;
}

CFX_RectF CFX_Graphics::GetClipRect() const {
  if (m_type != FX_CONTEXT_Device || !m_renderDevice)
    return CFX_RectF();

  FX_RECT r = m_renderDevice->GetClipBox();
  return CFX_Rect(r.left, r.top, r.Width(), r.Height()).As<FX_FLOAT>();
}

void CFX_Graphics::SetClipRect(const CFX_RectF& rect) {
  if (m_type == FX_CONTEXT_Device && m_renderDevice) {
    m_renderDevice->SetClip_Rect(
        FX_RECT(FXSYS_round(rect.left), FXSYS_round(rect.top),
                FXSYS_round(rect.right()), FXSYS_round(rect.bottom())));
  }
}

CFX_RenderDevice* CFX_Graphics::GetRenderDevice() {
  return m_renderDevice;
}

void CFX_Graphics::RenderDeviceSetLineDash(FX_DashStyle dashStyle) {
  switch (dashStyle) {
    case FX_DASHSTYLE_Solid: {
      m_info.graphState.SetDashCount(0);
      return;
    }
    case FX_DASHSTYLE_Dash: {
      FX_FLOAT dashArray[] = {3, 1};
      SetLineDash(0, dashArray, 2);
      return;
    }
    case FX_DASHSTYLE_Dot: {
      FX_FLOAT dashArray[] = {1, 1};
      SetLineDash(0, dashArray, 2);
      return;
    }
    case FX_DASHSTYLE_DashDot: {
      FX_FLOAT dashArray[] = {3, 1, 1, 1};
      SetLineDash(0, dashArray, 4);
      return;
    }
    case FX_DASHSTYLE_DashDotDot: {
      FX_FLOAT dashArray[] = {4, 1, 2, 1, 2, 1};
      SetLineDash(0, dashArray, 6);
      return;
    }
    default:
      return;
  }
}

void CFX_Graphics::RenderDeviceStrokePath(CFX_Path* path, CFX_Matrix* matrix) {
  if (!m_info.strokeColor)
    return;
  CFX_Matrix m(m_info.CTM.a, m_info.CTM.b, m_info.CTM.c, m_info.CTM.d,
               m_info.CTM.e, m_info.CTM.f);
  if (matrix) {
    m.Concat(*matrix);
  }
  switch (m_info.strokeColor->m_type) {
    case FX_COLOR_Solid: {
      m_renderDevice->DrawPath(path->GetPathData(), &m, &m_info.graphState, 0x0,
                               m_info.strokeColor->m_info.argb, 0);
      return;
    }
    default:
      return;
  }
}

void CFX_Graphics::RenderDeviceFillPath(CFX_Path* path,
                                        FX_FillMode fillMode,
                                        CFX_Matrix* matrix) {
  if (!m_info.fillColor)
    return;
  CFX_Matrix m(m_info.CTM.a, m_info.CTM.b, m_info.CTM.c, m_info.CTM.d,
               m_info.CTM.e, m_info.CTM.f);
  if (matrix) {
    m.Concat(*matrix);
  }
  switch (m_info.fillColor->m_type) {
    case FX_COLOR_Solid: {
      m_renderDevice->DrawPath(path->GetPathData(), &m, &m_info.graphState,
                               m_info.fillColor->m_info.argb, 0x0, fillMode);
      return;
    }
    case FX_COLOR_Pattern:
      FillPathWithPattern(path, fillMode, &m);
      return;
    case FX_COLOR_Shading:
      FillPathWithShading(path, fillMode, &m);
      return;
    default:
      return;
  }
}

void CFX_Graphics::RenderDeviceStretchImage(CFX_DIBSource* source,
                                            const CFX_RectF& rect,
                                            CFX_Matrix* matrix) {
  CFX_Matrix m1(m_info.CTM.a, m_info.CTM.b, m_info.CTM.c, m_info.CTM.d,
                m_info.CTM.e, m_info.CTM.f);
  if (matrix) {
    m1.Concat(*matrix);
  }
  std::unique_ptr<CFX_DIBitmap> bmp1 =
      source->StretchTo((int32_t)rect.Width(), (int32_t)rect.Height());
  CFX_Matrix m2(rect.Width(), 0.0, 0.0, rect.Height(), rect.left, rect.top);
  m2.Concat(m1);

  int32_t left;
  int32_t top;
  std::unique_ptr<CFX_DIBitmap> bmp2 = bmp1->FlipImage(false, true);
  std::unique_ptr<CFX_DIBitmap> bmp3 = bmp2->TransformTo(&m2, left, top);
  CFX_RectF r = GetClipRect();
  CFX_DIBitmap* bitmap = m_renderDevice->GetBitmap();
  bitmap->CompositeBitmap(FXSYS_round(r.left), FXSYS_round(r.top),
                          FXSYS_round(r.Width()), FXSYS_round(r.Height()),
                          bmp3.get(), FXSYS_round(r.left - left),
                          FXSYS_round(r.top - top));
}

void CFX_Graphics::FillPathWithPattern(CFX_Path* path,
                                       FX_FillMode fillMode,
                                       CFX_Matrix* matrix) {
  CFX_Pattern* pattern = m_info.fillColor->m_info.pattern;
  CFX_DIBitmap* bitmap = m_renderDevice->GetBitmap();
  int32_t width = bitmap->GetWidth();
  int32_t height = bitmap->GetHeight();
  CFX_DIBitmap bmp;
  bmp.Create(width, height, FXDIB_Argb);
  m_renderDevice->GetDIBits(&bmp, 0, 0);

  FX_HatchStyle hatchStyle = m_info.fillColor->m_info.pattern->m_hatchStyle;
  const FX_HATCHDATA& data = hatchBitmapData[static_cast<int>(hatchStyle)];

  CFX_DIBitmap mask;
  mask.Create(data.width, data.height, FXDIB_1bppMask);
  FXSYS_memcpy(mask.GetBuffer(), data.maskBits, mask.GetPitch() * data.height);
  CFX_FloatRect rectf = path->GetPathData()->GetBoundingBox();
  if (matrix)
    matrix->TransformRect(rectf);

  FX_RECT rect(FXSYS_round(rectf.left), FXSYS_round(rectf.top),
               FXSYS_round(rectf.right), FXSYS_round(rectf.bottom));
  CFX_FxgeDevice device;
  device.Attach(&bmp, false, nullptr, false);
  device.FillRect(&rect, m_info.fillColor->m_info.pattern->m_backArgb);
  for (int32_t j = rect.bottom; j < rect.top; j += mask.GetHeight()) {
    for (int32_t i = rect.left; i < rect.right; i += mask.GetWidth()) {
      device.SetBitMask(&mask, i, j,
                        m_info.fillColor->m_info.pattern->m_foreArgb);
    }
  }

  m_renderDevice->SaveState();
  m_renderDevice->SetClip_PathFill(path->GetPathData(), matrix, fillMode);
  SetDIBitsWithMatrix(&bmp, &pattern->m_matrix);
  m_renderDevice->RestoreState(false);
}

void CFX_Graphics::FillPathWithShading(CFX_Path* path,
                                       FX_FillMode fillMode,
                                       CFX_Matrix* matrix) {
  CFX_DIBitmap* bitmap = m_renderDevice->GetBitmap();
  int32_t width = bitmap->GetWidth();
  int32_t height = bitmap->GetHeight();
  FX_FLOAT start_x = m_info.fillColor->m_shading->m_beginPoint.x;
  FX_FLOAT start_y = m_info.fillColor->m_shading->m_beginPoint.y;
  FX_FLOAT end_x = m_info.fillColor->m_shading->m_endPoint.x;
  FX_FLOAT end_y = m_info.fillColor->m_shading->m_endPoint.y;
  CFX_DIBitmap bmp;
  bmp.Create(width, height, FXDIB_Argb);
  m_renderDevice->GetDIBits(&bmp, 0, 0);
  int32_t pitch = bmp.GetPitch();
  bool result = false;
  switch (m_info.fillColor->m_shading->m_type) {
    case FX_SHADING_Axial: {
      FX_FLOAT x_span = end_x - start_x;
      FX_FLOAT y_span = end_y - start_y;
      FX_FLOAT axis_len_square = (x_span * x_span) + (y_span * y_span);
      for (int32_t row = 0; row < height; row++) {
        uint32_t* dib_buf = (uint32_t*)(bmp.GetBuffer() + row * pitch);
        for (int32_t column = 0; column < width; column++) {
          FX_FLOAT x = (FX_FLOAT)(column);
          FX_FLOAT y = (FX_FLOAT)(row);
          FX_FLOAT scale =
              (((x - start_x) * x_span) + ((y - start_y) * y_span)) /
              axis_len_square;
          if (scale < 0) {
            if (!m_info.fillColor->m_shading->m_isExtendedBegin) {
              continue;
            }
            scale = 0;
          } else if (scale > 1.0f) {
            if (!m_info.fillColor->m_shading->m_isExtendedEnd) {
              continue;
            }
            scale = 1.0f;
          }
          int32_t index = (int32_t)(scale * (FX_SHADING_Steps - 1));
          dib_buf[column] = m_info.fillColor->m_shading->m_argbArray[index];
        }
      }
      result = true;
      break;
    }
    case FX_SHADING_Radial: {
      FX_FLOAT start_r = m_info.fillColor->m_shading->m_beginRadius;
      FX_FLOAT end_r = m_info.fillColor->m_shading->m_endRadius;
      FX_FLOAT a = ((start_x - end_x) * (start_x - end_x)) +
                   ((start_y - end_y) * (start_y - end_y)) -
                   ((start_r - end_r) * (start_r - end_r));
      for (int32_t row = 0; row < height; row++) {
        uint32_t* dib_buf = (uint32_t*)(bmp.GetBuffer() + row * pitch);
        for (int32_t column = 0; column < width; column++) {
          FX_FLOAT x = (FX_FLOAT)(column);
          FX_FLOAT y = (FX_FLOAT)(row);
          FX_FLOAT b = -2 * (((x - start_x) * (end_x - start_x)) +
                             ((y - start_y) * (end_y - start_y)) +
                             (start_r * (end_r - start_r)));
          FX_FLOAT c = ((x - start_x) * (x - start_x)) +
                       ((y - start_y) * (y - start_y)) - (start_r * start_r);
          FX_FLOAT s;
          if (a == 0) {
            s = -c / b;
          } else {
            FX_FLOAT b2_4ac = (b * b) - 4 * (a * c);
            if (b2_4ac < 0) {
              continue;
            }
            FX_FLOAT root = (FXSYS_sqrt(b2_4ac));
            FX_FLOAT s1, s2;
            if (a > 0) {
              s1 = (-b - root) / (2 * a);
              s2 = (-b + root) / (2 * a);
            } else {
              s2 = (-b - root) / (2 * a);
              s1 = (-b + root) / (2 * a);
            }
            if (s2 <= 1.0f || m_info.fillColor->m_shading->m_isExtendedEnd) {
              s = (s2);
            } else {
              s = (s1);
            }
            if ((start_r) + s * (end_r - start_r) < 0) {
              continue;
            }
          }
          if (s < 0) {
            if (!m_info.fillColor->m_shading->m_isExtendedBegin) {
              continue;
            }
            s = 0;
          }
          if (s > 1.0f) {
            if (!m_info.fillColor->m_shading->m_isExtendedEnd) {
              continue;
            }
            s = 1.0f;
          }
          int index = (int32_t)(s * (FX_SHADING_Steps - 1));
          dib_buf[column] = m_info.fillColor->m_shading->m_argbArray[index];
        }
      }
      result = true;
      break;
    }
    default: {
      result = false;
      break;
    }
  }
  if (result) {
    m_renderDevice->SaveState();
    m_renderDevice->SetClip_PathFill(path->GetPathData(), matrix, fillMode);
    SetDIBitsWithMatrix(&bmp, matrix);
    m_renderDevice->RestoreState(false);
  }
}

void CFX_Graphics::SetDIBitsWithMatrix(CFX_DIBSource* source,
                                       CFX_Matrix* matrix) {
  if (matrix->IsIdentity()) {
    m_renderDevice->SetDIBits(source, 0, 0);
  } else {
    CFX_Matrix m((FX_FLOAT)source->GetWidth(), 0, 0,
                 (FX_FLOAT)source->GetHeight(), 0, 0);
    m.Concat(*matrix);
    int32_t left;
    int32_t top;
    std::unique_ptr<CFX_DIBitmap> bmp1 = source->FlipImage(false, true);
    std::unique_ptr<CFX_DIBitmap> bmp2 = bmp1->TransformTo(&m, left, top);
    m_renderDevice->SetDIBits(bmp2.get(), left, top);
  }
}

CFX_Graphics::TInfo::TInfo()
    : isActOnDash(false), strokeColor(nullptr), fillColor(nullptr) {}

CFX_Graphics::TInfo::TInfo(const TInfo& info)
    : graphState(info.graphState),
      CTM(info.CTM),
      isActOnDash(info.isActOnDash),
      strokeColor(info.strokeColor),
      fillColor(info.fillColor) {}

CFX_Graphics::TInfo& CFX_Graphics::TInfo::operator=(const TInfo& other) {
  graphState.Copy(other.graphState);
  CTM = other.CTM;
  isActOnDash = other.isActOnDash;
  strokeColor = other.strokeColor;
  fillColor = other.fillColor;
  return *this;
}
