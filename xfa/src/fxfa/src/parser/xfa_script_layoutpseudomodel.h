// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _SCRIPT_LAYOUTPSEUDOMODEL_H_
#define _SCRIPT_LAYOUTPSEUDOMODEL_H_
enum XFA_LAYOUTMODEL_HWXY {
  XFA_LAYOUTMODEL_H,
  XFA_LAYOUTMODEL_W,
  XFA_LAYOUTMODEL_X,
  XFA_LAYOUTMODEL_Y
};
class CScript_LayoutPseudoModel : public CXFA_OrdinaryObject {
 public:
  CScript_LayoutPseudoModel(CXFA_Document* pDocument);
  ~CScript_LayoutPseudoModel();

  void Script_LayoutPseudoModel_Ready(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);

  void Script_LayoutPseudoModel_HWXY(CFXJSE_Arguments* pArguments,
                                     XFA_LAYOUTMODEL_HWXY layoutModel);
  void Script_LayoutPseudoModel_H(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_W(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_X(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_Y(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_NumberedPageCount(CFXJSE_Arguments* pArguments,
                                                  FX_BOOL bNumbered);
  void Script_LayoutPseudoModel_PageCount(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_PageSpan(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_Page(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_PageContent(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_AbsPageCount(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_AbsPageCountInBatch(
      CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_SheetCountInBatch(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_Relayout(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_AbsPageSpan(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_AbsPageInBatch(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_SheetInBatch(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_Sheet(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_RelayoutPageArea(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_SheetCount(CFXJSE_Arguments* pArguments);
  void Script_LayoutPseudoModel_AbsPage(CFXJSE_Arguments* pArguments);

 protected:
  void Script_LayoutPseudoModel_GetObjArray(IXFA_DocLayout* pDocLayout,
                                            int32_t iPageNo,
                                            const CFX_WideString& wsType,
                                            FX_BOOL bOnPageArea,
                                            CXFA_NodeArray& retArray);
  void Script_LayoutPseudoModel_PageImp(CFXJSE_Arguments* pArguments,
                                        FX_BOOL bAbsPage);
};
#endif
