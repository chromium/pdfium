// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_datawindow.h"

#include "fxjs/xfa/cjx_datawindow.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CScript_DataWindow::CScript_DataWindow(CXFA_Document* doc)
    : CXFA_Object(doc,
                  XFA_ObjectType::Object,
                  XFA_Element::DataWindow,
                  cppgc::MakeGarbageCollected<CJX_DataWindow>(
                      doc->GetHeap()->GetAllocationHandle(),
                      this)) {}

CScript_DataWindow::~CScript_DataWindow() = default;
