// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _SCRIPT_HOSTPSEUDOMODEL_H_
#define _SCRIPT_HOSTPSEUDOMODEL_H_
class CScript_HostPseudoModel : public CXFA_OrdinaryObject {
 public:
  CScript_HostPseudoModel(CXFA_Document* pDocument);
  virtual ~CScript_HostPseudoModel();

  void Script_HostPseudoModel_AppType(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_FoxitAppType(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_CalculationsEnabled(FXJSE_HVALUE hValue,
                                                  FX_BOOL bSetting,
                                                  XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_CurrentPage(FXJSE_HVALUE hValue,
                                          FX_BOOL bSetting,
                                          XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_Language(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_NumPages(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_Platform(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_Title(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_ValidationsEnabled(FXJSE_HVALUE hValue,
                                                 FX_BOOL bSetting,
                                                 XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_Variation(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_Version(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_FoxitVersion(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_Name(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_HostPseudoModel_FoxitName(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute);

  void Script_HostPseudoModel_GotoURL(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_OpenList(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_Response(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_DocumentInBatch(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_ResetData(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_Beep(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_SetFocus(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_GetFocus(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_MessageBox(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_DocumentCountInBatch(
      CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_Print(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_ImportData(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_ExportData(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_PageUp(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_PageDown(CFXJSE_Arguments* pArguments);
  void Script_HostPseudoModel_CurrentDateTime(CFXJSE_Arguments* pArguments);

 protected:
  void Script_HostPseudoModel_LoadString(FXJSE_HVALUE hValue,
                                         IXFA_Notify* pNotify,
                                         FX_DWORD dwFlag);
  FX_BOOL Script_HostPseudoModel_ValidateArgsForMsg(
      CFXJSE_Arguments* pArguments,
      int32_t iArgIndex,
      CFX_WideString& wsValue);
};
#endif
