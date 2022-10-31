// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_signaturepseudomodel.h"

#include "fxjs/xfa/cjx_signaturepseudomodel.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CScript_SignaturePseudoModel::CScript_SignaturePseudoModel(CXFA_Document* doc)
    : CXFA_Object(doc,
                  XFA_ObjectType::Object,
                  XFA_Element::SignaturePseudoModel,
                  cppgc::MakeGarbageCollected<CJX_SignaturePseudoModel>(
                      doc->GetHeap()->GetAllocationHandle(),
                      this)) {}

CScript_SignaturePseudoModel::~CScript_SignaturePseudoModel() = default;
