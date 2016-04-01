// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FXFA_XFA_RENDERCONTEXT_H_
#define XFA_INCLUDE_FXFA_XFA_RENDERCONTEXT_H_

#include "xfa/include/fxfa/fxfa.h"

class CXFA_RenderContext {
 public:
  CXFA_RenderContext();
  ~CXFA_RenderContext();

  void Release() { delete this; }
  int32_t StartRender(CXFA_FFPageView* pPageView,
                      CFX_Graphics* pGS,
                      const CFX_Matrix& matrix,
                      const CXFA_RenderOptions& options);
  int32_t DoRender(IFX_Pause* pPause = NULL);
  void StopRender();

 protected:
  IXFA_WidgetIterator* m_pWidgetIterator;
  CXFA_FFWidget* m_pWidget;
  CXFA_FFPageView* m_pPageView;
  CFX_Graphics* m_pGS;
  CFX_Matrix m_matrix;
  CXFA_RenderOptions m_options;
  uint32_t m_dwStatus;
  CFX_RectF m_rtClipRect;
};

#endif  // XFA_INCLUDE_FXFA_XFA_RENDERCONTEXT_H_
