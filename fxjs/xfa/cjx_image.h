// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_IMAGE_H_
#define FXJS_XFA_CJX_IMAGE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Image;

class CJX_Image final : public CJX_Node {
 public:
  explicit CJX_Image(CXFA_Image* node);
  ~CJX_Image() override;

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(aspect);
  JSE_PROP(contentType);
  JSE_PROP(href);
  JSE_PROP(transferEncoding);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(value);
};

#endif  // FXJS_XFA_CJX_IMAGE_H_
