// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_ComboBox* CFWL_ComboBox::Create()
{
    return FX_NEW CFWL_ComboBox;
}
FWL_ERR	CFWL_ComboBox::Initialize(const CFWL_WidgetProperties *pProperties )
{
    _FWL_RETURN_VALUE_IF_FAIL(!m_pImp, FWL_ERR_Indefinite);
    if (pProperties) {
        *m_pProperties = *pProperties;
    }
    CFWL_WidgetImpProperties prop;
    prop.m_ctmOnParent = m_pProperties->m_ctmOnParent;
    prop.m_rtWidget = m_pProperties->m_rtWidget;
    prop.m_dwStyles = m_pProperties->m_dwStyles;
    prop.m_dwStyleExes = m_pProperties->m_dwStyleExes;
    prop.m_dwStates = m_pProperties->m_dwStates;
    prop.m_pDataProvider = &m_comboBoxData;
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    m_pImp = IFWL_ComboBox::Create();
    FWL_ERR ret = ((IFWL_ComboBox*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
FX_INT32 CFWL_ComboBox::AddString(FX_WSTR wsText)
{
    CFWL_ComboBoxItem *pItem = FX_NEW CFWL_ComboBoxItem;
    pItem->m_wsText = wsText;
    pItem->m_dwStyles = 0;
    return m_comboBoxData.m_arrItem.Add(pItem);
}
FX_INT32 CFWL_ComboBox::AddString(FX_WSTR wsText, CFX_DIBitmap *pIcon)
{
    CFWL_ComboBoxItem *pItem = FX_NEW CFWL_ComboBoxItem;
    pItem->m_wsText = wsText;
    pItem->m_dwStyles = 0;
    pItem->m_pDIB = pIcon;
    return m_comboBoxData.m_arrItem.Add(pItem);
}
FX_INT32 CFWL_ComboBox::RemoveAt(FX_INT32 iIndex)
{
    return m_comboBoxData.m_arrItem.RemoveAt(iIndex);
}
FX_INT32 CFWL_ComboBox::RemoveAll()
{
    m_comboBoxData.m_arrItem.RemoveAll();
    return 0;
}
FX_INT32 CFWL_ComboBox::CountItems()
{
    return m_comboBoxData.CountItems(GetWidget());
}
FWL_ERR CFWL_ComboBox::GetTextByIndex(FX_INT32 iIndex, CFX_WideString &wsText)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)(m_comboBoxData.GetItem((IFWL_Widget *)this, iIndex));
    _FWL_RETURN_VALUE_IF_FAIL(pItem, FWL_ERR_Indefinite);
    wsText = pItem->m_wsText;
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_ComboBox::GetCurSel()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, -1);
    return ((IFWL_ComboBox*)m_pImp)->GetCurSel();
}
FWL_ERR CFWL_ComboBox::SetCurSel(FX_INT32 iSel)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ComboBox*)m_pImp)->SetCurSel(iSel);
}
FWL_ERR CFWL_ComboBox::SetEditText(FX_WSTR wsText)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ComboBox*)m_pImp)->SetEditText(wsText);
}
FX_INT32 CFWL_ComboBox::GetEditTextLength() const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ComboBox*)m_pImp)->GetEditTextLength();
}
FWL_ERR CFWL_ComboBox::GetEditText(CFX_WideString &wsText, FX_INT32 nStart, FX_INT32 nCount) const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ComboBox*)m_pImp)->GetEditText(wsText, nStart, nCount);
}
FWL_ERR CFWL_ComboBox::SetEditSelRange(FX_INT32 nStart, FX_INT32 nCount)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ComboBox*)m_pImp)->SetEditSelRange(nStart, nCount);
}
FX_INT32 CFWL_ComboBox::GetEditSelRange(FX_INT32 nIndex, FX_INT32 &nStart)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ComboBox*)m_pImp)->GetEditSelRange(nIndex, nStart);
}
FX_INT32 CFWL_ComboBox::GetEditLimit()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ComboBox*)m_pImp)->GetEditLimit();
}
FWL_ERR CFWL_ComboBox::SetEditLimit(FX_INT32 nLimit)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ComboBox*)m_pImp)->SetEditLimit(nLimit);
}
FWL_ERR CFWL_ComboBox::EditDoClipboard(FX_INT32 iCmd)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ComboBox*)m_pImp)->EditDoClipboard(iCmd);
}
FX_BOOL CFWL_ComboBox::EditRedo(FX_BSTR bsRecord)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditRedo(bsRecord);
}
FX_BOOL CFWL_ComboBox::EditUndo(FX_BSTR bsRecord)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditUndo(bsRecord);
}
FWL_ERR CFWL_ComboBox::SetMaxListHeight(FX_FLOAT fMaxHeight)
{
    m_comboBoxData.m_fMaxListHeight = fMaxHeight;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_ComboBox::SetItemData(FX_INT32 iIndex, FX_LPVOID pData)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)(m_comboBoxData.GetItem((IFWL_Widget *)this, iIndex));
    _FWL_RETURN_VALUE_IF_FAIL(pItem, FWL_ERR_Indefinite);
    pItem->m_pData = pData;
    return FWL_ERR_Succeeded;
}
FX_LPVOID CFWL_ComboBox::GetItemData(FX_INT32 iIndex)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)(m_comboBoxData.GetItem((IFWL_Widget *)this, iIndex));
    _FWL_RETURN_VALUE_IF_FAIL(pItem, NULL);
    return pItem->m_pData;
}
FWL_ERR CFWL_ComboBox::SetListTheme(IFWL_ThemeProvider *pTheme)
{
    return ((IFWL_ComboBox*)m_pImp)->GetListBoxt()->SetThemeProvider(pTheme);
}
FX_BOOL CFWL_ComboBox::AfterFocusShowDropList()
{
    return ((IFWL_ComboBox*)m_pImp)->AfterFocusShowDropList();
}
FWL_ERR CFWL_ComboBox::OpenDropDownList(FX_BOOL bActivate)
{
    return ((IFWL_ComboBox*)m_pImp)->OpenDropDownList(bActivate);
}
FX_BOOL	CFWL_ComboBox::EditCanUndo()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCanUndo();
}
FX_BOOL	CFWL_ComboBox::EditCanRedo()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCanRedo();
}
FX_BOOL	CFWL_ComboBox::EditUndo()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditUndo();
}
FX_BOOL	CFWL_ComboBox::EditRedo()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditRedo();
}
FX_BOOL	CFWL_ComboBox::EditCanCopy()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCanCopy();
}
FX_BOOL	CFWL_ComboBox::EditCanCut()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCanCut();
}
FX_BOOL	CFWL_ComboBox::EditCanSelectAll()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCanSelectAll();
}
FX_BOOL	CFWL_ComboBox::EditCopy(CFX_WideString &wsCopy)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCopy(wsCopy);
}
FX_BOOL	CFWL_ComboBox::EditCut(CFX_WideString &wsCut)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditCut(wsCut);
}
FX_BOOL	CFWL_ComboBox::EditPaste(const CFX_WideString &wsPaste)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditPaste(wsPaste);
}
FX_BOOL	CFWL_ComboBox::EditSelectAll()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditSelectAll();
}
FX_BOOL	CFWL_ComboBox::EditDelete()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditDelete();
}
FX_BOOL	CFWL_ComboBox::EditDeSelect()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditDeSelect();
}
FWL_ERR	CFWL_ComboBox::GetBBox(CFX_RectF &rect)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->GetBBox(rect);
}
FWL_ERR 	CFWL_ComboBox::EditModifyStylesEx(FX_DWORD dwStylesExAdded,
        FX_DWORD dwStylesExRemoved)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ComboBox*)m_pImp)->EditModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
