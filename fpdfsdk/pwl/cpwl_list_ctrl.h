// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_LIST_CTRL_H_
#define FPDFSDK_PWL_CPWL_LIST_CTRL_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fpdfsdk/pwl/cpwl_edit_impl.h"

class IPVT_FontMap;

class CPWL_ListCtrl {
 public:
  class NotifyIface {
   public:
    virtual ~NotifyIface();

    virtual void OnSetScrollInfoY(float fPlateMin,
                                  float fPlateMax,
                                  float fContentMin,
                                  float fContentMax,
                                  float fSmallStep,
                                  float fBigStep) = 0;
    virtual void OnSetScrollPosY(float fy) = 0;

    // Returns true if `this` is still allocated.
    [[nodiscard]] virtual bool OnInvalidateRect(const CFX_FloatRect& rect) = 0;
  };

  CPWL_ListCtrl();
  ~CPWL_ListCtrl();

  void SetNotify(NotifyIface* pNotify) { notify_ = pNotify; }
  void OnMouseDown(const CFX_PointF& point, bool bShift, bool bCtrl);
  void OnMouseMove(const CFX_PointF& point, bool bShift, bool bCtrl);
  void OnVK_UP(bool bShift, bool bCtrl);
  void OnVK_DOWN(bool bShift, bool bCtrl);
  void OnVK_LEFT(bool bShift, bool bCtrl);
  void OnVK_RIGHT(bool bShift, bool bCtrl);
  void OnVK_HOME(bool bShift, bool bCtrl);
  void OnVK_END(bool bShift, bool bCtrl);
  bool OnChar(uint16_t nChar, bool bShift, bool bCtrl);

  void SetScrollPos(const CFX_PointF& point);
  void ScrollToListItem(int32_t nItemIndex);
  CFX_FloatRect GetItemRect(int32_t nIndex) const;
  int32_t GetCaret() const { return caret_index_; }
  int32_t GetSelect() const { return sel_item_; }
  int32_t GetTopItem() const;
  CFX_FloatRect GetContentRect() const;

  int32_t GetItemIndex(const CFX_PointF& point) const;
  void AddString(const WideString& str);
  void SetTopItem(int32_t nIndex);
  void Select(int32_t nItemIndex);
  void Deselect(int32_t nItemIndex);
  void SetCaret(int32_t nItemIndex);
  WideString GetText() const;

  void SetFontMap(IPVT_FontMap* font_map) { font_map_ = font_map; }
  void SetFontSize(float fFontSize) { font_size_ = fFontSize; }
  CFX_FloatRect GetPlateRect() const { return plate_rect_; }
  void SetPlateRect(const CFX_FloatRect& rect);

  float GetFontSize() const { return font_size_; }
  CPWL_EditImpl* GetItemEdit(int32_t nIndex) const;
  int32_t GetCount() const;
  bool IsItemSelected(int32_t nIndex) const;
  float GetFirstHeight() const;
  void SetMultipleSel(bool bMultiple) { multiple_ = bMultiple; }
  bool IsMultipleSel() const { return multiple_; }
  int32_t FindNext(int32_t nIndex, wchar_t nChar) const;
  int32_t GetFirstSelected() const;

 private:
  class Item {
   public:
    Item();
    ~Item();

    void SetFontMap(IPVT_FontMap* font_map);
    CPWL_EditImpl* GetEdit() const { return edit_.get(); }

    void SetRect(const CFX_FloatRect& rect) { list_item_rect_ = rect; }
    void SetSelect(bool bSelected) { selected_ = bSelected; }
    void SetText(const WideString& text);
    void SetFontSize(float fFontSize);
    WideString GetText() const;

    CFX_FloatRect GetRect() const { return list_item_rect_; }
    bool IsSelected() const { return selected_; }
    float GetItemHeight() const;
    uint16_t GetFirstChar() const;

   private:
    CPWL_EditImpl::Iterator* GetIterator() const;

    bool selected_ = false;
    CFX_FloatRect list_item_rect_;
    std::unique_ptr<CPWL_EditImpl> const edit_;
  };

  class SelectState {
   public:
    enum State { DESELECTING = -1, NORMAL = 0, SELECTING = 1 };
    using const_iterator = std::map<int32_t, State>::const_iterator;

    SelectState();
    ~SelectState();

    void Add(int32_t nItemIndex);
    void Add(int32_t nBeginIndex, int32_t nEndIndex);
    void Sub(int32_t nItemIndex);
    void Sub(int32_t nBeginIndex, int32_t nEndIndex);
    void DeselectAll();
    void Done();

    const_iterator begin() const { return items_.begin(); }
    const_iterator end() const { return items_.end(); }

   private:
    std::map<int32_t, State> items_;
  };

  CFX_PointF InToOut(const CFX_PointF& point) const;
  CFX_PointF OutToIn(const CFX_PointF& point) const;
  CFX_FloatRect InToOut(const CFX_FloatRect& rect) const;
  CFX_FloatRect OutToIn(const CFX_FloatRect& rect) const;

  CFX_PointF InnerToOuter(const CFX_PointF& point) const;
  CFX_PointF OuterToInner(const CFX_PointF& point) const;
  CFX_FloatRect InnerToOuter(const CFX_FloatRect& rect) const;
  CFX_FloatRect OuterToInner(const CFX_FloatRect& rect) const;

  void OnVK(int32_t nItemIndex, bool bShift, bool bCtrl);
  bool IsValid(int32_t nItemIndex) const;

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
  void AddItem(const WideString& str);
  WideString GetItemText(int32_t nIndex) const;
  void SetItemSelect(int32_t nIndex, bool bSelected);
  int32_t GetLastSelected() const;
  CFX_PointF GetBTPoint() const {
    return CFX_PointF(plate_rect_.left, plate_rect_.top);
  }

  CFX_FloatRect plate_rect_;
  CFX_FloatRect content_rect_;
  CFX_PointF scroll_pos_point_;
  float font_size_ = 0.0f;

  // For single:
  int32_t sel_item_ = -1;

  // For multiple:
  SelectState select_state_;
  int32_t foot_index_ = -1;
  int32_t caret_index_ = -1;
  bool ctrl_sel_ = false;

  bool multiple_ = false;
  bool notify_flag_ = false;
  UnownedPtr<NotifyIface> notify_;
  std::vector<std::unique_ptr<Item>> list_items_;
  UnownedPtr<IPVT_FontMap> font_map_;
};

#endif  // FPDFSDK_PWL_CPWL_LIST_CTRL_H_
