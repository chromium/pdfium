// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFIMAGE_H_
#define XFA_FXFA_CXFA_FFIMAGE_H_

#include "v8/include/cppgc/prefinalizer.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

class CXFA_FFImage final : public CXFA_FFWidget {
  CPPGC_USING_PRE_FINALIZER(CXFA_FFImage, PreFinalize);

 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFImage() override;

  void PreFinalize();

  // CXFA_FFWidget:
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  bool IsLoaded() override;
  bool LoadWidget() override;

 private:
  explicit CXFA_FFImage(CXFA_Node* pNode);
};

#endif  // XFA_FXFA_CXFA_FFIMAGE_H_
