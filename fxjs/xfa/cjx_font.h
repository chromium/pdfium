// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_FONT_H_
#define FXJS_XFA_CJX_FONT_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Font;

class CJX_Font final : public CJX_Node {
 public:
  explicit CJX_Font(CXFA_Font* node);
  ~CJX_Font() override;

  JSE_PROP(baselineShift);
  JSE_PROP(fontHorizontalScale);
  JSE_PROP(fontVerticalScale);
  JSE_PROP(kerningMode);
  JSE_PROP(letterSpacing);
  JSE_PROP(lineThrough);
  JSE_PROP(lineThroughPeriod);
  JSE_PROP(overline);
  JSE_PROP(overlinePeriod);
  JSE_PROP(posture);
  JSE_PROP(size);
  JSE_PROP(typeface);
  JSE_PROP(underline);
  JSE_PROP(underlinePeriod);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(weight);
};

#endif  // FXJS_XFA_CJX_FONT_H_
