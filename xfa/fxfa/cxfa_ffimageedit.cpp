// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffimageedit.h"

#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "v8/include/cppgc/visitor.h"
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

CXFA_FFImageEdit::CXFA_FFImageEdit(CXFA_Node* pNode) : CXFA_FFField(pNode) {}

CXFA_FFImageEdit::~CXFA_FFImageEdit() = default;

void CXFA_FFImageEdit::PreFinalize() {
  node_->SetEditImage(nullptr);
}

void CXFA_FFImageEdit::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFField::Trace(visitor);
  visitor->Trace(old_delegate_);
}

bool CXFA_FFImageEdit::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_PictureBox* pPictureBox = cppgc::MakeGarbageCollected<CFWL_PictureBox>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp());
  SetNormalWidget(pPictureBox);
  pPictureBox->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pPictureBox->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pPictureBox, pPictureBox);
  old_delegate_ = pPictureBox->GetDelegate();
  pPictureBox->SetDelegate(this);

  CXFA_FFField::LoadWidget();
  if (!node_->GetEditImage()) {
    UpdateFWLData();
  }

  return true;
}

void CXFA_FFImageEdit::RenderWidget(CFGAS_GEGraphics* pGS,
                                    const CFX_Matrix& matrix,
                                    HighlightOption highlight) {
  if (!HasVisibleStatus()) {
    return;
  }

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);
  DrawBorder(pGS, node_->GetUIBorder(), uirect_, mtRotate);
  RenderCaption(pGS, mtRotate);
  RetainPtr<CFX_DIBitmap> pDIBitmap = node_->GetEditImage();
  if (!pDIBitmap) {
    return;
  }

  CFX_RectF rtImage = GetNormalWidget()->GetWidgetRect();
  XFA_AttributeValue iHorzAlign = XFA_AttributeValue::Left;
  XFA_AttributeValue iVertAlign = XFA_AttributeValue::Top;
  CXFA_Para* para = node_->GetParaIfExists();
  if (para) {
    iHorzAlign = para->GetHorizontalAlign();
    iVertAlign = para->GetVerticalAlign();
  }

  XFA_AttributeValue iAspect = XFA_AttributeValue::Fit;
  CXFA_Value* value = node_->GetFormValueIfExists();
  if (value) {
    CXFA_Image* image = value->GetImageIfExists();
    if (image) {
      iAspect = image->GetAspect();
    }
  }

  XFA_DrawImage(pGS, rtImage, mtRotate, std::move(pDIBitmap), iAspect,
                node_->GetEditImageDpi(), iHorzAlign, iVertAlign);
}

bool CXFA_FFImageEdit::AcceptsFocusOnButtonDown(
    Mask<XFA_FWL_KeyFlag> dwFlags,
    const CFX_PointF& point,
    CFWL_MessageMouse::MouseCommand command) {
  if (command != CFWL_MessageMouse::MouseCommand::kLeftButtonDown) {
    return CXFA_FFField::AcceptsFocusOnButtonDown(dwFlags, point, command);
  }

  if (!node_->IsOpenAccess()) {
    return false;
  }
  if (!PtInActiveRect(point)) {
    return false;
  }

  return true;
}

bool CXFA_FFImageEdit::OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                                     const CFX_PointF& point) {
  SetButtonDown(true);
  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeftButtonDown,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

void CXFA_FFImageEdit::SetFWLRect() {
  if (!GetNormalWidget()) {
    return;
  }

  CFX_RectF rtUIMargin = node_->GetUIMargin();
  CFX_RectF rtImage(uirect_);
  rtImage.Deflate(rtUIMargin.left, rtUIMargin.top, rtUIMargin.width,
                  rtUIMargin.height);
  GetNormalWidget()->SetWidgetRect(rtImage);
}

bool CXFA_FFImageEdit::CommitData() {
  return true;
}

void CXFA_FFImageEdit::UpdateFWLData() {
  node_->SetEditImage(nullptr);
  node_->LoadEditImage(GetDoc());
}

void CXFA_FFImageEdit::OnProcessMessage(CFWL_Message* pMessage) {
  old_delegate_->OnProcessMessage(pMessage);
}

void CXFA_FFImageEdit::OnProcessEvent(pdfium::CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  old_delegate_->OnProcessEvent(pEvent);
}

void CXFA_FFImageEdit::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                    const CFX_Matrix& matrix) {
  old_delegate_->OnDrawWidget(pGraphics, matrix);
}

FormFieldType CXFA_FFImageEdit::GetFormFieldType() {
  return FormFieldType::kXFA_ImageField;
}
