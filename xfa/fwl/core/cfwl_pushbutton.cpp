// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_pushbutton.h"

#include <memory>

#include "third_party/base/ptr_util.h"

CFWL_PushButton::CFWL_PushButton(const CFWL_App* app) : CFWL_Widget(app) {}

CFWL_PushButton::~CFWL_PushButton() {}

void CFWL_PushButton::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_PushButton>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>());

  CFWL_Widget::Initialize();
}

