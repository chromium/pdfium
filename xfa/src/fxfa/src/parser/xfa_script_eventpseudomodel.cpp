// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_script_eventpseudomodel.h"
CScript_EventPseudoModel::CScript_EventPseudoModel(CXFA_Document* pDocument)
    : CXFA_OrdinaryObject(pDocument, XFA_ELEMENT_EventPseudoModel) {
  m_uScriptHash = XFA_HASHCODE_Event;
}
CScript_EventPseudoModel::~CScript_EventPseudoModel() {}
void Script_EventPseudoModel_StringProperty(FXJSE_HVALUE hValue,
                                            CFX_WideString& wsValue,
                                            FX_BOOL bSetting) {
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsValue = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
  } else {
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue));
  }
}
void Script_EventPseudoModel_InterProperty(FXJSE_HVALUE hValue,
                                           int32_t& iValue,
                                           FX_BOOL bSetting) {
  if (bSetting) {
    iValue = FXJSE_Value_ToInteger(hValue);
  } else {
    FXJSE_Value_SetInteger(hValue, iValue);
  }
}
void Script_EventPseudoModel_BooleanProperty(FXJSE_HVALUE hValue,
                                             FX_BOOL& bValue,
                                             FX_BOOL bSetting) {
  if (bSetting) {
    bValue = FXJSE_Value_ToBoolean(hValue);
  } else {
    FXJSE_Value_SetBoolean(hValue, bValue);
  }
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Property(
    FXJSE_HVALUE hValue,
    FX_DWORD dwFlag,
    FX_BOOL bSetting) {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam) {
    return;
  }
  switch (dwFlag) {
    case XFA_EVENT_CANCELACTION:
      Script_EventPseudoModel_BooleanProperty(
          hValue, pEventParam->m_bCancelAction, bSetting);
      break;
    case XFA_EVENT_CHANGE:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsChange,
                                             bSetting);
      break;
    case XFA_EVENT_COMMITKEY:
      Script_EventPseudoModel_InterProperty(hValue, pEventParam->m_iCommitKey,
                                            bSetting);
      break;
    case XFA_EVENT_FULLTEXT:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsFullText,
                                             bSetting);
      break;
    case XFA_EVENT_KEYDOWN:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bKeyDown,
                                              bSetting);
      break;
    case XFA_EVENT_MODIFIER:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bModifier,
                                              bSetting);
      break;
    case XFA_EVENT_NEWCONTENTTYPE:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsNewContentType, bSetting);
      break;
    case XFA_EVENT_NEWTEXT:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsNewText,
                                             bSetting);
      break;
    case XFA_EVENT_PREVCONTENTTYPE:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsPrevContentType, bSetting);
      break;
    case XFA_EVENT_PREVTEXT:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsPrevText,
                                             bSetting);
      break;
    case XFA_EVENT_REENTER:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bReenter,
                                              bSetting);
      break;
    case XFA_EVENT_SELEND:
      Script_EventPseudoModel_InterProperty(hValue, pEventParam->m_iSelEnd,
                                            bSetting);
      break;
    case XFA_EVENT_SELSTART:
      Script_EventPseudoModel_InterProperty(hValue, pEventParam->m_iSelStart,
                                            bSetting);
      break;
    case XFA_EVENT_SHIFT:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bShift,
                                              bSetting);
      break;
    case XFA_EVENT_SOAPFAULTCODE:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsSoapFaultCode, bSetting);
      break;
    case XFA_EVENT_SOAPFAULTSTRING:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsSoapFaultString, bSetting);
      break;
    case XFA_EVENT_TARGET:
      break;
    default:
      break;
  }
}
void CScript_EventPseudoModel::Script_EventPseudoModel_CancelAction(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_CANCELACTION, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Change(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_CHANGE, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_CommitKey(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_COMMITKEY, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_FullText(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_FULLTEXT, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_KeyDown(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_KEYDOWN, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Modifier(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_MODIFIER, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_NewContentType(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_NEWCONTENTTYPE, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_NewText(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_NEWTEXT, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_PrevContentType(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_PREVCONTENTTYPE, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_PrevText(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_PREVTEXT, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Reenter(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_REENTER, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SelEnd(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_SELEND, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SelStart(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_SELSTART, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Shift(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_SHIFT, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SoapFaultCode(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_SOAPFAULTCODE, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SoapFaultString(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_SOAPFAULTSTRING, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Target(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_EVENT_TARGET, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Emit(
    CFXJSE_Arguments* pArguments) {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam) {
    return;
  }
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  IXFA_WidgetHandler* pWidgetHandler = pNotify->GetWidgetHandler();
  if (!pWidgetHandler) {
    return;
  }
  pWidgetHandler->ProcessEvent(pEventParam->m_pTarget, pEventParam);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Reset(
    CFXJSE_Arguments* pArguments) {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam) {
    return;
  }
  pEventParam->Reset();
}
