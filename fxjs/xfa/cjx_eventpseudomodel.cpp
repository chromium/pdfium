// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_eventpseudomodel.h"

#include <algorithm>
#include <vector>

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "third_party/base/notreached.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cscript_eventpseudomodel.h"

namespace {

void StringProperty(v8::Isolate* pIsolate,
                    v8::Local<v8::Value>* pReturn,
                    WideString* wsValue,
                    bool bSetting) {
  if (bSetting) {
    *wsValue = fxv8::ReentrantToWideStringHelper(pIsolate, *pReturn);
    return;
  }
  *pReturn = fxv8::NewStringHelper(pIsolate, wsValue->ToUTF8().AsStringView());
}

void IntegerProperty(v8::Isolate* pIsolate,
                     v8::Local<v8::Value>* pReturn,
                     int32_t* iValue,
                     bool bSetting) {
  if (bSetting) {
    *iValue = fxv8::ReentrantToInt32Helper(pIsolate, *pReturn);
    return;
  }
  *pReturn = fxv8::NewNumberHelper(pIsolate, *iValue);
}

void BooleanProperty(v8::Isolate* pIsolate,
                     v8::Local<v8::Value>* pReturn,
                     bool* bValue,
                     bool bSetting) {
  if (bSetting) {
    *bValue = fxv8::ReentrantToBooleanHelper(pIsolate, *pReturn);
    return;
  }
  *pReturn = fxv8::NewBooleanHelper(pIsolate, *bValue);
}

}  // namespace

const CJX_MethodSpec CJX_EventPseudoModel::MethodSpecs[] = {
    {"emit", emit_static},
    {"reset", reset_static}};

CJX_EventPseudoModel::CJX_EventPseudoModel(CScript_EventPseudoModel* model)
    : CJX_Object(model) {
  DefineMethods(MethodSpecs);
}

CJX_EventPseudoModel::~CJX_EventPseudoModel() = default;

bool CJX_EventPseudoModel::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_EventPseudoModel::cancelAction(v8::Isolate* pIsolate,
                                        v8::Local<v8::Value>* pValue,
                                        bool bSetting,
                                        XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::CancelAction, bSetting);
}

void CJX_EventPseudoModel::change(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::Change, bSetting);
}

void CJX_EventPseudoModel::commitKey(v8::Isolate* pIsolate,
                                     v8::Local<v8::Value>* pValue,
                                     bool bSetting,
                                     XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::CommitKey, bSetting);
}

void CJX_EventPseudoModel::fullText(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value>* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::FullText, bSetting);
}

void CJX_EventPseudoModel::keyDown(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::Keydown, bSetting);
}

void CJX_EventPseudoModel::modifier(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value>* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::Modifier, bSetting);
}

void CJX_EventPseudoModel::newContentType(v8::Isolate* pIsolate,
                                          v8::Local<v8::Value>* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::NewContentType, bSetting);
}

void CJX_EventPseudoModel::newText(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  if (bSetting)
    return;

  CXFA_EventParam* pEventParam =
      GetDocument()->GetScriptContext()->GetEventParam();
  if (!pEventParam)
    return;

  *pValue = fxv8::NewStringHelper(
      pIsolate, pEventParam->GetNewText().ToUTF8().AsStringView());
}

void CJX_EventPseudoModel::prevContentType(v8::Isolate* pIsolate,
                                           v8::Local<v8::Value>* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::PreviousContentType, bSetting);
}

void CJX_EventPseudoModel::prevText(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value>* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::PreviousText, bSetting);
}

void CJX_EventPseudoModel::reenter(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::Reenter, bSetting);
}

void CJX_EventPseudoModel::selEnd(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::SelectionEnd, bSetting);
}

void CJX_EventPseudoModel::selStart(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value>* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::SelectionStart, bSetting);
}

void CJX_EventPseudoModel::shift(v8::Isolate* pIsolate,
                                 v8::Local<v8::Value>* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::Shift, bSetting);
}

void CJX_EventPseudoModel::soapFaultCode(v8::Isolate* pIsolate,
                                         v8::Local<v8::Value>* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::SoapFaultCode, bSetting);
}

void CJX_EventPseudoModel::soapFaultString(v8::Isolate* pIsolate,
                                           v8::Local<v8::Value>* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::SoapFaultString, bSetting);
}

