// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _SCRIPT_EVENTPSEUDOMODEL_H_
#define _SCRIPT_EVENTPSEUDOMODEL_H_
#define XFA_EVENT_CHANGE 0
#define XFA_EVENT_COMMITKEY 1
#define XFA_EVENT_FULLTEXT 2
#define XFA_EVENT_KEYDOWN 3
#define XFA_EVENT_MODIFIER 4
#define XFA_EVENT_NEWCONTENTTYPE 5
#define XFA_EVENT_NEWTEXT 6
#define XFA_EVENT_PREVCONTENTTYPE 7
#define XFA_EVENT_PREVTEXT 8
#define XFA_EVENT_REENTER 9
#define XFA_EVENT_SELEND 10
#define XFA_EVENT_SELSTART 11
#define XFA_EVENT_SHIFT 12
#define XFA_EVENT_SOAPFAULTCODE 13
#define XFA_EVENT_SOAPFAULTSTRING 14
#define XFA_EVENT_TARGET 15
#define XFA_EVENT_CANCELACTION 16
class CScript_EventPseudoModel : public CXFA_OrdinaryObject {
 public:
  CScript_EventPseudoModel(CXFA_Document* pDocument);
  virtual ~CScript_EventPseudoModel();

  void Script_EventPseudoModel_CancelAction(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute);
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
                                        FX_DWORD dwFlag,
                                        FX_BOOL bSetting);
};
#endif