CFWL_ComboBox::CFWL_ComboBox()
{
}
CFWL_ComboBox::~CFWL_ComboBox()
{
}
CFWL_ComboBox::CFWL_ComboBoxDP::CFWL_ComboBoxDP()
{
    m_fItemHeight = 0;
    m_fMaxListHeight = 0;
}
CFWL_ComboBox::CFWL_ComboBoxDP::~CFWL_ComboBoxDP()
{
    FX_INT32 nCount = m_arrItem.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)m_arrItem[i];
        if (pItem) {
            delete pItem;
        }
    }
    m_arrItem.RemoveAll();
}
FX_INT32 CFWL_ComboBox::CFWL_ComboBoxDP::CountItems(IFWL_Widget *pWidget)
{
    return m_arrItem.GetSize();
}
FWL_HLISTITEM CFWL_ComboBox::CFWL_ComboBoxDP::GetItem(IFWL_Widget *pWidget, FX_INT32 nIndex)
{
    FX_INT32 iCount = m_arrItem.GetSize();
    if (nIndex >= iCount || nIndex < 0) {
        return NULL;
    }
    return (FWL_HLISTITEM)m_arrItem[nIndex];
}
FX_INT32 CFWL_ComboBox::CFWL_ComboBoxDP::GetItemIndex(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    return m_arrItem.Find(hItem);
}
FX_BOOL CFWL_ComboBox::CFWL_ComboBoxDP::SetItemIndex(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_INT32 nIndex)
{
    return m_arrItem.SetAt(nIndex, hItem);
}
FX_DWORD CFWL_ComboBox::CFWL_ComboBoxDP::GetItemStyles(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, 0);
    return ((CFWL_ComboBoxItem*)hItem)->m_dwStyles;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::GetItemText(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_WideString &wsText)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    wsText = ((CFWL_ComboBoxItem*)hItem)->m_wsText;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::GetItemRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_RectF& rtItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)hItem;
    rtItem.Set(pItem->m_rtItem.left,
               pItem->m_rtItem.top,
               pItem->m_rtItem.width,
               pItem->m_rtItem.height);
    return FWL_ERR_Succeeded;
}
FX_LPVOID CFWL_ComboBox::CFWL_ComboBoxDP::GetItemData(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, NULL);
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)hItem;
    return pItem->m_pData;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemStyles(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_DWORD dwStyle)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    ((CFWL_ComboBoxItem*)hItem)->m_dwStyles = dwStyle;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemText(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_LPCWSTR pszText)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    ((CFWL_ComboBoxItem*)hItem)->m_wsText = pszText;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, const CFX_RectF& rtItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    ((CFWL_ComboBoxItem*)hItem)->m_rtItem = rtItem;
    return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_ComboBox::CFWL_ComboBoxDP::GetItemHeight(IFWL_Widget *pWidget)
{
    return m_fItemHeight;
}
CFX_DIBitmap* CFWL_ComboBox::CFWL_ComboBoxDP::GetItemIcon(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, NULL);
    return ((CFWL_ComboBoxItem*)hItem)->m_pDIB;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::GetItemCheckRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_RectF& rtCheck)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)hItem;
    rtCheck = pItem->m_rtCheckBox;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemCheckRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, const CFX_RectF& rtCheck)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)hItem;
    pItem->m_rtCheckBox = rtCheck;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ComboBox::CFWL_ComboBoxDP::GetItemCheckState(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)hItem;
    return pItem->m_dwCheckState;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemCheckState(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_DWORD dwCheckState)
{
    CFWL_ComboBoxItem *pItem = (CFWL_ComboBoxItem*)hItem;
    pItem->m_dwCheckState = dwCheckState;
    return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_ComboBox::CFWL_ComboBoxDP::GetListHeight(IFWL_Widget *pWidget)
{
    return m_fMaxListHeight;
}
