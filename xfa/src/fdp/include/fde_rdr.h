// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FDP_INCLUDE_FDE_RDR_H_
#define XFA_SRC_FDP_INCLUDE_FDE_RDR_H_

#include "core/include/fxcrt/fx_coordinates.h"
#include "xfa/src/fdp/include/fde_psr.h"

class IFDE_Page;
class IFDE_RenderDevice;

void FDE_GetPageMatrix(CFX_Matrix& pageMatrix,
                       const CFX_RectF& docPageRect,
                       const CFX_Rect& devicePageRect,
                       int32_t iRotate,
                       FX_DWORD dwCoordinatesType = 0);
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

#endif  // XFA_SRC_FDP_INCLUDE_FDE_RDR_H_
