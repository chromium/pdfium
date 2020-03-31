// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_document.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_read_validator.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fxcodec/jbig2/JBig2_DocumentContext.h"
#include "core/fxcrt/fx_codepage.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const int kMaxPageLevel = 1024;

int CountPages(CPDF_Dictionary* pPages,
               std::set<CPDF_Dictionary*>* visited_pages) {
  int count = pPages->GetIntegerFor("Count");
  if (count > 0 && count < CPDF_Document::kPageMaxNum)
    return count;
  CPDF_Array* pKidList = pPages->GetArrayFor("Kids");
  if (!pKidList)
    return 0;
  count = 0;
  for (size_t i = 0; i < pKidList->size(); i++) {
    CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
    if (!pKid || pdfium::ContainsKey(*visited_pages, pKid))
      continue;
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
  pPages->SetNewFor<CPDF_Number>("Count", count);
  return count;
}

int FindPageIndex(const CPDF_Dictionary* pNode,
                  uint32_t* skip_count,
                  uint32_t objnum,
                  int* index,
                  int level) {
  if (!pNode->KeyExist("Kids")) {
    if (objnum == pNode->GetObjNum())
      return *index;

    (*skip_count)--;
    (*index)++;
    return -1;
  }

  const CPDF_Array* pKidList = pNode->GetArrayFor("Kids");
  if (!pKidList)
    return -1;

  if (level >= kMaxPageLevel)
    return -1;

  size_t count = pNode->GetIntegerFor("Count");
  if (count <= *skip_count) {
    (*skip_count) -= count;
    (*index) += count;
    return -1;
  }

  if (count && count == pKidList->size()) {
    for (size_t i = 0; i < count; i++) {
      const CPDF_Reference* pKid = ToReference(pKidList->GetObjectAt(i));
      if (pKid && pKid->GetRefObjNum() == objnum)
        return static_cast<int>(*index + i);
    }
  }

  for (size_t i = 0; i < pKidList->size(); i++) {
    const CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
    if (!pKid || pKid == pNode)
      continue;

    int found_index = FindPageIndex(pKid, skip_count, objnum, index, level + 1);
    if (found_index >= 0)
      return found_index;
  }
  return -1;
}

}  // namespace

CPDF_Document::CPDF_Document(std::unique_ptr<RenderDataIface> pRenderData,
                             std::unique_ptr<PageDataIface> pPageData)
    : m_pDocRender(std::move(pRenderData)),
      m_pDocPage(std::move(pPageData)),
      m_StockFontClearer(m_pDocPage.get()) {
  m_pDocRender->SetDocument(this);
  m_pDocPage->SetDocument(this);
}

CPDF_Document::~CPDF_Document() = default;

// static
bool CPDF_Document::IsValidPageObject(const CPDF_Object* obj) {
  const CPDF_Dictionary* dict = ToDictionary(obj);
  if (!dict)
    return false;

  const CPDF_Name* name = ToName(dict->GetObjectFor("Type"));
  return name && name->GetString() == "Page";
}

RetainPtr<CPDF_Object> CPDF_Document::ParseIndirectObject(uint32_t objnum) {
  return m_pParser ? m_pParser->ParseIndirectObject(objnum) : nullptr;
}

bool CPDF_Document::TryInit() {
  SetLastObjNum(m_pParser->GetLastObjNum());

  CPDF_Object* pRootObj = GetOrParseIndirectObject(m_pParser->GetRootObjNum());
  if (pRootObj)
    m_pRootDict.Reset(pRootObj->GetDict());

  LoadPages();
  return GetRoot() && GetPageCount() > 0;
}

CPDF_Parser::Error CPDF_Document::LoadDoc(
    const RetainPtr<IFX_SeekableReadStream>& pFileAccess,
    const char* password) {
  if (!m_pParser)
    SetParser(pdfium::MakeUnique<CPDF_Parser>(this));

  return HandleLoadResult(m_pParser->StartParse(pFileAccess, password));
}

CPDF_Parser::Error CPDF_Document::LoadLinearizedDoc(
    const RetainPtr<CPDF_ReadValidator>& validator,
    const char* password) {
  if (!m_pParser)
    SetParser(pdfium::MakeUnique<CPDF_Parser>(this));

  return HandleLoadResult(m_pParser->StartLinearizedParse(validator, password));
}

