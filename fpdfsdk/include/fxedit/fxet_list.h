// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FXEDIT_FXET_LIST_H_
#define FPDFSDK_INCLUDE_FXEDIT_FXET_LIST_H_

#include "core/include/fpdfapi/fpdf_parser.h"  // For CPDF_Point.
#include "fx_edit.h"

class IFX_Edit;

class CLST_Size {
 public:
  CLST_Size() : x(0.0f), y(0.0f) {}

  CLST_Size(FX_FLOAT other_x, FX_FLOAT other_y) {
    x = other_x;
    y = other_y;
  }

  void Default() {
    x = 0.0f;
    y = 0.0f;
  }

  FX_BOOL operator!=(const CLST_Size& size) const {
    return FXSYS_memcmp(this, &size, sizeof(CLST_Size)) != 0;
  }

  FX_FLOAT x, y;
};

class CLST_Rect : public CPDF_Rect {
 public:
  CLST_Rect() { left = top = right = bottom = 0.0f; }

  CLST_Rect(FX_FLOAT other_left,
            FX_FLOAT other_top,
            FX_FLOAT other_right,
            FX_FLOAT other_bottom) {
    left = other_left;
    top = other_top;
    right = other_right;
    bottom = other_bottom;
  }

  CLST_Rect(const CPDF_Rect& rect) {
    left = rect.left;
    top = rect.top;
    right = rect.right;
    bottom = rect.bottom;
  }

  ~CLST_Rect() {}

  void Default() { left = top = right = bottom = 0.0f; }

  const CLST_Rect operator=(const CPDF_Rect& rect) {
    left = rect.left;
    top = rect.top;
    right = rect.right;
    bottom = rect.bottom;

    return *this;
  }

  FX_BOOL operator==(const CLST_Rect& rect) const {
    return FXSYS_memcmp(this, &rect, sizeof(CLST_Rect)) == 0;
  }

  FX_BOOL operator!=(const CLST_Rect& rect) const {
    return FXSYS_memcmp(this, &rect, sizeof(CLST_Rect)) != 0;
  }

  FX_FLOAT Width() const { return right - left; }

  FX_FLOAT Height() const {
    if (top > bottom)
      return top - bottom;
    return bottom - top;
  }

  CPDF_Point LeftTop() const { return CPDF_Point(left, top); }

  CPDF_Point RightBottom() const { return CPDF_Point(right, bottom); }

  const CLST_Rect operator+=(const CPDF_Point& point) {
    left += point.x;
    right += point.x;
    top += point.y;
    bottom += point.y;

    return *this;
  }

  const CLST_Rect operator-=(const CPDF_Point& point) {
    left -= point.x;
    right -= point.x;
    top -= point.y;
    bottom -= point.y;

    return *this;
  }

  CLST_Rect operator+(const CPDF_Point& point) const {
    return CLST_Rect(left + point.x, top + point.y, right + point.x,
                     bottom + point.y);
  }

  CLST_Rect operator-(const CPDF_Point& point) const {
    return CLST_Rect(left - point.x, top - point.y, right - point.x,
                     bottom - point.y);
  }
};

class CFX_ListItem {
 public:
  CFX_ListItem();
  virtual ~CFX_ListItem();

  void SetFontMap(IFX_Edit_FontMap* pFontMap);
  IFX_Edit_Iterator* GetIterator() const;
  IFX_Edit* GetEdit() const;

 public:
  void SetRect(const CLST_Rect& rect);
  void SetSelect(FX_BOOL bSelected);
  void SetCaret(FX_BOOL bCaret);
  void SetText(const FX_WCHAR* text);
  void SetFontSize(FX_FLOAT fFontSize);
  CFX_WideString GetText() const;

  CLST_Rect GetRect() const;
  FX_BOOL IsSelected() const;
  FX_BOOL IsCaret() const;
  FX_FLOAT GetItemHeight() const;
  FX_WORD GetFirstChar() const;

 private:
  IFX_Edit* m_pEdit;
  FX_BOOL m_bSelected;
  FX_BOOL m_bCaret;
  CLST_Rect m_rcListItem;
};

