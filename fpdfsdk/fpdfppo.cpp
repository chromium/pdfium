// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_ppo.h"

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/stl_util.h"

class CPDF_PageOrganizer {
 public:
  using ObjectNumberMap = std::map<uint32_t, uint32_t>;
  CPDF_PageOrganizer();
  ~CPDF_PageOrganizer();

  bool PDFDocInit(CPDF_Document* pDestPDFDoc, CPDF_Document* pSrcPDFDoc);
  bool ExportPage(CPDF_Document* pSrcPDFDoc,
                  std::vector<uint16_t>* pPageNums,
                  CPDF_Document* pDestPDFDoc,
                  int nIndex);
  CPDF_Object* PageDictGetInheritableTag(CPDF_Dictionary* pDict,
                                         const CFX_ByteString& bsSrctag);
  bool UpdateReference(CPDF_Object* pObj,
                       CPDF_Document* pDoc,
                       ObjectNumberMap* pObjNumberMap);
  uint32_t GetNewObjId(CPDF_Document* pDoc,
                       ObjectNumberMap* pObjNumberMap,
                       CPDF_Reference* pRef);
};

CPDF_PageOrganizer::CPDF_PageOrganizer() {}

CPDF_PageOrganizer::~CPDF_PageOrganizer() {}

bool CPDF_PageOrganizer::PDFDocInit(CPDF_Document* pDestPDFDoc,
                                    CPDF_Document* pSrcPDFDoc) {
  if (!pDestPDFDoc || !pSrcPDFDoc)
    return false;

  CPDF_Dictionary* pNewRoot = pDestPDFDoc->GetRoot();
  if (!pNewRoot)
    return false;

  CPDF_Dictionary* DInfoDict = pDestPDFDoc->GetInfo();
  if (!DInfoDict)
    return false;

  CFX_ByteString producerstr;
  producerstr.Format("PDFium");
  DInfoDict->SetFor("Producer", new CPDF_String(producerstr, false));

  CFX_ByteString cbRootType = pNewRoot->GetStringFor("Type", "");
  if (cbRootType.IsEmpty())
    pNewRoot->SetFor("Type", new CPDF_Name("Catalog"));

  CPDF_Object* pElement = pNewRoot->GetObjectFor("Pages");
  CPDF_Dictionary* pNewPages =
      pElement ? ToDictionary(pElement->GetDirect()) : nullptr;
  if (!pNewPages) {
    pNewPages = new CPDF_Dictionary(pDestPDFDoc->GetByteStringPool());
    pNewRoot->SetReferenceFor("Pages", pDestPDFDoc,
                              pDestPDFDoc->AddIndirectObject(pNewPages));
  }

  CFX_ByteString cbPageType = pNewPages->GetStringFor("Type", "");
  if (cbPageType == "") {
    pNewPages->SetFor("Type", new CPDF_Name("Pages"));
  }

  if (!pNewPages->GetArrayFor("Kids")) {
    pNewPages->SetIntegerFor("Count", 0);
    pNewPages->SetReferenceFor("Kids", pDestPDFDoc,
                               pDestPDFDoc->AddIndirectObject(new CPDF_Array));
  }

  return true;
}

bool CPDF_PageOrganizer::ExportPage(CPDF_Document* pSrcPDFDoc,
                                    std::vector<uint16_t>* pPageNums,
                                    CPDF_Document* pDestPDFDoc,
                                    int nIndex) {
  int curpage = nIndex;
  std::unique_ptr<ObjectNumberMap> pObjNumberMap(new ObjectNumberMap);
  int nSize = pdfium::CollectionSize<int>(*pPageNums);
  for (int i = 0; i < nSize; ++i) {
    CPDF_Dictionary* pCurPageDict = pDestPDFDoc->CreateNewPage(curpage);
    CPDF_Dictionary* pSrcPageDict = pSrcPDFDoc->GetPage(pPageNums->at(i) - 1);
    if (!pSrcPageDict || !pCurPageDict)
      return false;

    // Clone the page dictionary
    for (const auto& it : *pSrcPageDict) {
      const CFX_ByteString& cbSrcKeyStr = it.first;
      CPDF_Object* pObj = it.second;
      if (cbSrcKeyStr.Compare(("Type")) && cbSrcKeyStr.Compare(("Parent"))) {
        if (pCurPageDict->KeyExist(cbSrcKeyStr))
          pCurPageDict->RemoveFor(cbSrcKeyStr);
        pCurPageDict->SetFor(cbSrcKeyStr, pObj->Clone());
      }
    }

    // inheritable item
    CPDF_Object* pInheritable = nullptr;
    // 1 MediaBox  //required
    if (!pCurPageDict->KeyExist("MediaBox")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "MediaBox");
      if (!pInheritable) {
        // Search the "CropBox" from source page dictionary,
        // if not exists,we take the letter size.
        pInheritable = PageDictGetInheritableTag(pSrcPageDict, "CropBox");
        if (pInheritable) {
          pCurPageDict->SetFor("MediaBox", pInheritable->Clone());
        } else {
          // Make the default size to be letter size (8.5'x11')
          CPDF_Array* pArray = new CPDF_Array;
          pArray->AddNumber(0);
          pArray->AddNumber(0);
          pArray->AddNumber(612);
          pArray->AddNumber(792);
          pCurPageDict->SetFor("MediaBox", pArray);
        }
      } else {
        pCurPageDict->SetFor("MediaBox", pInheritable->Clone());
      }
    }
    // 2 Resources //required
    if (!pCurPageDict->KeyExist("Resources")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "Resources");
      if (!pInheritable)
        return false;
      pCurPageDict->SetFor("Resources", pInheritable->Clone());
    }
    // 3 CropBox  //Optional
    if (!pCurPageDict->KeyExist("CropBox")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "CropBox");
      if (pInheritable)
        pCurPageDict->SetFor("CropBox", pInheritable->Clone());
    }
    // 4 Rotate  //Optional
    if (!pCurPageDict->KeyExist("Rotate")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "Rotate");
      if (pInheritable)
        pCurPageDict->SetFor("Rotate", pInheritable->Clone());
    }

    // Update the reference
    uint32_t dwOldPageObj = pSrcPageDict->GetObjNum();
    uint32_t dwNewPageObj = pCurPageDict->GetObjNum();

    (*pObjNumberMap)[dwOldPageObj] = dwNewPageObj;

    UpdateReference(pCurPageDict, pDestPDFDoc, pObjNumberMap.get());
    ++curpage;
  }

  return true;
}

