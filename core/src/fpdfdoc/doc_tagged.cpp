// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfapi/fpdf_parser.h"
#include "../../include/fpdfapi/fpdf_page.h"
#include "../../include/fpdfdoc/fpdf_tagged.h"
#include "tagged_int.h"
const int nMaxRecursion = 32;
static FX_BOOL IsTagged(const CPDF_Document* pDoc)
{
    CPDF_Dictionary* pCatalog = pDoc->GetRoot();
    CPDF_Dictionary* pMarkInfo = pCatalog->GetDict(FX_BSTRC("MarkInfo"));
    return pMarkInfo != NULL && pMarkInfo->GetInteger(FX_BSTRC("Marked"));
}
CPDF_StructTree* CPDF_StructTree::LoadPage(const CPDF_Document* pDoc, const CPDF_Dictionary* pPageDict)
{
    if (!IsTagged(pDoc)) {
        return NULL;
    }
    CPDF_StructTreeImpl* pTree = FX_NEW CPDF_StructTreeImpl(pDoc);
    if (pTree == NULL) {
        return NULL;
    }
    pTree->LoadPageTree(pPageDict);
    return pTree;
}
CPDF_StructTree* CPDF_StructTree::LoadDoc(const CPDF_Document* pDoc)
{
    if (!IsTagged(pDoc)) {
        return NULL;
    }
    CPDF_StructTreeImpl* pTree = FX_NEW CPDF_StructTreeImpl(pDoc);
    if (pTree == NULL) {
        return NULL;
    }
    pTree->LoadDocTree();
    return pTree;
}
CPDF_StructTreeImpl::CPDF_StructTreeImpl(const CPDF_Document* pDoc)
{
    CPDF_Dictionary* pCatalog = pDoc->GetRoot();
    m_pTreeRoot = pCatalog->GetDict(FX_BSTRC("StructTreeRoot"));
    if (m_pTreeRoot == NULL) {
        return;
    }
    m_pRoleMap = m_pTreeRoot->GetDict(FX_BSTRC("RoleMap"));
}
CPDF_StructTreeImpl::~CPDF_StructTreeImpl()
{
    for (int i = 0; i < m_Kids.GetSize(); i ++)
        if (m_Kids[i]) {
            m_Kids[i]->Release();
        }
}
void CPDF_StructTreeImpl::LoadDocTree()
{
    m_pPage = NULL;
    if (m_pTreeRoot == NULL) {
        return;
    }
    CPDF_Object* pKids = m_pTreeRoot->GetElementValue(FX_BSTRC("K"));
    if (pKids == NULL) {
        return;
    }
    if (pKids->GetType() == PDFOBJ_DICTIONARY) {
        CPDF_StructElementImpl* pStructElementImpl = FX_NEW CPDF_StructElementImpl(this, NULL, (CPDF_Dictionary*)pKids);
        if (pStructElementImpl == NULL) {
            return;
        }
        m_Kids.Add(pStructElementImpl);
        return;
    }
    if (pKids->GetType() != PDFOBJ_ARRAY) {
        return;
    }
    CPDF_Array* pArray = (CPDF_Array*)pKids;
    for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pArray->GetDict(i);
        CPDF_StructElementImpl* pStructElementImpl = FX_NEW CPDF_StructElementImpl(this, NULL, pKid);
        if (pStructElementImpl == NULL) {
            return;
        }
        m_Kids.Add(pStructElementImpl);
    }
}
void CPDF_StructTreeImpl::LoadPageTree(const CPDF_Dictionary* pPageDict)
{
    m_pPage = pPageDict;
    if (m_pTreeRoot == NULL) {
        return;
    }
    CPDF_Object* pKids = m_pTreeRoot->GetElementValue(FX_BSTRC("K"));
    if (pKids == NULL) {
        return;
    }
    FX_DWORD dwKids = 0;
    if (pKids->GetType() == PDFOBJ_DICTIONARY) {
        dwKids = 1;
    } else if (pKids->GetType() == PDFOBJ_ARRAY) {
        dwKids = ((CPDF_Array*)pKids)->GetCount();
    } else {
        return;
    }
    FX_DWORD i;
    m_Kids.SetSize(dwKids);
    for (i = 0; i < dwKids; i ++) {
        m_Kids[i] = NULL;
    }
    CFX_MapPtrToPtr element_map;
    CPDF_Dictionary* pParentTree = m_pTreeRoot->GetDict(FX_BSTRC("ParentTree"));
    if (pParentTree == NULL) {
        return;
    }
    CPDF_NumberTree parent_tree(pParentTree);
    int parents_id = pPageDict->GetInteger(FX_BSTRC("StructParents"), -1);
    if (parents_id >= 0) {
        CPDF_Object* pParents = parent_tree.LookupValue(parents_id);
        if (pParents == NULL || pParents->GetType() != PDFOBJ_ARRAY) {
            return;
        }
        CPDF_Array* pParentArray = (CPDF_Array*)pParents;
        for (i = 0; i < pParentArray->GetCount(); i ++) {
            CPDF_Dictionary* pParent = pParentArray->GetDict(i);
            if (pParent == NULL) {
                continue;
            }
            AddPageNode(pParent, element_map);
        }
    }
}
CPDF_StructElementImpl* CPDF_StructTreeImpl::AddPageNode(CPDF_Dictionary* pDict, CFX_MapPtrToPtr& map, int nLevel)
{
    if (nLevel > nMaxRecursion) {
        return NULL;
    }
    CPDF_StructElementImpl* pElement = NULL;
    if (map.Lookup(pDict, (FX_LPVOID&)pElement)) {
        return pElement;
    }
    pElement = FX_NEW CPDF_StructElementImpl(this, NULL, pDict);
    if (pElement == NULL) {
        return NULL;
    }
    map.SetAt(pDict, pElement);
    CPDF_Dictionary* pParent = pDict->GetDict(FX_BSTRC("P"));
    if (pParent == NULL || pParent->GetString(FX_BSTRC("Type")) == FX_BSTRC("StructTreeRoot")) {
        if (!AddTopLevelNode(pDict, pElement)) {
            pElement->Release();
            map.RemoveKey(pDict);
        }
    } else {
        CPDF_StructElementImpl* pParentElement = AddPageNode(pParent, map, nLevel + 1);
        FX_BOOL bSave = FALSE;
        for (int i = 0; i < pParentElement->m_Kids.GetSize(); i ++) {
            if (pParentElement->m_Kids[i].m_Type != CPDF_StructKid::Element) {
                continue;
            }
            if (pParentElement->m_Kids[i].m_Element.m_pDict != pDict) {
                continue;
            }
            pParentElement->m_Kids[i].m_Element.m_pElement = pElement->Retain();
            bSave = TRUE;
        }
        if (!bSave) {
            pElement->Release();
            map.RemoveKey(pDict);
        }
    }
    return pElement;
}
FX_BOOL CPDF_StructTreeImpl::AddTopLevelNode(CPDF_Dictionary* pDict, CPDF_StructElementImpl* pElement)
{
    CPDF_Object *pObj = m_pTreeRoot->GetElementValue(FX_BSTRC("K"));
    if (!pObj) {
        return FALSE;
    }
    if (pObj->GetType() == PDFOBJ_DICTIONARY) {
        if (pObj->GetObjNum() == pDict->GetObjNum()) {
            if (m_Kids[0]) {
                m_Kids[0]->Release();
            }
            m_Kids[0] = pElement->Retain();
        } else {
            return FALSE;
        }
    }
    if (pObj->GetType() == PDFOBJ_ARRAY) {
        CPDF_Array* pTopKids = (CPDF_Array*)pObj;
        FX_DWORD i;
        FX_BOOL bSave = FALSE;
        for (i = 0; i < pTopKids->GetCount(); i ++) {
            CPDF_Object* pKidRef = pTopKids->GetElement(i);
            if (pKidRef == NULL || pKidRef->GetType() != PDFOBJ_REFERENCE) {
                continue;
            }
            if (((CPDF_Reference*) pKidRef)->GetRefObjNum() != pDict->GetObjNum()) {
                continue;
            }
            if (m_Kids[i]) {
                m_Kids[i]->Release();
            }
            m_Kids[i] = pElement->Retain();
            bSave = TRUE;
        }
        if (!bSave) {
            return FALSE;
        }
    }
    return TRUE;
}
CPDF_StructElementImpl::CPDF_StructElementImpl(CPDF_StructTreeImpl* pTree, CPDF_StructElementImpl* pParent, CPDF_Dictionary* pDict)
    : m_RefCount(0)
{
    m_pTree = pTree;
    m_pDict = pDict;
    m_Type = pDict->GetString(FX_BSTRC("S"));
    if (pTree->m_pRoleMap) {
        CFX_ByteString mapped = pTree->m_pRoleMap->GetString(m_Type);
        if (!mapped.IsEmpty()) {
            m_Type = mapped;
        }
    }
    m_pParent = pParent;
    LoadKids(pDict);
}
CPDF_StructElementImpl::~CPDF_StructElementImpl()
{
    for (int i = 0; i < m_Kids.GetSize(); i ++) {
        if (m_Kids[i].m_Type == CPDF_StructKid::Element && m_Kids[i].m_Element.m_pElement) {
            ((CPDF_StructElementImpl*)m_Kids[i].m_Element.m_pElement)->Release();
        }
    }
}
CPDF_StructElementImpl* CPDF_StructElementImpl::Retain()
{
    m_RefCount++;
    return this;
}
void CPDF_StructElementImpl::Release()
{
    if(--m_RefCount < 1) {
        delete this;
    }
}
void CPDF_StructElementImpl::LoadKids(CPDF_Dictionary* pDict)
{
    CPDF_Object* pObj = pDict->GetElement(FX_BSTRC("Pg"));
    FX_DWORD PageObjNum = 0;
    if (pObj && pObj->GetType() == PDFOBJ_REFERENCE) {
        PageObjNum = ((CPDF_Reference*)pObj)->GetRefObjNum();
    }
    CPDF_Object* pKids = pDict->GetElementValue(FX_BSTRC("K"));
    if (pKids == NULL) {
        return;
    }
    if (pKids->GetType() == PDFOBJ_ARRAY) {
        CPDF_Array* pArray = (CPDF_Array*)pKids;
        m_Kids.SetSize(pArray->GetCount());
        for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
            CPDF_Object* pKid = pArray->GetElementValue(i);
            LoadKid(PageObjNum, pKid, &m_Kids[i]);
        }
    } else {
        m_Kids.SetSize(1);
        LoadKid(PageObjNum, pKids, &m_Kids[0]);
    }
}
void CPDF_StructElementImpl::LoadKid(FX_DWORD PageObjNum, CPDF_Object* pKidObj, CPDF_StructKid* pKid)
{
    pKid->m_Type = CPDF_StructKid::Invalid;
    if (pKidObj == NULL) {
        return;
    }
    if (pKidObj->GetType() == PDFOBJ_NUMBER) {
        if (m_pTree->m_pPage && m_pTree->m_pPage->GetObjNum() != PageObjNum) {
            return;
        }
        pKid->m_Type = CPDF_StructKid::PageContent;
        pKid->m_PageContent.m_ContentId = pKidObj->GetInteger();
        pKid->m_PageContent.m_PageObjNum = PageObjNum;
        return;
    }
    if (pKidObj->GetType() != PDFOBJ_DICTIONARY) {
        return;
    }
    CPDF_Dictionary* pKidDict = (CPDF_Dictionary*)pKidObj;
    CPDF_Object* pPageObj = pKidDict->GetElement(FX_BSTRC("Pg"));
    if (pPageObj && pPageObj->GetType() == PDFOBJ_REFERENCE) {
        PageObjNum = ((CPDF_Reference*)pPageObj)->GetRefObjNum();
    }
    CFX_ByteString type = pKidDict->GetString(FX_BSTRC("Type"));
    if (type == FX_BSTRC("MCR")) {
        if (m_pTree->m_pPage && m_pTree->m_pPage->GetObjNum() != PageObjNum) {
            return;
        }
        pKid->m_Type = CPDF_StructKid::StreamContent;
        CPDF_Object* pStreamObj = pKidDict->GetElement(FX_BSTRC("Stm"));
        if (pStreamObj && pStreamObj->GetType() == PDFOBJ_REFERENCE) {
            pKid->m_StreamContent.m_RefObjNum = ((CPDF_Reference*)pStreamObj)->GetRefObjNum();
        } else {
            pKid->m_StreamContent.m_RefObjNum = 0;
        }
        pKid->m_StreamContent.m_PageObjNum = PageObjNum;
        pKid->m_StreamContent.m_ContentId = pKidDict->GetInteger(FX_BSTRC("MCID"));
    } else if (type == FX_BSTRC("OBJR")) {
        if (m_pTree->m_pPage && m_pTree->m_pPage->GetObjNum() != PageObjNum) {
            return;
        }
        pKid->m_Type = CPDF_StructKid::Object;
        CPDF_Object* pObj = pKidDict->GetElement(FX_BSTRC("Obj"));
        if (pObj && pObj->GetType() == PDFOBJ_REFERENCE) {
            pKid->m_Object.m_RefObjNum = ((CPDF_Reference*)pObj)->GetRefObjNum();
        } else {
            pKid->m_Object.m_RefObjNum = 0;
        }
        pKid->m_Object.m_PageObjNum = PageObjNum;
    } else {
        pKid->m_Type = CPDF_StructKid::Element;
        pKid->m_Element.m_pDict = pKidDict;
        if (m_pTree->m_pPage == NULL) {
            pKid->m_Element.m_pElement = FX_NEW CPDF_StructElementImpl(m_pTree, this, pKidDict);
        } else {
            pKid->m_Element.m_pElement = NULL;
        }
    }
}
static CPDF_Dictionary* FindAttrDict(CPDF_Object* pAttrs, FX_BSTR owner, FX_FLOAT nLevel = 0.0F)
{
    if (nLevel > nMaxRecursion) {
        return NULL;
    }
    if (pAttrs == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pDict = NULL;
    if (pAttrs->GetType() == PDFOBJ_DICTIONARY) {
        pDict = (CPDF_Dictionary*)pAttrs;
    } else if (pAttrs->GetType() == PDFOBJ_STREAM) {
        pDict = ((CPDF_Stream*)pAttrs)->GetDict();
    } else if (pAttrs->GetType() == PDFOBJ_ARRAY) {
        CPDF_Array* pArray = (CPDF_Array*)pAttrs;
        for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
            CPDF_Object* pElement = pArray->GetElementValue(i);
            pDict = FindAttrDict(pElement, owner, nLevel + 1);
            if (pDict) {
                return pDict;
            }
        }
    }
    if (pDict && pDict->GetString(FX_BSTRC("O")) == owner) {
        return pDict;
    }
    return NULL;
}
CPDF_Object* CPDF_StructElementImpl::GetAttr(FX_BSTR owner, FX_BSTR name, FX_BOOL bInheritable, FX_FLOAT fLevel)
{
    if (fLevel > nMaxRecursion) {
        return NULL;
    }
    if (bInheritable) {
        CPDF_Object* pAttr = GetAttr(owner, name, FALSE);
        if (pAttr) {
            return pAttr;
        }
        if (m_pParent == NULL) {
            return NULL;
        }
        return m_pParent->GetAttr(owner, name, TRUE, fLevel + 1);
    }
    CPDF_Object* pA = m_pDict->GetElementValue(FX_BSTRC("A"));
    if (pA) {
        CPDF_Dictionary* pAttrDict = FindAttrDict(pA, owner);
        if (pAttrDict) {
            CPDF_Object* pAttr = pAttrDict->GetElementValue(name);
            if (pAttr) {
                return pAttr;
            }
        }
    }
    CPDF_Object* pC = m_pDict->GetElementValue(FX_BSTRC("C"));
    if (pC == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pClassMap = m_pTree->m_pTreeRoot->GetDict(FX_BSTRC("ClassMap"));
    if (pClassMap == NULL) {
        return NULL;
    }
    if (pC->GetType() == PDFOBJ_ARRAY) {
        CPDF_Array* pArray = (CPDF_Array*)pC;
        for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
            CFX_ByteString class_name = pArray->GetString(i);
            CPDF_Dictionary* pClassDict = pClassMap->GetDict(class_name);
            if (pClassDict && pClassDict->GetString(FX_BSTRC("O")) == owner) {
                return pClassDict->GetElementValue(name);
            }
        }
        return NULL;
    }
    CFX_ByteString class_name = pC->GetString();
    CPDF_Dictionary* pClassDict = pClassMap->GetDict(class_name);
    if (pClassDict && pClassDict->GetString(FX_BSTRC("O")) == owner) {
        return pClassDict->GetElementValue(name);
    }
    return NULL;
}
CPDF_Object* CPDF_StructElementImpl::GetAttr(FX_BSTR owner, FX_BSTR name, FX_BOOL bInheritable, int subindex)
{
    CPDF_Object* pAttr = GetAttr(owner, name, bInheritable);
    if (pAttr == NULL || subindex == -1 || pAttr->GetType() != PDFOBJ_ARRAY) {
        return pAttr;
    }
    CPDF_Array* pArray = (CPDF_Array*)pAttr;
    if (subindex >= (int)pArray->GetCount()) {
        return pAttr;
    }
    return pArray->GetElementValue(subindex);
}
CFX_ByteString CPDF_StructElementImpl::GetName(FX_BSTR owner, FX_BSTR name, FX_BSTR default_value, FX_BOOL bInheritable, int subindex)
{
    CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
    if (pAttr == NULL || pAttr->GetType() != PDFOBJ_NAME) {
        return default_value;
    }
    return pAttr->GetString();
}
FX_ARGB	CPDF_StructElementImpl::GetColor(FX_BSTR owner, FX_BSTR name, FX_ARGB default_value, FX_BOOL bInheritable, int subindex)
{
    CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
    if (pAttr == NULL || pAttr->GetType() != PDFOBJ_ARRAY) {
        return default_value;
    }
    CPDF_Array* pArray = (CPDF_Array*)pAttr;
    return 0xff000000 | ((int)(pArray->GetNumber(0) * 255) << 16) | ((int)(pArray->GetNumber(1) * 255) << 8) | (int)(pArray->GetNumber(2) * 255);
}
FX_FLOAT CPDF_StructElementImpl::GetNumber(FX_BSTR owner, FX_BSTR name, FX_FLOAT default_value, FX_BOOL bInheritable, int subindex)
{
    CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
    if (pAttr == NULL || pAttr->GetType() != PDFOBJ_NUMBER) {
        return default_value;
    }
    return pAttr->GetNumber();
}
int	CPDF_StructElementImpl::GetInteger(FX_BSTR owner, FX_BSTR name, int default_value, FX_BOOL bInheritable, int subindex)
{
    CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
    if (pAttr == NULL || pAttr->GetType() != PDFOBJ_NUMBER) {
        return default_value;
    }
    return pAttr->GetInteger();
}
