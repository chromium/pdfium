// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_OCCUR_H_
#define FXJS_XFA_CJX_OCCUR_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Occur;

class CJX_Occur final : public CJX_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Occur() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_PROP(max);
  JSE_PROP(min);

 private:
  explicit CJX_Occur(CXFA_Occur* node);

  using Type__ = CJX_Occur;
  using ParentType__ = CJX_Node;

  static const TypeTag static_type__ = TypeTag::Occur;
};

#endif  // FXJS_XFA_CJX_OCCUR_H_
