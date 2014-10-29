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
class CFWL_ComboBox : public CFWL_Widget
{
public:
    static CFWL_ComboBox* Create();
    FWL_ERR		Initialize(const CFWL_WidgetProperties *pProperties = NULL);
    FX_INT32	AddString(FX_WSTR wsText);
    FX_INT32	AddString(FX_WSTR wsText, CFX_DIBitmap *pIcon);
    FX_INT32	RemoveAt(FX_INT32 iIndex);
    FX_INT32    RemoveAll();
    FX_INT32	CountItems();
    FWL_ERR		GetTextByIndex(FX_INT32 iIndex, CFX_WideString &wsText);
    FX_INT32	GetCurSel();
    FWL_ERR		SetCurSel(FX_INT32 iSel);
    FWL_ERR		SetEditText(FX_WSTR wsText);
    FX_INT32	GetEditTextLength() const;
    FWL_ERR		GetEditText(CFX_WideString &wsText, FX_INT32 nStart = 0, FX_INT32 nCount = -1) const ;
    FWL_ERR		SetEditSelRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    FX_INT32	GetEditSelRange(FX_INT32 nIndex, FX_INT32 &nStart);
    FX_INT32	GetEditLimit();
    FWL_ERR		SetEditLimit(FX_INT32 nLimit);
    FWL_ERR		EditDoClipboard(FX_INT32 iCmd);
    FX_BOOL		EditRedo(FX_BSTR bsRecord);
    FX_BOOL		EditUndo(FX_BSTR bsRecord);
    FWL_ERR		SetMaxListHeight(FX_FLOAT fMaxHeight);
    FWL_ERR		SetItemData(FX_INT32 iIndex, FX_LPVOID pData);
    FX_LPVOID	GetItemData(FX_INT32 iIndex);
    FWL_ERR		SetListTheme(IFWL_ThemeProvider *pTheme);
    FX_BOOL		AfterFocusShowDropList();
    FWL_ERR		OpenDropDownList(FX_BOOL bActivate);
public:
    FX_BOOL		EditCanUndo();
    FX_BOOL		EditCanRedo();
    FX_BOOL		EditUndo();
    FX_BOOL		EditRedo();
    FX_BOOL		EditCanCopy();
    FX_BOOL		EditCanCut();
    FX_BOOL		EditCanSelectAll();
    FX_BOOL		EditCopy(CFX_WideString &wsCopy);
    FX_BOOL		EditCut(CFX_WideString &wsCut);
    FX_BOOL		EditPaste(const CFX_WideString &wsPaste);
    FX_BOOL		EditSelectAll();
    FX_BOOL		EditDelete();
    FX_BOOL		EditDeSelect();
    FWL_ERR		GetBBox(CFX_RectF &rect);
    FWL_ERR		EditModifyStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved);
    CFWL_ComboBox();
    virtual ~CFWL_ComboBox();
protected:
    class CFWL_ComboBoxDP : public IFWL_ComboBoxDP
    {
    public:
        CFWL_ComboBoxDP();
        ~CFWL_ComboBoxDP();
        virtual FWL_ERR			GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption)
        {
            return FWL_ERR_Succeeded;
        }

        virtual	FX_INT32		CountItems(IFWL_Widget *pWidget);
        virtual	FWL_HLISTITEM	GetItem(IFWL_Widget *pWidget, FX_INT32 nIndex);
        virtual	FX_INT32		GetItemIndex(IFWL_Widget *pWidget, FWL_HLISTITEM hItem);
        virtual FX_BOOL			SetItemIndex(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_INT32 nIndex);

        virtual	FX_DWORD		GetItemStyles(IFWL_Widget *pWidget, FWL_HLISTITEM hItem);
        virtual	FWL_ERR			GetItemText(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_WideString &wsText);
        virtual FWL_ERR			GetItemRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_RectF& rtItem);
        virtual FX_LPVOID		GetItemData(IFWL_Widget *pWidget, FWL_HLISTITEM hItem);
        virtual	FWL_ERR			SetItemStyles(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_DWORD dwStyle);
        virtual FWL_ERR			SetItemText(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_LPCWSTR pszText);
        virtual FWL_ERR			SetItemRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, const CFX_RectF& rtItem);
        virtual FX_FLOAT		GetItemHeight(IFWL_Widget *pWidget);
        virtual CFX_DIBitmap*	GetItemIcon(IFWL_Widget *pWidget, FWL_HLISTITEM hItem);
        virtual FWL_ERR			GetItemCheckRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_RectF& rtCheck);
        virtual FWL_ERR			SetItemCheckRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, const CFX_RectF& rtCheck);
        virtual	FX_DWORD		GetItemCheckState(IFWL_Widget *pWidget, FWL_HLISTITEM hItem);
        virtual	FWL_ERR			SetItemCheckState(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_DWORD dwCheckState);
        virtual FX_FLOAT		GetListHeight(IFWL_Widget *pWidget);

        CFX_PtrArray	m_arrItem;
        FX_FLOAT		m_fMaxListHeight;
        FX_FLOAT		m_fItemHeight;
    };
    CFWL_ComboBoxDP m_comboBoxData;
};
class CFWL_ComboBoxItem : public CFX_Object
{
public:
    CFWL_ComboBoxItem()
    {
        m_pDIB = NULL;
        m_pData = NULL;
    }
    CFX_RectF		m_rtItem;
    FX_DWORD		m_dwStyles;
    CFX_WideString	m_wsText;
    CFX_DIBitmap	*m_pDIB;
    FX_DWORD		m_dwCheckState;
    CFX_RectF		m_rtCheckBox;
    FX_LPVOID		m_pData;
};
#endif
