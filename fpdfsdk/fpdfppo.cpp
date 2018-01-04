// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_ppo.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/ptr_util.h"

namespace {

CPDF_Object* PageDictGetInheritableTag(CPDF_Dictionary* pDict,
                                       const ByteString& bsSrcTag) {
  if (!pDict || bsSrcTag.IsEmpty())
    return nullptr;
  if (!pDict->KeyExist("Parent") || !pDict->KeyExist("Type"))
    return nullptr;

  CPDF_Object* pType = pDict->GetObjectFor("Type")->GetDirect();
  if (!ToName(pType))
    return nullptr;
  if (pType->GetString().Compare("Page"))
    return nullptr;

  CPDF_Dictionary* pp =
      ToDictionary(pDict->GetObjectFor("Parent")->GetDirect());
  if (!pp)
    return nullptr;

  if (pDict->KeyExist(bsSrcTag))
    return pDict->GetObjectFor(bsSrcTag);

  while (pp) {
    if (pp->KeyExist(bsSrcTag))
      return pp->GetObjectFor(bsSrcTag);
    if (!pp->KeyExist("Parent"))
      break;
    pp = ToDictionary(pp->GetObjectFor("Parent")->GetDirect());
  }
  return nullptr;
}

bool CopyInheritable(CPDF_Dictionary* pCurPageDict,
                     CPDF_Dictionary* pSrcPageDict,
                     const ByteString& key) {
  if (pCurPageDict->KeyExist(key))
    return true;

  CPDF_Object* pInheritable = PageDictGetInheritableTag(pSrcPageDict, key);
  if (!pInheritable)
    return false;

  pCurPageDict->SetFor(key, pInheritable->Clone());
  return true;
}

bool ParserPageRangeString(ByteString rangstring,
                           std::vector<uint16_t>* pageArray,
                           int nCount) {
  if (rangstring.IsEmpty())
    return true;

  rangstring.Remove(' ');
  size_t nLength = rangstring.GetLength();
  ByteString cbCompareString("0123456789-,");
  for (size_t i = 0; i < nLength; ++i) {
    if (!cbCompareString.Contains(rangstring[i]))
      return false;
  }

  ByteString cbMidRange;
  size_t nStringFrom = 0;
  Optional<size_t> nStringTo = 0;
  while (nStringTo < nLength) {
    nStringTo = rangstring.Find(',', nStringFrom);
    if (!nStringTo.has_value())
      nStringTo = nLength;
    cbMidRange = rangstring.Mid(nStringFrom, nStringTo.value() - nStringFrom);
    auto nMid = cbMidRange.Find('-');
    if (!nMid.has_value()) {
      uint16_t pageNum =
          pdfium::base::checked_cast<uint16_t>(atoi(cbMidRange.c_str()));
      if (pageNum <= 0 || pageNum > nCount)
        return false;
      pageArray->push_back(pageNum);
    } else {
      uint16_t nStartPageNum = pdfium::base::checked_cast<uint16_t>(
          atoi(cbMidRange.Left(nMid.value()).c_str()));
      if (nStartPageNum == 0)
        return false;

      nMid = nMid.value() + 1;
      size_t nEnd = cbMidRange.GetLength() - nMid.value();
      if (nEnd == 0)
        return false;

      uint16_t nEndPageNum = pdfium::base::checked_cast<uint16_t>(
          atoi(cbMidRange.Mid(nMid.value(), nEnd).c_str()));
      if (nStartPageNum < 0 || nStartPageNum > nEndPageNum ||
          nEndPageNum > nCount) {
        return false;
      }
      for (uint16_t i = nStartPageNum; i <= nEndPageNum; ++i) {
        pageArray->push_back(i);
      }
    }
    nStringFrom = nStringTo.value() + 1;
  }
  return true;
}

}  // namespace

class CPDF_PageOrganizer {
 public:
  CPDF_PageOrganizer(CPDF_Document* pDestPDFDoc, CPDF_Document* pSrcPDFDoc);
  ~CPDF_PageOrganizer();

  bool PDFDocInit();
  bool ExportPage(const std::vector<uint16_t>& pageNums, int nIndex);

 private:
  using ObjectNumberMap = std::map<uint32_t, uint32_t>;

  bool UpdateReference(CPDF_Object* pObj, ObjectNumberMap* pObjNumberMap);
  uint32_t GetNewObjId(ObjectNumberMap* pObjNumberMap, CPDF_Reference* pRef);

  UnownedPtr<CPDF_Document> m_pDestPDFDoc;
  UnownedPtr<CPDF_Document> m_pSrcPDFDoc;
};

