// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_list.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_treelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_List::CXFA_List(CXFA_Document* pDocument, CJX_Object* obj)
    : CXFA_List(pDocument, XFA_ObjectType::List, XFA_Element::List, obj) {}

CXFA_List::CXFA_List(CXFA_Document* pDocument,
                     XFA_ObjectType objectType,
                     XFA_Element eType,
                     CJX_Object* obj)
    : CXFA_Object(pDocument, objectType, eType, obj) {}

CXFA_List::~CXFA_List() = default;
