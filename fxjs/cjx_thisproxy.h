// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_THISPROXY_H_
#define FXJS_CJX_THISPROXY_H_

#include "fxjs/cjx_object.h"

class CXFA_ThisProxy;

class CJX_ThisProxy : public CJX_Object {
 public:
  explicit CJX_ThisProxy(CXFA_ThisProxy* proxy);
  ~CJX_ThisProxy() override;
};

#endif  // FXJS_CJX_THISPROXY_H_
