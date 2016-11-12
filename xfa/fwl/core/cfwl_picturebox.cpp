// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_picturebox.h"

#include <memory>

#include "third_party/base/ptr_util.h"

CFWL_PictureBox::CFWL_PictureBox(const IFWL_App* app)
    : CFWL_Widget(app),
      m_pBitmap(nullptr),
      m_iOpacity(0),
      m_iFlipMode(0),
      m_fRotation(0.0f),
      m_fScaleX(1.0f),
      m_fScaleY(1.0f),
      m_fOffSetX(0.0f),
      m_fOffSetY(0.0f) {}

CFWL_PictureBox::~CFWL_PictureBox() {}

void CFWL_PictureBox::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_PictureBox>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>(this));

  CFWL_Widget::Initialize();
}

CFX_DIBitmap* CFWL_PictureBox::GetPicture() {
  return m_pBitmap;
}

FWL_Error CFWL_PictureBox::SetPicture(CFX_DIBitmap* pBitmap) {
  m_pBitmap = pBitmap;
  return FWL_Error::Succeeded;
}

FX_FLOAT CFWL_PictureBox::GetRotation() {
  return m_fRotation;
}

FWL_Error CFWL_PictureBox::SetRotation(FX_FLOAT fRotation) {
  m_fRotation = fRotation;
  return FWL_Error::Succeeded;
}

int32_t CFWL_PictureBox::GetFlipMode() {
  return GetFlipMode(m_pIface.get());
}

FWL_Error CFWL_PictureBox::SetFlipMode(int32_t iFlipMode) {
  m_iFlipMode = iFlipMode;
  return FWL_Error::Succeeded;
}

int32_t CFWL_PictureBox::GetOpacity() {
  return GetOpacity(m_pIface.get());
}

FWL_Error CFWL_PictureBox::SetOpacity(int32_t iOpacity) {
  m_iOpacity = iOpacity;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_PictureBox::GetScale(FX_FLOAT& fScaleX, FX_FLOAT& fScaleY) {
  CFX_Matrix matrix;
  GetMatrix(m_pIface.get(), matrix);
  matrix.Scale(fScaleX, fScaleY);
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_PictureBox::SetScale(FX_FLOAT fScaleX, FX_FLOAT fScaleY) {
  m_fScaleX = fScaleX;
  m_fScaleY = fScaleY;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_PictureBox::GetOffset(FX_FLOAT& fx, FX_FLOAT& fy) {
  CFX_Matrix matrix;
  GetMatrix(m_pIface.get(), matrix);
  fx = matrix.e;
  fy = matrix.f;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_PictureBox::SetOffset(FX_FLOAT fx, FX_FLOAT fy) {
  m_fOffSetX = fx;
  m_fOffSetY = fy;
  return FWL_Error::Succeeded;
}

void CFWL_PictureBox::GetCaption(IFWL_Widget* pWidget,
                                 CFX_WideString& wsCaption) {}

CFX_DIBitmap* CFWL_PictureBox::GetPicture(IFWL_Widget* pWidget) {
  return m_pBitmap;
}

CFX_DIBitmap* CFWL_PictureBox::GetErrorPicture(IFWL_Widget* pWidget) {
  return m_pBitmap;
}

CFX_DIBitmap* CFWL_PictureBox::GetInitialPicture(IFWL_Widget* pWidget) {
  return m_pBitmap;
}

int32_t CFWL_PictureBox::GetOpacity(IFWL_Widget* pWidget) {
  return m_iOpacity;
}

FWL_Error CFWL_PictureBox::GetMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) {
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
  return FWL_Error::Succeeded;
}

int32_t CFWL_PictureBox::GetFlipMode(IFWL_Widget* pWidget) {
  return m_iFlipMode;
}
