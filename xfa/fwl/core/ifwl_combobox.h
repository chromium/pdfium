// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_COMBOBOX_H_
#define XFA_FWL_CORE_IFWL_COMBOBOX_H_

#include "xfa/fwl/core/ifwl_form.h"
#include "xfa/fwl/core/ifwl_listbox.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFWL_ComboBoxImpDelegate;
class CFWL_ComboEditImpDelegate;
class CFWL_ComboListImpDelegate;
class CFWL_ComboProxyImpDelegate;
class CFWL_ListBoxImpDelegate;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_ComboBox;
class IFWL_ComboEdit;
class IFWL_ComboList;
class IFWL_FormProxy;
class IFWL_ListBox;
class IFWL_Widget;

#define FWL_CLASS_ComboBox L"FWL_COMBOBOX"
#define FWL_STYLEEXT_CMB_DropList (0L << 0)
#define FWL_STYLEEXT_CMB_DropDown (1L << 0)
#define FWL_STYLEEXT_CMB_Sort (1L << 1)
#define FWL_STYLEEXT_CMB_ListDrag (1L << 2)
#define FWL_STYLEEXT_CMB_OwnerDraw (1L << 3)
#define FWL_STYLEEXT_CMB_EditHNear (0L << 4)
#define FWL_STYLEEXT_CMB_EditHCenter (1L << 4)
#define FWL_STYLEEXT_CMB_EditHFar (2L << 4)
#define FWL_STYLEEXT_CMB_EditVNear (0L << 6)
#define FWL_STYLEEXT_CMB_EditVCenter (1L << 6)
#define FWL_STYLEEXT_CMB_EditVFar (2L << 6)
#define FWL_STYLEEXT_CMB_EditJustified (1L << 8)
#define FWL_STYLEEXT_CMB_EditDistributed (2L << 8)
#define FWL_STYLEEXT_CMB_EditHAlignMask (3L << 4)
#define FWL_STYLEEXT_CMB_EditVAlignMask (3L << 6)
#define FWL_STYLEEXT_CMB_EditHAlignModeMask (3L << 8)
#define FWL_STYLEEXT_CMB_ListItemLeftAlign (0L << 10)
#define FWL_STYLEEXT_CMB_ListItemCenterAlign (1L << 10)
#define FWL_STYLEEXT_CMB_ListItemRightAlign (2L << 10)
#define FWL_STYLEEXT_CMB_ListItemAlignMask (3L << 10)
#define FWL_STYLEEXT_CMB_ListItemText (0L << 12)
#define FWL_STYLEEXT_CMB_ListItemIconText (1L << 12)
#define FWL_STYLEEXT_CMB_ReadOnly (1L << 13)

FWL_EVENT_DEF(CFWL_EvtCmbPreDropDown, CFWL_EventType::PreDropDown)

FWL_EVENT_DEF(CFWL_EvtCmbPostDropDown, CFWL_EventType::PostDropDown)

FWL_EVENT_DEF(CFWL_EvtCmbCloseUp, CFWL_EventType::CloseUp)

FWL_EVENT_DEF(CFWL_EvtCmbEditChanged,
              CFWL_EventType::EditChanged,
              int32_t nChangeType;
              CFX_WideString wsInsert;
              CFX_WideString wsDelete;)

FWL_EVENT_DEF(CFWL_EvtCmbSelChanged,
              CFWL_EventType::SelectChanged,
              CFX_Int32Array iArraySels;
              FX_BOOL bLButtonUp;)

FWL_EVENT_DEF(CFWL_EvtCmbHoverChanged,
              CFWL_EventType::HoverChanged,
              int32_t m_iCurHover;)

FWL_EVENT_DEF(CFWL_EvtCmbDrawItem,
              CFWL_EventType::DrawItem,
              CFX_Graphics* m_pGraphics;
              CFX_Matrix m_matrix;
              int32_t m_index;
              CFX_RectF m_rtItem;)

class IFWL_ComboBoxDP : public IFWL_ListBoxDP {
 public:
  virtual FX_FLOAT GetListHeight(IFWL_Widget* pWidget) = 0;
};

class IFWL_ComboBox : public IFWL_Widget {
 public:
  static IFWL_ComboBox* Create(const CFWL_WidgetImpProperties& properties);

  IFWL_ComboBox(const CFWL_WidgetImpProperties& properties,
                IFWL_Widget* pOuter);
  ~IFWL_ComboBox() override;

