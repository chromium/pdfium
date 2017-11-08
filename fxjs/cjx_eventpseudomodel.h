// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_EVENTPSEUDOMODEL_H_
#define FXJS_CJX_EVENTPSEUDOMODEL_H_

#include "fxjs/cjx_object.h"

class CFXJSE_Arguments;
class CFXJSE_Value;
class CScript_EventPseudoModel;

enum class XFA_Event {
  Change = 0,
  CommitKey,
  FullText,
  Keydown,
  Modifier,
  NewContentType,
  NewText,
  PreviousContentType,
  PreviousText,
  Reenter,
  SelectionEnd,
  SelectionStart,
  Shift,
  SoapFaultCode,
  SoapFaultString,
  Target,
  CancelAction
};

class CJX_EventPseudoModel : public CJX_Object {
 public:
  explicit CJX_EventPseudoModel(CScript_EventPseudoModel* model);
  ~CJX_EventPseudoModel() override;

  void Change(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void CommitKey(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void FullText(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void KeyDown(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Modifier(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void NewContentType(CFXJSE_Value* pValue,
                      bool bSetting,
                      XFA_Attribute eAttribute);
  void NewText(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void PrevContentType(CFXJSE_Value* pValue,
                       bool bSetting,
                       XFA_Attribute eAttribute);
  void PrevText(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Reenter(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void SelEnd(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void SelStart(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void Shift(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);
  void SoapFaultCode(CFXJSE_Value* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute);
  void SoapFaultString(CFXJSE_Value* pValue,
                       bool bSetting,
                       XFA_Attribute eAttribute);
  void Target(CFXJSE_Value* pValue, bool bSetting, XFA_Attribute eAttribute);

  void Emit(CFXJSE_Arguments* pArguments);
  void Reset(CFXJSE_Arguments* pArguments);

 private:
  void Property(CFXJSE_Value* pValue, XFA_Event dwFlag, bool bSetting);
};

#endif  // FXJS_CJX_EVENTPSEUDOMODEL_H_
