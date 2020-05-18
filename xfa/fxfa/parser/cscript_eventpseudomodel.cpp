// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_eventpseudomodel.h"

#include <memory>

#include "fxjs/xfa/cjx_eventpseudomodel.h"

CScript_EventPseudoModel::CScript_EventPseudoModel(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::EventPseudoModel,
                  std::make_unique<CJX_EventPseudoModel>(this)) {}

CScript_EventPseudoModel::~CScript_EventPseudoModel() = default;
