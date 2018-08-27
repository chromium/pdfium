// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_DATAVALUE_H_
#define FXJS_XFA_CJX_DATAVALUE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_DataValue;

class CJX_DataValue final : public CJX_Node {
 public:
  explicit CJX_DataValue(CXFA_DataValue* node);
  ~CJX_DataValue() override;

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(contains);
  JSE_PROP(contentType);
  JSE_PROP(isNull);
  JSE_PROP(value);
};

#endif  // FXJS_XFA_CJX_DATAVALUE_H_
