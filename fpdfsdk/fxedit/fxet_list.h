// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FXEDIT_FXET_LIST_H_
#define FPDFSDK_FXEDIT_FXET_LIST_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "fpdfsdk/fxedit/fx_edit.h"

class CFX_Edit;
class CFX_Edit_Iterator;
class CPWL_List_Notify;

class CFX_ListItem final {
 public:
  CFX_ListItem();
  ~CFX_ListItem();

  void SetFontMap(IPVT_FontMap* pFontMap);
  CFX_Edit* GetEdit() const;

  void SetRect(const CFX_FloatRect& rect);
  void SetSelect(bool bSelected);
  void SetText(const CFX_WideString& text);
  void SetFontSize(float fFontSize);
  CFX_WideString GetText() const;

  CFX_FloatRect GetRect() const;
  bool IsSelected() const;
  float GetItemHeight() const;
  uint16_t GetFirstChar() const;

 private:
  CFX_Edit_Iterator* GetIterator() const;

  std::unique_ptr<CFX_Edit> m_pEdit;
  bool m_bSelected;
  CFX_FloatRect m_rcListItem;
};

class CFX_ListContainer {
 public:
  CFX_ListContainer();
  virtual ~CFX_ListContainer();

  virtual void SetPlateRect(const CFX_FloatRect& rect);

  CFX_FloatRect GetPlateRect() const { return m_rcPlate; }
  void SetContentRect(const CFX_FloatRect& rect) { m_rcContent = rect; }
  CFX_FloatRect GetContentRect() const { return m_rcContent; }
  CFX_PointF GetBTPoint() const {
    return CFX_PointF(m_rcPlate.left, m_rcPlate.top);
  }
  CFX_PointF GetETPoint() const {
    return CFX_PointF(m_rcPlate.right, m_rcPlate.bottom);
  }

 public:
  CFX_PointF InnerToOuter(const CFX_PointF& point) const {
    return CFX_PointF(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
  }
  CFX_PointF OuterToInner(const CFX_PointF& point) const {
    return CFX_PointF(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
  }
  CFX_FloatRect InnerToOuter(const CFX_FloatRect& rect) const {
    CFX_PointF ptLeftTop = InnerToOuter(CFX_PointF(rect.left, rect.top));
    CFX_PointF ptRightBottom =
        InnerToOuter(CFX_PointF(rect.right, rect.bottom));
    return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                         ptLeftTop.y);
  }
  CFX_FloatRect OuterToInner(const CFX_FloatRect& rect) const {
    CFX_PointF ptLeftTop = OuterToInner(CFX_PointF(rect.left, rect.top));
    CFX_PointF ptRightBottom =
        OuterToInner(CFX_PointF(rect.right, rect.bottom));
    return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                         ptLeftTop.y);
  }

 private:
  CFX_FloatRect m_rcPlate;
  CFX_FloatRect m_rcContent;
};

class CPLST_Select {
 public:
  enum State { DESELECTING = -1, NORMAL = 0, SELECTING = 1 };
  using const_iterator = std::map<int32_t, State>::const_iterator;

  CPLST_Select();
  virtual ~CPLST_Select();

  void Add(int32_t nItemIndex);
  void Add(int32_t nBeginIndex, int32_t nEndIndex);
  void Sub(int32_t nItemIndex);
  void Sub(int32_t nBeginIndex, int32_t nEndIndex);
  void DeselectAll();
  void Done();

  const_iterator begin() const { return m_Items.begin(); }
  const_iterator end() const { return m_Items.end(); }

 private:
  std::map<int32_t, State> m_Items;
};

class CFX_ListCtrl : protected CFX_ListContainer {
 public:
  CFX_ListCtrl();
  ~CFX_ListCtrl() override;

  // CFX_ListContainer
  void SetPlateRect(const CFX_FloatRect& rect) override;