CPDF_Object* CPDF_PageOrganizer::PageDictGetInheritableTag(
    CPDF_Dictionary* pDict,
    const CFX_ByteString& bsSrcTag) {
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

bool CPDF_PageOrganizer::UpdateReference(CPDF_Object* pObj,
                                         CPDF_Document* pDoc,
                                         ObjectNumberMap* pObjNumberMap) {
  switch (pObj->GetType()) {
    case CPDF_Object::REFERENCE: {
      CPDF_Reference* pReference = pObj->AsReference();
      uint32_t newobjnum = GetNewObjId(pDoc, pObjNumberMap, pReference);
      if (newobjnum == 0)
        return false;
      pReference->SetRef(pDoc, newobjnum);
      break;
    }
    case CPDF_Object::DICTIONARY: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      auto it = pDict->begin();
      while (it != pDict->end()) {
        const CFX_ByteString& key = it->first;
        CPDF_Object* pNextObj = it->second;
        ++it;
        if (key == "Parent" || key == "Prev" || key == "First")
          continue;
        if (!pNextObj)
          return false;
        if (!UpdateReference(pNextObj, pDoc, pObjNumberMap))
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
        if (!UpdateReference(pNextObj, pDoc, pObjNumberMap))
          return false;
      }
      break;
    }
    case CPDF_Object::STREAM: {
      CPDF_Stream* pStream = pObj->AsStream();
      CPDF_Dictionary* pDict = pStream->GetDict();
      if (pDict) {
        if (!UpdateReference(pDict, pDoc, pObjNumberMap))
          return false;
      } else {
        return false;
      }
      break;
    }
    default:
      break;
  }

  return true;
}

uint32_t CPDF_PageOrganizer::GetNewObjId(CPDF_Document* pDoc,
                                         ObjectNumberMap* pObjNumberMap,
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

  CPDF_Object* pClone = pDirect->Clone();
  if (!pClone)
    return 0;

  if (CPDF_Dictionary* pDictClone = pClone->AsDictionary()) {
    if (pDictClone->KeyExist("Type")) {
      CFX_ByteString strType = pDictClone->GetStringFor("Type");
      if (!FXSYS_stricmp(strType.c_str(), "Pages")) {
        pDictClone->Release();
        return 4;
      }
      if (!FXSYS_stricmp(strType.c_str(), "Page")) {
        pDictClone->Release();
        return 0;
      }
    }
  }
  dwNewObjNum = pDoc->AddIndirectObject(pClone);
  (*pObjNumberMap)[dwObjnum] = dwNewObjNum;
  if (!UpdateReference(pClone, pDoc, pObjNumberMap)) {
    pClone->Release();
    return 0;
  }
  return dwNewObjNum;
}

FPDF_BOOL ParserPageRangeString(CFX_ByteString rangstring,
                                std::vector<uint16_t>* pageArray,
                                int nCount) {
  if (rangstring.GetLength() != 0) {
    rangstring.Remove(' ');
    int nLength = rangstring.GetLength();
    CFX_ByteString cbCompareString("0123456789-,");
    for (int i = 0; i < nLength; ++i) {
      if (cbCompareString.Find(rangstring[i]) == -1)
        return false;
    }
    CFX_ByteString cbMidRange;
    int nStringFrom = 0;
    int nStringTo = 0;
    while (nStringTo < nLength) {
      nStringTo = rangstring.Find(',', nStringFrom);
      if (nStringTo == -1)
        nStringTo = nLength;
      cbMidRange = rangstring.Mid(nStringFrom, nStringTo - nStringFrom);
      int nMid = cbMidRange.Find('-');
      if (nMid == -1) {
        long lPageNum = atol(cbMidRange.c_str());
        if (lPageNum <= 0 || lPageNum > nCount)
          return false;
        pageArray->push_back((uint16_t)lPageNum);
      } else {
        int nStartPageNum = atol(cbMidRange.Mid(0, nMid).c_str());
        if (nStartPageNum == 0)
          return false;

        ++nMid;
        int nEnd = cbMidRange.GetLength() - nMid;
        if (nEnd == 0)
          return false;

        int nEndPageNum = atol(cbMidRange.Mid(nMid, nEnd).c_str());
        if (nStartPageNum < 0 || nStartPageNum > nEndPageNum ||
            nEndPageNum > nCount) {
          return false;
        }
        for (int i = nStartPageNum; i <= nEndPageNum; ++i) {
          pageArray->push_back(i);
        }
      }
      nStringFrom = nStringTo + 1;
    }
  }
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_ImportPages(FPDF_DOCUMENT dest_doc,
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

  CPDF_PageOrganizer pageOrg;
  pageOrg.PDFDocInit(pDestDoc, pSrcDoc);
  return pageOrg.ExportPage(pSrcDoc, &pageArray, pDestDoc, index);
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc,
                                                       FPDF_DOCUMENT src_doc) {
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