class CFX_ListContainer {
 public:
  CFX_ListContainer()
      : m_rcPlate(0.0f, 0.0f, 0.0f, 0.0f),
        m_rcContent(0.0f, 0.0f, 0.0f, 0.0f) {}
  virtual ~CFX_ListContainer() {}
  virtual void SetPlateRect(const CPDF_Rect& rect) { m_rcPlate = rect; }
  CPDF_Rect GetPlateRect() const { return m_rcPlate; }
  void SetContentRect(const CLST_Rect& rect) { m_rcContent = rect; }
  CLST_Rect GetContentRect() const { return m_rcContent; }
  CPDF_Point GetBTPoint() const {
    return CPDF_Point(m_rcPlate.left, m_rcPlate.top);
  }
  CPDF_Point GetETPoint() const {
    return CPDF_Point(m_rcPlate.right, m_rcPlate.bottom);
  }

 public:
  CPDF_Point InnerToOuter(const CPDF_Point& point) const {
    return CPDF_Point(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
  }
  CPDF_Point OuterToInner(const CPDF_Point& point) const {
    return CPDF_Point(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
  }
  CPDF_Rect InnerToOuter(const CLST_Rect& rect) const {
    CPDF_Point ptLeftTop = InnerToOuter(CPDF_Point(rect.left, rect.top));
    CPDF_Point ptRightBottom =
        InnerToOuter(CPDF_Point(rect.right, rect.bottom));
    return CPDF_Rect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                     ptLeftTop.y);
  }
  CLST_Rect OuterToInner(const CPDF_Rect& rect) const {
    CPDF_Point ptLeftTop = OuterToInner(CPDF_Point(rect.left, rect.top));
    CPDF_Point ptRightBottom =
        OuterToInner(CPDF_Point(rect.right, rect.bottom));
    return CLST_Rect(ptLeftTop.x, ptLeftTop.y, ptRightBottom.x,
                     ptRightBottom.y);
  }

 private:
  CPDF_Rect m_rcPlate;
  CLST_Rect m_rcContent;  // positive forever!
};

template <class TYPE>
class CLST_ArrayTemplate : public CFX_ArrayTemplate<TYPE> {
 public:
  FX_BOOL IsEmpty() { return CFX_ArrayTemplate<TYPE>::GetSize() <= 0; }
  TYPE GetAt(int32_t nIndex) const {
    if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize())
      return CFX_ArrayTemplate<TYPE>::GetAt(nIndex);
    return NULL;
  }
  void RemoveAt(int32_t nIndex) {
    if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize())
      CFX_ArrayTemplate<TYPE>::RemoveAt(nIndex);
  }
};

class CFX_List : protected CFX_ListContainer, public IFX_List {
 public:
  CFX_List();
  ~CFX_List() override;

  // IFX_List:
  void SetFontMap(IFX_Edit_FontMap* pFontMap) override;
  void SetFontSize(FX_FLOAT fFontSize) override;
  CPDF_Rect GetPlateRect() const override;
  CPDF_Rect GetContentRect() const override;
  FX_FLOAT GetFontSize() const override;
  IFX_Edit* GetItemEdit(int32_t nIndex) const override;
  int32_t GetCount() const override;
  FX_BOOL IsItemSelected(int32_t nIndex) const override;
  FX_FLOAT GetFirstHeight() const override;
  void SetMultipleSel(FX_BOOL bMultiple) override;
  FX_BOOL IsMultipleSel() const override;
  FX_BOOL IsValid(int32_t nItemIndex) const override;
  int32_t FindNext(int32_t nIndex, FX_WCHAR nChar) const override;
  void Empty() override;
  CPDF_Rect GetItemRect(int32_t nIndex) const override;
  int32_t GetItemIndex(const CPDF_Point& point) const override;
  int32_t GetFirstSelected() const override;

 protected:
  void AddItem(const FX_WCHAR* str);
  virtual void ReArrange(int32_t nItemIndex);
  CFX_WideString GetItemText(int32_t nIndex) const;
  void SetItemSelect(int32_t nItemIndex, FX_BOOL bSelected);
  void SetItemCaret(int32_t nItemIndex, FX_BOOL bCaret);
  int32_t GetLastSelected() const;
  FX_WCHAR Toupper(FX_WCHAR c) const;

 private:
  CLST_ArrayTemplate<CFX_ListItem*> m_aListItems;
  FX_FLOAT m_fFontSize;
  IFX_Edit_FontMap* m_pFontMap;
  FX_BOOL m_bMultiple;
};

struct CPLST_Select_Item {
  CPLST_Select_Item(int32_t other_nItemIndex, int32_t other_nState) {
    nItemIndex = other_nItemIndex;
    nState = other_nState;
  }

