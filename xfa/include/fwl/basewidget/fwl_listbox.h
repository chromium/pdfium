// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_LISTBOX_H
#define _FWL_LISTBOX_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_ListBoxDP;
class IFWL_ListBox;
#define FWL_CLASS_ListBox L"FWL_LISTBOX"
#define FWL_CLASSHASH_ListBox 1777358317
#define FWL_STYLEEXT_LTB_MultiSelection (1L << 0)
#define FWL_STYLEEXT_LTB_Sort (1L << 1)
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
#define FWL_PART_LTB_Border 1
#define FWL_PART_LTB_Edge 2
#define FWL_PART_LTB_Background 3
#define FWL_PART_LTB_ListItem 4
#define FWL_PART_LTB_Check 5
#define FWL_PART_LTB_Icon 6
#define FWL_PARTSTATE_LTB_Normal (0L << 0)
#define FWL_PARTSTATE_LTB_Selected (1L << 0)
#define FWL_PARTSTATE_LTB_Disabled (2L << 0)
#define FWL_PARTSTATE_LTB_Focused (1L << 2)
#define FWL_PARTSTATE_LTB_UnChecked (0L << 3)
#define FWL_PARTSTATE_LTB_Checked (1L << 3)
#define FWL_PARTSTATE_LTB_Mask (3L << 0)
#define FWL_WGTHITTEST_LTB_Item FWL_WGTHITTEST_MAX + 1
#define FWL_WGTHITTEST_LTB_HScrollBar FWL_WGTHITTEST_MAX + 2
#define FWL_WGTHITTEST_LTB_VScrollBar FWL_WGTHITTEST_MAX + 3
#define FWL_EVT_LTB_SelChanged L"FWL_EVENT_LTB_SelChanged"
#define FWL_EVT_LTB_DrawItem L"FWL_EVENT_LTB_DrawItem"
#define FWL_EVTHASH_LTB_SelChanged 1701781688
#define FWL_EVTHASH_LTB_DrawItem 1050853991
BEGIN_FWL_EVENT_DEF(CFWL_EvtLtbSelChanged, FWL_EVTHASH_LTB_SelChanged)
CFX_Int32Array iarraySels;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtLtbDrawItem, FWL_EVTHASH_LTB_DrawItem)
CFX_Graphics* m_pGraphics;
CFX_Matrix m_matrix;
int32_t m_index;
CFX_RectF m_rect;
END_FWL_EVENT_DEF
typedef struct _FWL_HLISTITEM { void* pData; } * FWL_HLISTITEM;
typedef struct _FWL_ListBoxItemData {
  IFWL_ListBoxDP* pDataProvider;
  int32_t iIndex;
} FWL_ListBoxItemData;
class IFWL_ListBoxDP : public IFWL_DataProvider {
 public:
  virtual int32_t CountItems(IFWL_Widget* pWidget) = 0;
  virtual FWL_HLISTITEM GetItem(IFWL_Widget* pWidget, int32_t nIndex) = 0;
  virtual int32_t GetItemIndex(IFWL_Widget* pWidget, FWL_HLISTITEM hItem) = 0;
  virtual FX_BOOL SetItemIndex(IFWL_Widget* pWidget,
                               FWL_HLISTITEM hItem,
                               int32_t nIndex) = 0;
  virtual FX_DWORD GetItemStyles(IFWL_Widget* pWidget, FWL_HLISTITEM hItem) = 0;
  virtual FWL_ERR GetItemText(IFWL_Widget* pWidget,
                              FWL_HLISTITEM hItem,
                              CFX_WideString& wsText) = 0;
  virtual FWL_ERR GetItemRect(IFWL_Widget* pWidget,
                              FWL_HLISTITEM hItem,
                              CFX_RectF& rtItem) = 0;
  virtual void* GetItemData(IFWL_Widget* pWidget, FWL_HLISTITEM hItem) = 0;
  virtual FWL_ERR SetItemStyles(IFWL_Widget* pWidget,
                                FWL_HLISTITEM hItem,
                                FX_DWORD dwStyle) = 0;
  virtual FWL_ERR SetItemText(IFWL_Widget* pWidget,
                              FWL_HLISTITEM hItem,
                              const FX_WCHAR* pszText) = 0;
  virtual FWL_ERR SetItemRect(IFWL_Widget* pWidget,
                              FWL_HLISTITEM hItem,
                              const CFX_RectF& rtItem) = 0;
  virtual FX_FLOAT GetItemHeight(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetItemIcon(IFWL_Widget* pWidget,
                                    FWL_HLISTITEM hItem) = 0;
  virtual FWL_ERR GetItemCheckRect(IFWL_Widget* pWidget,
                                   FWL_HLISTITEM hItem,
                                   CFX_RectF& rtCheck) = 0;
  virtual FWL_ERR SetItemCheckRect(IFWL_Widget* pWidget,
                                   FWL_HLISTITEM hItem,
                                   const CFX_RectF& rtCheck) = 0;
  virtual FX_DWORD GetItemCheckState(IFWL_Widget* pWidget,
                                     FWL_HLISTITEM hItem) = 0;
  virtual FWL_ERR SetItemCheckState(IFWL_Widget* pWidget,
                                    FWL_HLISTITEM hItem,
                                    FX_DWORD dwCheckState) = 0;
};
class IFWL_ListBoxCompare {
 public:
  virtual ~IFWL_ListBoxCompare() {}
  virtual int32_t Compare(FWL_HLISTITEM hLeft, FWL_HLISTITEM hRight) = 0;
};
class IFWL_ListBox : public IFWL_Widget {
 public:
  static IFWL_ListBox* Create(const CFWL_WidgetImpProperties& properties,
                              IFWL_Widget* pOuter);
  static IFWL_ListBox* CreateComboList(
      const CFWL_WidgetImpProperties& properties,
      IFWL_Widget* pOuter);

  int32_t CountSelItems();
  FWL_HLISTITEM GetSelItem(int32_t nIndexSel);
  int32_t GetSelIndex(int32_t nIndex);
  FWL_ERR SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect = TRUE);
  FWL_ERR GetItemText(FWL_HLISTITEM hItem, CFX_WideString& wsText);
  FWL_ERR GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert = TRUE);
  FWL_ERR* Sort(IFWL_ListBoxCompare* pCom);

 protected:
  IFWL_ListBox();
};
#endif
