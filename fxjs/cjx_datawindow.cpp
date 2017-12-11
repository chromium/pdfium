// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_datawindow.h"

#include <vector>

#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_datawindow.h"

const CJX_MethodSpec CJX_DataWindow::MethodSpecs[] = {
    {"gotoRecord", gotoRecord_static},
    {"isRecordGroup", isRecordGroup_static},
    {"moveCurrentRecord", moveCurrentRecord_static},
    {"record", record_static},
    {"", nullptr}};

CJX_DataWindow::CJX_DataWindow(CScript_DataWindow* window)
    : CJX_Object(window) {
  DefineMethods(MethodSpecs);
}

CJX_DataWindow::~CJX_DataWindow() {}

CJS_Return CJX_DataWindow::moveCurrentRecord(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

CJS_Return CJX_DataWindow::record(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

CJS_Return CJX_DataWindow::gotoRecord(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

CJS_Return CJX_DataWindow::isRecordGroup(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

void CJX_DataWindow::RecordsBefore(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {}

void CJX_DataWindow::CurrentRecordNumber(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_DataWindow::RecordsAfter(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {}

void CJX_DataWindow::IsDefined(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {}
