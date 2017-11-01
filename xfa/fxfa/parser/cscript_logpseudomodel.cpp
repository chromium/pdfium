// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_logpseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "third_party/base/ptr_util.h"

CScript_LogPseudoModel::CScript_LogPseudoModel(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::LogPseudoModel,
                  WideStringView(L"logPseudoModel"),
                  pdfium::MakeUnique<CJX_LogPseudoModel>(this)) {}

CScript_LogPseudoModel::~CScript_LogPseudoModel() {}

void CScript_LogPseudoModel::Message(CFXJSE_Arguments* pArguments) {
  JSLogPseudoModel()->Message(pArguments);
}

void CScript_LogPseudoModel::TraceEnabled(CFXJSE_Arguments* pArguments) {
  JSLogPseudoModel()->TraceEnabled(pArguments);
}

void CScript_LogPseudoModel::TraceActivate(CFXJSE_Arguments* pArguments) {
  JSLogPseudoModel()->TraceActivate(pArguments);
}

void CScript_LogPseudoModel::TraceDeactivate(CFXJSE_Arguments* pArguments) {
  JSLogPseudoModel()->TraceDeactivate(pArguments);
}

void CScript_LogPseudoModel::Trace(CFXJSE_Arguments* pArguments) {
  JSLogPseudoModel()->Trace(pArguments);
}
