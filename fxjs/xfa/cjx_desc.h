// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_DESC_H_
#define FXJS_XFA_CJX_DESC_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Desc;

class CJX_Desc final : public CJX_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Desc() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(metadata);

 private:
  explicit CJX_Desc(CXFA_Desc* desc);

  using Type__ = CJX_Desc;
  using ParentType__ = CJX_Node;

  static const TypeTag static_type__ = TypeTag::Desc;
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_DESC_H_
