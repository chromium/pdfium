// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_eventpseudomodel.h"

#include "fxjs/xfa/cjx_eventpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CScript_EventPseudoModel::CScript_EventPseudoModel(CXFA_Document* doc)
    : CXFA_Object(doc,
                  XFA_ObjectType::Object,
                  XFA_Element::EventPseudoModel,
                  cppgc::MakeGarbageCollected<CJX_EventPseudoModel>(
                      doc->GetHeap()->GetAllocationHandle(),
                      this)) {}

CScript_EventPseudoModel::~CScript_EventPseudoModel() = default;
