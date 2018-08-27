// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_HOSTPSEUDOMODEL_H_
#define FXJS_XFA_CJX_HOSTPSEUDOMODEL_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Value;
class CScript_HostPseudoModel;

class CJX_HostPseudoModel final : public CJX_Object {
 public:
  explicit CJX_HostPseudoModel(CScript_HostPseudoModel* model);
  ~CJX_HostPseudoModel() override;

  JSE_METHOD(beep, CJX_HostPseudoModel);
  JSE_METHOD(documentCountInBatch, CJX_HostPseudoModel);
  JSE_METHOD(documentInBatch, CJX_HostPseudoModel);
  JSE_METHOD(exportData, CJX_HostPseudoModel);
  JSE_METHOD(getFocus, CJX_HostPseudoModel);
  JSE_METHOD(gotoURL, CJX_HostPseudoModel);
  JSE_METHOD(importData, CJX_HostPseudoModel);
  JSE_METHOD(messageBox, CJX_HostPseudoModel);
  JSE_METHOD(openList, CJX_HostPseudoModel);
  JSE_METHOD(pageDown, CJX_HostPseudoModel);
  JSE_METHOD(pageUp, CJX_HostPseudoModel);
  JSE_METHOD(print, CJX_HostPseudoModel);
  JSE_METHOD(resetData, CJX_HostPseudoModel);
  JSE_METHOD(response, CJX_HostPseudoModel);
  JSE_METHOD(setFocus, CJX_HostPseudoModel);

  JSE_PROP(appType);
  JSE_PROP(calculationsEnabled);
  JSE_PROP(currentPage);
  JSE_PROP(language);
  JSE_PROP(numPages);
  JSE_PROP(platform);
  JSE_PROP(title);
  JSE_PROP(validationsEnabled);
  JSE_PROP(variation);
  JSE_PROP(version);

  // TODO(dsinclair): Remove when xfa_basic_data_element_script is removed.
  // Doesn't exist in spec
  JSE_PROP(name);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_HOSTPSEUDOMODEL_H_
