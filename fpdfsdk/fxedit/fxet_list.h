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

class CLST_Rect : public CFX_FloatRect {
 public:
  CLST_Rect() { left = top = right = bottom = 0.0f; }

  CLST_Rect(float other_left,
            float other_top,
            float other_right,
            float other_bottom) {
    left = other_left;
    top = other_top;
    right = other_right;
    bottom = other_bottom;
  }

  explicit CLST_Rect(const CFX_FloatRect& rect) {
    left = rect.left;
    top = rect.top;
    right = rect.right;
    bottom = rect.bottom;
  }

  ~CLST_Rect() {}

  void Default() { left = top = right = bottom = 0.0f; }

  const CLST_Rect operator=(const CFX_FloatRect& rect) {
    left = rect.left;
    top = rect.top;
    right = rect.right;
    bottom = rect.bottom;

    return *this;
  }

  bool operator==(const CLST_Rect& rect) const {
    return memcmp(this, &rect, sizeof(CLST_Rect)) == 0;
  }

  bool operator!=(const CLST_Rect& rect) const { return !(*this == rect); }

  float Width() const { return right - left; }

  float Height() const {
    if (top > bottom)
      return top - bottom;
    return bottom - top;
  }

  CFX_PointF LeftTop() const { return CFX_PointF(left, top); }

  CFX_PointF RightBottom() const { return CFX_PointF(right, bottom); }

  const CLST_Rect operator+=(const CFX_PointF& point) {
    left += point.x;
    right += point.x;
    top += point.y;
    bottom += point.y;

    return *this;
  }

  const CLST_Rect operator-=(const CFX_PointF& point) {
    left -= point.x;
    right -= point.x;
    top -= point.y;
    bottom -= point.y;

    return *this;
  }

  CLST_Rect operator+(const CFX_PointF& point) const {
    return CLST_Rect(left + point.x, top + point.y, right + point.x,
                     bottom + point.y);
  }

  CLST_Rect operator-(const CFX_PointF& point) const {
    return CLST_Rect(left - point.x, top - point.y, right - point.x,
                     bottom - point.y);
  }
};

class CFX_ListItem final {
 public:
  CFX_ListItem();
  ~CFX_ListItem();

  void SetFontMap(IPVT_FontMap* pFontMap);
  CFX_Edit* GetEdit() const;

  void SetRect(const CLST_Rect& rect);
  void SetSelect(bool bSelected);
  void SetText(const CFX_WideString& text);
  void SetFontSize(float fFontSize);
  CFX_WideString GetText() const;

  CLST_Rect GetRect() const;
  bool IsSelected() const;
  float GetItemHeight() const;
  uint16_t GetFirstChar() const;

 private:
  CFX_Edit_Iterator* GetIterator() const;

  std::unique_ptr<CFX_Edit> m_pEdit;
  bool m_bSelected;
  CLST_Rect m_rcListItem;
};

class CFX_ListContainer {
 public:
  CFX_ListContainer();
  virtual ~CFX_ListContainer();

  virtual void SetPlateRect(const CFX_FloatRect& rect);

  CFX_FloatRect GetPlateRect() const { return m_rcPlate; }
  void SetContentRect(const CLST_Rect& rect) { m_rcContent = rect; }
  CLST_Rect GetContentRect() const { return m_rcContent; }
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
  CFX_FloatRect InnerToOuter(const CLST_Rect& rect) const {
    CFX_PointF ptLeftTop = InnerToOuter(CFX_PointF(rect.left, rect.top));
    CFX_PointF ptRightBottom =
        InnerToOuter(CFX_PointF(rect.right, rect.bottom));
    return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                         ptLeftTop.y);
  }
  CLST_Rect OuterToInner(const CFX_FloatRect& rect) const {
    CFX_PointF ptLeftTop = OuterToInner(CFX_PointF(rect.left, rect.top));
    CFX_PointF ptRightBottom =
        OuterToInner(CFX_PointF(rect.right, rect.bottom));
    return CLST_Rect(ptLeftTop.x, ptLeftTop.y, ptRightBottom.x,
                     ptRightBottom.y);
  }

 private:
  CFX_FloatRect m_rcPlate;
  CLST_Rect m_rcContent;  // positive forever!
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
