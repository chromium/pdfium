// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_SCRIPT_EVENTPSEUDOMODEL_H_
#define XFA_FXFA_PARSER_XFA_SCRIPT_EVENTPSEUDOMODEL_H_

#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxjse/cfxjse_arguments.h"

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

class CScript_EventPseudoModel : public CXFA_OrdinaryObject {
 public:
  explicit CScript_EventPseudoModel(CXFA_Document* pDocument);
  virtual ~CScript_EventPseudoModel();

  void Script_EventPseudoModel_Change(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_CommitKey(FXJSE_HVALUE hValue,
                                         FX_BOOL bSetting,
                                         XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_FullText(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_KeyDown(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_Modifier(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_NewContentType(FXJSE_HVALUE hValue,
                                              FX_BOOL bSetting,
                                              XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_NewText(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_PrevContentType(FXJSE_HVALUE hValue,
                                               FX_BOOL bSetting,
                                               XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_PrevText(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_Reenter(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_SelEnd(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_SelStart(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_Shift(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_SoapFaultCode(FXJSE_HVALUE hValue,
                                             FX_BOOL bSetting,
                                             XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_SoapFaultString(FXJSE_HVALUE hValue,
                                               FX_BOOL bSetting,
                                               XFA_ATTRIBUTE eAttribute);
  void Script_EventPseudoModel_Target(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);

  void Script_EventPseudoModel_Emit(CFXJSE_Arguments* pArguments);
  void Script_EventPseudoModel_Reset(CFXJSE_Arguments* pArguments);

 protected:
  void Script_EventPseudoModel_Property(FXJSE_HVALUE hValue,
                                        XFA_Event dwFlag,
                                        FX_BOOL bSetting);
};

#endif  // XFA_FXFA_PARSER_XFA_SCRIPT_EVENTPSEUDOMODEL_H_