  void SetNotify(CPWL_List_Notify* pNotify);
  void OnMouseDown(const CFX_PointF& point, bool bShift, bool bCtrl);
  void OnMouseMove(const CFX_PointF& point, bool bShift, bool bCtrl);
  void OnVK_UP(bool bShift, bool bCtrl);
  void OnVK_DOWN(bool bShift, bool bCtrl);
  void OnVK_LEFT(bool bShift, bool bCtrl);
  void OnVK_RIGHT(bool bShift, bool bCtrl);
  void OnVK_HOME(bool bShift, bool bCtrl);
  void OnVK_END(bool bShift, bool bCtrl);
  void OnVK(int32_t nItemIndex, bool bShift, bool bCtrl);
  bool OnChar(uint16_t nChar, bool bShift, bool bCtrl);

  void SetScrollPos(const CFX_PointF& point);
  void ScrollToListItem(int32_t nItemIndex);
  CFX_FloatRect GetItemRect(int32_t nIndex) const;
  int32_t GetCaret() const;
  int32_t GetSelect() const;
  int32_t GetTopItem() const;
  CFX_FloatRect GetContentRect() const;
  int32_t GetItemIndex(const CFX_PointF& point) const;
  void AddString(const CFX_WideString& str);
  void SetTopItem(int32_t nIndex);
  void Select(int32_t nItemIndex);
  void SetCaret(int32_t nItemIndex);
  void Empty();
  void Cancel();
  CFX_WideString GetText() const;

  void SetFontMap(IPVT_FontMap* pFontMap);
  void SetFontSize(float fFontSize);
  CFX_FloatRect GetPlateRect() const;
  float GetFontSize() const;
  CFX_Edit* GetItemEdit(int32_t nIndex) const;
  int32_t GetCount() const;
  bool IsItemSelected(int32_t nIndex) const;
  float GetFirstHeight() const;
  void SetMultipleSel(bool bMultiple);
  bool IsMultipleSel() const;
  bool IsValid(int32_t nItemIndex) const;
  int32_t FindNext(int32_t nIndex, wchar_t nChar) const;
  int32_t GetFirstSelected() const;

  CFX_PointF InToOut(const CFX_PointF& point) const;
  CFX_PointF OutToIn(const CFX_PointF& point) const;
  CFX_FloatRect InToOut(const CFX_FloatRect& rect) const;
  CFX_FloatRect OutToIn(const CFX_FloatRect& rect) const;

 private:
  void ReArrange(int32_t nItemIndex);
  CFX_FloatRect GetItemRectInternal(int32_t nIndex) const;
  CFX_FloatRect GetContentRectInternal() const;
  void SetMultipleSelect(int32_t nItemIndex, bool bSelected);
  void SetSingleSelect(int32_t nItemIndex);
  void InvalidateItem(int32_t nItemIndex);
  void SelectItems();
  bool IsItemVisible(int32_t nItemIndex) const;
  void SetScrollInfo();
  void SetScrollPosY(float fy);
  void AddItem(const CFX_WideString& str);
  CFX_WideString GetItemText(int32_t nIndex) const;
  void SetItemSelect(int32_t nItemIndex, bool bSelected);
  int32_t GetLastSelected() const;

  CFX_UnownedPtr<CPWL_List_Notify> m_pNotify;
  bool m_bNotifyFlag;
  CFX_PointF m_ptScrollPos;
  CPLST_Select m_aSelItems;  // for multiple
  int32_t m_nSelItem;        // for single
  int32_t m_nFootIndex;      // for multiple
  bool m_bCtrlSel;           // for multiple
  int32_t m_nCaretIndex;     // for multiple
  std::vector<std::unique_ptr<CFX_ListItem>> m_ListItems;
  float m_fFontSize;
  CFX_UnownedPtr<IPVT_FontMap> m_pFontMap;
  bool m_bMultiple;
};

#endif  // FPDFSDK_FXEDIT_FXET_LIST_H_
