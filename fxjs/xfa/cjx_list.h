// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_LIST_H_
#define FXJS_XFA_CJX_LIST_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_object.h"

class CXFA_List;

class CJX_List : public CJX_Object {
 public:
  explicit CJX_List(CXFA_List* list);
  ~CJX_List() override;

  JSE_METHOD(append, CJX_List);
  JSE_METHOD(insert, CJX_List);
  JSE_METHOD(item, CJX_List);
  JSE_METHOD(remove, CJX_List);

  JSE_PROP(length);

 private:
  CXFA_List* GetXFAList();

  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_LIST_H_
