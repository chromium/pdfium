// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_COMBOBOX_IMP_H
#define _FWL_COMBOBOX_IMP_H

#include <memory>

#include "xfa/src/fwl/src/basewidget/include/fwl_editimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_listboximp.h"

class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class CFWL_ListBoxImp;
class CFWL_ListBoxImpDelegate;
class CFWL_FormProxyImp;
class IFWL_Widget;
class CFWL_ComboEditImp;
class CFWL_ComboEditImpDelegate;
class CFWL_ComboListImp;
class CFWL_ComboListImpDelegate;
class CFWL_ComboBoxImp;
class CFWL_ComboBoxImpDelegate;
class CFWL_ComboProxyImpDelegate;
class CFWL_ComboEditImp : public CFWL_EditImp {
 public:
  CFWL_ComboEditImp(const CFWL_WidgetImpProperties& properties,
                    IFWL_Widget* pOuter);

  void ClearSelected();
  void SetSelected();
  void EndCaret();
  void FlagFocus(FX_BOOL bSet);

 protected:
  void SetComboBoxFocus(FX_BOOL bSet);
  CFWL_ComboBoxImp* m_pOuter;
  friend class CFWL_ComboEditImpDelegate;
};
class CFWL_ComboEditImpDelegate : public CFWL_EditImpDelegate {
 public:
  CFWL_ComboEditImpDelegate(CFWL_ComboEditImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;

 protected:
  CFWL_ComboEditImp* m_pOwner;
};
class CFWL_ComboListImp : public CFWL_ListBoxImp {
 public:
  CFWL_ComboListImp(const CFWL_WidgetImpProperties& properties,
                    IFWL_Widget* pOuter);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  int32_t MatchItem(const CFX_WideString& wsMatch);
  void ChangeSelected(int32_t iSel);
  int32_t CountItems();
  void GetItemRect(int32_t nIndex, CFX_RectF& rtItem);
  void ClientToOuter(FX_FLOAT& fx, FX_FLOAT& fy);
  void SetFocus(FX_BOOL bSet);
  FX_BOOL m_bNotifyOwner;
  friend class CFWL_ComboListImpDelegate;
  friend class CFWL_ComboBoxImp;
};
class CFWL_ComboListImpDelegate : public CFWL_ListBoxImpDelegate {
 public:
  CFWL_ComboListImpDelegate(CFWL_ComboListImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;

 protected:
  void OnDropListFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  int32_t OnDropListMouseMove(CFWL_MsgMouse* pMsg);
  int32_t OnDropListLButtonDown(CFWL_MsgMouse* pMsg);
  int32_t OnDropListLButtonUp(CFWL_MsgMouse* pMsg);
  int32_t OnDropListKey(CFWL_MsgKey* pKey);
  void OnDropListKeyDown(CFWL_MsgKey* pKey);
  CFWL_ComboListImp* m_pOwner;
};
class CFWL_ComboBoxImp : public CFWL_WidgetImp {
 public:
  CFWL_ComboBoxImp(const CFWL_WidgetImpProperties& properties,
                   IFWL_Widget* pOuter);
  virtual ~CFWL_ComboBoxImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR ModifyStylesEx(FX_DWORD dwStylesExAdded,
                                 FX_DWORD dwStylesExRemoved);
  virtual FWL_ERR SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE);
  virtual FWL_ERR Update();
  virtual FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  virtual int32_t GetCurSel();
  virtual FWL_ERR SetCurSel(int32_t iSel);
  virtual FWL_ERR SetEditText(const CFX_WideString& wsText);
  virtual int32_t GetEditTextLength() const;
  virtual FWL_ERR GetEditText(CFX_WideString& wsText,
                              int32_t nStart = 0,
                              int32_t nCount = -1) const;
  virtual FWL_ERR SetEditSelRange(int32_t nStart, int32_t nCount = -1);
  virtual int32_t GetEditSelRange(int32_t nIndex, int32_t& nStart);
  virtual int32_t GetEditLimit();
  virtual FWL_ERR SetEditLimit(int32_t nLimit);
  virtual FWL_ERR EditDoClipboard(int32_t iCmd);
  virtual FX_BOOL EditRedo(const CFX_ByteStringC& bsRecord);
  virtual FX_BOOL EditUndo(const CFX_ByteStringC& bsRecord);
  virtual IFWL_ListBox* GetListBoxt();
  virtual FX_BOOL AfterFocusShowDropList();
  virtual FX_ERR OpenDropDownList(FX_BOOL bActivate);
  virtual FX_BOOL EditCanUndo();
  virtual FX_BOOL EditCanRedo();
  virtual FX_BOOL EditUndo();
  virtual FX_BOOL EditRedo();
  virtual FX_BOOL EditCanCopy();
  virtual FX_BOOL EditCanCut();
  virtual FX_BOOL EditCanSelectAll();
  virtual FX_BOOL EditCopy(CFX_WideString& wsCopy);
  virtual FX_BOOL EditCut(CFX_WideString& wsCut);
  virtual FX_BOOL EditPaste(const CFX_WideString& wsPaste);
  virtual FX_BOOL EditSelectAll();
  virtual FX_BOOL EditDelete();
  virtual FX_BOOL EditDeSelect();
  virtual FWL_ERR GetBBox(CFX_RectF& rect);
  virtual FWL_ERR EditModifyStylesEx(FX_DWORD dwStylesExAdded,
                                     FX_DWORD dwStylesExRemoved);

