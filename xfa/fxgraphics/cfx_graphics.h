// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CFX_GRAPHICS_H_
#define XFA_FXGRAPHICS_CFX_GRAPHICS_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_fxgedevice.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_dib.h"
#include "core/fxge/fx_font.h"

class CFX_Color;
class CFX_Path;

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

class CFX_Graphics {
 public:
  explicit CFX_Graphics(CFX_RenderDevice* renderDevice);
  ~CFX_Graphics();

  void SaveGraphState();
  void RestoreGraphState();

  CFX_RectF GetClipRect() const;
  CFX_Matrix* GetMatrix();
  CFX_RenderDevice* GetRenderDevice();

  void SetLineCap(CFX_GraphStateData::LineCap lineCap);
  void SetLineDash(FX_FLOAT dashPhase, FX_FLOAT* dashArray, int32_t dashCount);
  void SetLineDash(FX_DashStyle dashStyle);
  void SetLineWidth(FX_FLOAT lineWidth, bool isActOnDash = false);
  void SetStrokeColor(CFX_Color* color);
  void SetFillColor(CFX_Color* color);
  void SetClipRect(const CFX_RectF& rect);
  void StrokePath(CFX_Path* path, CFX_Matrix* matrix = nullptr);
  void FillPath(CFX_Path* path,
                FX_FillMode fillMode = FXFILL_WINDING,
                CFX_Matrix* matrix = nullptr);
  void StretchImage(CFX_DIBSource* source,
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
    CFX_Color* strokeColor;
    CFX_Color* fillColor;
  } m_info;

  void RenderDeviceSetLineDash(FX_DashStyle dashStyle);
  void RenderDeviceStrokePath(CFX_Path* path, CFX_Matrix* matrix);
  void RenderDeviceFillPath(CFX_Path* path,
                            FX_FillMode fillMode,
                            CFX_Matrix* matrix);
  void RenderDeviceStretchImage(CFX_DIBSource* source,
                                const CFX_RectF& rect,
                                CFX_Matrix* matrix);

  void FillPathWithPattern(CFX_Path* path,
                           FX_FillMode fillMode,
                           CFX_Matrix* matrix);
  void FillPathWithShading(CFX_Path* path,
                           FX_FillMode fillMode,
                           CFX_Matrix* matrix);

  void SetDIBitsWithMatrix(CFX_DIBSource* source, CFX_Matrix* matrix);

  CFX_RenderDevice* m_renderDevice;
  std::vector<std::unique_ptr<TInfo>> m_infoStack;
};

#endif  // XFA_FXGRAPHICS_CFX_GRAPHICS_H_