  // IFWL_Widget
  FWL_Error GetClassName(CFX_WideString& wsClass) const override;
  FWL_Type GetClassID() const override;
  FWL_Error Initialize() override;
  FWL_Error Finalize() override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error ModifyStylesEx(uint32_t dwStylesExAdded,
                           uint32_t dwStylesExRemoved) override;
  void SetStates(uint32_t dwStates, FX_BOOL bSet = TRUE) override;
  FWL_Error Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
  FWL_Error SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) override;

  int32_t GetCurSel();
  FWL_Error SetCurSel(int32_t iSel);
  FWL_Error SetEditText(const CFX_WideString& wsText);
  int32_t GetEditTextLength() const;
  FWL_Error GetEditText(CFX_WideString& wsText,
                        int32_t nStart = 0,
                        int32_t nCount = -1) const;
  FWL_Error SetEditSelRange(int32_t nStart, int32_t nCount = -1);
  int32_t GetEditSelRange(int32_t nIndex, int32_t& nStart);
  int32_t GetEditLimit();
  FWL_Error SetEditLimit(int32_t nLimit);
  FWL_Error EditDoClipboard(int32_t iCmd);
  FX_BOOL EditRedo(const IFDE_TxtEdtDoRecord* pRecord);
  FX_BOOL EditUndo(const IFDE_TxtEdtDoRecord* pRecord);
  IFWL_ListBox* GetListBoxt();
  FX_BOOL AfterFocusShowDropList();
  FWL_Error OpenDropDownList(FX_BOOL bActivate);
  FX_BOOL EditCanUndo();
  FX_BOOL EditCanRedo();
  FX_BOOL EditUndo();
  FX_BOOL EditRedo();
  FX_BOOL EditCanCopy();
  FX_BOOL EditCanCut();
  FX_BOOL EditCanSelectAll();
  FX_BOOL EditCopy(CFX_WideString& wsCopy);
  FX_BOOL EditCut(CFX_WideString& wsCut);
  FX_BOOL EditPaste(const CFX_WideString& wsPaste);
  FX_BOOL EditSelectAll();
  FX_BOOL EditDelete();
  FX_BOOL EditDeSelect();
  FWL_Error GetBBox(CFX_RectF& rect);
  FWL_Error EditModifyStylesEx(uint32_t dwStylesExAdded,
                               uint32_t dwStylesExRemoved);

 protected:
  friend class CFWL_ComboBoxImpDelegate;
  friend class CFWL_ComboEditImpDelegate;
  friend class CFWL_ComboListImpDelegate;
  friend class CFWL_ComboProxyImpDelegate;
  friend class IFWL_ComboEdit;
  friend class IFWL_ComboList;

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
  FWL_Error DisForm_Initialize();
  void DisForm_InitComboList();
  void DisForm_InitComboEdit();
  void DisForm_ShowDropList(FX_BOOL bActivate);
  FX_BOOL DisForm_IsDropListShowed();
  FWL_Error DisForm_ModifyStylesEx(uint32_t dwStylesExAdded,
                                   uint32_t dwStylesExRemoved);
  FWL_Error DisForm_Update();
  FWL_WidgetHit DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy);
  FWL_Error DisForm_DrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = nullptr);
  FWL_Error DisForm_GetBBox(CFX_RectF& rect);
  void DisForm_Layout();

  CFX_RectF m_rtClient;
  CFX_RectF m_rtContent;
  CFX_RectF m_rtBtn;
  CFX_RectF m_rtList;
  CFX_RectF m_rtProxy;
  CFX_RectF m_rtHandler;
  std::unique_ptr<IFWL_ComboEdit> m_pEdit;
  std::unique_ptr<IFWL_ComboList> m_pListBox;
  IFWL_FormProxy* m_pForm;
  FX_BOOL m_bLButtonDown;
  FX_BOOL m_bUpFormHandler;
  int32_t m_iCurSel;
  int32_t m_iBtnState;
  FX_FLOAT m_fComboFormHandler;
  FX_FLOAT m_fItemHeight;
  FX_BOOL m_bNeedShowList;
  CFWL_ComboProxyImpDelegate* m_pListProxyDelegate;
};

class CFWL_ComboBoxImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ComboBoxImpDelegate(IFWL_ComboBox* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnKey(CFWL_MsgKey* pMsg);
  void DoSubCtrlKey(CFWL_MsgKey* pMsg);
  void DisForm_OnProcessMessage(CFWL_Message* pMessage);
  void DisForm_OnLButtonDown(CFWL_MsgMouse* pMsg);
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void DisForm_OnKey(CFWL_MsgKey* pMsg);

  IFWL_ComboBox* m_pOwner;
  friend class CFWL_ComboEditImpDelegate;
  friend class CFWL_ComboListImpDelegate;
};

class CFWL_ComboProxyImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ComboProxyImpDelegate(IFWL_Form* pForm, IFWL_ComboBox* pComboBox);
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;
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
  IFWL_ComboBox* m_pComboBox;
};

#endif  // XFA_FWL_CORE_IFWL_COMBOBOX_H_
