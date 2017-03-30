// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_RENDERCONTEXT_H_
#define XFA_FDE_CFDE_RENDERCONTEXT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fde/cfde_renderdevice.h"
#include "xfa/fde/cfde_visualsetiterator.h"
#include "xfa/fde/ifde_visualset.h"

class CFDE_RenderDevice;
class CFDE_TxtEdtTextSet;

enum FDE_RENDERSTATUS {
  FDE_RENDERSTATUS_Reset = 0,
  FDE_RENDERSTATUS_Paused,
  FDE_RENDERSTATUS_Done,
  FDE_RENDERSTATUS_Failed,
};

class CFDE_RenderContext {
 public:
  CFDE_RenderContext();
  ~CFDE_RenderContext();

  bool StartRender(CFDE_RenderDevice* pRenderDevice,
                   CFDE_TxtEdtPage* pCanvasSet,
                   const CFX_Matrix& tmDoc2Device);
  FDE_RENDERSTATUS GetStatus() const { return m_eStatus; }
  FDE_RENDERSTATUS DoRender(IFX_Pause* pPause = nullptr);
  void StopRender();
  void RenderText(CFDE_TxtEdtTextSet* pTextSet, FDE_TEXTEDITPIECE* pText);

 protected:
  FDE_RENDERSTATUS m_eStatus;
  CFDE_RenderDevice* m_pRenderDevice;
  CFX_Matrix m_Transform;
  std::vector<FXTEXT_CHARPOS> m_CharPos;
  std::unique_ptr<CFDE_Brush> m_pBrush;
  std::unique_ptr<CFDE_VisualSetIterator> m_pIterator;
};

#endif  // XFA_FDE_CFDE_RENDERCONTEXT_H_
