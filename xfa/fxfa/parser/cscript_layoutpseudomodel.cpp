// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"

#include "fxjs/xfa/cjx_layoutpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CScript_LayoutPseudoModel::CScript_LayoutPseudoModel(CXFA_Document* doc)
    : CXFA_Object(doc,
                  XFA_ObjectType::Object,
                  XFA_Element::LayoutPseudoModel,
                  cppgc::MakeGarbageCollected<CJX_LayoutPseudoModel>(
                      doc->GetHeap()->GetAllocationHandle(),
                      this)) {}

CScript_LayoutPseudoModel::~CScript_LayoutPseudoModel() = default;
