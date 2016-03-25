// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_RENDER_H_
#define XFA_FDE_FDE_RENDER_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "xfa/fde/fde_visualset.h"

class IFDE_RenderDevice;

void FDE_GetPageMatrix(CFX_Matrix& pageMatrix,
                       const CFX_RectF& docPageRect,
                       const CFX_Rect& devicePageRect,
                       int32_t iRotate,
                       uint32_t dwCoordinatesType = 0);
enum FDE_RENDERSTATUS {
  FDE_RENDERSTATUS_Reset = 0,
  FDE_RENDERSTATUS_Paused,
  FDE_RENDERSTATUS_Done,
  FDE_RENDERSTATUS_Failed,
};

class IFDE_RenderContext {
 public:
  static IFDE_RenderContext* Create();
  virtual ~IFDE_RenderContext() {}
  virtual void Release() = 0;
  virtual FX_BOOL StartRender(IFDE_RenderDevice* pRenderDevice,
                              IFDE_CanvasSet* pCanvasSet,
                              const CFX_Matrix& tmDoc2Device) = 0;
  virtual FDE_RENDERSTATUS GetStatus() const = 0;
  virtual FDE_RENDERSTATUS DoRender(IFX_Pause* pPause = NULL) = 0;
  virtual void StopRender() = 0;
};

#endif  // XFA_FDE_FDE_RENDER_H_
