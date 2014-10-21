// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fpdfapi/fpdf_module.h"
extern FX_LPVOID PDFPreviewInitCache(CPDF_Document* pDoc);
extern void PDFPreviewClearCache(FX_LPVOID pCache);
CPDF_Document::CPDF_Document(IPDF_DocParser* pParser) : CPDF_IndirectObjects(pParser)
{
    ASSERT(pParser != NULL);
    m_pRootDict = NULL;
    m_pInfoDict = NULL;
    m_bLinearized = FALSE;
    m_dwFirstPageNo = 0;
    m_dwFirstPageObjNum = 0;
    m_pDocPage = CPDF_ModuleMgr::Get()->GetPageModule()->CreateDocData(this);
    m_pDocRender = CPDF_ModuleMgr::Get()->GetRenderModule()->CreateDocData(this);
}
CPDF_DocPageData* CPDF_Document::GetValidatePageData()
{
    if (m_pDocPage) {
        return m_pDocPage;
    }
    m_pDocPage = CPDF_ModuleMgr::Get()->GetPageModule()->CreateDocData(this);
    return m_pDocPage;
}
CPDF_DocRenderData* CPDF_Document::GetValidateRenderData()
{
    if (m_pDocRender) {
        return m_pDocRender;
    }
    m_pDocRender = CPDF_ModuleMgr::Get()->GetRenderModule()->CreateDocData(this);
    return m_pDocRender;
}
void CPDF_Document::LoadDoc()
{
    m_LastObjNum = m_pParser->GetLastObjNum();
    CPDF_Object* pRootObj = GetIndirectObject(m_pParser->GetRootObjNum());
    if (pRootObj == NULL) {
        return;
    }
    m_pRootDict = pRootObj->GetDict();
    if (m_pRootDict == NULL) {
        return;
    }
    CPDF_Object* pInfoObj = GetIndirectObject(m_pParser->GetInfoObjNum());
    if (pInfoObj) {
        m_pInfoDict = pInfoObj->GetDict();
    }
    CPDF_Array* pIDArray = m_pParser->GetIDArray();
    if (pIDArray) {
        m_ID1 = pIDArray->GetString(0);
        m_ID2 = pIDArray->GetString(1);
    }
    m_PageList.SetSize(_GetPageCount());
}
void CPDF_Document::LoadAsynDoc(CPDF_Dictionary *pLinearized)
{
    m_bLinearized = TRUE;
    m_LastObjNum = m_pParser->GetLastObjNum();
    CPDF_Object* indirectObj = GetIndirectObject(m_pParser->GetRootObjNum());
    m_pRootDict = indirectObj ? indirectObj->GetDict() : NULL;
    if (m_pRootDict == NULL) {
        return;
    }
    indirectObj = GetIndirectObject(m_pParser->GetInfoObjNum());
    m_pInfoDict = indirectObj ? indirectObj->GetDict() : NULL;
    CPDF_Array* pIDArray = m_pParser->GetIDArray();
    if (pIDArray) {
        m_ID1 = pIDArray->GetString(0);
        m_ID2 = pIDArray->GetString(1);
    }
    FX_DWORD dwPageCount = 0;
    CPDF_Object *pCount = pLinearized->GetElement(FX_BSTRC("N"));
    if (pCount && pCount->GetType() == PDFOBJ_NUMBER) {
        dwPageCount = pCount->GetInteger();
    }
    m_PageList.SetSize(dwPageCount);
    CPDF_Object *pNo = pLinearized->GetElement(FX_BSTRC("P"));
    if (pNo && pNo->GetType() == PDFOBJ_NUMBER) {
        m_dwFirstPageNo = pNo->GetInteger();
    }
    CPDF_Object *pObjNum = pLinearized->GetElement(FX_BSTRC("O"));
    if (pObjNum && pObjNum->GetType() == PDFOBJ_NUMBER) {
        m_dwFirstPageObjNum = pObjNum->GetInteger();
    }
}
void CPDF_Document::LoadPages()
{
    m_PageList.SetSize(_GetPageCount());
}
extern void FPDF_TTFaceMapper_ReleaseDoc(CPDF_Document*);
CPDF_Document::~CPDF_Document()
{
    if (m_pDocPage) {
        CPDF_ModuleMgr::Get()->GetPageModule()->ReleaseDoc(this);
        CPDF_ModuleMgr::Get()->GetPageModule()->ClearStockFont(this);
    }
    if (m_pDocRender) {
        CPDF_ModuleMgr::Get()->GetRenderModule()->DestroyDocData(m_pDocRender);
    }
}
#define		FX_MAX_PAGE_LEVEL			1024
CPDF_Dictionary* CPDF_Document::_FindPDFPage(CPDF_Dictionary* pPages, int iPage, int nPagesToGo, int level)
{
    CPDF_Array* pKidList = pPages->GetArray(FX_BSTRC("Kids"));
    if (pKidList == NULL) {
        if (nPagesToGo == 0) {
            return pPages;
        }
        return NULL;
    }
    if (level >= FX_MAX_PAGE_LEVEL) {
        return NULL;
    }
    int nKids = pKidList->GetCount();
    for (int i = 0; i < nKids; i ++) {
        CPDF_Dictionary* pKid = pKidList->GetDict(i);
        if (pKid == NULL) {
            nPagesToGo --;
            continue;
        }
        if (pKid == pPages) {
            continue;
        }
        if (!pKid->KeyExist(FX_BSTRC("Kids"))) {
            if (nPagesToGo == 0) {
                return pKid;
            }
            m_PageList.SetAt(iPage - nPagesToGo, pKid->GetObjNum());
            nPagesToGo --;
        } else {
            int nPages = pKid->GetInteger(FX_BSTRC("Count"));
            if (nPagesToGo < nPages) {
                return _FindPDFPage(pKid, iPage, nPagesToGo, level + 1);
            }
            nPagesToGo -= nPages;
        }
    }
    return NULL;
}
CPDF_Dictionary* CPDF_Document::GetPage(int iPage)
{
    if (iPage < 0 || iPage >= m_PageList.GetSize()) {
        return NULL;
    }
    if (m_bLinearized && (iPage == (int)m_dwFirstPageNo)) {
        CPDF_Object* pObj = GetIndirectObject(m_dwFirstPageObjNum);
        if (pObj && pObj->GetType() == PDFOBJ_DICTIONARY) {
            return (CPDF_Dictionary*)pObj;
        }
    }
    int objnum = m_PageList.GetAt(iPage);
    if (objnum) {
        CPDF_Object* pObj = GetIndirectObject(objnum);
        ASSERT(pObj->GetType() == PDFOBJ_DICTIONARY);
        return (CPDF_Dictionary*)pObj;
    }
    CPDF_Dictionary* pRoot = GetRoot();
    if (pRoot == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pPages = pRoot->GetDict(FX_BSTRC("Pages"));
    if (pPages == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pPage = _FindPDFPage(pPages, iPage, iPage, 0);
    if (pPage == NULL) {
        return NULL;
    }
    m_PageList.SetAt(iPage, pPage->GetObjNum());
    return pPage;
}
int CPDF_Document::_FindPageIndex(CPDF_Dictionary* pNode, FX_DWORD& skip_count, FX_DWORD objnum, int& index, int level)
{
    if (pNode->KeyExist(FX_BSTRC("Kids"))) {
        CPDF_Array* pKidList = pNode->GetArray(FX_BSTRC("Kids"));
        if (pKidList == NULL) {
            return -1;
        }
        if (level >= FX_MAX_PAGE_LEVEL) {
            return -1;
        }
        FX_DWORD count = pNode->GetInteger(FX_BSTRC("Count"));
        if (count <= skip_count) {
            skip_count -= count;
            index += count;
            return -1;
        }
        if (count && count == pKidList->GetCount()) {
            for (FX_DWORD i = 0; i < count; i ++) {
                CPDF_Object* pKid = pKidList->GetElement(i);
                if (pKid && pKid->GetType() == PDFOBJ_REFERENCE) {
                    if (((CPDF_Reference*) pKid)->GetRefObjNum() == objnum) {
                        m_PageList.SetAt(index + i, objnum);
                        return index + i;
                    }
                }
            }
        }
        for (FX_DWORD i = 0; i < pKidList->GetCount(); i ++) {
            CPDF_Dictionary* pKid = pKidList->GetDict(i);
            if (pKid == NULL) {
                continue;
            }
            if (pKid == pNode) {
                continue;
            }
            int found_index = _FindPageIndex(pKid, skip_count, objnum, index, level + 1);
            if (found_index >= 0) {
                return found_index;
            }
        }
    } else {
        if (objnum == pNode->GetObjNum()) {
            return index;
        }
        if (skip_count) {
            skip_count--;
        }
        index ++;
    }
    return -1;
}
int CPDF_Document::GetPageIndex(FX_DWORD objnum)
{
    FX_DWORD nPages = m_PageList.GetSize();
    FX_DWORD skip_count = 0;
    FX_BOOL bSkipped = FALSE;
    for (FX_DWORD i = 0; i < nPages; i ++) {
        FX_DWORD objnum1 = m_PageList.GetAt(i);
        if (objnum1 == objnum) {
            return i;
        }
        if (!bSkipped && objnum1 == 0) {
            skip_count = i;
            bSkipped = TRUE;
        }
    }
    CPDF_Dictionary* pRoot = GetRoot();
    if (pRoot == NULL) {
        return -1;
    }
    CPDF_Dictionary* pPages = pRoot->GetDict(FX_BSTRC("Pages"));
    if (pPages == NULL) {
        return -1;
    }
    int index = 0;
    return _FindPageIndex(pPages, skip_count, objnum, index);
}
int CPDF_Document::GetPageCount() const
{
    return m_PageList.GetSize();
}
static int _CountPages(CPDF_Dictionary* pPages, int level)
{
    if (level > 128) {
        return 0;
    }
    int count = pPages->GetInteger(FX_BSTRC("Count"));
    if (count > 0 && count < FPDF_PAGE_MAX_NUM) {
        return count;
    }
    CPDF_Array* pKidList = pPages->GetArray(FX_BSTRC("Kids"));
    if (pKidList == NULL) {
        return 0;
    }
    count = 0;
    for (FX_DWORD i = 0; i < pKidList->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pKidList->GetDict(i);
        if (pKid == NULL) {
            continue;
        }
        if (!pKid->KeyExist(FX_BSTRC("Kids"))) {
            count ++;
        } else {
            count += _CountPages(pKid, level + 1);
        }
    }
    pPages->SetAtInteger(FX_BSTRC("Count"), count);
    return count;
}
int CPDF_Document::_GetPageCount() const
{
    CPDF_Dictionary* pRoot = GetRoot();
    if (pRoot == NULL) {
        return 0;
    }
    CPDF_Dictionary* pPages = pRoot->GetDict(FX_BSTRC("Pages"));
    if (pPages == NULL) {
        return 0;
    }
    if (!pPages->KeyExist(FX_BSTRC("Kids"))) {
        return 1;
    }
    return _CountPages(pPages, 0);
}
static FX_BOOL _EnumPages(CPDF_Dictionary* pPages, IPDF_EnumPageHandler* pHandler)
{
    CPDF_Array* pKidList = pPages->GetArray(FX_BSTRC("Kids"));
    if (pKidList == NULL) {
        return pHandler->EnumPage(pPages);
    }
    for (FX_DWORD i = 0; i < pKidList->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pKidList->GetDict(i);
        if (pKid == NULL) {
            continue;
        }
        if (!pKid->KeyExist(FX_BSTRC("Kids"))) {
            if (!pHandler->EnumPage(pKid)) {
                return FALSE;
            }
        } else {
            return _EnumPages(pKid, pHandler);
        }
    }
    return TRUE;
}
void CPDF_Document::EnumPages(IPDF_EnumPageHandler* pHandler)
{
    CPDF_Dictionary* pRoot = GetRoot();
    if (pRoot == NULL) {
        return;
    }
    CPDF_Dictionary* pPages = pRoot->GetDict(FX_BSTRC("Pages"));
    if (pPages == NULL) {
        return;
    }
    _EnumPages(pPages, pHandler);
}
FX_BOOL CPDF_Document::IsContentUsedElsewhere(FX_DWORD objnum, CPDF_Dictionary* pThisPageDict)
{
    for (int i = 0; i < m_PageList.GetSize(); i ++) {
        CPDF_Dictionary* pPageDict = GetPage(i);
        if (pPageDict == pThisPageDict) {
            continue;
        }
        CPDF_Object* pContents = pPageDict ? pPageDict->GetElement(FX_BSTRC("Contents")) : NULL;
        if (pContents == NULL) {
            continue;
        }
        if (pContents->GetDirectType() == PDFOBJ_ARRAY) {
            CPDF_Array* pArray = (CPDF_Array*)pContents->GetDirect();
            for (FX_DWORD j = 0; j < pArray->GetCount(); j ++) {
                CPDF_Object* pRef = pArray->GetElement(j);
                if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
                    continue;
                }
                if (((CPDF_Reference*) pRef)->GetRefObjNum() == objnum) {
                    return TRUE;
                }
            }
        } else if (pContents->GetObjNum() == objnum) {
            return TRUE;
        }
    }
    return FALSE;
}
FX_DWORD CPDF_Document::GetUserPermissions(FX_BOOL bCheckRevision) const
{
    if (m_pParser == NULL) {
        return (FX_DWORD) - 1;
    }
    return m_pParser->GetPermissions(bCheckRevision);
}
FX_BOOL CPDF_Document::IsOwner() const
{
    if (m_pParser == NULL) {
        return TRUE;
    }
    return m_pParser->IsOwner();
}
FX_BOOL CPDF_Document::IsFormStream(FX_DWORD objnum, FX_BOOL& bForm) const
{
    {
        CPDF_Object* pObj;
        if (m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, (FX_LPVOID&)pObj)) {
            bForm = pObj->GetType() == PDFOBJ_STREAM &&
                    ((CPDF_Stream*)pObj)->GetDict()->GetString(FX_BSTRC("Subtype")) == FX_BSTRC("Form");
            return TRUE;
        }
    }
    if (m_pParser == NULL) {
        bForm = FALSE;
        return TRUE;
    }
    return m_pParser->IsFormStream(objnum, bForm);
}
void CPDF_Document::ClearPageData()
{
    if (m_pDocPage) {
        CPDF_ModuleMgr::Get()->GetPageModule()->ClearDoc(this);
    }
}
void CPDF_Document::ClearRenderData()
{
    if (m_pDocRender) {
        CPDF_ModuleMgr::Get()->GetRenderModule()->ClearDocData(m_pDocRender);
    }
}
