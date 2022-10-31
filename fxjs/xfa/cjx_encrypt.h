// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_ENCRYPT_H_
#define FXJS_XFA_CJX_ENCRYPT_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Encrypt;

class CJX_Encrypt final : public CJX_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Encrypt() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_PROP(format);

 private:
  explicit CJX_Encrypt(CXFA_Encrypt* node);

  using Type__ = CJX_Encrypt;
  using ParentType__ = CJX_Node;

  static const TypeTag static_type__ = TypeTag::Encrypt;
};

#endif  // FXJS_XFA_CJX_ENCRYPT_H_
