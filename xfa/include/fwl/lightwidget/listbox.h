// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_LISTBOX_LIGHT_H
#define _FWL_LISTBOX_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_ListBoxDP;
class CFWL_ListBox;
class CFWL_ListItem;
class CFWL_ListBox : public CFWL_Widget {
 public:
  static CFWL_ListBox* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FWL_ERR AddDIBitmap(CFX_DIBitmap* pDIB, FWL_HLISTITEM hItem);
  FWL_HLISTITEM AddString(const CFX_WideStringC& wsAdd,
                          FX_BOOL bSelect = FALSE);
  FX_BOOL DeleteString(FWL_HLISTITEM hItem);
  FX_BOOL DeleteAll();
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
  FX_DWORD GetItemStates(FWL_HLISTITEM hItem);
  CFWL_ListBox();
  virtual ~CFWL_ListBox();

 protected:
  class CFWL_ListBoxDP : public IFWL_ListBoxDP {
   public:
    CFWL_ListBoxDP();
    ~CFWL_ListBoxDP();
    virtual FWL_ERR GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);

    virtual int32_t CountItems(IFWL_Widget* pWidget);
    virtual FWL_HLISTITEM GetItem(IFWL_Widget* pWidget, int32_t nIndex);
    virtual int32_t GetItemIndex(IFWL_Widget* pWidget, FWL_HLISTITEM hItem);
    virtual FX_BOOL SetItemIndex(IFWL_Widget* pWidget,
                                 FWL_HLISTITEM hItem,
                                 int32_t nIndex);

    virtual FX_DWORD GetItemStyles(IFWL_Widget* pWidget, FWL_HLISTITEM hItem);
    virtual FWL_ERR GetItemText(IFWL_Widget* pWidget,
                                FWL_HLISTITEM hItem,
                                CFX_WideString& wsText);
    virtual FWL_ERR GetItemRect(IFWL_Widget* pWidget,
                                FWL_HLISTITEM hItem,
                                CFX_RectF& rtItem);
    virtual void* GetItemData(IFWL_Widget* pWidget, FWL_HLISTITEM hItem);

    virtual FWL_ERR SetItemStyles(IFWL_Widget* pWidget,
                                  FWL_HLISTITEM hItem,
                                  FX_DWORD dwStyle);
    virtual FWL_ERR SetItemText(IFWL_Widget* pWidget,
                                FWL_HLISTITEM hItem,
                                const FX_WCHAR* pszText);
    virtual FWL_ERR SetItemRect(IFWL_Widget* pWidget,
                                FWL_HLISTITEM hItem,
                                const CFX_RectF& rtItem);
    virtual FX_FLOAT GetItemHeight(IFWL_Widget* pWidget);
    virtual CFX_DIBitmap* GetItemIcon(IFWL_Widget* pWidget,
                                      FWL_HLISTITEM hItem);
    virtual FWL_ERR GetItemCheckRect(IFWL_Widget* pWidget,
                                     FWL_HLISTITEM hItem,
                                     CFX_RectF& rtCheck);
    virtual FWL_ERR SetItemCheckRect(IFWL_Widget* pWidget,
                                     FWL_HLISTITEM hItem,
                                     const CFX_RectF& rtCheck);
    virtual FX_DWORD GetItemCheckState(IFWL_Widget* pWidget,
                                       FWL_HLISTITEM hItem);
    virtual FWL_ERR SetItemCheckState(IFWL_Widget* pWidget,
                                      FWL_HLISTITEM hItem,
                                      FX_DWORD dwCheckState);

    CFX_PtrArray m_arrItem;
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
  FX_DWORD m_dwStates;
  CFX_WideString m_wsText;
  CFX_DIBitmap* m_pDIB;
  void* m_pData;
  FX_DWORD m_dwCheckState;
  CFX_RectF m_rtCheckBox;
};
#endif
