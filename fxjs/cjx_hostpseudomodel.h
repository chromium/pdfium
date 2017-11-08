// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_HOSTPSEUDOMODEL_H_
#define FXJS_CJX_HOSTPSEUDOMODEL_H_

#include "fxjs/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Arguments;
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
  void GotoURL(CFXJSE_Arguments* pArguments);
  void OpenList(CFXJSE_Arguments* pArguments);
  void Response(CFXJSE_Arguments* pArguments);
  void DocumentInBatch(CFXJSE_Arguments* pArguments);
  void ResetData(CFXJSE_Arguments* pArguments);
  void Beep(CFXJSE_Arguments* pArguments);
  void SetFocus(CFXJSE_Arguments* pArguments);
  void GetFocus(CFXJSE_Arguments* pArguments);
  void MessageBox(CFXJSE_Arguments* pArguments);
  void DocumentCountInBatch(CFXJSE_Arguments* pArguments);
  void Print(CFXJSE_Arguments* pArguments);
  void ImportData(CFXJSE_Arguments* pArguments);
  void ExportData(CFXJSE_Arguments* pArguments);
  void PageUp(CFXJSE_Arguments* pArguments);
  void PageDown(CFXJSE_Arguments* pArguments);
  void CurrentDateTime(CFXJSE_Arguments* pArguments);

 private:
  bool ValidateArgsForMsg(CFXJSE_Arguments* pArguments,
                          int32_t iArgIndex,
                          WideString& wsValue);
};

#endif  // FXJS_CJX_HOSTPSEUDOMODEL_H_