void CPDF_Document::LoadPages() {
  const CPDF_LinearizedHeader* linearized_header =
      m_pParser->GetLinearizedHeader();
  if (!linearized_header) {
    m_PageList.resize(RetrievePageCount());
    return;
  }

  uint32_t objnum = linearized_header->GetFirstPageObjNum();
  if (!IsValidPageObject(GetOrParseIndirectObject(objnum))) {
    m_PageList.resize(RetrievePageCount());
    return;
  }

  uint32_t first_page_num = linearized_header->GetFirstPageNo();
  uint32_t page_count = linearized_header->GetPageCount();
  ASSERT(first_page_num < page_count);
  m_PageList.resize(page_count);
  m_PageList[first_page_num] = objnum;
}

CPDF_Dictionary* CPDF_Document::TraversePDFPages(int iPage,
                                                 int* nPagesToGo,
                                                 size_t level) {
  if (*nPagesToGo < 0 || m_bReachedMaxPageLevel)
    return nullptr;

  CPDF_Dictionary* pPages = m_pTreeTraversal[level].first;
  CPDF_Array* pKidList = pPages->GetArrayFor("Kids");
  if (!pKidList) {
    m_pTreeTraversal.pop_back();
    if (*nPagesToGo != 1)
      return nullptr;
    m_PageList[iPage] = pPages->GetObjNum();
    return pPages;
  }
  if (level >= kMaxPageLevel) {
    m_pTreeTraversal.pop_back();
    m_bReachedMaxPageLevel = true;
    return nullptr;
  }
  CPDF_Dictionary* page = nullptr;
  for (size_t i = m_pTreeTraversal[level].second; i < pKidList->size(); i++) {
    if (*nPagesToGo == 0)
      break;
    pKidList->ConvertToIndirectObjectAt(i, this);
    CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
    if (!pKid) {
      (*nPagesToGo)--;
      m_pTreeTraversal[level].second++;
      continue;
    }
    if (pKid == pPages) {
      m_pTreeTraversal[level].second++;
      continue;
    }
    if (!pKid->KeyExist("Kids")) {
      m_PageList[iPage - (*nPagesToGo) + 1] = pKid->GetObjNum();
      (*nPagesToGo)--;
      m_pTreeTraversal[level].second++;
      if (*nPagesToGo == 0) {
        page = pKid;
        break;
      }
    } else {
      // If the vector has size level+1, the child is not in yet
      if (m_pTreeTraversal.size() == level + 1)
        m_pTreeTraversal.push_back(std::make_pair(pKid, 0));
      // Now m_pTreeTraversal[level+1] should exist and be equal to pKid.
      CPDF_Dictionary* pageKid = TraversePDFPages(iPage, nPagesToGo, level + 1);
      // Check if child was completely processed, i.e. it popped itself out
      if (m_pTreeTraversal.size() == level + 1)
        m_pTreeTraversal[level].second++;
      // If child did not finish, no pages to go, or max level reached, end
      if (m_pTreeTraversal.size() != level + 1 || *nPagesToGo == 0 ||
          m_bReachedMaxPageLevel) {
        page = pageKid;
        break;
      }
    }
  }
  if (m_pTreeTraversal[level].second == pKidList->size())
    m_pTreeTraversal.pop_back();
  return page;
}

void CPDF_Document::ResetTraversal() {
  m_iNextPageToTraverse = 0;
  m_bReachedMaxPageLevel = false;
  m_pTreeTraversal.clear();
}

void CPDF_Document::SetParser(std::unique_ptr<CPDF_Parser> pParser) {
  ASSERT(!m_pParser);
  m_pParser = std::move(pParser);
}

CPDF_Parser::Error CPDF_Document::HandleLoadResult(CPDF_Parser::Error error) {
  if (error == CPDF_Parser::SUCCESS)
    m_bHasValidCrossReferenceTable = !m_pParser->xref_table_rebuilt();
  return error;
}

const CPDF_Dictionary* CPDF_Document::GetPagesDict() const {
  const CPDF_Dictionary* pRoot = GetRoot();
  return pRoot ? pRoot->GetDictFor("Pages") : nullptr;
}

CPDF_Dictionary* CPDF_Document::GetPagesDict() {
  return const_cast<CPDF_Dictionary*>(
      static_cast<const CPDF_Document*>(this)->GetPagesDict());
}

