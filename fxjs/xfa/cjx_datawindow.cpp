// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_datawindow.h"

#include <vector>

#include "fxjs/xfa/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_datawindow.h"

const CJX_MethodSpec CJX_DataWindow::MethodSpecs[] = {
    {"gotoRecord", gotoRecord_static},
    {"isRecordGroup", isRecordGroup_static},
    {"moveCurrentRecord", moveCurrentRecord_static},
    {"record", record_static}};

CJX_DataWindow::CJX_DataWindow(CScript_DataWindow* window)
    : CJX_Object(window) {
  DefineMethods(MethodSpecs);
}

CJX_DataWindow::~CJX_DataWindow() = default;

bool CJX_DataWindow::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_DataWindow::moveCurrentRecord(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success();
}

CJS_Result CJX_DataWindow::record(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success();
}

CJS_Result CJX_DataWindow::gotoRecord(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success();
}

CJS_Result CJX_DataWindow::isRecordGroup(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success();
}

void CJX_DataWindow::recordsBefore(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {}

void CJX_DataWindow::currentRecordNumber(v8::Isolate* pIsolate,
                                         v8::Local<v8::Value>* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_DataWindow::recordsAfter(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {}

void CJX_DataWindow::isDefined(v8::Isolate* pIsolate,
                               v8::Local<v8::Value>* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {}
