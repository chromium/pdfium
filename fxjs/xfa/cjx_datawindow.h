// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_DATAWINDOW_H_
#define FXJS_XFA_CJX_DATAWINDOW_H_

#include "fxjs/xfa/cjx_object.h"
#include "fxjs/xfa/jse_define.h"
#include "xfa/fxfa/fxfa_basic.h"

class CScript_DataWindow;

class CJX_DataWindow final : public CJX_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_DataWindow() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(gotoRecord);
  JSE_METHOD(isRecordGroup);
  JSE_METHOD(moveCurrentRecord);
  JSE_METHOD(record);

  JSE_PROP(currentRecordNumber);
  JSE_PROP(isDefined);
  JSE_PROP(recordsAfter);
  JSE_PROP(recordsBefore);

 private:
  explicit CJX_DataWindow(CScript_DataWindow* window);

  using Type__ = CJX_DataWindow;
  using ParentType__ = CJX_Object;

  static const TypeTag static_type__ = TypeTag::DataWindow;
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_DATAWINDOW_H_