bool CPDF_Document::IsPageLoaded(int iPage) const {
  return !!m_PageList[iPage];
}

CPDF_Dictionary* CPDF_Document::GetPageDictionary(int iPage) {
  if (!pdfium::IndexInBounds(m_PageList, iPage))
    return nullptr;

  const uint32_t objnum = m_PageList[iPage];
  if (objnum) {
    CPDF_Dictionary* result = ToDictionary(GetOrParseIndirectObject(objnum));
    if (result)
      return result;
  }

  CPDF_Dictionary* pPages = GetPagesDict();
  if (!pPages)
    return nullptr;

  if (m_pTreeTraversal.empty()) {
    ResetTraversal();
    m_pTreeTraversal.push_back(std::make_pair(pPages, 0));
  }
  int nPagesToGo = iPage - m_iNextPageToTraverse + 1;
  CPDF_Dictionary* pPage = TraversePDFPages(iPage, &nPagesToGo, 0);
  m_iNextPageToTraverse = iPage + 1;
  return pPage;
}

void CPDF_Document::SetPageObjNum(int iPage, uint32_t objNum) {
  m_PageList[iPage] = objNum;
}

int CPDF_Document::GetPageIndex(uint32_t objnum) {
  uint32_t skip_count = 0;
  bool bSkipped = false;
  for (uint32_t i = 0; i < m_PageList.size(); ++i) {
    if (m_PageList[i] == objnum)
      return i;

    if (!bSkipped && m_PageList[i] == 0) {
      skip_count = i;
      bSkipped = true;
    }
  }
  const CPDF_Dictionary* pPages = GetPagesDict();
  if (!pPages)
    return -1;

  int start_index = 0;
  int found_index = FindPageIndex(pPages, &skip_count, objnum, &start_index, 0);

  // Corrupt page tree may yield out-of-range results.
  if (!pdfium::IndexInBounds(m_PageList, found_index))
    return -1;

  m_PageList[found_index] = objnum;
  return found_index;
}

int CPDF_Document::GetPageCount() const {
  return pdfium::CollectionSize<int>(m_PageList);
}

int CPDF_Document::RetrievePageCount() {
  CPDF_Dictionary* pPages = GetPagesDict();
  if (!pPages)
    return 0;

  if (!pPages->KeyExist("Kids"))
    return 1;

  std::set<CPDF_Dictionary*> visited_pages;
  visited_pages.insert(pPages);
  return CountPages(pPages, &visited_pages);
}

uint32_t CPDF_Document::GetUserPermissions() const {
  if (m_pParser)
    return m_pParser->GetPermissions();

  return m_pExtension ? m_pExtension->GetUserPermissions() : 0;
}

void CPDF_Document::CreateNewDoc() {
  ASSERT(!m_pRootDict);
  ASSERT(!m_pInfoDict);
  m_pRootDict.Reset(NewIndirect<CPDF_Dictionary>());
  m_pRootDict->SetNewFor<CPDF_Name>("Type", "Catalog");

  CPDF_Dictionary* pPages = NewIndirect<CPDF_Dictionary>();
  pPages->SetNewFor<CPDF_Name>("Type", "Pages");
  pPages->SetNewFor<CPDF_Number>("Count", 0);
  pPages->SetNewFor<CPDF_Array>("Kids");
  m_pRootDict->SetNewFor<CPDF_Reference>("Pages", this, pPages->GetObjNum());
  m_pInfoDict.Reset(NewIndirect<CPDF_Dictionary>());
}

CPDF_Dictionary* CPDF_Document::CreateNewPage(int iPage) {
  CPDF_Dictionary* pDict = NewIndirect<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Name>("Type", "Page");
  uint32_t dwObjNum = pDict->GetObjNum();
  if (!InsertNewPage(iPage, pDict)) {
    DeleteIndirectObject(dwObjNum);
    return nullptr;
  }
  return pDict;
}

