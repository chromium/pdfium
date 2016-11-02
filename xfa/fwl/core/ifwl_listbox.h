// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_LISTBOX_H_
#define XFA_FWL_CORE_IFWL_LISTBOX_H_

#include <memory>

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/ifwl_dataprovider.h"
#include "xfa/fwl/core/ifwl_edit.h"
#include "xfa/fwl/core/ifwl_listbox.h"
#include "xfa/fwl/core/ifwl_widget.h"

#define FWL_STYLEEXT_LTB_MultiSelection (1L << 0)
#define FWL_STYLEEXT_LTB_ShowScrollBarAlaways (1L << 2)
#define FWL_STYLEEXT_LTB_MultiColumn (1L << 3)
#define FWL_STYLEEXT_LTB_LeftAlign (0L << 4)
#define FWL_STYLEEXT_LTB_CenterAlign (1L << 4)
#define FWL_STYLEEXT_LTB_RightAlign (2L << 4)
#define FWL_STYLEEXT_LTB_MultiLine (1L << 6)
#define FWL_STYLEEXT_LTB_OwnerDraw (1L << 7)
#define FWL_STYLEEXT_LTB_Icon (1L << 8)
#define FWL_STYLEEXT_LTB_Check (1L << 9)
#define FWL_STYLEEXT_LTB_AlignMask (3L << 4)
#define FWL_STYLEEXT_LTB_ShowScrollBarFocus (1L << 10)
#define FWL_ITEMSTATE_LTB_Selected (1L << 0)
#define FWL_ITEMSTATE_LTB_Focused (1L << 1)
#define FWL_ITEMSTATE_LTB_Checked (1L << 2)

class CFWL_ListBoxImpDelegate;
class CFWL_MsgKillFocus;
class CFWL_MsgMouse;
class CFWL_MsgMouseWheel;
class CFX_DIBitmap;

FWL_EVENT_DEF(CFWL_EvtLtbSelChanged,
              CFWL_EventType::SelectChanged,
              CFX_Int32Array iarraySels;)

FWL_EVENT_DEF(CFWL_EvtLtbDrawItem,
              CFWL_EventType::DrawItem,
              CFX_Graphics* m_pGraphics;
              CFX_Matrix m_matrix;
              int32_t m_index;
              CFX_RectF m_rect;)

class IFWL_ListItem {};

class IFWL_ListBoxDP : public IFWL_DataProvider {
 public:
  virtual int32_t CountItems(const IFWL_Widget* pWidget) = 0;
  virtual IFWL_ListItem* GetItem(const IFWL_Widget* pWidget,
                                 int32_t nIndex) = 0;
  virtual int32_t GetItemIndex(IFWL_Widget* pWidget, IFWL_ListItem* pItem) = 0;
  virtual FX_BOOL SetItemIndex(IFWL_Widget* pWidget,
                               IFWL_ListItem* pItem,
                               int32_t nIndex) = 0;
  virtual uint32_t GetItemStyles(IFWL_Widget* pWidget,
                                 IFWL_ListItem* pItem) = 0;
  virtual FWL_Error GetItemText(IFWL_Widget* pWidget,
                                IFWL_ListItem* pItem,
                                CFX_WideString& wsText) = 0;
  virtual FWL_Error GetItemRect(IFWL_Widget* pWidget,
                                IFWL_ListItem* pItem,
                                CFX_RectF& rtItem) = 0;
  virtual void* GetItemData(IFWL_Widget* pWidget, IFWL_ListItem* pItem) = 0;
  virtual FWL_Error SetItemStyles(IFWL_Widget* pWidget,
                                  IFWL_ListItem* pItem,
                                  uint32_t dwStyle) = 0;
  virtual FWL_Error SetItemText(IFWL_Widget* pWidget,
                                IFWL_ListItem* pItem,
                                const FX_WCHAR* pszText) = 0;
  virtual FWL_Error SetItemRect(IFWL_Widget* pWidget,
                                IFWL_ListItem* pItem,
                                const CFX_RectF& rtItem) = 0;
  virtual FX_FLOAT GetItemHeight(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetItemIcon(IFWL_Widget* pWidget,
                                    IFWL_ListItem* pItem) = 0;
  virtual FWL_Error GetItemCheckRect(IFWL_Widget* pWidget,
                                     IFWL_ListItem* pItem,
                                     CFX_RectF& rtCheck) = 0;
  virtual FWL_Error SetItemCheckRect(IFWL_Widget* pWidget,
                                     IFWL_ListItem* pItem,
                                     const CFX_RectF& rtCheck) = 0;
  virtual uint32_t GetItemCheckState(IFWL_Widget* pWidget,
                                     IFWL_ListItem* pItem) = 0;
  virtual FWL_Error SetItemCheckState(IFWL_Widget* pWidget,
                                      IFWL_ListItem* pItem,
                                      uint32_t dwCheckState) = 0;
};

class IFWL_ListBoxCompare {
 public:
  virtual ~IFWL_ListBoxCompare() {}
  virtual int32_t Compare(IFWL_ListItem* hLeft, IFWL_ListItem* hRight) = 0;
};

class IFWL_ListBox : public IFWL_Widget {
 public:
  IFWL_ListBox(const IFWL_App* app,
               const CFWL_WidgetImpProperties& properties,
               IFWL_Widget* pOuter);
  ~IFWL_ListBox() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
  FWL_Error SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) override;

