// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_datawindow.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_datawindow.h"

CJX_DataWindow::CJX_DataWindow(CScript_DataWindow* window)
    : CJX_Object(window) {}

CJX_DataWindow::~CJX_DataWindow() {}

void CJX_DataWindow::MoveCurrentRecord(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::Record(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::GotoRecord(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::IsRecordGroup(CFXJSE_Arguments* pArguments) {}

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
