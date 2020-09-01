// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_LISTBOX_H_
#define XFA_FWL_CFWL_LISTBOX_H_

#include <memory>
#include <vector>

#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_widget.h"

#define FWL_STYLEEXT_LTB_MultiSelection (1L << 0)
#define FWL_STYLEEXT_LTB_LeftAlign (0L << 4)
#define FWL_STYLEEXT_LTB_CenterAlign (1L << 4)
#define FWL_STYLEEXT_LTB_RightAlign (2L << 4)
#define FWL_STYLEEXT_LTB_AlignMask (3L << 4)
#define FWL_STYLEEXT_LTB_ShowScrollBarFocus (1L << 10)
#define FWL_ITEMSTATE_LTB_Selected (1L << 0)
#define FWL_ITEMSTATE_LTB_Focused (1L << 1)

class CFWL_MessageKillFocus;
class CFWL_MessageMouse;
class CFWL_MessageMouseWheel;
class CFX_DIBitmap;

class CFWL_ListBox : public CFWL_Widget {
 public:
  class Item {
   public:
    explicit Item(const WideString& text);
    ~Item();

    CFX_RectF GetRect() const { return m_ItemRect; }
    void SetRect(const CFX_RectF& rect) { m_ItemRect = rect; }
    uint32_t GetStates() const { return m_dwStates; }
    void SetStates(uint32_t dwStates) { m_dwStates = dwStates; }
    WideString GetText() const { return m_wsText; }

   private:
    uint32_t m_dwStates = 0;
    CFX_RectF m_ItemRect;
    WideString m_wsText;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_ListBox() override;

  // CFWL_Widget:
  void Trace(cppgc::Visitor* visitor) const override;
  FWL_Type GetClassID() const override;
  void Update() override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void DrawWidget(CXFA_Graphics* pGraphics, const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CXFA_Graphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  int32_t CountItems(const CFWL_Widget* pWidget) const;
  Item* GetItem(const CFWL_Widget* pWidget, int32_t nIndex) const;
  int32_t GetItemIndex(CFWL_Widget* pWidget, Item* pItem);
  Item* AddString(const WideString& wsAdd);
  void RemoveAt(int32_t iIndex);
  void DeleteString(Item* pItem);
  void DeleteAll();
  int32_t CountSelItems();
  Item* GetSelItem(int32_t nIndexSel);
  int32_t GetSelIndex(int32_t nIndex);
  void SetSelItem(Item* hItem, bool bSelect);
  float GetItemHeight() const { return m_fItemHeight; }
  float CalcItemHeight();

 protected:
  CFWL_ListBox(CFWL_App* pApp,
               const Properties& properties,
               CFWL_Widget* pOuter);

  Item* GetListItem(Item* hItem, uint32_t dwKeyCode);
  void SetSelection(Item* hStart, Item* hEnd, bool bSelected);
  Item* GetItemAtPoint(const CFX_PointF& point);
  bool ScrollToVisible(Item* hItem);
  void InitVerticalScrollBar();
  void InitHorizontalScrollBar();
  bool IsShowScrollBar(bool bVert);
  CFWL_ScrollBar* GetVertScrollBar() const { return m_pVertScrollBar; }
  const CFX_RectF& GetRTClient() const { return m_ClientRect; }

 private:
  void SetSelectionDirect(Item* hItem, bool bSelect);
  bool IsMultiSelection() const;
  bool IsItemSelected(Item* hItem);
  void ClearSelection();
  void SelectAll();
  Item* GetFocusedItem();
  void SetFocusItem(Item* hItem);
  void DrawBkground(CXFA_Graphics* pGraphics, const CFX_Matrix* pMatrix);
  void DrawItems(CXFA_Graphics* pGraphics, const CFX_Matrix* pMatrix);
  void DrawItem(CXFA_Graphics* pGraphics,
                Item* hItem,
                int32_t Index,
                const CFX_RectF& rtItem,
                const CFX_Matrix* pMatrix);
  void DrawStatic(CXFA_Graphics* pGraphics);
  CFX_SizeF CalcSize(bool bAutoSize);
  void UpdateItemSize(Item* hItem,
                      CFX_SizeF& size,
                      float fWidth,
                      float fHeight,
                      bool bAutoSize) const;
  float GetMaxTextWidth();
  float GetScrollWidth();

  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseWheel(CFWL_MessageMouseWheel* pMsg);
  void OnKeyDown(CFWL_MessageKey* pMsg);
  void OnVK(Item* hItem, bool bShift, bool bCtrl);
  bool OnScroll(CFWL_ScrollBar* pScrollBar,
                CFWL_EventScroll::Code dwCode,
                float fPos);

  CFX_RectF m_ClientRect;
  CFX_RectF m_StaticRect;
  CFX_RectF m_ContentRect;
  cppgc::Member<CFWL_ScrollBar> m_pHorzScrollBar;
  cppgc::Member<CFWL_ScrollBar> m_pVertScrollBar;
  FDE_TextStyle m_TTOStyles;
  FDE_TextAlignment m_iTTOAligns = FDE_TextAlignment::kTopLeft;
  bool m_bLButtonDown = false;
  float m_fItemHeight = 0.0f;
  float m_fScorllBarWidth = 0.0f;
  Item* m_hAnchor = nullptr;
  std::vector<std::unique_ptr<Item>> m_ItemArray;
};

#endif  // XFA_FWL_CFWL_LISTBOX_H_
