// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_DATAWINDOW_H_
#define FXJS_CJX_DATAWINDOW_H_

#include "fxjs/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Arguments;
class CFXJSE_Value;
class CScript_DataWindow;

class CJX_DataWindow : public CJX_Object {
 public:
  explicit CJX_DataWindow(CScript_DataWindow* window);
  ~CJX_DataWindow() override;

  void MoveCurrentRecord(CFXJSE_Arguments* pArguments);
  void Record(CFXJSE_Arguments* pArguments);
  void GotoRecord(CFXJSE_Arguments* pArguments);
  void IsRecordGroup(CFXJSE_Arguments* pArguments);
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
};

#endif  // FXJS_CJX_DATAWINDOW_H_