 protected:
  void DrawStretchHandler(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix);
  FX_FLOAT GetListHeight();
  void ShowDropList(FX_BOOL bActivate);
  FX_BOOL IsDropListShowed();
  FX_BOOL IsDropDownStyle() const;
  void MatchEditText();
  void SynchrEditText(int32_t iListItem);
  void Layout();
  void ReSetTheme();
  void ReSetEditAlignment();
  void ReSetListItemAlignment();
  void ProcessSelChanged(FX_BOOL bLButtonUp);
  void InitProxyForm();
  FWL_ERR DisForm_Initialize();
  void DisForm_InitComboList();
  void DisForm_InitComboEdit();
  void DisForm_ShowDropList(FX_BOOL bActivate);
  FX_BOOL DisForm_IsDropListShowed();
  FWL_ERR DisForm_ModifyStylesEx(FX_DWORD dwStylesExAdded,
                                 FX_DWORD dwStylesExRemoved);
  FWL_ERR DisForm_Update();
  FX_DWORD DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy);
  FWL_ERR DisForm_DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  FWL_ERR DisForm_GetBBox(CFX_RectF& rect);
  void DisForm_Layout();

  CFX_RectF m_rtClient;
  CFX_RectF m_rtContent;
  CFX_RectF m_rtBtn;
  CFX_RectF m_rtList;
  CFX_RectF m_rtProxy;
  CFX_RectF m_rtHandler;
  std::unique_ptr<IFWL_Edit> m_pEdit;
  std::unique_ptr<IFWL_ListBox> m_pListBox;
  IFWL_Form* m_pForm;
  FX_BOOL m_bLButtonDown;
  FX_BOOL m_bUpFormHandler;
  int32_t m_iCurSel;
  int32_t m_iBtnState;
  FX_FLOAT m_fComboFormHandler;
  FX_FLOAT m_fItemHeight;
  FX_BOOL m_bNeedShowList;
  CFWL_FormProxyImp* m_pProxy;
  CFWL_ComboProxyImpDelegate* m_pListProxyDelegate;

  friend class CFWL_ComboListImp;
  friend class CFWL_ComboEditImp;
  friend class CFWL_ComboEditImpDelegate;
  friend class CFWL_ComboListImpDelegate;
  friend class CFWL_ComboBoxImpDelegate;
  friend class CFWL_ComboProxyImpDelegate;
};
class CFWL_ComboBoxImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ComboBoxImpDelegate(CFWL_ComboBoxImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnKey(CFWL_MsgKey* pMsg);
  void DoSubCtrlKey(CFWL_MsgKey* pMsg);

 protected:
  int32_t DisForm_OnProcessMessage(CFWL_Message* pMessage);
  void DisForm_OnLButtonDown(CFWL_MsgMouse* pMsg);
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void DisForm_OnKey(CFWL_MsgKey* pMsg);

 protected:
  CFWL_ComboBoxImp* m_pOwner;
  friend class CFWL_ComboEditImpDelegate;
  friend class CFWL_ComboListImpDelegate;
};
class CFWL_ComboProxyImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ComboProxyImpDelegate(IFWL_Form* pForm, CFWL_ComboBoxImp* pComboBox);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;
  void Reset() { m_bLButtonUpSelf = FALSE; }

 protected:
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnDeactive(CFWL_MsgDeactivate* pMsg);
  void OnFocusChanged(CFWL_MsgKillFocus* pMsg, FX_BOOL bSet);
  FX_BOOL m_bLButtonDown;
  FX_BOOL m_bLButtonUpSelf;
  FX_FLOAT m_fStartPos;
  IFWL_Form* m_pForm;
  CFWL_ComboBoxImp* m_pComboBox;
};
#endif
