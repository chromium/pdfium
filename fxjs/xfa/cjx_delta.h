// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_DELTA_H_
#define FXJS_XFA_CJX_DELTA_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_object.h"

class CXFA_Delta;

class CJX_Delta final : public CJX_Object {
 public:
  explicit CJX_Delta(CXFA_Delta* delta);
  ~CJX_Delta() override;

  JSE_METHOD(restore, CJX_Delta);

  JSE_PROP(currentValue);
  JSE_PROP(savedValue);
  JSE_PROP(target);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_DELTA_H_
