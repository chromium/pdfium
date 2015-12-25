// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_LISTBOX_IMP_H
#define _FWL_LISTBOX_IMP_H

#include <memory>

class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class CFWL_ScrollBarImp;
class IFWL_Widget;
class CFWL_ListBoxImp;
class CFWL_ListBoxImpDelegate;
class CFWL_ListBoxImp : public CFWL_WidgetImp {
 public:
  CFWL_ListBoxImp(const CFWL_WidgetImpProperties& properties,
                  IFWL_Widget* pOuter);
  ~CFWL_ListBoxImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  virtual int32_t CountSelItems();
  virtual FWL_HLISTITEM GetSelItem(int32_t nIndexSel);
  virtual int32_t GetSelIndex(int32_t nIndex);
  virtual FWL_ERR SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect = TRUE);
  virtual FWL_ERR GetItemText(FWL_HLISTITEM hItem, CFX_WideString& wsText);
  virtual FWL_ERR GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert = TRUE);
  virtual FWL_ERR* Sort(IFWL_ListBoxCompare* pCom);

 protected:
  FWL_HLISTITEM GetItem(FWL_HLISTITEM hItem, FX_DWORD dwKeyCode);
  void SetSelection(FWL_HLISTITEM hStart,
                    FWL_HLISTITEM hEnd,
                    FX_BOOL bSelected);
  void SetSelectionDirect(FWL_HLISTITEM hItem, FX_BOOL bSelect);
  FX_BOOL IsItemSelected(FWL_HLISTITEM hItem);
  void ClearSelection();
  void SelectAll();
  FWL_HLISTITEM GetFocusedItem();
  void SetFocusItem(FWL_HLISTITEM hItem);
  FWL_HLISTITEM GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  FX_BOOL GetItemCheckRect(FWL_HLISTITEM hItem, CFX_RectF& rtCheck);
  FX_BOOL SetItemChecked(FWL_HLISTITEM hItem, FX_BOOL bChecked);
  FX_BOOL GetItemChecked(FWL_HLISTITEM hItem);
  FX_BOOL ScrollToVisible(FWL_HLISTITEM hItem);
  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix = NULL);
  void DrawItems(CFX_Graphics* pGraphics,
                 IFWL_ThemeProvider* pTheme,
                 const CFX_Matrix* pMatrix = NULL);
  void DrawItem(CFX_Graphics* pGraphics,
                IFWL_ThemeProvider* pTheme,
                FWL_HLISTITEM hItem,
                int32_t Index,
                const CFX_RectF& rtItem,
                const CFX_Matrix* pMatrix = NULL);
  void DrawStatic(CFX_Graphics* pGraphics, IFWL_ThemeProvider* pTheme);
  CFX_SizeF CalcSize(FX_BOOL bAutoSize = FALSE);
  void GetItemSize(CFX_SizeF& size,
                   FWL_HLISTITEM hItem,
                   FX_FLOAT fWidth,
                   FX_FLOAT fHeight,
                   FX_BOOL bAutoSize = FALSE);
  FX_FLOAT GetMaxTextWidth();
  FX_FLOAT GetScrollWidth();
  FX_FLOAT GetItemHeigt();
  void InitScrollBar(FX_BOOL bVert = TRUE);
  void SortItem();
  FX_BOOL IsShowScrollBar(FX_BOOL bVert);
  void ProcessSelChanged();

 protected:
  CFX_RectF m_rtClient;
  CFX_RectF m_rtStatic;
  CFX_RectF m_rtConent;
  std::unique_ptr<IFWL_ScrollBar> m_pHorzScrollBar;
  std::unique_ptr<IFWL_ScrollBar> m_pVertScrollBar;
  FX_DWORD m_dwTTOStyles;
  int32_t m_iTTOAligns;
  FWL_HLISTITEM m_hAnchor;
  FX_FLOAT m_fItemHeight;
  FX_FLOAT m_fScorllBarWidth;
  FX_BOOL m_bLButtonDown;
  IFWL_ThemeProvider* m_pScrollBarTP;
  friend class CFWL_ListBoxImpDelegate;
};
class CFWL_ListBoxImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ListBoxImpDelegate(CFWL_ListBoxImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseWheel(CFWL_MsgMouseWheel* pMsg);
  void OnKeyDown(CFWL_MsgKey* pMsg);
  void OnVK(FWL_HLISTITEM hItem, FX_BOOL bShift, FX_BOOL bCtrl);
  FX_BOOL OnScroll(IFWL_ScrollBar* pScrollBar, FX_DWORD dwCode, FX_FLOAT fPos);
  void DispatchSelChangedEv();
  CFWL_ListBoxImp* m_pOwner;
};
#endif
