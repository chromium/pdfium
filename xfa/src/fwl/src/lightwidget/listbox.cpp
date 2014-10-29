// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_ListBox* CFWL_ListBox::Create()
{
    return FX_NEW CFWL_ListBox;
}
FWL_ERR	CFWL_ListBox::Initialize(const CFWL_WidgetProperties *pProperties )
{
    _FWL_RETURN_VALUE_IF_FAIL(!m_pImp, FWL_ERR_Indefinite);
    if (pProperties) {
        *m_pProperties = *pProperties;
    }
    CFWL_WidgetImpProperties prop;
    prop.m_dwStyles = m_pProperties->m_dwStyles;
    prop.m_dwStyleExes = m_pProperties->m_dwStyleExes;
    prop.m_dwStates = m_pProperties->m_dwStates;
    prop.m_ctmOnParent = m_pProperties->m_ctmOnParent;
    prop.m_pDataProvider = &m_ListBoxDP;
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    prop.m_rtWidget = m_pProperties->m_rtWidget;
    m_pImp = IFWL_ListBox::Create();
    FWL_ERR ret = ((IFWL_ListBox*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
FWL_ERR CFWL_ListBox::AddDIBitmap(CFX_DIBitmap *pDIB, FWL_HLISTITEM hItem)
{
    ((CFWL_ListItem*)hItem)->m_pDIB = pDIB;
    return FWL_ERR_Succeeded;
}
FWL_HLISTITEM CFWL_ListBox::AddString(FX_WSTR wsAdd, FX_BOOL bSelect)
{
    CFWL_ListItem * pItem = FX_NEW CFWL_ListItem;
    pItem->m_dwStates = 0;
    pItem->m_wsText = wsAdd;
    pItem->m_dwStates = bSelect ? FWL_ITEMSTATE_LTB_Selected : 0;
    m_ListBoxDP.m_arrItem.Add(pItem);
    return (FWL_HLISTITEM)pItem;
}
FX_BOOL CFWL_ListBox::DeleteString(FWL_HLISTITEM hItem)
{
    FX_INT32 nIndex = m_ListBoxDP.GetItemIndex(GetWidget(), hItem);
    if (nIndex < 0 || nIndex >= m_ListBoxDP.m_arrItem.GetSize()) {
        return FALSE;
    }
    CFWL_ListItem *pDelItem = (CFWL_ListItem*)m_ListBoxDP.GetItem((IFWL_ListBox*)this, nIndex);
    FX_INT32 iCount = m_ListBoxDP.CountItems((IFWL_ListBox*)this);
    FX_INT32 iSel = nIndex + 1;
    if (iSel >= iCount) {
        iSel = nIndex - 1;
        if (iSel < 0) {
            iSel =  -1;
        }
    }
    if (iSel >= 0) {
        CFWL_ListItem *pSel = (CFWL_ListItem*)m_ListBoxDP.GetItem((IFWL_ListBox*)this, iSel);
        pSel->m_dwStates |= FWL_ITEMSTATE_LTB_Selected;
    }
    m_ListBoxDP.m_arrItem.RemoveAt(nIndex);
    delete pDelItem;
    return TRUE;
}
FX_BOOL	CFWL_ListBox::DeleteAll()
{
    FX_INT32 iCount = m_ListBoxDP.CountItems((IFWL_ListBox*)this);
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_ListItem *pItem = (CFWL_ListItem*)m_ListBoxDP.GetItem((IFWL_ListBox*)this, i);
        delete pItem;
    }
    m_ListBoxDP.m_arrItem.RemoveAll();
    return TRUE;
}
FX_INT32 CFWL_ListBox::CountSelItems()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ListBox*)m_pImp)->CountSelItems();
}
FWL_HLISTITEM CFWL_ListBox::GetSelItem(FX_INT32 nIndexSel)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, NULL);
    return ((IFWL_ListBox*)m_pImp)->GetSelItem(nIndexSel);
}
FX_INT32 CFWL_ListBox::GetSelIndex(FX_INT32 nIndex)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ListBox*)m_pImp)->GetSelIndex(nIndex);
}
FWL_ERR CFWL_ListBox::SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ListBox*)m_pImp)->SetSelItem(hItem, bSelect);
}
FWL_ERR CFWL_ListBox::GetItemText(FWL_HLISTITEM hItem, CFX_WideString &wsText)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ListBox*)m_pImp)->GetItemText(hItem, wsText);
}
FWL_ERR CFWL_ListBox::GetScrollPos(FX_FLOAT &fPos, FX_BOOL bVert)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ListBox*)m_pImp)->GetScrollPos(fPos, bVert);
}
FWL_ERR CFWL_ListBox::SetItemHeight(FX_FLOAT fItemHeight)
{
    m_ListBoxDP.m_fItemHeight = fItemHeight;
    return FWL_ERR_Succeeded;
}
FWL_HLISTITEM CFWL_ListBox::GetFocusItem()
{
    for (FX_INT32 i = 0; i < m_ListBoxDP.m_arrItem.GetSize(); i++) {
        CFWL_ListItem * hItem = (CFWL_ListItem *)(m_ListBoxDP.m_arrItem[i]);
        if (hItem->m_dwStates & FWL_ITEMSTATE_LTB_Focused) {
            return (FWL_HLISTITEM)hItem;
        }
    }
    return NULL;
}
FWL_ERR	CFWL_ListBox::SetFocusItem(FWL_HLISTITEM hItem)
{
    FX_INT32 nIndex = m_ListBoxDP.GetItemIndex(GetWidget(), hItem);
    ((CFWL_ListItem *)(m_ListBoxDP.m_arrItem[nIndex]))->m_dwStates |= FWL_ITEMSTATE_LTB_Focused;
    return FWL_ERR_Succeeded;
}
FWL_ERR* CFWL_ListBox::Sort(IFWL_ListBoxCompare *pCom)
{
    return ((IFWL_ListBox*)m_pImp)->Sort(pCom);
}
FX_INT32 CFWL_ListBox::CountItems()
{
    return m_ListBoxDP.m_arrItem.GetSize();
}
FWL_HLISTITEM CFWL_ListBox::GetItem(FX_INT32 nIndex)
{
    FX_INT32 nCount = m_ListBoxDP.m_arrItem.GetSize();
    if (nIndex > nCount - 1 && nIndex < 0) {
        return NULL;
    }
    return (FWL_HLISTITEM)m_ListBoxDP.m_arrItem[nIndex];
}
FWL_ERR	CFWL_ListBox::SetItemString(FWL_HLISTITEM hItem, FX_WSTR wsText)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    pItem->m_wsText = wsText;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::GetItemString(FWL_HLISTITEM hItem, CFX_WideString &wsText)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    wsText = pItem->m_wsText;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_ListBox::SetItemData(FWL_HLISTITEM hItem, FX_LPVOID pData)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    pItem->m_pData = pData;
    return FWL_ERR_Succeeded;
}
FX_LPVOID CFWL_ListBox::GetItemData(FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, NULL);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    return pItem->m_pData;
}
FWL_HLISTITEM CFWL_ListBox::GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy)
{
    CFX_RectF rtClient;
    m_pImp->GetClientRect(rtClient);
    fx -= rtClient.left;
    fy -= rtClient.top;
    FX_FLOAT fPosX = 0;
    FX_FLOAT fPosY = 0;
    ((IFWL_ListBox*)m_pImp)->GetScrollPos(fx);
    ((IFWL_ListBox*)m_pImp)->GetScrollPos(fy, FALSE);
    FX_INT32 nCount = m_ListBoxDP.CountItems(NULL);
    for (FX_INT32 i = 0; i < nCount; i ++) {
        FWL_HLISTITEM hItem = m_ListBoxDP.GetItem(NULL, i);
        if (!hItem) {
            continue;
        }
        CFX_RectF rtItem;
        m_ListBoxDP.GetItemRect(NULL, hItem, rtItem);
        rtItem.Offset(-fPosX, -fPosY);
        if (rtItem.Contains(fx, fy)) {
            return hItem;
        }
    }
    return NULL;
}
FX_DWORD CFWL_ListBox::GetItemStates(FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, 0);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    return pItem->m_dwStates | pItem->m_dwCheckState;
}
CFWL_ListBox::CFWL_ListBox()
{
}
CFWL_ListBox::~CFWL_ListBox()
{
}
CFWL_ListBox::CFWL_ListBoxDP::CFWL_ListBoxDP()
{
}
CFWL_ListBox::CFWL_ListBoxDP::~CFWL_ListBoxDP()
{
    FX_INT32 nCount = m_arrItem.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFWL_ListItem *pItem = (CFWL_ListItem*)m_arrItem[i];
        if (pItem != NULL) {
            delete pItem;
        }
    }
    m_arrItem.RemoveAll();
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption)
{
    wsCaption = m_wsData;
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_ListBox::CFWL_ListBoxDP::CountItems(IFWL_Widget *pWidget)
{
    return m_arrItem.GetSize();
}
FWL_HLISTITEM CFWL_ListBox::CFWL_ListBoxDP::GetItem(IFWL_Widget *pWidget, FX_INT32 nIndex)
{
    if (nIndex >= m_arrItem.GetSize() || nIndex < 0) {
        return NULL;
    } else {
        return (FWL_HLISTITEM)m_arrItem[nIndex];
    }
}
FX_INT32 CFWL_ListBox::CFWL_ListBoxDP::GetItemIndex(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    return m_arrItem.Find(hItem);
}
FX_BOOL CFWL_ListBox::CFWL_ListBoxDP::SetItemIndex(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_INT32 nIndex)
{
    return m_arrItem.SetAt(nIndex, hItem);
}
FX_DWORD CFWL_ListBox::CFWL_ListBoxDP::GetItemStyles(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, -1);
    return ((CFWL_ListItem*)hItem)->m_dwStates;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetItemText(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_WideString &wsText)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    wsText = ((CFWL_ListItem*)hItem)->m_wsText;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetItemRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_RectF &rtItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    rtItem = pItem->m_rtItem;
    return FWL_ERR_Succeeded;
}
FX_LPVOID CFWL_ListBox::CFWL_ListBoxDP::GetItemData(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, NULL);
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    return pItem->m_pData;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemStyles(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_DWORD dwStyle)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    ((CFWL_ListItem*)hItem)->m_dwStates = dwStyle;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemText(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_LPCWSTR pszText)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    ((CFWL_ListItem*)hItem)->m_wsText = pszText;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, const CFX_RectF &rtItem)
{
    _FWL_RETURN_VALUE_IF_FAIL(hItem, FWL_ERR_Indefinite);
    ((CFWL_ListItem*)hItem)->m_rtItem = rtItem;
    return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_ListBox::CFWL_ListBoxDP::GetItemHeight(IFWL_Widget *pWidget)
{
    return m_fItemHeight;
}
CFX_DIBitmap*	CFWL_ListBox::CFWL_ListBoxDP::GetItemIcon(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    return ((CFWL_ListItem*)hItem)->m_pDIB;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetItemCheckRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, CFX_RectF& rtCheck)
{
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    rtCheck = pItem->m_rtCheckBox;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemCheckRect(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, const CFX_RectF& rtCheck)
{
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    pItem->m_rtCheckBox = rtCheck;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ListBox::CFWL_ListBoxDP::GetItemCheckState(IFWL_Widget *pWidget, FWL_HLISTITEM hItem)
{
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    return pItem->m_dwCheckState;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemCheckState(IFWL_Widget *pWidget, FWL_HLISTITEM hItem, FX_DWORD dwCheckState)
{
    CFWL_ListItem *pItem = (CFWL_ListItem*)hItem;
    pItem->m_dwCheckState = dwCheckState;
    return FWL_ERR_Succeeded;
}
