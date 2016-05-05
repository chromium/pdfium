// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/lightwidget/cfwl_widgetdelegate.h"

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_message.h"

CFWL_WidgetDelegate::CFWL_WidgetDelegate() {}

CFWL_WidgetDelegate::~CFWL_WidgetDelegate() {}

void CFWL_WidgetDelegate::OnProcessMessage(CFWL_Message* pMessage) {}

void CFWL_WidgetDelegate::OnProcessEvent(CFWL_Event* pEvent) {}

void CFWL_WidgetDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {}
