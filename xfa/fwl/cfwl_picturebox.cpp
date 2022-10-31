// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_picturebox.h"

CFWL_PictureBox::CFWL_PictureBox(CFWL_App* app)
    : CFWL_Widget(app, CFWL_Widget::Properties(), nullptr) {}

CFWL_PictureBox::~CFWL_PictureBox() = default;

FWL_Type CFWL_PictureBox::GetClassID() const {
  return FWL_Type::PictureBox;
}

void CFWL_PictureBox::Update() {
  if (IsLocked())
    return;

  m_ClientRect = GetClientRect();
}

void CFWL_PictureBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
}

void CFWL_PictureBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}
