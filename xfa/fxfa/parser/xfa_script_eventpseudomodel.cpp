// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_script_eventpseudomodel.h"

#include "xfa/fxfa/app/xfa_ffnotify.h"
#include "xfa/fxfa/fm2js/xfa_fm2jsapi.h"
#include "xfa/fxfa/include/xfa_ffwidgethandler.h"
#include "xfa/fxfa/parser/xfa_doclayout.h"
#include "xfa/fxfa/parser/xfa_document.h"
#include "xfa/fxfa/parser/xfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_parser.h"
#include "xfa/fxfa/parser/xfa_parser_imp.h"
#include "xfa/fxfa/parser/xfa_script.h"
#include "xfa/fxfa/parser/xfa_script_imp.h"
#include "xfa/fxfa/parser/xfa_utils.h"
#include "xfa/fxjse/cfxjse_arguments.h"

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
    wsValue = CFX_WideString::FromUTF8(bsValue.AsStringC());
  } else {
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue).AsStringC());
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
  if (bSetting)
    bValue = FXJSE_Value_ToBoolean(hValue);
  else
    FXJSE_Value_SetBoolean(hValue, bValue);
}

void CScript_EventPseudoModel::Script_EventPseudoModel_Property(
    FXJSE_HVALUE hValue,
    XFA_Event dwFlag,
    FX_BOOL bSetting) {
  CXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext)
    return;

  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam)
    return;

  switch (dwFlag) {
    case XFA_Event::CancelAction:
      Script_EventPseudoModel_BooleanProperty(
          hValue, pEventParam->m_bCancelAction, bSetting);
      break;
    case XFA_Event::Change:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsChange,
                                             bSetting);
      break;
    case XFA_Event::CommitKey:
      Script_EventPseudoModel_InterProperty(hValue, pEventParam->m_iCommitKey,
                                            bSetting);
      break;
    case XFA_Event::FullText:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsFullText,
                                             bSetting);
      break;
    case XFA_Event::Keydown:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bKeyDown,
                                              bSetting);
      break;
    case XFA_Event::Modifier:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bModifier,
                                              bSetting);
      break;
    case XFA_Event::NewContentType:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsNewContentType, bSetting);
      break;
    case XFA_Event::NewText:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsNewText,
                                             bSetting);
      break;
    case XFA_Event::PreviousContentType:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsPrevContentType, bSetting);
      break;
    case XFA_Event::PreviousText:
      Script_EventPseudoModel_StringProperty(hValue, pEventParam->m_wsPrevText,
                                             bSetting);
      break;
    case XFA_Event::Reenter:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bReenter,
                                              bSetting);
      break;
    case XFA_Event::SelectionEnd:
      Script_EventPseudoModel_InterProperty(hValue, pEventParam->m_iSelEnd,
                                            bSetting);
      break;
    case XFA_Event::SelectionStart:
      Script_EventPseudoModel_InterProperty(hValue, pEventParam->m_iSelStart,
                                            bSetting);
      break;
    case XFA_Event::Shift:
      Script_EventPseudoModel_BooleanProperty(hValue, pEventParam->m_bShift,
                                              bSetting);
      break;
    case XFA_Event::SoapFaultCode:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsSoapFaultCode, bSetting);
      break;
    case XFA_Event::SoapFaultString:
      Script_EventPseudoModel_StringProperty(
          hValue, pEventParam->m_wsSoapFaultString, bSetting);
      break;
    case XFA_Event::Target:
      break;
    default:
      break;
  }
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Change(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::Change, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_CommitKey(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::CommitKey, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_FullText(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::FullText, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_KeyDown(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::Keydown, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Modifier(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::Modifier, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_NewContentType(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::NewContentType, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_NewText(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::NewText, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_PrevContentType(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::PreviousContentType,
                                   bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_PrevText(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::PreviousText, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Reenter(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::Reenter, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SelEnd(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::SelectionEnd, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SelStart(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::SelectionStart, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Shift(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::Shift, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SoapFaultCode(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::SoapFaultCode, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_SoapFaultString(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::SoapFaultString,
                                   bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Target(
    FXJSE_HVALUE hValue,
    FX_BOOL bSetting,
    XFA_ATTRIBUTE eAttribute) {
  Script_EventPseudoModel_Property(hValue, XFA_Event::Target, bSetting);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Emit(
    CFXJSE_Arguments* pArguments) {
  CXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam) {
    return;
  }
  CXFA_FFNotify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  CXFA_FFWidgetHandler* pWidgetHandler = pNotify->GetWidgetHandler();
  if (!pWidgetHandler) {
    return;
  }
  pWidgetHandler->ProcessEvent(pEventParam->m_pTarget, pEventParam);
}
void CScript_EventPseudoModel::Script_EventPseudoModel_Reset(
    CFXJSE_Arguments* pArguments) {
  CXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam) {
    return;
  }
  pEventParam->Reset();
}
