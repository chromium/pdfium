// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_SIGNATUREPSEUDOMODEL_H_
#define FXJS_XFA_CJX_SIGNATUREPSEUDOMODEL_H_

#include "fxjs/xfa/cjx_object.h"
#include "fxjs/xfa/jse_define.h"

class CScript_SignaturePseudoModel;

class CJX_SignaturePseudoModel final : public CJX_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_SignaturePseudoModel() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(verifySignature /*verify*/);
  JSE_METHOD(sign);
  JSE_METHOD(enumerate);
  JSE_METHOD(clear);

 private:
  explicit CJX_SignaturePseudoModel(CScript_SignaturePseudoModel* model);

  using Type__ = CJX_SignaturePseudoModel;
  using ParentType__ = CJX_Object;

  static const TypeTag static_type__ = TypeTag::SignaturePesudoModel;
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_SIGNATUREPSEUDOMODEL_H_
