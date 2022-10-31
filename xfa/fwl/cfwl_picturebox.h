// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_PICTUREBOX_H_
#define XFA_FWL_CFWL_PICTUREBOX_H_

#include "xfa/fwl/cfwl_widget.h"

class CFWL_PictureBox final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_PictureBox() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void Update() override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

 private:
  explicit CFWL_PictureBox(CFWL_App* pApp);

  CFX_RectF m_ClientRect;
  CFX_RectF m_ImageRect;
  CFX_Matrix m_matrix;
};

#endif  // XFA_FWL_CFWL_PICTUREBOX_H_