  int32_t nItemIndex;
  int32_t nState;  // 0:normal select -1:to deselect 1: to select
};

class CPLST_Select {
 public:
  CPLST_Select();
  virtual ~CPLST_Select();

 public:
  void Add(int32_t nItemIndex);
  void Add(int32_t nBeginIndex, int32_t nEndIndex);
  void Sub(int32_t nItemIndex);
  void Sub(int32_t nBeginIndex, int32_t nEndIndex);
  FX_BOOL IsExist(int32_t nItemIndex) const;
  int32_t Find(int32_t nItemIndex) const;
  int32_t GetCount() const;
  int32_t GetItemIndex(int32_t nIndex) const;
  int32_t GetState(int32_t nIndex) const;
  void Done();
  void DeselectAll();

 private:
  CFX_ArrayTemplate<CPLST_Select_Item*> m_aItems;
};

class CFX_ListCtrl : public CFX_List {
 public:
  CFX_ListCtrl();
  ~CFX_ListCtrl() override;

  // CFX_List
  void SetNotify(IFX_List_Notify* pNotify) override;
  void OnMouseDown(const CPDF_Point& point,
                   FX_BOOL bShift,
                   FX_BOOL bCtrl) override;
  void OnMouseMove(const CPDF_Point& point,
                   FX_BOOL bShift,
                   FX_BOOL bCtrl) override;
  void OnVK_UP(FX_BOOL bShift, FX_BOOL bCtrl) override;
  void OnVK_DOWN(FX_BOOL bShift, FX_BOOL bCtrl) override;
  void OnVK_LEFT(FX_BOOL bShift, FX_BOOL bCtrl) override;
  void OnVK_RIGHT(FX_BOOL bShift, FX_BOOL bCtrl) override;
  void OnVK_HOME(FX_BOOL bShift, FX_BOOL bCtrl) override;
  void OnVK_END(FX_BOOL bShift, FX_BOOL bCtrl) override;
  void OnVK(int32_t nItemIndex, FX_BOOL bShift, FX_BOOL bCtrl) override;
  FX_BOOL OnChar(FX_WORD nChar, FX_BOOL bShift, FX_BOOL bCtrl) override;
  void SetPlateRect(const CPDF_Rect& rect) override;
  void SetScrollPos(const CPDF_Point& point) override;
  void ScrollToListItem(int32_t nItemIndex) override;
  CPDF_Rect GetItemRect(int32_t nIndex) const override;
  int32_t GetCaret() const override { return m_nCaretIndex; }
  int32_t GetSelect() const override { return m_nSelItem; }
  int32_t GetTopItem() const override;
  CPDF_Rect GetContentRect() const override;
  int32_t GetItemIndex(const CPDF_Point& point) const override;
  void AddString(const FX_WCHAR* string) override;
  void SetTopItem(int32_t nIndex) override;
  void Select(int32_t nItemIndex) override;
  void SetCaret(int32_t nItemIndex) override;
  void Empty() override;
  void Cancel() override;
  CFX_WideString GetText() const override;
  void ReArrange(int32_t nItemIndex) override;

  virtual CPDF_Point InToOut(const CPDF_Point& point) const;
  virtual CPDF_Point OutToIn(const CPDF_Point& point) const;
  virtual CPDF_Rect InToOut(const CPDF_Rect& rect) const;
  virtual CPDF_Rect OutToIn(const CPDF_Rect& rect) const;

 private:
  void SetMultipleSelect(int32_t nItemIndex, FX_BOOL bSelected);
  void SetSingleSelect(int32_t nItemIndex);
  void InvalidateItem(int32_t nItemIndex);
  void SelectItems();
  FX_BOOL IsItemVisible(int32_t nItemIndex) const;
  void SetScrollInfo();
  void SetScrollPosY(FX_FLOAT fy);

 private:
  IFX_List_Notify* m_pNotify;
  FX_BOOL m_bNotifyFlag;
  CPDF_Point m_ptScrollPos;
  CPLST_Select m_aSelItems;  // for multiple
  int32_t m_nSelItem;        // for single
  int32_t m_nFootIndex;      // for multiple
  FX_BOOL m_bCtrlSel;        // for multiple
  int32_t m_nCaretIndex;     // for multiple
};

#endif  // FPDFSDK_INCLUDE_FXEDIT_FXET_LIST_H_
