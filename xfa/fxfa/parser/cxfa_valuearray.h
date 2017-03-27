// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_VALUEARRAY_H_
#define XFA_FXFA_PARSER_CXFA_VALUEARRAY_H_

#include <memory>
#include <vector>

#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/fxfa.h"

class CXFA_ValueArray {
 public:
  explicit CXFA_ValueArray(v8::Isolate* pIsolate);
  ~CXFA_ValueArray();

  std::vector<CXFA_Object*> GetAttributeObject();

  v8::Isolate* const m_pIsolate;
  std::vector<std::unique_ptr<CFXJSE_Value>> m_Values;
};

#endif  // XFA_FXFA_PARSER_CXFA_VALUEARRAY_H_
