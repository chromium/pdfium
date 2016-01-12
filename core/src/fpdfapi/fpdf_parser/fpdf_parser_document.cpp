// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_parser.h"

#include "core/include/fpdfapi/fpdf_module.h"

CPDF_Document::CPDF_Document(CPDF_Parser* pParser)
    : CPDF_IndirectObjectHolder(pParser) {
  ASSERT(pParser);
  m_pRootDict = NULL;
  m_pInfoDict = NULL;
  m_bLinearized = FALSE;
  m_dwFirstPageNo = 0;
  m_dwFirstPageObjNum = 0;
  m_pDocPage = CPDF_ModuleMgr::Get()->GetPageModule()->CreateDocData(this);
  m_pDocRender = CPDF_ModuleMgr::Get()->GetRenderModule()->CreateDocData(this);
}
CPDF_DocPageData* CPDF_Document::GetValidatePageData() {
  if (m_pDocPage) {
    return m_pDocPage;
  }
  m_pDocPage = CPDF_ModuleMgr::Get()->GetPageModule()->CreateDocData(this);
  return m_pDocPage;
}
CPDF_DocRenderData* CPDF_Document::GetValidateRenderData() {
  if (m_pDocRender) {
    return m_pDocRender;
  }
  m_pDocRender = CPDF_ModuleMgr::Get()->GetRenderModule()->CreateDocData(this);
  return m_pDocRender;
}
void CPDF_Document::LoadDoc() {
  m_LastObjNum = m_pParser->GetLastObjNum();
  CPDF_Object* pRootObj =
      GetIndirectObject(m_pParser->GetRootObjNum(), nullptr);
  if (!pRootObj) {
    return;
  }
  m_pRootDict = pRootObj->GetDict();
  if (!m_pRootDict) {
    return;
  }
  CPDF_Object* pInfoObj =
      GetIndirectObject(m_pParser->GetInfoObjNum(), nullptr);
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
void CPDF_Document::LoadAsynDoc(CPDF_Dictionary* pLinearized) {
  m_bLinearized = TRUE;
  m_LastObjNum = m_pParser->GetLastObjNum();
  CPDF_Object* pIndirectObj =
      GetIndirectObject(m_pParser->GetRootObjNum(), nullptr);
  m_pRootDict = pIndirectObj ? pIndirectObj->GetDict() : nullptr;
  if (!m_pRootDict) {
    return;
  }
  pIndirectObj = GetIndirectObject(m_pParser->GetInfoObjNum(), nullptr);
  m_pInfoDict = pIndirectObj ? pIndirectObj->GetDict() : nullptr;
  CPDF_Array* pIDArray = m_pParser->GetIDArray();
  if (pIDArray) {
    m_ID1 = pIDArray->GetString(0);
    m_ID2 = pIDArray->GetString(1);
  }
  FX_DWORD dwPageCount = 0;
  CPDF_Object* pCount = pLinearized->GetElement("N");
  if (ToNumber(pCount))
    dwPageCount = pCount->GetInteger();

  m_PageList.SetSize(dwPageCount);
  CPDF_Object* pNo = pLinearized->GetElement("P");
  if (ToNumber(pNo))
    m_dwFirstPageNo = pNo->GetInteger();

  CPDF_Object* pObjNum = pLinearized->GetElement("O");
  if (ToNumber(pObjNum))
    m_dwFirstPageObjNum = pObjNum->GetInteger();
}
void CPDF_Document::LoadPages() {
  m_PageList.SetSize(_GetPageCount());
}
CPDF_Document::~CPDF_Document() {
  if (m_pDocPage) {
    CPDF_ModuleMgr::Get()->GetPageModule()->ReleaseDoc(this);
    CPDF_ModuleMgr::Get()->GetPageModule()->ClearStockFont(this);
  }
  if (m_pDocRender) {
    CPDF_ModuleMgr::Get()->GetRenderModule()->DestroyDocData(m_pDocRender);
  }
}
#define FX_MAX_PAGE_LEVEL 1024
CPDF_Dictionary* CPDF_Document::_FindPDFPage(CPDF_Dictionary* pPages,
                                             int iPage,
                                             int nPagesToGo,
                                             int level) {
  CPDF_Array* pKidList = pPages->GetArray("Kids");
  if (!pKidList) {
    if (nPagesToGo == 0) {
      return pPages;
    }
    return NULL;
  }
  if (level >= FX_MAX_PAGE_LEVEL) {
    return NULL;
  }
  int nKids = pKidList->GetCount();
  for (int i = 0; i < nKids; i++) {
    CPDF_Dictionary* pKid = pKidList->GetDict(i);
    if (!pKid) {
      nPagesToGo--;
      continue;
    }
    if (pKid == pPages) {
      continue;
    }
    if (!pKid->KeyExist("Kids")) {
      if (nPagesToGo == 0) {
        return pKid;
      }
      m_PageList.SetAt(iPage - nPagesToGo, pKid->GetObjNum());
      nPagesToGo--;
    } else {
      int nPages = pKid->GetInteger("Count");
      if (nPagesToGo < nPages) {
        return _FindPDFPage(pKid, iPage, nPagesToGo, level + 1);
      }
      nPagesToGo -= nPages;
    }
  }
  return NULL;
}

CPDF_Dictionary* CPDF_Document::GetPage(int iPage) {
  if (iPage < 0 || iPage >= m_PageList.GetSize())
    return nullptr;

  if (m_bLinearized && (iPage == (int)m_dwFirstPageNo)) {
    if (CPDF_Dictionary* pDict =
            ToDictionary(GetIndirectObject(m_dwFirstPageObjNum, nullptr)))
      return pDict;
  }

  int objnum = m_PageList.GetAt(iPage);
  if (objnum) {
    if (CPDF_Dictionary* pDict =
            ToDictionary(GetIndirectObject(objnum, nullptr))) {
      return pDict;
    }
  }

  CPDF_Dictionary* pRoot = GetRoot();
  if (!pRoot)
    return nullptr;

  CPDF_Dictionary* pPages = pRoot->GetDict("Pages");
  if (!pPages)
    return nullptr;

  CPDF_Dictionary* pPage = _FindPDFPage(pPages, iPage, iPage, 0);
  if (!pPage)
    return nullptr;

  m_PageList.SetAt(iPage, pPage->GetObjNum());
  return pPage;
}

int CPDF_Document::_FindPageIndex(CPDF_Dictionary* pNode,
                                  FX_DWORD& skip_count,
                                  FX_DWORD objnum,
                                  int& index,
                                  int level) {
  if (pNode->KeyExist("Kids")) {
    CPDF_Array* pKidList = pNode->GetArray("Kids");
    if (!pKidList) {
      return -1;
    }
    if (level >= FX_MAX_PAGE_LEVEL) {
      return -1;
    }
    FX_DWORD count = pNode->GetInteger("Count");
    if (count <= skip_count) {
      skip_count -= count;
      index += count;
      return -1;
    }
    if (count && count == pKidList->GetCount()) {
      for (FX_DWORD i = 0; i < count; i++) {
        if (CPDF_Reference* pKid = ToReference(pKidList->GetElement(i))) {
          if (pKid->GetRefObjNum() == objnum) {
            m_PageList.SetAt(index + i, objnum);
            return index + i;
          }
        }
      }
    }
    for (FX_DWORD i = 0; i < pKidList->GetCount(); i++) {
      CPDF_Dictionary* pKid = pKidList->GetDict(i);
      if (!pKid) {
        continue;
      }
      if (pKid == pNode) {
        continue;
      }
      int found_index =
          _FindPageIndex(pKid, skip_count, objnum, index, level + 1);
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
    index++;
  }
  return -1;
}
int CPDF_Document::GetPageIndex(FX_DWORD objnum) {
  FX_DWORD nPages = m_PageList.GetSize();
  FX_DWORD skip_count = 0;
  FX_BOOL bSkipped = FALSE;
  for (FX_DWORD i = 0; i < nPages; i++) {
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
  if (!pRoot) {
    return -1;
  }
  CPDF_Dictionary* pPages = pRoot->GetDict("Pages");
  if (!pPages) {
    return -1;
  }
  int index = 0;
  return _FindPageIndex(pPages, skip_count, objnum, index);
}
int CPDF_Document::GetPageCount() const {
  return m_PageList.GetSize();
}
static int _CountPages(CPDF_Dictionary* pPages, int level) {
  if (level > 128) {
    return 0;
  }
  int count = pPages->GetInteger("Count");
  if (count > 0 && count < FPDF_PAGE_MAX_NUM) {
    return count;
  }
  CPDF_Array* pKidList = pPages->GetArray("Kids");
  if (!pKidList) {
    return 0;
  }
  count = 0;
  for (FX_DWORD i = 0; i < pKidList->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKidList->GetDict(i);
    if (!pKid) {
      continue;
    }
    if (!pKid->KeyExist("Kids")) {
      count++;
    } else {
      count += _CountPages(pKid, level + 1);
    }
  }
  pPages->SetAtInteger("Count", count);
  return count;
}
int CPDF_Document::_GetPageCount() const {
  CPDF_Dictionary* pRoot = GetRoot();
  if (!pRoot) {
    return 0;
  }
  CPDF_Dictionary* pPages = pRoot->GetDict("Pages");
  if (!pPages) {
    return 0;
  }
  if (!pPages->KeyExist("Kids")) {
    return 1;
  }
  return _CountPages(pPages, 0);
}
FX_BOOL CPDF_Document::IsContentUsedElsewhere(FX_DWORD objnum,
                                              CPDF_Dictionary* pThisPageDict) {
  for (int i = 0; i < m_PageList.GetSize(); i++) {
    CPDF_Dictionary* pPageDict = GetPage(i);
    if (pPageDict == pThisPageDict) {
      continue;
    }
    CPDF_Object* pContents =
        pPageDict ? pPageDict->GetElement("Contents") : NULL;
    if (!pContents) {
      continue;
    }
    if (pContents->GetDirectType() == PDFOBJ_ARRAY) {
      CPDF_Array* pArray = pContents->GetDirect()->AsArray();
      for (FX_DWORD j = 0; j < pArray->GetCount(); j++) {
        CPDF_Reference* pRef = ToReference(pArray->GetElement(j));
        if (pRef && pRef->GetRefObjNum() == objnum)
          return TRUE;
      }
    } else if (pContents->GetObjNum() == objnum) {
      return TRUE;
    }
  }
  return FALSE;
}
FX_DWORD CPDF_Document::GetUserPermissions(FX_BOOL bCheckRevision) const {
  if (!m_pParser) {
    return (FX_DWORD)-1;
  }
  return m_pParser->GetPermissions(bCheckRevision);
}
FX_BOOL CPDF_Document::IsOwner() const {
  return !m_pParser || m_pParser->IsOwner();
}
FX_BOOL CPDF_Document::IsFormStream(FX_DWORD objnum, FX_BOOL& bForm) const {
  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end()) {
    CPDF_Stream* pStream = it->second->AsStream();
    bForm = pStream && pStream->GetDict()->GetString("Subtype") == "Form";
    return TRUE;
  }
  if (!m_pParser) {
    bForm = FALSE;
    return TRUE;
  }
  return m_pParser->IsFormStream(objnum, bForm);
}
void CPDF_Document::ClearPageData() {
  if (m_pDocPage) {
    CPDF_ModuleMgr::Get()->GetPageModule()->ClearDoc(this);
  }
}
void CPDF_Document::ClearRenderData() {
  if (m_pDocRender) {
    CPDF_ModuleMgr::Get()->GetRenderModule()->ClearDocData(m_pDocRender);
  }
}