CPDF_PageOrganizer::CPDF_PageOrganizer(CPDF_Document* pDestPDFDoc,
                                       CPDF_Document* pSrcPDFDoc)
    : m_pDestPDFDoc(pDestPDFDoc), m_pSrcPDFDoc(pSrcPDFDoc) {}

CPDF_PageOrganizer::~CPDF_PageOrganizer() {}

bool CPDF_PageOrganizer::PDFDocInit() {
  ASSERT(m_pDestPDFDoc);
  ASSERT(m_pSrcPDFDoc);

  CPDF_Dictionary* pNewRoot = m_pDestPDFDoc->GetRoot();
  if (!pNewRoot)
    return false;

  CPDF_Dictionary* pDocInfoDict = m_pDestPDFDoc->GetInfo();
  if (!pDocInfoDict)
    return false;

  pDocInfoDict->SetNewFor<CPDF_String>("Producer", "PDFium", false);

  ByteString cbRootType = pNewRoot->GetStringFor("Type", "");
  if (cbRootType.IsEmpty())
    pNewRoot->SetNewFor<CPDF_Name>("Type", "Catalog");

  CPDF_Object* pElement = pNewRoot->GetObjectFor("Pages");
  CPDF_Dictionary* pNewPages =
      pElement ? ToDictionary(pElement->GetDirect()) : nullptr;
  if (!pNewPages) {
    pNewPages = m_pDestPDFDoc->NewIndirect<CPDF_Dictionary>();
    pNewRoot->SetNewFor<CPDF_Reference>("Pages", m_pDestPDFDoc.Get(),
                                        pNewPages->GetObjNum());
  }

  ByteString cbPageType = pNewPages->GetStringFor("Type", "");
  if (cbPageType.IsEmpty())
    pNewPages->SetNewFor<CPDF_Name>("Type", "Pages");

  if (!pNewPages->GetArrayFor("Kids")) {
    pNewPages->SetNewFor<CPDF_Number>("Count", 0);
    pNewPages->SetNewFor<CPDF_Reference>(
        "Kids", m_pDestPDFDoc.Get(),
        m_pDestPDFDoc->NewIndirect<CPDF_Array>()->GetObjNum());
  }

  return true;
}

bool CPDF_PageOrganizer::ExportPage(const std::vector<uint16_t>& pageNums,
                                    int nIndex) {
  int curpage = nIndex;
  auto pObjNumberMap = pdfium::MakeUnique<ObjectNumberMap>();
  for (size_t i = 0; i < pageNums.size(); ++i) {
    CPDF_Dictionary* pCurPageDict = m_pDestPDFDoc->CreateNewPage(curpage);
    CPDF_Dictionary* pSrcPageDict = m_pSrcPDFDoc->GetPage(pageNums[i] - 1);
    if (!pSrcPageDict || !pCurPageDict)
      return false;

    // Clone the page dictionary
    for (const auto& it : *pSrcPageDict) {
      const ByteString& cbSrcKeyStr = it.first;
      if (cbSrcKeyStr == "Type" || cbSrcKeyStr == "Parent")
        continue;

      CPDF_Object* pObj = it.second.get();
      pCurPageDict->SetFor(cbSrcKeyStr, pObj->Clone());
    }

    // inheritable item
    // Even though some entries are required by the PDF spec, there exist
    // PDFs that omit them. Set some defaults in this case.
    // 1 MediaBox - required
    if (!CopyInheritable(pCurPageDict, pSrcPageDict, "MediaBox")) {
      // Search for "CropBox" in the source page dictionary.
      // If it does not exist, use the default letter size.
      CPDF_Object* pInheritable =
          PageDictGetInheritableTag(pSrcPageDict, "CropBox");
      if (pInheritable) {
        pCurPageDict->SetFor("MediaBox", pInheritable->Clone());
      } else {
        // Make the default size letter size (8.5"x11")
        CPDF_Array* pArray = pCurPageDict->SetNewFor<CPDF_Array>("MediaBox");
        pArray->AddNew<CPDF_Number>(0);
        pArray->AddNew<CPDF_Number>(0);
        pArray->AddNew<CPDF_Number>(612);
        pArray->AddNew<CPDF_Number>(792);
      }
    }

    // 2 Resources - required
    if (!CopyInheritable(pCurPageDict, pSrcPageDict, "Resources")) {
      // Use a default empty resources if it does not exist.
      pCurPageDict->SetNewFor<CPDF_Dictionary>("Resources");
    }

    // 3 CropBox - optional
    CopyInheritable(pCurPageDict, pSrcPageDict, "CropBox");
    // 4 Rotate - optional
    CopyInheritable(pCurPageDict, pSrcPageDict, "Rotate");

    // Update the reference
    uint32_t dwOldPageObj = pSrcPageDict->GetObjNum();
    uint32_t dwNewPageObj = pCurPageDict->GetObjNum();
    (*pObjNumberMap)[dwOldPageObj] = dwNewPageObj;
    UpdateReference(pCurPageDict, pObjNumberMap.get());
    ++curpage;
  }

  return true;
}

