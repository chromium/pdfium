// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_FFCOMBOBOX_H_
#define XFA_FXFA_APP_CXFA_FFCOMBOBOX_H_

#include "xfa/fxfa/app/cxfa_fffield.h"

class CXFA_FFComboBox : public CXFA_FFField {
 public:
  explicit CXFA_FFComboBox(CXFA_WidgetAcc* pDataAcc);
  ~CXFA_FFComboBox() override;

  // CXFA_FFField
  CFX_RectF GetBBox(uint32_t dwStatus, bool bDrawFocus = false) override;
  bool LoadWidget() override;
  void UpdateWidgetProperty() override;
  bool OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) override;
  bool OnKillFocus(CXFA_FFWidget* pNewWidget) override;
  bool CanUndo() override;
  bool CanRedo() override;
  bool Undo() override;
  bool Redo() override;

  bool CanCopy() override;
  bool CanCut() override;
  bool CanPaste() override;
  bool CanSelectAll() override;
  bool Copy(CFX_WideString& wsCopy) override;
  bool Cut(CFX_WideString& wsCut) override;
  bool Paste(const CFX_WideString& wsPaste) override;
  void SelectAll() override;
  void Delete() override;
  void DeSelect() override;

  // IFWL_WidgetDelegate
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CXFA_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

  virtual void OpenDropDownList();

  void OnTextChanged(CFWL_Widget* pWidget, const CFX_WideString& wsChanged);
  void OnSelectChanged(CFWL_Widget* pWidget, bool bLButtonUp);
  void OnPreOpen(CFWL_Widget* pWidget);
  void OnPostOpen(CFWL_Widget* pWidget);
  void SetItemState(int32_t nIndex, bool bSelected);
  void InsertItem(const CFX_WideStringC& wsLabel, int32_t nIndex);
  void DeleteItem(int32_t nIndex);

 private:
  // CXFA_FFField
  bool PtInActiveRect(const CFX_PointF& point) override;
  bool CommitData() override;
  bool UpdateFWLData() override;
  bool IsDataChanged() override;

  uint32_t GetAlignment();
  void FWLEventSelChange(CXFA_EventParam* pParam);

  CFX_WideString m_wsNewValue;
  IFWL_WidgetDelegate* m_pOldDelegate;
};

#endif  // XFA_FXFA_APP_CXFA_FFCOMBOBOX_H_
