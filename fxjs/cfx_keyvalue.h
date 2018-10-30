// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CFX_KEYVALUE_H_
#define FXJS_CFX_KEYVALUE_H_

#include "core/fxcrt/fx_string.h"
#include "fxjs/cfx_globalarray.h"

enum class JS_GlobalDataType { NUMBER = 0, BOOLEAN, STRING, OBJECT, NULLOBJ };

class CFX_KeyValue {
 public:
  CFX_KeyValue();
  ~CFX_KeyValue();

  ByteString sKey;
  JS_GlobalDataType nType;
  double dData;
  bool bData;
  ByteString sData;
  CFX_GlobalArray objData;
};

#endif  // FXJS_CFX_KEYVALUE_H_
