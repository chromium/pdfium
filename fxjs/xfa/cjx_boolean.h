// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_BOOLEAN_H_
#define FXJS_XFA_CJX_BOOLEAN_H_

#include "fxjs/xfa/cjx_object.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Boolean;

class CJX_Boolean final : public CJX_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Boolean() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(value);

 private:
  explicit CJX_Boolean(CXFA_Boolean* node);

  using Type__ = CJX_Boolean;
  using ParentType__ = CJX_Object;

  static const TypeTag static_type__ = TypeTag::Boolean;
};

#endif  // FXJS_XFA_CJX_BOOLEAN_H_
