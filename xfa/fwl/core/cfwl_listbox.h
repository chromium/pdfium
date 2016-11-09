// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_LISTBOX_H_
#define XFA_FWL_CORE_CFWL_LISTBOX_H_

#include <memory>
#include <vector>

#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_listbox.h"
#include "xfa/fwl/core/ifwl_widget.h"

class CFWL_ListBox : public CFWL_Widget, public IFWL_ListBoxDP {
 public:
  CFWL_ListBox(const IFWL_App*);
  ~CFWL_ListBox() override;

  void Initialize();

  FWL_Error AddDIBitmap(CFX_DIBitmap* pDIB, CFWL_ListItem* pItem);
  CFWL_ListItem* AddString(const CFX_WideStringC& wsAdd, bool bSelect = false);
  bool DeleteString(CFWL_ListItem* pItem);
  void DeleteAll();
  int32_t CountSelItems();
  CFWL_ListItem* GetSelItem(int32_t nIndexSel);
  int32_t GetSelIndex(int32_t nIndex);
  FWL_Error SetSelItem(CFWL_ListItem* pItem, bool bSelect = true);
  FWL_Error GetItemText(CFWL_ListItem* pItem, CFX_WideString& wsText);
  FWL_Error GetScrollPos(FX_FLOAT& fPos, bool bVert = true);
  FWL_Error SetItemHeight(FX_FLOAT fItemHeight);
  CFWL_ListItem* GetFocusItem();
  FWL_Error SetFocusItem(CFWL_ListItem* pItem);
  int32_t CountItems();
  CFWL_ListItem* GetItem(int32_t nIndex);
  FWL_Error SetItemString(CFWL_ListItem* pItem, const CFX_WideStringC& wsText);
  FWL_Error GetItemString(CFWL_ListItem* pItem, CFX_WideString& wsText);
  FWL_Error SetItemData(CFWL_ListItem* pItem, void* pData);
  void* GetItemData(CFWL_ListItem* pItem);
  CFWL_ListItem* GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  uint32_t GetItemStates(CFWL_ListItem* pItem);

  // IFWL_DataProvider:
  FWL_Error GetCaption(IFWL_Widget* pWidget,
                       CFX_WideString& wsCaption) override;

  // IFWL_ListBoxDP:
  int32_t CountItems(const IFWL_Widget* pWidget) override;
  CFWL_ListItem* GetItem(const IFWL_Widget* pWidget, int32_t nIndex) override;
  int32_t GetItemIndex(IFWL_Widget* pWidget, CFWL_ListItem* pItem) override;
  bool SetItemIndex(IFWL_Widget* pWidget,
                    CFWL_ListItem* pItem,
                    int32_t nIndex) override;
  uint32_t GetItemStyles(IFWL_Widget* pWidget, CFWL_ListItem* pItem) override;
  FWL_Error GetItemText(IFWL_Widget* pWidget,
                        CFWL_ListItem* pItem,
                        CFX_WideString& wsText) override;
  FWL_Error GetItemRect(IFWL_Widget* pWidget,
                        CFWL_ListItem* pItem,
                        CFX_RectF& rtItem) override;
  void* GetItemData(IFWL_Widget* pWidget, CFWL_ListItem* pItem) override;
  FWL_Error SetItemStyles(IFWL_Widget* pWidget,
                          CFWL_ListItem* pItem,
                          uint32_t dwStyle) override;
  FWL_Error SetItemText(IFWL_Widget* pWidget,
                        CFWL_ListItem* pItem,
                        const FX_WCHAR* pszText) override;
  FWL_Error SetItemRect(IFWL_Widget* pWidget,
                        CFWL_ListItem* pItem,
                        const CFX_RectF& rtItem) override;
  FX_FLOAT GetItemHeight(IFWL_Widget* pWidget) override;
  CFX_DIBitmap* GetItemIcon(IFWL_Widget* pWidget,
                            CFWL_ListItem* pItem) override;
  FWL_Error GetItemCheckRect(IFWL_Widget* pWidget,
                             CFWL_ListItem* pItem,
                             CFX_RectF& rtCheck) override;
  FWL_Error SetItemCheckRect(IFWL_Widget* pWidget,
                             CFWL_ListItem* pItem,
                             const CFX_RectF& rtCheck) override;
  uint32_t GetItemCheckState(IFWL_Widget* pWidget,
                             CFWL_ListItem* pItem) override;
  FWL_Error SetItemCheckState(IFWL_Widget* pWidget,
                              CFWL_ListItem* pItem,
                              uint32_t dwCheckState) override;

 private:
  std::vector<std::unique_ptr<CFWL_ListItem>> m_ItemArray;
  CFX_WideString m_wsData;
  FX_FLOAT m_fItemHeight;
};

#endif  // XFA_FWL_CORE_CFWL_LISTBOX_H_
