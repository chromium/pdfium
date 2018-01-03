// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffimage.h"

#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdraw.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_value.h"

CXFA_FFImage::CXFA_FFImage(CXFA_WidgetAcc* pDataAcc) : CXFA_FFDraw(pDataAcc) {}

CXFA_FFImage::~CXFA_FFImage() {
  CXFA_FFImage::UnloadWidget();
}

bool CXFA_FFImage::IsLoaded() {
  return !!GetDataAcc()->GetImageImage();
}

bool CXFA_FFImage::LoadWidget() {
  if (GetDataAcc()->GetImageImage())
    return true;

  return GetDataAcc()->LoadImageImage() ? CXFA_FFDraw::LoadWidget() : false;
}

void CXFA_FFImage::UnloadWidget() {
  GetDataAcc()->SetImageImage(nullptr);
}

void CXFA_FFImage::RenderWidget(CXFA_Graphics* pGS,
                                const CFX_Matrix& matrix,
                                uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, dwStatus);

  RetainPtr<CFX_DIBitmap> pDIBitmap = GetDataAcc()->GetImageImage();
  if (!pDIBitmap)
    return;

  CFX_RectF rtImage = GetRectWithoutRotate();
  CXFA_Margin* margin = m_pDataAcc->GetMargin();
  if (margin)
    XFA_RectWidthoutMargin(rtImage, margin);

  XFA_AttributeEnum iHorzAlign = XFA_AttributeEnum::Left;
  XFA_AttributeEnum iVertAlign = XFA_AttributeEnum::Top;
  CXFA_Para* para = m_pDataAcc->GetPara();
  if (para) {
    iHorzAlign = para->GetHorizontalAlign();
    iVertAlign = para->GetVerticalAlign();
  }

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  m_pDataAcc->GetImageDpi(iImageXDpi, iImageYDpi);
  auto* value = m_pDataAcc->GetFormValue();
  CXFA_ImageData imageData =
      value ? value->GetImageData() : CXFA_ImageData(nullptr);

  XFA_DrawImage(pGS, rtImage, mtRotate, pDIBitmap, imageData.GetAspect(),
                iImageXDpi, iImageYDpi, iHorzAlign, iVertAlign);
}
