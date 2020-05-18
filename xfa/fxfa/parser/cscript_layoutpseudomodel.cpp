// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"

#include <memory>

#include "fxjs/xfa/cjx_layoutpseudomodel.h"

CScript_LayoutPseudoModel::CScript_LayoutPseudoModel(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::LayoutPseudoModel,
                  std::make_unique<CJX_LayoutPseudoModel>(this)) {}

CScript_LayoutPseudoModel::~CScript_LayoutPseudoModel() = default;
