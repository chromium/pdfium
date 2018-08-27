// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_PAGEAREA_H_
#define FXJS_XFA_CJX_PAGEAREA_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_container.h"

class CXFA_PageArea;

class CJX_PageArea final : public CJX_Container {
 public:
  explicit CJX_PageArea(CXFA_PageArea* node);
  ~CJX_PageArea() override;

  JSE_PROP(blankOrNotBlank);
  JSE_PROP(initialNumber);
  JSE_PROP(numbered);
  JSE_PROP(oddOrEven);
  JSE_PROP(pagePosition);
  JSE_PROP(relevant);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_PAGEAREA_H_
