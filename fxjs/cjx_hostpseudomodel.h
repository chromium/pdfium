// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_HOSTPSEUDOMODEL_H_
#define FXJS_CJX_HOSTPSEUDOMODEL_H_

#include "fxjs/CJX_Define.h"
#include "fxjs/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Value;
class CScript_HostPseudoModel;

class CJX_HostPseudoModel : public CJX_Object {
 public:
  explicit CJX_HostPseudoModel(CScript_HostPseudoModel* model);
  ~CJX_HostPseudoModel() override;

  void AppType(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void CalculationsEnabled(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void CurrentPage(CFXJSE_Value* pValue,
                   bool bSetting,
                   XFA_Attribute eAttribute);
  void Language(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void NumPages(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Platform(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Title(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void ValidationsEnabled(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute);
  void Variation(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Version(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Name(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);

  JS_METHOD(beep, CJX_HostPseudoModel);
  JS_METHOD(documentCountInBatch, CJX_HostPseudoModel);
  JS_METHOD(documentInBatch, CJX_HostPseudoModel);
  JS_METHOD(exportData, CJX_HostPseudoModel);
  JS_METHOD(getFocus, CJX_HostPseudoModel);
  JS_METHOD(gotoURL, CJX_HostPseudoModel);
  JS_METHOD(importData, CJX_HostPseudoModel);
  JS_METHOD(messageBox, CJX_HostPseudoModel);
  JS_METHOD(openList, CJX_HostPseudoModel);
  JS_METHOD(pageDown, CJX_HostPseudoModel);
  JS_METHOD(pageUp, CJX_HostPseudoModel);
  JS_METHOD(print, CJX_HostPseudoModel);
  JS_METHOD(resetData, CJX_HostPseudoModel);
  JS_METHOD(response, CJX_HostPseudoModel);
  JS_METHOD(setFocus, CJX_HostPseudoModel);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_CJX_HOSTPSEUDOMODEL_H_
