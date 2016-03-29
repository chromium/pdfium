// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

#include <set>

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_parser.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_reference.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfapi/fpdf_render/render_int.h"
#include "core/fpdfapi/include/cpdf_modulemgr.h"
#include "core/fpdfapi/ipdf_rendermodule.h"
#include "core/include/fxge/fx_font.h"
#include "third_party/base/stl_util.h"

namespace {

int CountPages(CPDF_Dictionary* pPages,
               std::set<CPDF_Dictionary*>* visited_pages) {
  int count = pPages->GetIntegerBy("Count");
  if (count > 0 && count < FPDF_PAGE_MAX_NUM) {
    return count;
  }
  CPDF_Array* pKidList = pPages->GetArrayBy("Kids");
  if (!pKidList) {
    return 0;
  }
  count = 0;
  for (uint32_t i = 0; i < pKidList->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
    if (!pKid || pdfium::ContainsKey(*visited_pages, pKid)) {
      continue;
    }
    if (pKid->KeyExist("Kids")) {
      // Use |visited_pages| to help detect circular references of pages.
      pdfium::ScopedSetInsertion<CPDF_Dictionary*> local_add(visited_pages,
                                                             pKid);
      count += CountPages(pKid, visited_pages);
    } else {
      // This page is a leaf node.
      count++;
    }
  }
  pPages->SetAtInteger("Count", count);
  return count;
}

}  // namespace

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
  CPDF_Object* pRootObj = GetIndirectObject(m_pParser->GetRootObjNum());
  if (!pRootObj) {
    return;
  }
  m_pRootDict = pRootObj->GetDict();
  if (!m_pRootDict) {
    return;
  }
  CPDF_Object* pInfoObj = GetIndirectObject(m_pParser->GetInfoObjNum());
  if (pInfoObj) {
    m_pInfoDict = pInfoObj->GetDict();
  }
  CPDF_Array* pIDArray = m_pParser->GetIDArray();
  if (pIDArray) {
    m_ID1 = pIDArray->GetStringAt(0);
    m_ID2 = pIDArray->GetStringAt(1);
  }
  m_PageList.SetSize(RetrievePageCount());
}
void CPDF_Document::LoadAsynDoc(CPDF_Dictionary* pLinearized) {
  m_bLinearized = TRUE;
  m_LastObjNum = m_pParser->GetLastObjNum();
  CPDF_Object* pIndirectObj = GetIndirectObject(m_pParser->GetRootObjNum());
  m_pRootDict = pIndirectObj ? pIndirectObj->GetDict() : nullptr;
  if (!m_pRootDict) {
    return;
  }
  pIndirectObj = GetIndirectObject(m_pParser->GetInfoObjNum());
  m_pInfoDict = pIndirectObj ? pIndirectObj->GetDict() : nullptr;
  CPDF_Array* pIDArray = m_pParser->GetIDArray();
  if (pIDArray) {
    m_ID1 = pIDArray->GetStringAt(0);
    m_ID2 = pIDArray->GetStringAt(1);
  }
  uint32_t dwPageCount = 0;
  CPDF_Object* pCount = pLinearized->GetObjectBy("N");
  if (ToNumber(pCount))
    dwPageCount = pCount->GetInteger();

  m_PageList.SetSize(dwPageCount);
  CPDF_Object* pNo = pLinearized->GetObjectBy("P");
  if (ToNumber(pNo))
    m_dwFirstPageNo = pNo->GetInteger();

  CPDF_Object* pObjNum = pLinearized->GetObjectBy("O");
  if (ToNumber(pObjNum))
    m_dwFirstPageObjNum = pObjNum->GetInteger();
}
void CPDF_Document::LoadPages() {
  m_PageList.SetSize(RetrievePageCount());
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
  CPDF_Array* pKidList = pPages->GetArrayBy("Kids");
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
    CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
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
      int nPages = pKid->GetIntegerBy("Count");
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
            ToDictionary(GetIndirectObject(m_dwFirstPageObjNum))) {
      return pDict;
    }
  }

  int objnum = m_PageList.GetAt(iPage);
  if (objnum) {
    if (CPDF_Dictionary* pDict = ToDictionary(GetIndirectObject(objnum)))
      return pDict;
  }

  CPDF_Dictionary* pRoot = GetRoot();
  if (!pRoot)
    return nullptr;

  CPDF_Dictionary* pPages = pRoot->GetDictBy("Pages");
  if (!pPages)
    return nullptr;

  CPDF_Dictionary* pPage = _FindPDFPage(pPages, iPage, iPage, 0);
  if (!pPage)
    return nullptr;

  m_PageList.SetAt(iPage, pPage->GetObjNum());
  return pPage;
}

