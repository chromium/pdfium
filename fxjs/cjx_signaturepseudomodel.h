// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_SIGNATUREPSEUDOMODEL_H_
#define FXJS_CJX_SIGNATUREPSEUDOMODEL_H_

#include "fxjs/cjx_object.h"

class CFXJSE_Arguments;
class CFXJSE_Value;
class CScript_SignaturePseudoModel;

class CJX_SignaturePseudoModel : public CJX_Object {
 public:
  explicit CJX_SignaturePseudoModel(CScript_SignaturePseudoModel* model);
  ~CJX_SignaturePseudoModel() override;

  void Verify(CFXJSE_Arguments* pArguments);
  void Sign(CFXJSE_Arguments* pArguments);
  void Enumerate(CFXJSE_Arguments* pArguments);
  void Clear(CFXJSE_Arguments* pArguments);
};

#endif  // FXJS_CJX_SIGNATUREPSEUDOMODEL_H_
