// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_SCRIPT_SIGNATUREPSEUDOMODEL_H_
#define XFA_FXFA_PARSER_XFA_SCRIPT_SIGNATUREPSEUDOMODEL_H_

#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxjse/cfxjse_arguments.h"

class CScript_SignaturePseudoModel : public CXFA_OrdinaryObject {
 public:
  CScript_SignaturePseudoModel(CXFA_Document* pDocument);
  ~CScript_SignaturePseudoModel();
  void Script_SignaturePseudoModel_Verify(CFXJSE_Arguments* pArguments);
  void Script_SignaturePseudoModel_Sign(CFXJSE_Arguments* pArguments);
  void Script_SignaturePseudoModel_Enumerate(CFXJSE_Arguments* pArguments);
  void Script_SignaturePseudoModel_Clear(CFXJSE_Arguments* pArguments);
};

#endif  // XFA_FXFA_PARSER_XFA_SCRIPT_SIGNATUREPSEUDOMODEL_H_
