// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_COMBOBOX_LIGHT_H
#define _FWL_COMBOBOX_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_ComboBoxDP;
class CFWL_ComboBox;
class CFWL_ComboBoxDP;
class CFWL_ComboBoxItem;
class CFWL_ComboBox : public CFWL_Widget {
 public:
  static CFWL_ComboBox* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  int32_t AddString(const CFX_WideStringC& wsText);
  int32_t AddString(const CFX_WideStringC& wsText, CFX_DIBitmap* pIcon);
  int32_t RemoveAt(int32_t iIndex);
  int32_t RemoveAll();
  int32_t CountItems();
  FWL_ERR GetTextByIndex(int32_t iIndex, CFX_WideString& wsText);
  int32_t GetCurSel();
  FWL_ERR SetCurSel(int32_t iSel);
  FWL_ERR SetEditText(const CFX_WideStringC& wsText);
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
  FWL_ERR SetMaxListHeight(FX_FLOAT fMaxHeight);
  FWL_ERR SetItemData(int32_t iIndex, void* pData);
  void* GetItemData(int32_t iIndex);
  FWL_ERR SetListTheme(IFWL_ThemeProvider* pTheme);
  FX_BOOL AfterFocusShowDropList();
  FWL_ERR OpenDropDownList(FX_BOOL bActivate);

 public:
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
  CFWL_ComboBox();
  virtual ~CFWL_ComboBox();

 protected:
  class CFWL_ComboBoxDP : public IFWL_ComboBoxDP {
   public:
    CFWL_ComboBoxDP();
    ~CFWL_ComboBoxDP();
    virtual FWL_ERR GetCaption(IFWL_Widget* pWidget,
                               CFX_WideString& wsCaption) {
      return FWL_ERR_Succeeded;
    }

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
    virtual FX_FLOAT GetListHeight(IFWL_Widget* pWidget);

    CFX_PtrArray m_arrItem;
    FX_FLOAT m_fMaxListHeight;
    FX_FLOAT m_fItemHeight;
  };
  CFWL_ComboBoxDP m_comboBoxData;
};
class CFWL_ComboBoxItem {
 public:
  CFWL_ComboBoxItem() {
    m_pDIB = NULL;
    m_pData = NULL;
  }
  CFX_RectF m_rtItem;
  FX_DWORD m_dwStyles;
  CFX_WideString m_wsText;
  CFX_DIBitmap* m_pDIB;
  FX_DWORD m_dwCheckState;
  CFX_RectF m_rtCheckBox;
  void* m_pData;
};
#endif
