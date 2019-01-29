// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_RENDERCONTEXT_H_
#define XFA_FXFA_CXFA_RENDERCONTEXT_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"

class CXFA_Graphics;
class CXFA_FFPageView;
class CXFA_FFWidget;
class IXFA_WidgetIterator;

class CXFA_RenderContext {
 public:
  CXFA_RenderContext(CXFA_FFPageView* pPageView,
                     const CFX_RectF& clipRect,
                     const CFX_Matrix& matrix);
  ~CXFA_RenderContext();

  void DoRender(CXFA_Graphics* gs);

 private:
  std::unique_ptr<IXFA_WidgetIterator> const m_pWidgetIterator;
  UnownedPtr<CXFA_FFWidget> m_pWidget;
  const CFX_Matrix m_matrix;
  const CFX_RectF m_rtClipRect;
};

#endif  // XFA_FXFA_CXFA_RENDERCONTEXT_H_
