// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_hostpseudomodel.h"

#include "fxjs/xfa/cjx_hostpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CScript_HostPseudoModel::CScript_HostPseudoModel(CXFA_Document* doc)
    : CXFA_Object(doc,
                  XFA_ObjectType::Object,
                  XFA_Element::HostPseudoModel,
                  cppgc::MakeGarbageCollected<CJX_HostPseudoModel>(
                      doc->GetHeap()->GetAllocationHandle(),
                      this)) {}

CScript_HostPseudoModel::~CScript_HostPseudoModel() = default;
