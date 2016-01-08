// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "xfa/src/foxitlib.h"

CFWL_PictureBox* CFWL_PictureBox::Create() {
  return new CFWL_PictureBox;
}
FWL_ERR CFWL_PictureBox::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_PictureBox> pPictureBox(IFWL_PictureBox::Create(
      m_pProperties->MakeWidgetImpProperties(&m_PictureBoxDP), nullptr));
  FWL_ERR ret = pPictureBox->Initialize();
  if (ret != FWL_ERR_Succeeded) {
    return ret;
  }
  m_pIface = pPictureBox.release();
  CFWL_Widget::Initialize();
  return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PictureBox::GetPicture() {
  return m_PictureBoxDP.m_pBitmap;
}
FWL_ERR CFWL_PictureBox::SetPicture(CFX_DIBitmap* pBitmap) {
  m_PictureBoxDP.m_pBitmap = pBitmap;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_PictureBox::GetRotation() {
  return m_PictureBoxDP.m_fRotation;
}
FWL_ERR CFWL_PictureBox::SetRotation(FX_FLOAT fRotation) {
  m_PictureBoxDP.m_fRotation = fRotation;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_PictureBox::GetFlipMode() {
  return m_PictureBoxDP.GetFlipMode(m_pIface);
}
FWL_ERR CFWL_PictureBox::SetFlipMode(int32_t iFlipMode) {
  m_PictureBoxDP.m_iFlipMode = iFlipMode;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_PictureBox::GetOpacity() {
  return m_PictureBoxDP.GetOpacity(m_pIface);
}
FWL_ERR CFWL_PictureBox::SetOpacity(int32_t iOpacity) {
  m_PictureBoxDP.m_iOpacity = iOpacity;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::GetScale(FX_FLOAT& fScaleX, FX_FLOAT& fScaleY) {
  CFX_Matrix matrix;
  m_PictureBoxDP.GetMatrix(m_pIface, matrix);
  matrix.Scale(fScaleX, fScaleY);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::SetScale(FX_FLOAT fScaleX, FX_FLOAT fScaleY) {
  m_PictureBoxDP.m_fScaleX = fScaleX;
  m_PictureBoxDP.m_fScaleY = fScaleY;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::GetOffset(FX_FLOAT& fx, FX_FLOAT& fy) {
  CFX_Matrix matrix;
  m_PictureBoxDP.GetMatrix(m_pIface, matrix);
  fx = matrix.e;
  fy = matrix.f;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::SetOffset(FX_FLOAT fx, FX_FLOAT fy) {
  m_PictureBoxDP.m_fOffSetX = fx;
  m_PictureBoxDP.m_fOffSetY = fy;
  return FWL_ERR_Succeeded;
}
CFWL_PictureBox::CFWL_PictureBox() {}
CFWL_PictureBox::~CFWL_PictureBox() {}
FWL_ERR CFWL_PictureBox::CFWL_PictureBoxDP::GetCaption(
    IFWL_Widget* pWidget,
    CFX_WideString& wsCaption) {
  return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PictureBox::CFWL_PictureBoxDP::GetPicture(
    IFWL_Widget* pWidget) {
  return m_pBitmap;
}
CFX_DIBitmap* CFWL_PictureBox::CFWL_PictureBoxDP::GetErrorPicture(
    IFWL_Widget* pWidget) {
  return m_pBitmap;
}
CFX_DIBitmap* CFWL_PictureBox::CFWL_PictureBoxDP::GetInitialPicture(
    IFWL_Widget* pWidget) {
  return m_pBitmap;
}
int32_t CFWL_PictureBox::CFWL_PictureBoxDP::GetOpacity(IFWL_Widget* pWidget) {
  return m_iOpacity;
}
FWL_ERR CFWL_PictureBox::CFWL_PictureBoxDP::GetMatrix(IFWL_Widget* pWidget,
                                                      CFX_Matrix& matrix) {
  CFX_RectF rect;
  pWidget->GetClientRect(rect);
  FX_FLOAT fLen = rect.width / 2;
  FX_FLOAT fWid = rect.height / 2;
  matrix.SetIdentity();
  matrix.Translate(-fLen, -fWid);
  matrix.Rotate(m_fRotation);
  matrix.Translate(fLen, fWid);
  matrix.Scale(m_fScaleX, m_fScaleY);
  matrix.Translate(m_fOffSetX, m_fOffSetY);
  return FWL_ERR_Succeeded;
}
int32_t CFWL_PictureBox::CFWL_PictureBoxDP::GetFlipMode(IFWL_Widget* pWidget) {
  return m_iFlipMode;
}