bool CPDF_PageOrganizer::UpdateReference(CPDF_Object* pObj,
                                         ObjectNumberMap* pObjNumberMap) {
  switch (pObj->GetType()) {
    case CPDF_Object::REFERENCE: {
      CPDF_Reference* pReference = pObj->AsReference();
      uint32_t newobjnum = GetNewObjId(pObjNumberMap, pReference);
      if (newobjnum == 0)
        return false;
      pReference->SetRef(m_pDestPDFDoc.Get(), newobjnum);
      break;
    }
    case CPDF_Object::DICTIONARY: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      auto it = pDict->begin();
      while (it != pDict->end()) {
        const ByteString& key = it->first;
        CPDF_Object* pNextObj = it->second.get();
        ++it;
        if (key == "Parent" || key == "Prev" || key == "First")
          continue;
        if (!pNextObj)
          return false;
        if (!UpdateReference(pNextObj, pObjNumberMap))
          pDict->RemoveFor(key);
      }
      break;
    }
    case CPDF_Object::ARRAY: {
      CPDF_Array* pArray = pObj->AsArray();
      for (size_t i = 0; i < pArray->GetCount(); ++i) {
        CPDF_Object* pNextObj = pArray->GetObjectAt(i);
        if (!pNextObj)
          return false;
        if (!UpdateReference(pNextObj, pObjNumberMap))
          return false;
      }
      break;
    }
    case CPDF_Object::STREAM: {
      CPDF_Stream* pStream = pObj->AsStream();
      CPDF_Dictionary* pDict = pStream->GetDict();
      if (!pDict)
        return false;
      if (!UpdateReference(pDict, pObjNumberMap))
        return false;
      break;
    }
    default:
      break;
  }

  return true;
}

uint32_t CPDF_PageOrganizer::GetNewObjId(ObjectNumberMap* pObjNumberMap,
                                         CPDF_Reference* pRef) {
  if (!pRef)
    return 0;

  uint32_t dwObjnum = pRef->GetRefObjNum();
  uint32_t dwNewObjNum = 0;
  const auto it = pObjNumberMap->find(dwObjnum);
  if (it != pObjNumberMap->end())
    dwNewObjNum = it->second;
  if (dwNewObjNum)
    return dwNewObjNum;

  CPDF_Object* pDirect = pRef->GetDirect();
  if (!pDirect)
    return 0;

  std::unique_ptr<CPDF_Object> pClone = pDirect->Clone();
  if (CPDF_Dictionary* pDictClone = pClone->AsDictionary()) {
    if (pDictClone->KeyExist("Type")) {
      ByteString strType = pDictClone->GetStringFor("Type");
      if (!FXSYS_stricmp(strType.c_str(), "Pages"))
        return 4;
      if (!FXSYS_stricmp(strType.c_str(), "Page"))
        return 0;
    }
  }
  CPDF_Object* pUnownedClone =
      m_pDestPDFDoc->AddIndirectObject(std::move(pClone));
  dwNewObjNum = pUnownedClone->GetObjNum();
  (*pObjNumberMap)[dwObjnum] = dwNewObjNum;
  if (!UpdateReference(pUnownedClone, pObjNumberMap))
    return 0;

  return dwNewObjNum;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_ImportPages(FPDF_DOCUMENT dest_doc,
                                                     FPDF_DOCUMENT src_doc,
                                                     FPDF_BYTESTRING pagerange,
                                                     int index) {
  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest_doc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  std::vector<uint16_t> pageArray;
  int nCount = pSrcDoc->GetPageCount();
  if (pagerange) {
    if (!ParserPageRangeString(pagerange, &pageArray, nCount))
      return false;
  } else {
    for (int i = 1; i <= nCount; ++i) {
      pageArray.push_back(i);
    }
  }

  CPDF_PageOrganizer pageOrg(pDestDoc, pSrcDoc);
  return pageOrg.PDFDocInit() && pageOrg.ExportPage(pageArray, index);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc, FPDF_DOCUMENT src_doc) {
  CPDF_Document* pDstDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!pDstDoc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  CPDF_Dictionary* pSrcDict = pSrcDoc->GetRoot();
  pSrcDict = pSrcDict->GetDictFor("ViewerPreferences");
  if (!pSrcDict)
    return false;

  CPDF_Dictionary* pDstDict = pDstDoc->GetRoot();
  if (!pDstDict)
    return false;

  pDstDict->SetFor("ViewerPreferences", pSrcDict->CloneDirectObject());
  return true;
}
