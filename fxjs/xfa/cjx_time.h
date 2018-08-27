// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_TIME_H_
#define FXJS_XFA_CJX_TIME_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_content.h"

class CXFA_Time;

class CJX_Time final : public CJX_Content {
 public:
  explicit CJX_Time(CXFA_Time* node);
  ~CJX_Time() override;

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(value);
};

#endif  // FXJS_XFA_CJX_TIME_H_
