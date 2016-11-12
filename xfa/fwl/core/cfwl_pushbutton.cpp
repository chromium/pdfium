// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_pushbutton.h"

#include <memory>

#include "third_party/base/ptr_util.h"

CFWL_PushButton::CFWL_PushButton(const IFWL_App* app)
    : CFWL_Widget(app), m_pBitmap(nullptr) {}

CFWL_PushButton::~CFWL_PushButton() {}

void CFWL_PushButton::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_PushButton>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>(this));

  CFWL_Widget::Initialize();
}

FWL_Error CFWL_PushButton::SetCaption(const CFX_WideStringC& wsCaption) {
  m_wsCaption = wsCaption;
  return FWL_Error::Succeeded;
}

CFX_DIBitmap* CFWL_PushButton::GetPicture() {
  return m_pBitmap;
}

FWL_Error CFWL_PushButton::SetPicture(CFX_DIBitmap* pBitmap) {
  m_pBitmap = pBitmap;
  return FWL_Error::Succeeded;
}

void CFWL_PushButton::GetCaption(IFWL_Widget* pWidget,
                                 CFX_WideString& wsCaption) {
  wsCaption = m_wsCaption;
}

CFX_DIBitmap* CFWL_PushButton::GetPicture(IFWL_Widget* pWidget) {
  return m_pBitmap;
}
