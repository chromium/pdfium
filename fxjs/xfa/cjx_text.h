// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_TEXT_H_
#define FXJS_XFA_CJX_TEXT_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_content.h"

class CXFA_Text;

class CJX_Text final : public CJX_Content {
 public:
  explicit CJX_Text(CXFA_Text* node);
  ~CJX_Text() override;

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(maxChars);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(value);
};

#endif  // FXJS_XFA_CJX_TEXT_H_
