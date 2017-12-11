// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_DATAWINDOW_H_
#define FXJS_XFA_CJX_DATAWINDOW_H_

#include "fxjs/CJX_Define.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Value;
class CScript_DataWindow;

class CJX_DataWindow : public CJX_Object {
 public:
  explicit CJX_DataWindow(CScript_DataWindow* window);
  ~CJX_DataWindow() override;

  void RecordsBefore(CFXJSE_Value* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute);
  void CurrentRecordNumber(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void RecordsAfter(CFXJSE_Value* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute);
  void IsDefined(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);

  JS_METHOD(gotoRecord, CJX_DataWindow);
  JS_METHOD(isRecordGroup, CJX_DataWindow);
  JS_METHOD(moveCurrentRecord, CJX_DataWindow);
  JS_METHOD(record, CJX_DataWindow);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_DATAWINDOW_H_
