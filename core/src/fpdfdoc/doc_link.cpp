// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../src/fxcrt/fx_safe_types.h"

CPDF_LinkList::~CPDF_LinkList()
{
    FX_POSITION pos = m_PageMap.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_PageMap.GetNextAssoc(pos, key, value);
        delete (CFX_PtrArray*)value;
    }
}
CFX_PtrArray* CPDF_LinkList::GetPageLinks(CPDF_Page* pPage)
{
    FX_DWORD objnum = pPage->m_pFormDict->GetObjNum();
    if (objnum == 0) {
        return NULL;
    }
    CFX_PtrArray* pPageLinkList = NULL;
    if (!m_PageMap.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, (FX_LPVOID&)pPageLinkList)) {
        pPageLinkList = FX_NEW CFX_PtrArray;
        if (pPageLinkList == NULL) {
            return NULL;
        }
        m_PageMap.SetAt((FX_LPVOID)(FX_UINTPTR)objnum, pPageLinkList);
        LoadPageLinks(pPage, pPageLinkList);
    }
    return pPageLinkList;
}
int CPDF_LinkList::CountLinks(CPDF_Page* pPage)
{
    CFX_PtrArray* pPageLinkList = GetPageLinks(pPage);
    if (pPageLinkList == NULL) {
        return 0;
    }
    return pPageLinkList->GetSize();
}
CPDF_Link CPDF_LinkList::GetLink(CPDF_Page* pPage, int index)
{
    CFX_PtrArray* pPageLinkList = GetPageLinks(pPage);
    if (pPageLinkList == NULL) {
        return NULL;
    }
    return (CPDF_Dictionary*)pPageLinkList->GetAt(index);
}
CPDF_Link CPDF_LinkList::GetLinkAtPoint(CPDF_Page* pPage, FX_FLOAT pdf_x, FX_FLOAT pdf_y)
{
    CFX_PtrArray* pPageLinkList = GetPageLinks(pPage);
    if (pPageLinkList == NULL) {
        return NULL;
    }
    int size = pPageLinkList->GetSize();
    for (int i = 0; i < size; i ++) {
        CPDF_Link Link = (CPDF_Dictionary*)pPageLinkList->GetAt(i);
        CPDF_Rect rect = Link.GetRect();
        if (rect.Contains(pdf_x, pdf_y)) {
            return Link;
        }
    }
    return NULL;
}
void CPDF_LinkList::LoadPageLinks(CPDF_Page* pPage, CFX_PtrArray* pList)
{
    CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArray("Annots");
    if (pAnnotList == NULL) {
        return;
    }
    for (FX_DWORD i = 0; i < pAnnotList->GetCount(); i ++) {
        CPDF_Dictionary* pAnnot = pAnnotList->GetDict(i);
        if (pAnnot == NULL) {
            continue;
        }
        if (pAnnot->GetString("Subtype") != "Link") {
            continue;
        }
        pList->Add(pAnnot);
    }
}
CPDF_Rect CPDF_Link::GetRect()
{
    return m_pDict->GetRect("Rect");
}
CPDF_Dest CPDF_Link::GetDest(CPDF_Document* pDoc)
{
    CPDF_Object* pDest = m_pDict->GetElementValue("Dest");
    if (pDest == NULL) {
        return NULL;
    }
    if (pDest->GetType() == PDFOBJ_STRING || pDest->GetType() == PDFOBJ_NAME) {
        CPDF_NameTree name_tree(pDoc, FX_BSTRC("Dests"));
        CFX_ByteStringC name = pDest->GetString();
        return name_tree.LookupNamedDest(pDoc, name);
    } else if (pDest->GetType() == PDFOBJ_ARRAY) {
        return (CPDF_Array*)pDest;
    }
    return NULL;
}
CPDF_Action CPDF_Link::GetAction()
{
    return m_pDict->GetDict("A");
}
