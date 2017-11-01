// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_eventpseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cjx_object.h"
#include "third_party/base/ptr_util.h"

CScript_EventPseudoModel::CScript_EventPseudoModel(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::EventPseudoModel,
                  WideStringView(L"eventPseudoModel"),
                  pdfium::MakeUnique<CJX_EventPseudoModel>(this)) {}

CScript_EventPseudoModel::~CScript_EventPseudoModel() {}

void CScript_EventPseudoModel::Change(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->Change(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::CommitKey(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->CommitKey(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::FullText(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->FullText(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::KeyDown(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->KeyDown(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::Modifier(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->Modifier(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::NewContentType(CFXJSE_Value* pValue,
                                              bool bSetting,
                                              XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->NewContentType(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::NewText(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->NewText(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::PrevContentType(CFXJSE_Value* pValue,
                                               bool bSetting,
                                               XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->PrevContentType(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::PrevText(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->PrevText(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::Reenter(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->Reenter(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::SelEnd(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->SelEnd(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::SelStart(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->SelStart(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::Shift(CFXJSE_Value* pValue,
                                     bool bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->Shift(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::SoapFaultCode(CFXJSE_Value* pValue,
                                             bool bSetting,
                                             XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->SoapFaultString(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::SoapFaultString(CFXJSE_Value* pValue,
                                               bool bSetting,
                                               XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->SoapFaultString(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::Target(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  JSEventPseudoModel()->Target(pValue, bSetting, eAttribute);
}

void CScript_EventPseudoModel::Emit(CFXJSE_Arguments* pArguments) {
  JSEventPseudoModel()->Emit(pArguments);
}

void CScript_EventPseudoModel::Reset(CFXJSE_Arguments* pArguments) {
  JSEventPseudoModel()->Reset(pArguments);
}