void CJX_EventPseudoModel::target(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  Property(pIsolate, pValue, XFA_Event::Target, bSetting);
}

CJS_Result CJX_EventPseudoModel::emit(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_EventParam* pEventParam = runtime->GetEventParam();
  if (!pEventParam)
    return CJS_Result::Success();

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  pNotify->HandleWidgetEvent(runtime->GetEventTarget(), pEventParam);
  return CJS_Result::Success();
}

CJS_Result CJX_EventPseudoModel::reset(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_EventParam* pEventParam = runtime->GetEventParam();
  if (pEventParam)
    *pEventParam = CXFA_EventParam();

  return CJS_Result::Success();
}

void CJX_EventPseudoModel::Property(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value>* pValue,
                                    XFA_Event dwFlag,
                                    bool bSetting) {
  // Only the cancelAction, selStart, selEnd and change properties are writable.
  if (bSetting && dwFlag != XFA_Event::CancelAction &&
      dwFlag != XFA_Event::SelectionStart &&
      dwFlag != XFA_Event::SelectionEnd && dwFlag != XFA_Event::Change) {
    return;
  }

  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  CXFA_EventParam* pEventParam = pScriptContext->GetEventParam();
  if (!pEventParam)
    return;

  switch (dwFlag) {
    case XFA_Event::CancelAction:
      BooleanProperty(pIsolate, pValue, &pEventParam->m_bCancelAction,
                      bSetting);
      break;
    case XFA_Event::Change:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsChange, bSetting);
      break;
    case XFA_Event::CommitKey:
      IntegerProperty(pIsolate, pValue, &pEventParam->m_iCommitKey, bSetting);
      break;
    case XFA_Event::FullText:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsFullText, bSetting);
      break;
    case XFA_Event::Keydown:
      BooleanProperty(pIsolate, pValue, &pEventParam->m_bKeyDown, bSetting);
      break;
    case XFA_Event::Modifier:
      BooleanProperty(pIsolate, pValue, &pEventParam->m_bModifier, bSetting);
      break;
    case XFA_Event::NewContentType:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsNewContentType,
                     bSetting);
      break;
    case XFA_Event::NewText:
      NOTREACHED_NORETURN();
    case XFA_Event::PreviousContentType:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsPrevContentType,
                     bSetting);
      break;
    case XFA_Event::PreviousText:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsPrevText, bSetting);
      break;
    case XFA_Event::Reenter:
      BooleanProperty(pIsolate, pValue, &pEventParam->m_bReenter, bSetting);
      break;
    case XFA_Event::SelectionEnd:
      IntegerProperty(pIsolate, pValue, &pEventParam->m_iSelEnd, bSetting);

      pEventParam->m_iSelEnd = std::max(0, pEventParam->m_iSelEnd);
      pEventParam->m_iSelEnd = std::min(
          pEventParam->m_iSelEnd, pdfium::base::checked_cast<int32_t>(
                                      pEventParam->m_wsPrevText.GetLength()));
      pEventParam->m_iSelStart =
          std::min(pEventParam->m_iSelStart, pEventParam->m_iSelEnd);
      break;
    case XFA_Event::SelectionStart:
      IntegerProperty(pIsolate, pValue, &pEventParam->m_iSelStart, bSetting);
      pEventParam->m_iSelStart = std::max(0, pEventParam->m_iSelStart);
      pEventParam->m_iSelStart = std::min(
          pEventParam->m_iSelStart, pdfium::base::checked_cast<int32_t>(
                                        pEventParam->m_wsPrevText.GetLength()));
      pEventParam->m_iSelEnd =
          std::max(pEventParam->m_iSelStart, pEventParam->m_iSelEnd);
      break;
    case XFA_Event::Shift:
      BooleanProperty(pIsolate, pValue, &pEventParam->m_bShift, bSetting);
      break;
    case XFA_Event::SoapFaultCode:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsSoapFaultCode,
                     bSetting);
      break;
    case XFA_Event::SoapFaultString:
      StringProperty(pIsolate, pValue, &pEventParam->m_wsSoapFaultString,
                     bSetting);
      break;
    case XFA_Event::Target:
      break;
  }
}
