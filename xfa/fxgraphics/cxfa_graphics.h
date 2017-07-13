// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CXFA_GRAPHICS_H_
#define XFA_FXGRAPHICS_CXFA_GRAPHICS_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_dib.h"
#include "core/fxge/fx_font.h"

class CXFA_Color;
class CXFA_Path;

using FX_FillMode = int32_t;

enum FX_DashStyle {
  FX_DASHSTYLE_Solid = 0,
  FX_DASHSTYLE_Dash = 1,
  FX_DASHSTYLE_Dot = 2,
  FX_DASHSTYLE_DashDot = 3,
  FX_DASHSTYLE_DashDotDot = 4
};

enum class FX_HatchStyle {
  Horizontal = 0,
  Vertical = 1,
  ForwardDiagonal = 2,
  BackwardDiagonal = 3,
  Cross = 4,
  DiagonalCross = 5
};

class CFX_RenderDevice;

class CXFA_Graphics {
 public:
  explicit CXFA_Graphics(CFX_RenderDevice* renderDevice);
  ~CXFA_Graphics();

  void SaveGraphState();
  void RestoreGraphState();

  CFX_RectF GetClipRect() const;
  CFX_Matrix* GetMatrix();
  CFX_RenderDevice* GetRenderDevice();

  void SetLineCap(CFX_GraphStateData::LineCap lineCap);
  void SetLineDash(float dashPhase, float* dashArray, int32_t dashCount);
  void SetLineDash(FX_DashStyle dashStyle);
  void SetLineWidth(float lineWidth, bool isActOnDash = false);
  void SetStrokeColor(CXFA_Color* color);
  void SetFillColor(CXFA_Color* color);
  void SetClipRect(const CFX_RectF& rect);
  void StrokePath(CXFA_Path* path, CFX_Matrix* matrix = nullptr);
  void FillPath(CXFA_Path* path,
                FX_FillMode fillMode = FXFILL_WINDING,
                CFX_Matrix* matrix = nullptr);
  void StretchImage(const CFX_RetainPtr<CFX_DIBSource>& source,
                    const CFX_RectF& rect,
                    CFX_Matrix* matrix = nullptr);
  void ConcatMatrix(const CFX_Matrix* matrix);

 protected:
  int32_t m_type;

 private:
  struct TInfo {
    TInfo();
    explicit TInfo(const TInfo& info);
    TInfo& operator=(const TInfo& other);

    CFX_GraphStateData graphState;
    CFX_Matrix CTM;
    bool isActOnDash;
    CXFA_Color* strokeColor;
    CXFA_Color* fillColor;
  } m_info;

  void RenderDeviceSetLineDash(FX_DashStyle dashStyle);
  void RenderDeviceStrokePath(CXFA_Path* path, CFX_Matrix* matrix);
  void RenderDeviceFillPath(CXFA_Path* path,
                            FX_FillMode fillMode,
                            CFX_Matrix* matrix);
  void RenderDeviceStretchImage(const CFX_RetainPtr<CFX_DIBSource>& source,
                                const CFX_RectF& rect,
                                CFX_Matrix* matrix);

  void FillPathWithPattern(CXFA_Path* path,
                           FX_FillMode fillMode,
                           CFX_Matrix* matrix);
  void FillPathWithShading(CXFA_Path* path,
                           FX_FillMode fillMode,
                           CFX_Matrix* matrix);

  void SetDIBitsWithMatrix(const CFX_RetainPtr<CFX_DIBSource>& source,
                           CFX_Matrix* matrix);

  CFX_RenderDevice* const m_renderDevice;  // Not owned.
  std::vector<std::unique_ptr<TInfo>> m_infoStack;
};

#endif  // XFA_FXGRAPHICS_CXFA_GRAPHICS_H_
