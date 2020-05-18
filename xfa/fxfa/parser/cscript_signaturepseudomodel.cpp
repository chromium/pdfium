// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_signaturepseudomodel.h"

#include <memory>

#include "fxjs/xfa/cjx_signaturepseudomodel.h"

CScript_SignaturePseudoModel::CScript_SignaturePseudoModel(
    CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::SignaturePseudoModel,
                  std::make_unique<CJX_SignaturePseudoModel>(this)) {}

CScript_SignaturePseudoModel::~CScript_SignaturePseudoModel() = default;
