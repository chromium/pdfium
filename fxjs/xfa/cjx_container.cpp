// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_container.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_field.h"

const CJX_MethodSpec CJX_Container::MethodSpecs[] = {
    {"getDelta", getDelta_static},
    {"getDeltas", getDeltas_static},
    {"", nullptr}};

CJX_Container::CJX_Container(CXFA_Node* node) : CJX_Node(node) {
  DefineMethods(MethodSpecs);
}

CJX_Container::~CJX_Container() {}

void CJX_Container::getDelta(CFXJSE_Arguments* pArguments) {}

void CJX_Container::getDeltas(CFXJSE_Arguments* pArguments) {
  CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(GetDocument());
  pArguments->GetReturnValue()->SetObject(
      pFormNodes, GetDocument()->GetScriptContext()->GetJseNormalClass());
}
