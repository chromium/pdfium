// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_CHOICELIST_IMP_H
#define _FXFA_FORMFILLER_CHOICELIST_IMP_H
class CXFA_FFListBox : public CXFA_FFField {
 public:
  CXFA_FFListBox(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFListBox();
  virtual FX_BOOL LoadWidget();
  virtual FX_BOOL OnKillFocus(CXFA_FFWidget* pNewWidget);

 protected:
  virtual FX_BOOL CommitData();
  virtual FX_BOOL UpdateFWLData();
  virtual FX_BOOL IsDataChanged();
  FX_DWORD GetAlignment();

 public:
  void OnSelectChanged(IFWL_Widget* pWidget, const CFX_Int32Array& arrSels);
  void SetItemState(int32_t nIndex, FX_BOOL bSelected);
  void InsertItem(const CFX_WideStringC& wsLabel, int32_t nIndex = -1);
  void DeleteItem(int32_t nIndex);
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);

 protected:
  IFWL_WidgetDelegate* m_pOldDelegate;
};
class CXFA_FFComboBox : public CXFA_FFField {
 public:
  CXFA_FFComboBox(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFComboBox();
  virtual FX_BOOL GetBBox(CFX_RectF& rtBox,
                          FX_DWORD dwStatus,
                          FX_BOOL bDrawFocus = FALSE);
  virtual FX_BOOL LoadWidget();
  virtual void UpdateWidgetProperty();
  virtual FX_BOOL OnRButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnKillFocus(CXFA_FFWidget* pNewWidget);
  virtual FX_BOOL CanUndo();
  virtual FX_BOOL CanRedo();
  virtual FX_BOOL Undo();
  virtual FX_BOOL Redo();

  virtual FX_BOOL CanCopy();
  virtual FX_BOOL CanCut();
  virtual FX_BOOL CanPaste();
  virtual FX_BOOL CanSelectAll();
  virtual FX_BOOL Copy(CFX_WideString& wsCopy);
  virtual FX_BOOL Cut(CFX_WideString& wsCut);
  virtual FX_BOOL Paste(const CFX_WideString& wsPaste);
  virtual FX_BOOL SelectAll();
  virtual FX_BOOL Delete();
  virtual FX_BOOL DeSelect();
  void OpenDropDownList();

 protected:
  virtual FX_BOOL PtInActiveRect(FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL CommitData();
  virtual FX_BOOL UpdateFWLData();
  virtual FX_BOOL IsDataChanged();
  FX_DWORD GetAlignment();
  void FWLEventSelChange(CXFA_EventParam* pParam);

  CFX_WideString m_wsNewValue;

 public:
  void OnTextChanged(IFWL_Widget* pWidget, const CFX_WideString& wsChanged);
  void OnSelectChanged(IFWL_Widget* pWidget,
                       const CFX_Int32Array& arrSels,
                       FX_BOOL bLButtonUp);
  void OnPreOpen(IFWL_Widget* pWidget);
  void OnPostOpen(IFWL_Widget* pWidget);
  void OnAddDoRecord(IFWL_Widget* pWidget);
  void SetItemState(int32_t nIndex, FX_BOOL bSelected);
  void InsertItem(const CFX_WideStringC& wsLabel, int32_t nIndex = -1);
  void DeleteItem(int32_t nIndex);
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);

 protected:
  IFWL_WidgetDelegate* m_pOldDelegate;
};
#endif
