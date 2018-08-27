// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_LOGPSEUDOMODEL_H_
#define FXJS_XFA_CJX_LOGPSEUDOMODEL_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_object.h"

class CScript_LogPseudoModel;

// TODO(dsinclair): This doesn't exist in the spec. Remove after
// xfa_basic_data_element_script is removed.
class CJX_LogPseudoModel final : public CJX_Object {
 public:
  explicit CJX_LogPseudoModel(CScript_LogPseudoModel* model);
  ~CJX_LogPseudoModel() override;

  JSE_METHOD(message, CJX_LogPseudoModel);
  JSE_METHOD(traceEnabled, CJX_LogPseudoModel);
  JSE_METHOD(traceActivate, CJX_LogPseudoModel);
  JSE_METHOD(traceDeactivate, CJX_LogPseudoModel);
  JSE_METHOD(trace, CJX_LogPseudoModel);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_LOGPSEUDOMODEL_H_