  int32_t CountSelItems();
  IFWL_ListItem* GetSelItem(int32_t nIndexSel);
  int32_t GetSelIndex(int32_t nIndex);
  FWL_Error SetSelItem(IFWL_ListItem* hItem, FX_BOOL bSelect = TRUE);
  FWL_Error GetItemText(IFWL_ListItem* hItem, CFX_WideString& wsText);
  FWL_Error GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert = TRUE);
  FWL_Error* Sort(IFWL_ListBoxCompare* pCom);

 protected:
  friend class CFWL_ListBoxImpDelegate;

  IFWL_ListItem* GetItem(IFWL_ListItem* hItem, uint32_t dwKeyCode);
  void SetSelection(IFWL_ListItem* hStart,
                    IFWL_ListItem* hEnd,
                    FX_BOOL bSelected);
  void SetSelectionDirect(IFWL_ListItem* hItem, FX_BOOL bSelect);
  FX_BOOL IsItemSelected(IFWL_ListItem* hItem);
  void ClearSelection();
  void SelectAll();
  IFWL_ListItem* GetFocusedItem();
  void SetFocusItem(IFWL_ListItem* hItem);
  IFWL_ListItem* GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  FX_BOOL GetItemCheckRect(IFWL_ListItem* hItem, CFX_RectF& rtCheck);
  FX_BOOL SetItemChecked(IFWL_ListItem* hItem, FX_BOOL bChecked);
  FX_BOOL GetItemChecked(IFWL_ListItem* hItem);
  FX_BOOL ScrollToVisible(IFWL_ListItem* hItem);
  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix = nullptr);
  void DrawItems(CFX_Graphics* pGraphics,
                 IFWL_ThemeProvider* pTheme,
                 const CFX_Matrix* pMatrix = nullptr);
  void DrawItem(CFX_Graphics* pGraphics,
                IFWL_ThemeProvider* pTheme,
                IFWL_ListItem* hItem,
                int32_t Index,
                const CFX_RectF& rtItem,
                const CFX_Matrix* pMatrix = nullptr);
  void DrawStatic(CFX_Graphics* pGraphics, IFWL_ThemeProvider* pTheme);
  CFX_SizeF CalcSize(FX_BOOL bAutoSize = FALSE);
  void GetItemSize(CFX_SizeF& size,
                   IFWL_ListItem* hItem,
                   FX_FLOAT fWidth,
                   FX_FLOAT fHeight,
                   FX_BOOL bAutoSize = FALSE);
  FX_FLOAT GetMaxTextWidth();
  FX_FLOAT GetScrollWidth();
  FX_FLOAT GetItemHeigt();
  void InitScrollBar(FX_BOOL bVert = TRUE);
  FX_BOOL IsShowScrollBar(FX_BOOL bVert);
  void ProcessSelChanged();

  CFX_RectF m_rtClient;
  CFX_RectF m_rtStatic;
  CFX_RectF m_rtConent;
  std::unique_ptr<IFWL_ScrollBar> m_pHorzScrollBar;
  std::unique_ptr<IFWL_ScrollBar> m_pVertScrollBar;
  uint32_t m_dwTTOStyles;
  int32_t m_iTTOAligns;
  IFWL_ListItem* m_hAnchor;
  FX_FLOAT m_fItemHeight;
  FX_FLOAT m_fScorllBarWidth;
  FX_BOOL m_bLButtonDown;
  IFWL_ThemeProvider* m_pScrollBarTP;
};

class CFWL_ListBoxImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ListBoxImpDelegate(IFWL_ListBox* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseWheel(CFWL_MsgMouseWheel* pMsg);
  void OnKeyDown(CFWL_MsgKey* pMsg);
  void OnVK(IFWL_ListItem* hItem, FX_BOOL bShift, FX_BOOL bCtrl);
  FX_BOOL OnScroll(IFWL_ScrollBar* pScrollBar, uint32_t dwCode, FX_FLOAT fPos);
  void DispatchSelChangedEv();
  IFWL_ListBox* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_LISTBOX_H_
