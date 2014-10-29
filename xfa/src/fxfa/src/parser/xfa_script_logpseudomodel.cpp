// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_utils.h"
#include "../common/xfa_object.h"
#include "../common/xfa_document.h"
#include "../common/xfa_parser.h"
#include "../common/xfa_script.h"
#include "../common/xfa_docdata.h"
#include "../common/xfa_doclayout.h"
#include "../common/xfa_debug.h"
#include "../common/xfa_localemgr.h"
#include "../common/xfa_fm2jsapi.h"
#include "xfa_debug_parser.h"
#include "xfa_script_logpseudomodel.h"
CScript_LogPseudoModel::CScript_LogPseudoModel(CXFA_Document* pDocument)
    : CXFA_OrdinaryObject(pDocument, XFA_ELEMENT_LogPseudoModel)
{
    m_uScriptHash = XFA_HASHCODE_Log;
}
CScript_LogPseudoModel::~CScript_LogPseudoModel()
{
}
void CScript_LogPseudoModel::Script_LogPseudoModel_Message(CFXJSE_Arguments* pArguments)
{
}
void CScript_LogPseudoModel::Script_LogPseudoModel_TraceEnabled(CFXJSE_Arguments* pArguments)
{
}
void CScript_LogPseudoModel::Script_LogPseudoModel_TraceActivate(CFXJSE_Arguments* pArguments)
{
}
void CScript_LogPseudoModel::Script_LogPseudoModel_TraceDeactivate(CFXJSE_Arguments* pArguments)
{
}
void CScript_LogPseudoModel::Script_LogPseudoModel_Trace(CFXJSE_Arguments* pArguments)
{
}