int CPDF_Document::_FindPageIndex(CPDF_Dictionary* pNode,
                                  uint32_t& skip_count,
                                  uint32_t objnum,
                                  int& index,
                                  int level) {
  if (pNode->KeyExist("Kids")) {
    CPDF_Array* pKidList = pNode->GetArrayBy("Kids");
    if (!pKidList) {
      return -1;
    }
    if (level >= FX_MAX_PAGE_LEVEL) {
      return -1;
    }
    uint32_t count = pNode->GetIntegerBy("Count");
    if (count <= skip_count) {
      skip_count -= count;
      index += count;
      return -1;
    }
    if (count && count == pKidList->GetCount()) {
      for (uint32_t i = 0; i < count; i++) {
        if (CPDF_Reference* pKid = ToReference(pKidList->GetObjectAt(i))) {
          if (pKid->GetRefObjNum() == objnum) {
            m_PageList.SetAt(index + i, objnum);
            return index + i;
          }
        }
      }
    }
    for (uint32_t i = 0; i < pKidList->GetCount(); i++) {
      CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
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
int CPDF_Document::GetPageIndex(uint32_t objnum) {
  uint32_t nPages = m_PageList.GetSize();
  uint32_t skip_count = 0;
  FX_BOOL bSkipped = FALSE;
  for (uint32_t i = 0; i < nPages; i++) {
    uint32_t objnum1 = m_PageList.GetAt(i);
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
  CPDF_Dictionary* pPages = pRoot->GetDictBy("Pages");
  if (!pPages) {
    return -1;
  }
  int index = 0;
  return _FindPageIndex(pPages, skip_count, objnum, index);
}
int CPDF_Document::GetPageCount() const {
  return m_PageList.GetSize();
}

int CPDF_Document::RetrievePageCount() const {
  CPDF_Dictionary* pRoot = GetRoot();
  if (!pRoot) {
    return 0;
  }
  CPDF_Dictionary* pPages = pRoot->GetDictBy("Pages");
  if (!pPages) {
    return 0;
  }
  if (!pPages->KeyExist("Kids")) {
    return 1;
  }
  std::set<CPDF_Dictionary*> visited_pages;
  visited_pages.insert(pPages);
  return CountPages(pPages, &visited_pages);
}

uint32_t CPDF_Document::GetUserPermissions(FX_BOOL bCheckRevision) const {
  if (!m_pParser) {
    return (uint32_t)-1;
  }
  return m_pParser->GetPermissions(bCheckRevision);
}

FX_BOOL CPDF_Document::IsFormStream(uint32_t objnum, FX_BOOL& bForm) const {
  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end()) {
    CPDF_Stream* pStream = it->second->AsStream();
    bForm = pStream && pStream->GetDict()->GetStringBy("Subtype") == "Form";
    return TRUE;
  }
  if (!m_pParser) {
    bForm = FALSE;
    return TRUE;
  }
  return m_pParser->IsFormStream(objnum, bForm);
}

void CPDF_Document::ClearPageData() {
  if (m_pDocPage)
    CPDF_ModuleMgr::Get()->GetPageModule()->ClearDoc(this);
}

void CPDF_Document::ClearRenderData() {
  if (m_pDocRender)
    CPDF_ModuleMgr::Get()->GetRenderModule()->ClearDocData(m_pDocRender);
}

void CPDF_Document::ClearRenderFont() {
  if (!m_pDocRender)
    return;

  CFX_FontCache* pCache = m_pDocRender->GetFontCache();
  if (pCache)
    pCache->FreeCache(FALSE);
}
