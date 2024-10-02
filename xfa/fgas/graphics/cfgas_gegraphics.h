// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GEGRAPHICS_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GEGRAPHICS_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "xfa/fgas/graphics/cfgas_gecolor.h"

class CFGAS_GEPath;
class CFX_DIBBase;
class CFX_RenderDevice;

class CFGAS_GEGraphics {
 public:
  class StateRestorer {
   public:
    FX_STACK_ALLOCATED();

    explicit StateRestorer(CFGAS_GEGraphics* graphics);
    ~StateRestorer();

   private:
    UnownedPtr<CFGAS_GEGraphics> const graphics_;
  };

  explicit CFGAS_GEGraphics(CFX_RenderDevice* renderDevice);
  ~CFGAS_GEGraphics();

  CFX_RectF GetClipRect() const;
  const CFX_Matrix* GetMatrix() const;
  CFX_RenderDevice* GetRenderDevice();

  void SetLineCap(CFX_GraphStateData::LineCap lineCap);
  // Dash phase is always set to 0.
  void SetLineDash(std::vector<float> dash_array);
  void SetSolidLineDash();
  void SetLineWidth(float lineWidth);
  void EnableActOnDash();
  void SetStrokeColor(const CFGAS_GEColor& color);
  void SetFillColor(const CFGAS_GEColor& color);
  void SetClipRect(const CFX_RectF& rect);
  void StrokePath(const CFGAS_GEPath& path, const CFX_Matrix& matrix);
  void FillPath(const CFGAS_GEPath& path,
                CFX_FillRenderOptions::FillType fill_type,
                const CFX_Matrix& matrix);
  void ConcatMatrix(const CFX_Matrix& matrix);

 private:
  struct TInfo {
    TInfo();
    explicit TInfo(const TInfo& info);
    TInfo& operator=(const TInfo& other);

    CFX_GraphStateData graphState;
    CFX_Matrix CTM;
    bool isActOnDash = false;
    CFGAS_GEColor strokeColor{nullptr};
    CFGAS_GEColor fillColor{nullptr};
  };

  void SaveGraphState();
  void RestoreGraphState();

  void RenderDeviceStrokePath(const CFGAS_GEPath& path,
                              const CFX_Matrix& matrix);
  void RenderDeviceFillPath(const CFGAS_GEPath& path,
                            CFX_FillRenderOptions::FillType fill_type,
                            const CFX_Matrix& matrix);
  void FillPathWithPattern(const CFGAS_GEPath& path,
                           const CFX_FillRenderOptions& fill_options,
                           const CFX_Matrix& matrix);
  void FillPathWithShading(const CFGAS_GEPath& path,
                           const CFX_FillRenderOptions& fill_options,
                           const CFX_Matrix& matrix);
  void SetDIBitsWithMatrix(RetainPtr<CFX_DIBBase> source,
                           const CFX_Matrix& matrix);

  UnownedPtr<CFX_RenderDevice> const m_renderDevice;
  TInfo m_info;
  std::vector<std::unique_ptr<TInfo>> m_infoStack;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GEGRAPHICS_H_