bool CPDF_Document::InsertDeletePDFPage(CPDF_Dictionary* pPages,
                                        int nPagesToGo,
                                        CPDF_Dictionary* pPageDict,
                                        bool bInsert,
                                        std::set<CPDF_Dictionary*>* pVisited) {
  CPDF_Array* pKidList = pPages->GetArrayFor("Kids");
  if (!pKidList)
    return false;

  for (size_t i = 0; i < pKidList->size(); i++) {
    CPDF_Dictionary* pKid = pKidList->GetDictAt(i);
    if (pKid->GetStringFor("Type") == "Page") {
      if (nPagesToGo != 0) {
        nPagesToGo--;
        continue;
      }
      if (bInsert) {
        pKidList->InsertNewAt<CPDF_Reference>(i, this, pPageDict->GetObjNum());
        pPageDict->SetNewFor<CPDF_Reference>("Parent", this,
                                             pPages->GetObjNum());
      } else {
        pKidList->RemoveAt(i);
      }
      pPages->SetNewFor<CPDF_Number>(
          "Count", pPages->GetIntegerFor("Count") + (bInsert ? 1 : -1));
      ResetTraversal();
      break;
    }
    int nPages = pKid->GetIntegerFor("Count");
    if (nPagesToGo >= nPages) {
      nPagesToGo -= nPages;
      continue;
    }
    if (pdfium::ContainsKey(*pVisited, pKid))
      return false;

    pdfium::ScopedSetInsertion<CPDF_Dictionary*> insertion(pVisited, pKid);
    if (!InsertDeletePDFPage(pKid, nPagesToGo, pPageDict, bInsert, pVisited))
      return false;

    pPages->SetNewFor<CPDF_Number>(
        "Count", pPages->GetIntegerFor("Count") + (bInsert ? 1 : -1));
    break;
  }
  return true;
}

bool CPDF_Document::InsertNewPage(int iPage, CPDF_Dictionary* pPageDict) {
  CPDF_Dictionary* pRoot = GetRoot();
  CPDF_Dictionary* pPages = pRoot ? pRoot->GetDictFor("Pages") : nullptr;
  if (!pPages)
    return false;

  int nPages = GetPageCount();
  if (iPage < 0 || iPage > nPages)
    return false;

  if (iPage == nPages) {
    CPDF_Array* pPagesList = pPages->GetArrayFor("Kids");
    if (!pPagesList)
      pPagesList = pPages->SetNewFor<CPDF_Array>("Kids");
    pPagesList->AppendNew<CPDF_Reference>(this, pPageDict->GetObjNum());
    pPages->SetNewFor<CPDF_Number>("Count", nPages + 1);
    pPageDict->SetNewFor<CPDF_Reference>("Parent", this, pPages->GetObjNum());
    ResetTraversal();
  } else {
    std::set<CPDF_Dictionary*> stack = {pPages};
    if (!InsertDeletePDFPage(pPages, iPage, pPageDict, true, &stack))
      return false;
  }
  m_PageList.insert(m_PageList.begin() + iPage, pPageDict->GetObjNum());
  return true;
}

CPDF_Dictionary* CPDF_Document::GetInfo() {
  if (m_pInfoDict)
    return m_pInfoDict.Get();

  if (!m_pParser || !m_pParser->GetInfoObjNum())
    return nullptr;

  auto ref =
      pdfium::MakeRetain<CPDF_Reference>(this, m_pParser->GetInfoObjNum());
  m_pInfoDict.Reset(ToDictionary(ref->GetDirect()));
  return m_pInfoDict.Get();
}

void CPDF_Document::DeletePage(int iPage) {
  CPDF_Dictionary* pPages = GetPagesDict();
  if (!pPages)
    return;

  int nPages = pPages->GetIntegerFor("Count");
  if (iPage < 0 || iPage >= nPages)
    return;

  std::set<CPDF_Dictionary*> stack = {pPages};
  if (!InsertDeletePDFPage(pPages, iPage, nullptr, false, &stack))
    return;

  m_PageList.erase(m_PageList.begin() + iPage);
}

void CPDF_Document::SetRootForTesting(CPDF_Dictionary* root) {
  m_pRootDict.Reset(root);
}

void CPDF_Document::ResizePageListForTesting(size_t size) {
  m_PageList.resize(size);
}

CPDF_Document::StockFontClearer::StockFontClearer(
    CPDF_Document::PageDataIface* pPageData)
    : m_pPageData(pPageData) {}

CPDF_Document::StockFontClearer::~StockFontClearer() {
  m_pPageData->ClearStockFont();
}

CPDF_Document::PageDataIface::PageDataIface() = default;

CPDF_Document::PageDataIface::~PageDataIface() = default;

CPDF_Document::RenderDataIface::RenderDataIface() = default;

CPDF_Document::RenderDataIface::~RenderDataIface() = default;
