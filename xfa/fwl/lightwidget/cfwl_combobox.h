// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_COMBOBOX_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_COMBOBOX_H_

#include <memory>
#include <vector>

#include "xfa/fwl/basewidget/ifwl_combobox.h"
#include "xfa/fwl/lightwidget/cfwl_widget.h"

class CFWL_WidgetProperties;
class IFWL_ComboBoxDP;
class CFWL_ComboBoxDP;
class CFWL_ComboBoxItem;

class CFWL_ComboBox : public CFWL_Widget {
 public:
  static CFWL_ComboBox* Create();
  FWL_Error Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  int32_t AddString(const CFX_WideStringC& wsText);
  int32_t AddString(const CFX_WideStringC& wsText, CFX_DIBitmap* pIcon);
  bool RemoveAt(int32_t iIndex);  // Returns false iff |iIndex| out of range.
  void RemoveAll();
  int32_t CountItems();
  FWL_Error GetTextByIndex(int32_t iIndex, CFX_WideString& wsText);
  int32_t GetCurSel();
  FWL_Error SetCurSel(int32_t iSel);
  FWL_Error SetEditText(const CFX_WideString& wsText);
  int32_t GetEditTextLength() const;
  FWL_Error GetEditText(CFX_WideString& wsText,
                        int32_t nStart = 0,
                        int32_t nCount = -1) const;
  FWL_Error SetEditSelRange(int32_t nStart, int32_t nCount = -1);
  int32_t GetEditSelRange(int32_t nIndex, int32_t& nStart);
  int32_t GetEditLimit();
  FWL_Error SetEditLimit(int32_t nLimit);
  FWL_Error EditDoClipboard(int32_t iCmd);
  FX_BOOL EditRedo(const CFX_ByteStringC& bsRecord);
  FX_BOOL EditUndo(const CFX_ByteStringC& bsRecord);
  FWL_Error SetMaxListHeight(FX_FLOAT fMaxHeight);
  FWL_Error SetItemData(int32_t iIndex, void* pData);
  void* GetItemData(int32_t iIndex);
  FWL_Error SetListTheme(IFWL_ThemeProvider* pTheme);
  FX_BOOL AfterFocusShowDropList();
  FWL_Error OpenDropDownList(FX_BOOL bActivate);

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
  FWL_Error GetBBox(CFX_RectF& rect);
  FWL_Error EditModifyStylesEx(uint32_t dwStylesExAdded,
                               uint32_t dwStylesExRemoved);
  CFWL_ComboBox();
  virtual ~CFWL_ComboBox();

 protected:
  class CFWL_ComboBoxDP : public IFWL_ComboBoxDP {
   public:
    CFWL_ComboBoxDP();
    ~CFWL_ComboBoxDP();
    virtual FWL_Error GetCaption(IFWL_Widget* pWidget,
                                 CFX_WideString& wsCaption) {
      return FWL_Error::Succeeded;
    }

    virtual int32_t CountItems(IFWL_Widget* pWidget);
    virtual FWL_HLISTITEM GetItem(IFWL_Widget* pWidget, int32_t nIndex);
    virtual int32_t GetItemIndex(IFWL_Widget* pWidget, FWL_HLISTITEM hItem);
    virtual FX_BOOL SetItemIndex(IFWL_Widget* pWidget,
                                 FWL_HLISTITEM hItem,
                                 int32_t nIndex);

    virtual uint32_t GetItemStyles(IFWL_Widget* pWidget, FWL_HLISTITEM hItem);
    virtual FWL_Error GetItemText(IFWL_Widget* pWidget,
                                  FWL_HLISTITEM hItem,
                                  CFX_WideString& wsText);
    virtual FWL_Error GetItemRect(IFWL_Widget* pWidget,
                                  FWL_HLISTITEM hItem,
                                  CFX_RectF& rtItem);
    virtual void* GetItemData(IFWL_Widget* pWidget, FWL_HLISTITEM hItem);
    virtual FWL_Error SetItemStyles(IFWL_Widget* pWidget,
                                    FWL_HLISTITEM hItem,
                                    uint32_t dwStyle);
    virtual FWL_Error SetItemText(IFWL_Widget* pWidget,
                                  FWL_HLISTITEM hItem,
                                  const FX_WCHAR* pszText);
    virtual FWL_Error SetItemRect(IFWL_Widget* pWidget,
                                  FWL_HLISTITEM hItem,
                                  const CFX_RectF& rtItem);
    virtual FX_FLOAT GetItemHeight(IFWL_Widget* pWidget);
    virtual CFX_DIBitmap* GetItemIcon(IFWL_Widget* pWidget,
                                      FWL_HLISTITEM hItem);
    virtual FWL_Error GetItemCheckRect(IFWL_Widget* pWidget,
                                       FWL_HLISTITEM hItem,
                                       CFX_RectF& rtCheck);
    virtual FWL_Error SetItemCheckRect(IFWL_Widget* pWidget,
                                       FWL_HLISTITEM hItem,
                                       const CFX_RectF& rtCheck);
    virtual uint32_t GetItemCheckState(IFWL_Widget* pWidget,
                                       FWL_HLISTITEM hItem);
    virtual FWL_Error SetItemCheckState(IFWL_Widget* pWidget,
                                        FWL_HLISTITEM hItem,
                                        uint32_t dwCheckState);
    virtual FX_FLOAT GetListHeight(IFWL_Widget* pWidget);

    std::vector<std::unique_ptr<CFWL_ComboBoxItem>> m_ItemArray;
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
  uint32_t m_dwStyles;
  CFX_WideString m_wsText;
  CFX_DIBitmap* m_pDIB;
  uint32_t m_dwCheckState;
  CFX_RectF m_rtCheckBox;
  void* m_pData;
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_COMBOBOX_H_
