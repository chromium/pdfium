// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_LOGPSEUDOMODEL_H_
#define FXJS_CJX_LOGPSEUDOMODEL_H_

#include "fxjs/cjx_object.h"

class CFXJSE_Arguments;
class CFXJSE_Value;
class CScript_LogPseudoModel;

class CJX_LogPseudoModel : public CJX_Object {
 public:
  explicit CJX_LogPseudoModel(CScript_LogPseudoModel* model);
  ~CJX_LogPseudoModel() override;

  void Message(CFXJSE_Arguments* pArguments);
  void TraceEnabled(CFXJSE_Arguments* pArguments);
  void TraceActivate(CFXJSE_Arguments* pArguments);
  void TraceDeactivate(CFXJSE_Arguments* pArguments);
  void Trace(CFXJSE_Arguments* pArguments);
};

#endif  // FXJS_CJX_LOGPSEUDOMODEL_H_
