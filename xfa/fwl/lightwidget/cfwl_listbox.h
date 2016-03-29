// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_LISTBOX_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_LISTBOX_H_

#include <memory>
#include <vector>

#include "xfa/fwl/basewidget/ifwl_listbox.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fwl/lightwidget/cfwl_widget.h"

class CFWL_ListItem;

class CFWL_ListBox : public CFWL_Widget {
 public:
  static CFWL_ListBox* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FWL_ERR AddDIBitmap(CFX_DIBitmap* pDIB, FWL_HLISTITEM hItem);
  FWL_HLISTITEM AddString(const CFX_WideStringC& wsAdd,
                          FX_BOOL bSelect = FALSE);
  FX_BOOL DeleteString(FWL_HLISTITEM hItem);
  void DeleteAll();
  int32_t CountSelItems();
  FWL_HLISTITEM GetSelItem(int32_t nIndexSel);
  int32_t GetSelIndex(int32_t nIndex);
  FWL_ERR SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect = TRUE);
  FWL_ERR GetItemText(FWL_HLISTITEM hItem, CFX_WideString& wsText);
  FWL_ERR GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert = TRUE);
  FWL_ERR SetItemHeight(FX_FLOAT fItemHeight);
  FWL_HLISTITEM GetFocusItem();
  FWL_ERR SetFocusItem(FWL_HLISTITEM hItem);
  FWL_ERR* Sort(IFWL_ListBoxCompare* pCom);
  int32_t CountItems();
  FWL_HLISTITEM GetItem(int32_t nIndex);
  FWL_ERR SetItemString(FWL_HLISTITEM hItem, const CFX_WideStringC& wsText);
  FWL_ERR GetItemString(FWL_HLISTITEM hItem, CFX_WideString& wsText);
  FWL_ERR SetItemData(FWL_HLISTITEM hItem, void* pData);
  void* GetItemData(FWL_HLISTITEM hItem);
  FWL_HLISTITEM GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  uint32_t GetItemStates(FWL_HLISTITEM hItem);
  CFWL_ListBox();
  virtual ~CFWL_ListBox();

 protected:
  class CFWL_ListBoxDP : public IFWL_ListBoxDP {
   public:
    CFWL_ListBoxDP();
    ~CFWL_ListBoxDP();

    // IFWL_DataProvider:
    FWL_ERR GetCaption(IFWL_Widget* pWidget,
                       CFX_WideString& wsCaption) override;

    // IFWL_ListBoxDP:
    int32_t CountItems(IFWL_Widget* pWidget) override;
    FWL_HLISTITEM GetItem(IFWL_Widget* pWidget, int32_t nIndex) override;
    int32_t GetItemIndex(IFWL_Widget* pWidget, FWL_HLISTITEM hItem) override;
    FX_BOOL SetItemIndex(IFWL_Widget* pWidget,
                         FWL_HLISTITEM hItem,
                         int32_t nIndex) override;
    uint32_t GetItemStyles(IFWL_Widget* pWidget, FWL_HLISTITEM hItem) override;
    FWL_ERR GetItemText(IFWL_Widget* pWidget,
                        FWL_HLISTITEM hItem,
                        CFX_WideString& wsText) override;
    FWL_ERR GetItemRect(IFWL_Widget* pWidget,
                        FWL_HLISTITEM hItem,
                        CFX_RectF& rtItem) override;
    void* GetItemData(IFWL_Widget* pWidget, FWL_HLISTITEM hItem) override;
    FWL_ERR SetItemStyles(IFWL_Widget* pWidget,
                          FWL_HLISTITEM hItem,
                          uint32_t dwStyle) override;
    FWL_ERR SetItemText(IFWL_Widget* pWidget,
                        FWL_HLISTITEM hItem,
                        const FX_WCHAR* pszText) override;
    FWL_ERR SetItemRect(IFWL_Widget* pWidget,
                        FWL_HLISTITEM hItem,
                        const CFX_RectF& rtItem) override;
    FX_FLOAT GetItemHeight(IFWL_Widget* pWidget) override;
    CFX_DIBitmap* GetItemIcon(IFWL_Widget* pWidget,
                              FWL_HLISTITEM hItem) override;
    FWL_ERR GetItemCheckRect(IFWL_Widget* pWidget,
                             FWL_HLISTITEM hItem,
                             CFX_RectF& rtCheck) override;
    FWL_ERR SetItemCheckRect(IFWL_Widget* pWidget,
                             FWL_HLISTITEM hItem,
                             const CFX_RectF& rtCheck) override;
    uint32_t GetItemCheckState(IFWL_Widget* pWidget,
                               FWL_HLISTITEM hItem) override;
    FWL_ERR SetItemCheckState(IFWL_Widget* pWidget,
                              FWL_HLISTITEM hItem,
                              uint32_t dwCheckState) override;

    std::vector<std::unique_ptr<CFWL_ListItem>> m_ItemArray;
    CFX_WideString m_wsData;
    FX_FLOAT m_fItemHeight;
  };

  CFWL_ListBoxDP m_ListBoxDP;
};

class CFWL_ListItem {
 public:
  CFWL_ListItem() {
    m_rtItem.Reset();
    m_dwStates = 0;
    m_wsText = L"";
    m_pDIB = NULL;
    m_pData = NULL;
    m_dwCheckState = 0;
    m_rtCheckBox.Reset();
  }
  CFX_RectF m_rtItem;
  uint32_t m_dwStates;
  CFX_WideString m_wsText;
  CFX_DIBitmap* m_pDIB;
  void* m_pData;
  uint32_t m_dwCheckState;
  CFX_RectF m_rtCheckBox;
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_LISTBOX_H_
