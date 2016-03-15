// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_script_logpseudomodel.h"

#include "xfa/fxfa/fm2js/xfa_fm2jsapi.h"
#include "xfa/fxfa/parser/xfa_docdata.h"
#include "xfa/fxfa/parser/xfa_doclayout.h"
#include "xfa/fxfa/parser/xfa_document.h"
#include "xfa/fxfa/parser/xfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_parser.h"
#include "xfa/fxfa/parser/xfa_script.h"
#include "xfa/fxfa/parser/xfa_utils.h"
#include "xfa/fxjse/cfxjse_arguments.h"

CScript_LogPseudoModel::CScript_LogPseudoModel(CXFA_Document* pDocument)
    : CXFA_OrdinaryObject(pDocument, XFA_ELEMENT_LogPseudoModel) {
  m_uScriptHash = XFA_HASHCODE_Log;
}
CScript_LogPseudoModel::~CScript_LogPseudoModel() {}
void CScript_LogPseudoModel::Script_LogPseudoModel_Message(
    CFXJSE_Arguments* pArguments) {}
void CScript_LogPseudoModel::Script_LogPseudoModel_TraceEnabled(
    CFXJSE_Arguments* pArguments) {}
void CScript_LogPseudoModel::Script_LogPseudoModel_TraceActivate(
    CFXJSE_Arguments* pArguments) {}
void CScript_LogPseudoModel::Script_LogPseudoModel_TraceDeactivate(
    CFXJSE_Arguments* pArguments) {}
void CScript_LogPseudoModel::Script_LogPseudoModel_Trace(
    CFXJSE_Arguments* pArguments) {}
