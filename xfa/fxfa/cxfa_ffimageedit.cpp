// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffimageedit.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_picturebox.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_value.h"

CXFA_FFImageEdit::CXFA_FFImageEdit(CXFA_Node* pNode)
    : CXFA_FFField(pNode), m_pOldDelegate(nullptr) {}

CXFA_FFImageEdit::~CXFA_FFImageEdit() {
  CXFA_FFImageEdit::UnloadWidget();
}

bool CXFA_FFImageEdit::LoadWidget() {
  auto pNew = pdfium::MakeUnique<CFWL_PictureBox>(GetFWLApp());
  CFWL_PictureBox* pPictureBox = pNew.get();
  m_pNormalWidget = std::move(pNew);
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget.get(),
                                   m_pNormalWidget.get());
  m_pOldDelegate = pPictureBox->GetDelegate();
  pPictureBox->SetDelegate(this);

  CXFA_FFField::LoadWidget();
  if (!m_pNode->GetWidgetAcc()->GetImageEditImage())
    UpdateFWLData();

  return true;
}

void CXFA_FFImageEdit::UnloadWidget() {
  m_pNode->GetWidgetAcc()->SetImageEditImage(nullptr);
  CXFA_FFField::UnloadWidget();
}

void CXFA_FFImageEdit::RenderWidget(CXFA_Graphics* pGS,
                                    const CFX_Matrix& matrix,
                                    uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, dwStatus);
  DrawBorder(pGS, m_pNode->GetWidgetAcc()->GetUIBorder(), m_rtUI, mtRotate);
  RenderCaption(pGS, &mtRotate);
  RetainPtr<CFX_DIBitmap> pDIBitmap =
      m_pNode->GetWidgetAcc()->GetImageEditImage();
  if (!pDIBitmap)
    return;

  CFX_RectF rtImage = m_pNormalWidget->GetWidgetRect();
  XFA_AttributeEnum iHorzAlign = XFA_AttributeEnum::Left;
  XFA_AttributeEnum iVertAlign = XFA_AttributeEnum::Top;
  CXFA_Para* para = m_pNode->GetParaIfExists();
  if (para) {
    iHorzAlign = para->GetHorizontalAlign();
    iVertAlign = para->GetVerticalAlign();
  }

  XFA_AttributeEnum iAspect = XFA_AttributeEnum::Fit;
  CXFA_Value* value = m_pNode->GetFormValueIfExists();
  if (value) {
    CXFA_Image* image = value->GetImageIfExists();
    if (image)
      iAspect = image->GetAspect();
  }

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  m_pNode->GetWidgetAcc()->GetImageEditDpi(iImageXDpi, iImageYDpi);
  XFA_DrawImage(pGS, rtImage, mtRotate, pDIBitmap, iAspect, iImageXDpi,
                iImageYDpi, iHorzAlign, iVertAlign);
}

bool CXFA_FFImageEdit::OnLButtonDown(uint32_t dwFlags,
                                     const CFX_PointF& point) {
  if (!m_pNode->IsOpenAccess())
    return false;
  if (!PtInActiveRect(point))
    return false;

  SetButtonDown(true);

  CFWL_MessageMouse ms(nullptr, m_pNormalWidget.get());
  ms.m_dwCmd = FWL_MouseCommand::LeftButtonDown;
  ms.m_dwFlags = dwFlags;
  ms.m_pos = FWLToClient(point);
  TranslateFWLMessage(&ms);
  return true;
}

void CXFA_FFImageEdit::SetFWLRect() {
  if (!m_pNormalWidget)
    return;

  CFX_RectF rtUIMargin = m_pNode->GetWidgetAcc()->GetUIMargin();
  CFX_RectF rtImage(m_rtUI);
  rtImage.Deflate(rtUIMargin.left, rtUIMargin.top, rtUIMargin.width,
                  rtUIMargin.height);
  m_pNormalWidget->SetWidgetRect(rtImage);
}

bool CXFA_FFImageEdit::CommitData() {
  return true;
}

bool CXFA_FFImageEdit::UpdateFWLData() {
  m_pNode->GetWidgetAcc()->SetImageEditImage(nullptr);
  m_pNode->GetWidgetAcc()->LoadImageEditImage(GetDoc());
  return true;
}

void CXFA_FFImageEdit::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFImageEdit::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFImageEdit::OnDrawWidget(CXFA_Graphics* pGraphics,
                                    const CFX_Matrix& matrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, matrix);
}

FormFieldType CXFA_FFImageEdit::GetFormFieldType() {
  return FormFieldType::kXFA_ImageField;
}
