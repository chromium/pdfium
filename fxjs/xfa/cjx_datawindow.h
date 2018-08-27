// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_DATAWINDOW_H_
#define FXJS_XFA_CJX_DATAWINDOW_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Value;
class CScript_DataWindow;

class CJX_DataWindow final : public CJX_Object {
 public:
  explicit CJX_DataWindow(CScript_DataWindow* window);
  ~CJX_DataWindow() override;

  JSE_METHOD(gotoRecord, CJX_DataWindow);
  JSE_METHOD(isRecordGroup, CJX_DataWindow);
  JSE_METHOD(moveCurrentRecord, CJX_DataWindow);
  JSE_METHOD(record, CJX_DataWindow);

  JSE_PROP(currentRecordNumber);
  JSE_PROP(isDefined);
  JSE_PROP(recordsAfter);
  JSE_PROP(recordsBefore);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_DATAWINDOW_H_
