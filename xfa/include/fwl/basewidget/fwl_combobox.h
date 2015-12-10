// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_COMBOBOX_H
#define _FWL_COMBOBOX_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_ListBox;
class IFWL_ComboBoxDP;
class IFWL_ComboBox;
#define FWL_CLASS_ComboBox L"FWL_COMBOBOX"
#define FWL_CLASSHASH_ComboBox 602353697
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
#define FWL_STYLEEXT_CMB_ListItemText (0L << 12)
#define FWL_STYLEEXT_CMB_ListItemIconText (1L << 12)
#define FWL_STYLEEXT_CMB_ListItemAlignMask (3L << 12)
#define FWL_STYLEEXT_CMB_ReadOnly (1L << 13)
#define FWL_PART_CMB_Border 1
#define FWL_PART_CMB_Edge 2
#define FWL_PART_CMB_Background 3
#define FWL_PART_CMB_DropDownButton 4
#define FWL_PART_CMB_Caption 5
#define FWL_PART_CMB_StretcgHandler 6
#define FWL_PARTSTATE_CMB_Normal (0L << 0)
#define FWL_PARTSTATE_CMB_Hovered (1L << 0)
#define FWL_PARTSTATE_CMB_Pressed (2L << 0)
#define FWL_PARTSTATE_CMB_Disabled (3L << 0)
#define FWL_PARTSTATE_CMB_Selected (1L << 2)
#define FWL_WGTCAPACITY_CMB_ComboFormHandler (FWL_WGTCAPACITY_MAX + 7)
enum FWL_CMB_TEXTCHANGED {
  FWL_CMB_TEXTCHANGED_Insert = 0,
  FWL_CMB_TEXTCHANGED_Delete,
  FWL_CMB_TEXTCHANGED_Replace,
};
#define FWL_EVT_CMB_DropDown L"FWL_EVENT_CMB_PreDropDown"
#define FWL_EVT_CMB_PostDropDown L"FWL_EVENT_CMB_PostDropDown"
#define FWL_EVT_CMB_CloseUp L"FWL_EVENT_CMB_CloseUp"
#define FWL_EVT_CMB_EditChanged L"FWL_EVENT_CMB_EditChanged"
#define FWL_EVT_CMB_SelChanged L"FWL_EVENT_CMB_SelChanged"
#define FWL_EVT_CMB_HoverChanged L"FWL_EVENT_CMB_HoverChanged"
#define FWL_EVT_CMB_DrawItem L"FWL_EVENT_CMB_DrawItem"
#define FWL_EVTHASH_CMB_PreDropDown 1357646798
#define FWL_EVTHASH_CMB_PostDropDown 3677010285
#define FWL_EVTHASH_CMB_CloseUp 2871271190
#define FWL_EVTHASH_CMB_EditChanged 1527034762
#define FWL_EVTHASH_CMB_SelChanged 2923227784
#define FWL_EVTHASH_CMB_HoverChanged 944325448
#define FWL_EVTHASH_CMB_DrawItem 917354551
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbPreDropDown, FWL_EVTHASH_CMB_PreDropDown)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbPostDropDown, FWL_EVTHASH_CMB_PostDropDown)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbCloseUp, FWL_EVTHASH_CMB_CloseUp)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbEditChanged, FWL_EVTHASH_CMB_EditChanged)
int32_t nChangeType;
CFX_WideString wsInsert;
CFX_WideString wsDelete;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbSelChanged, FWL_EVTHASH_CMB_SelChanged)
CFX_Int32Array iArraySels;
FX_BOOL bLButtonUp;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbHoverChanged, FWL_EVTHASH_CMB_HoverChanged)
int32_t m_iCurHover;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtCmbDrawItem, FWL_EVTHASH_CMB_DrawItem)
CFX_Graphics* m_pGraphics;
CFX_Matrix m_matrix;
int32_t m_index;
CFX_RectF m_rtItem;
END_FWL_EVENT_DEF
class IFWL_ComboBoxDP : public IFWL_ListBoxDP {
 public:
  virtual FX_FLOAT GetListHeight(IFWL_Widget* pWidget) = 0;
};
class IFWL_ComboBox : public IFWL_Widget {
 public:
  static IFWL_ComboBox* Create(const CFWL_WidgetImpProperties& properties);

  int32_t GetCurSel();
  FWL_ERR SetCurSel(int32_t iSel);
  FWL_ERR SetEditText(const CFX_WideString& wsText);
  int32_t GetEditTextLength() const;
  FWL_ERR GetEditText(CFX_WideString& wsText,
                      int32_t nStart = 0,
                      int32_t nCount = -1) const;
  FWL_ERR SetEditSelRange(int32_t nStart, int32_t nCount = -1);
  int32_t GetEditSelRange(int32_t nIndex, int32_t& nStart);
  int32_t GetEditLimit();
  FWL_ERR SetEditLimit(int32_t nLimit);
  FWL_ERR EditDoClipboard(int32_t iCmd);
  FX_BOOL EditRedo(const CFX_ByteStringC& bsRecord);
  FX_BOOL EditUndo(const CFX_ByteStringC& bsRecord);
  IFWL_ListBox* GetListBoxt();
  FX_BOOL AfterFocusShowDropList();
  FX_ERR OpenDropDownList(FX_BOOL bActivate);
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
  FWL_ERR GetBBox(CFX_RectF& rect);
  FWL_ERR EditModifyStylesEx(FX_DWORD dwStylesExAdded,
                             FX_DWORD dwStylesExRemoved);

 protected:
  IFWL_ComboBox();
};
#endif
