// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_SCRIPT_LOGPSEUDOMODEL_H_
#define XFA_FXFA_PARSER_XFA_SCRIPT_LOGPSEUDOMODEL_H_

#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxjse/cfxjse_arguments.h"

class CScript_LogPseudoModel : public CXFA_OrdinaryObject {
 public:
  explicit CScript_LogPseudoModel(CXFA_Document* pDocument);
  virtual ~CScript_LogPseudoModel();

  void Script_LogPseudoModel_Message(CFXJSE_Arguments* pArguments);
  void Script_LogPseudoModel_TraceEnabled(CFXJSE_Arguments* pArguments);
  void Script_LogPseudoModel_TraceActivate(CFXJSE_Arguments* pArguments);
  void Script_LogPseudoModel_TraceDeactivate(CFXJSE_Arguments* pArguments);
  void Script_LogPseudoModel_Trace(CFXJSE_Arguments* pArguments);
};

#endif  // XFA_FXFA_PARSER_XFA_SCRIPT_LOGPSEUDOMODEL_H_
