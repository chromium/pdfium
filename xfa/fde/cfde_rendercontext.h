// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_RENDERCONTEXT_H_
#define XFA_FDE_CFDE_RENDERCONTEXT_H_

#include "core/fxcrt/fx_coordinates.h"

class CFDE_RenderDevice;
class CFDE_TxtEdtPage;
class CFDE_TxtEdtTextSet;

class CFDE_RenderContext {
 public:
  explicit CFDE_RenderContext(CFDE_RenderDevice* pRenderDevice);
  ~CFDE_RenderContext();

  void Render(CFDE_TxtEdtPage* pCanvasSet, const CFX_Matrix& tmDoc2Device);

 private:
  CFDE_RenderDevice* m_pRenderDevice;
};

#endif  // XFA_FDE_CFDE_RENDERCONTEXT_H_
