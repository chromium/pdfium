// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffimage.h"

#include <utility>

#include "core/fxge/dib/cfx_dibitmap.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_value.h"

CXFA_FFImage::CXFA_FFImage(CXFA_Node* pNode) : CXFA_FFWidget(pNode) {}

CXFA_FFImage::~CXFA_FFImage() = default;

void CXFA_FFImage::PreFinalize() {
  GetNode()->SetLayoutImage(nullptr);
}

bool CXFA_FFImage::IsLoaded() {
  return !!GetNode()->GetLayoutImage();
}

bool CXFA_FFImage::LoadWidget() {
  if (GetNode()->GetLayoutImage())
    return true;

  return GetNode()->LoadLayoutImage(GetDoc()) && CXFA_FFWidget::LoadWidget();
}

void CXFA_FFImage::RenderWidget(CFGAS_GEGraphics* pGS,
                                const CFX_Matrix& matrix,
                                HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);

  RetainPtr<CFX_DIBitmap> pDIBitmap = GetNode()->GetLayoutImage();
  if (!pDIBitmap)
    return;

  CFX_RectF rtImage = GetRectWithoutRotate();
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  XFA_RectWithoutMargin(&rtImage, margin);

  XFA_AttributeValue iHorzAlign = XFA_AttributeValue::Left;
  XFA_AttributeValue iVertAlign = XFA_AttributeValue::Top;
  CXFA_Para* para = m_pNode->GetParaIfExists();
  if (para) {
    iHorzAlign = para->GetHorizontalAlign();
    iVertAlign = para->GetVerticalAlign();
  }

  auto* value = m_pNode->GetFormValueIfExists();
  if (!value)
    return;

  CXFA_Image* image = value->GetImageIfExists();
  if (!image)
    return;

  XFA_DrawImage(pGS, rtImage, mtRotate, std::move(pDIBitmap),
                image->GetAspect(), m_pNode->GetLayoutImageDpi(), iHorzAlign,
                iVertAlign);
}
