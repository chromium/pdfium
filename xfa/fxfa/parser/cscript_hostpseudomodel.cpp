// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_hostpseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "third_party/base/ptr_util.h"

CScript_HostPseudoModel::CScript_HostPseudoModel(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::HostPseudoModel,
                  WideStringView(L"hostPseudoModel"),
                  pdfium::MakeUnique<CJX_HostPseudoModel>(this)) {}

CScript_HostPseudoModel::~CScript_HostPseudoModel() {}

void CScript_HostPseudoModel::AppType(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->AppType(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::CalculationsEnabled(CFXJSE_Value* pValue,
                                                  bool bSetting,
                                                  XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->CalculationsEnabled(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::CurrentPage(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->CurrentPage(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::Language(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->Language(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::NumPages(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->NumPages(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::Platform(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->Platform(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::Title(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->Title(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::ValidationsEnabled(CFXJSE_Value* pValue,
                                                 bool bSetting,
                                                 XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->ValidationsEnabled(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::Variation(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->Variation(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::Version(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->Version(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::Name(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute) {
  JSHostPseudoModel()->Name(pValue, bSetting, eAttribute);
}

void CScript_HostPseudoModel::GotoURL(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->GotoURL(pArguments);
}

void CScript_HostPseudoModel::OpenList(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->OpenList(pArguments);
}

void CScript_HostPseudoModel::Response(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->Response(pArguments);
}

void CScript_HostPseudoModel::DocumentInBatch(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->DocumentInBatch(pArguments);
}

void CScript_HostPseudoModel::ResetData(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->ResetData(pArguments);
}

void CScript_HostPseudoModel::Beep(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->Beep(pArguments);
}

void CScript_HostPseudoModel::SetFocus(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->SetFocus(pArguments);
}

void CScript_HostPseudoModel::GetFocus(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->GetFocus(pArguments);
}

void CScript_HostPseudoModel::MessageBox(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->MessageBox(pArguments);
}

void CScript_HostPseudoModel::DocumentCountInBatch(
    CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->DocumentCountInBatch(pArguments);
}

void CScript_HostPseudoModel::Print(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->Print(pArguments);
}

void CScript_HostPseudoModel::ImportData(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->ImportData(pArguments);
}

void CScript_HostPseudoModel::ExportData(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->ExportData(pArguments);
}

void CScript_HostPseudoModel::PageUp(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->PageUp(pArguments);
}

void CScript_HostPseudoModel::PageDown(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->PageDown(pArguments);
}

void CScript_HostPseudoModel::CurrentDateTime(CFXJSE_Arguments* pArguments) {
  JSHostPseudoModel()->CurrentDateTime(pArguments);
}
