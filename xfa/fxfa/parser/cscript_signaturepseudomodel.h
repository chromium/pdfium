// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CSCRIPT_SIGNATUREPSEUDOMODEL_H_
#define XFA_FXFA_PARSER_CSCRIPT_SIGNATUREPSEUDOMODEL_H_

#include "xfa/fxfa/parser/cxfa_object.h"

class CFXJSE_Arguments;

class CScript_SignaturePseudoModel : public CXFA_Object {
 public:
  explicit CScript_SignaturePseudoModel(CXFA_Document* pDocument);
  ~CScript_SignaturePseudoModel() override;

  void Verify(CFXJSE_Arguments* pArguments);
  void Sign(CFXJSE_Arguments* pArguments);
  void Enumerate(CFXJSE_Arguments* pArguments);
  void Clear(CFXJSE_Arguments* pArguments);
};

#endif  // XFA_FXFA_PARSER_CSCRIPT_SIGNATUREPSEUDOMODEL_H_
