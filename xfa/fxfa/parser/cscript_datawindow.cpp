// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_datawindow.h"

#include <memory>

#include "fxjs/xfa/cjx_datawindow.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CScript_DataWindow::CScript_DataWindow(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::DataWindow,
                  std::make_unique<CJX_DataWindow>(this)) {}

CScript_DataWindow::~CScript_DataWindow() = default;
