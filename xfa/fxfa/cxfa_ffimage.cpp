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
  CXFA_MarginData marginData = m_pDataAcc->GetMarginData();
  if (marginData.HasValidNode())
    XFA_RectWidthoutMargin(rtImage, marginData);

  XFA_AttributeEnum iHorzAlign = XFA_AttributeEnum::Left;
  XFA_AttributeEnum iVertAlign = XFA_AttributeEnum::Top;
  CXFA_ParaData paraData = m_pDataAcc->GetParaData();
  if (paraData.HasValidNode()) {
    iHorzAlign = paraData.GetHorizontalAlign();
    iVertAlign = paraData.GetVerticalAlign();
  }

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  m_pDataAcc->GetImageDpi(iImageXDpi, iImageYDpi);
  XFA_DrawImage(pGS, rtImage, mtRotate, pDIBitmap,
                m_pDataAcc->GetFormValueData().GetImageData().GetAspect(),
                iImageXDpi, iImageYDpi, iHorzAlign, iVertAlign);
}
