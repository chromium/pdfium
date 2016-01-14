// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_ppo.h"

#include <memory>

#include "fpdfsdk/include/fsdk_define.h"

class CPDF_PageOrganizer {
 public:
  using ObjectNumberMap = std::map<FX_DWORD, FX_DWORD>;
  CPDF_PageOrganizer();
  ~CPDF_PageOrganizer();

  FX_BOOL PDFDocInit(CPDF_Document* pDestPDFDoc, CPDF_Document* pSrcPDFDoc);
  FX_BOOL ExportPage(CPDF_Document* pSrcPDFDoc,
                     CFX_WordArray* nPageNum,
                     CPDF_Document* pDestPDFDoc,
                     int nIndex);
  CPDF_Object* PageDictGetInheritableTag(CPDF_Dictionary* pDict,
                                         CFX_ByteString nSrctag);
  FX_BOOL UpdateReference(CPDF_Object* pObj,
                          CPDF_Document* pDoc,
                          ObjectNumberMap* pObjNumberMap);
  FX_DWORD GetNewObjId(CPDF_Document* pDoc,
                       ObjectNumberMap* pObjNumberMap,
                       CPDF_Reference* pRef);
};

CPDF_PageOrganizer::CPDF_PageOrganizer() {}

CPDF_PageOrganizer::~CPDF_PageOrganizer() {}

FX_BOOL CPDF_PageOrganizer::PDFDocInit(CPDF_Document* pDestPDFDoc,
                                       CPDF_Document* pSrcPDFDoc) {
  if (!pDestPDFDoc || !pSrcPDFDoc)
    return FALSE;

  CPDF_Dictionary* pNewRoot = pDestPDFDoc->GetRoot();
  if (!pNewRoot)
    return FALSE;

  // Set the document information
  CPDF_Dictionary* DInfoDict = pDestPDFDoc->GetInfo();
  if (!DInfoDict)
    return FALSE;

  CFX_ByteString producerstr;
  producerstr.Format("PDFium");
  DInfoDict->SetAt("Producer", new CPDF_String(producerstr, FALSE));

  // Set type
  CFX_ByteString cbRootType = pNewRoot->GetString("Type", "");
  if (cbRootType.Equal("")) {
    pNewRoot->SetAt("Type", new CPDF_Name("Catalog"));
  }

  CPDF_Object* pElement = pNewRoot->GetElement("Pages");
  CPDF_Dictionary* pNewPages =
      pElement ? ToDictionary(pElement->GetDirect()) : nullptr;
  if (!pNewPages) {
    pNewPages = new CPDF_Dictionary;
    FX_DWORD NewPagesON = pDestPDFDoc->AddIndirectObject(pNewPages);
    pNewRoot->SetAt("Pages", new CPDF_Reference(pDestPDFDoc, NewPagesON));
  }

  CFX_ByteString cbPageType = pNewPages->GetString("Type", "");
  if (cbPageType.Equal("")) {
    pNewPages->SetAt("Type", new CPDF_Name("Pages"));
  }

  CPDF_Array* pKeysArray = pNewPages->GetArray("Kids");
  if (!pKeysArray) {
    CPDF_Array* pNewKids = new CPDF_Array;
    FX_DWORD Kidsobjnum = -1;
    Kidsobjnum = pDestPDFDoc->AddIndirectObject(pNewKids);

    pNewPages->SetAt("Kids", new CPDF_Reference(pDestPDFDoc, Kidsobjnum));
    pNewPages->SetAt("Count", new CPDF_Number(0));
  }

  return TRUE;
}

FX_BOOL CPDF_PageOrganizer::ExportPage(CPDF_Document* pSrcPDFDoc,
                                       CFX_WordArray* nPageNum,
                                       CPDF_Document* pDestPDFDoc,
                                       int nIndex) {
  int curpage = nIndex;

  std::unique_ptr<ObjectNumberMap> pObjNumberMap(new ObjectNumberMap);

  for (int i = 0; i < nPageNum->GetSize(); ++i) {
    CPDF_Dictionary* pCurPageDict = pDestPDFDoc->CreateNewPage(curpage);
    CPDF_Dictionary* pSrcPageDict = pSrcPDFDoc->GetPage(nPageNum->GetAt(i) - 1);
    if (!pSrcPageDict || !pCurPageDict)
      return FALSE;

    // Clone the page dictionary
    for (const auto& it : *pSrcPageDict) {
      const CFX_ByteString& cbSrcKeyStr = it.first;
      CPDF_Object* pObj = it.second;
      if (cbSrcKeyStr.Compare(("Type")) && cbSrcKeyStr.Compare(("Parent"))) {
        if (pCurPageDict->KeyExist(cbSrcKeyStr))
          pCurPageDict->RemoveAt(cbSrcKeyStr);
        pCurPageDict->SetAt(cbSrcKeyStr, pObj->Clone());
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
          pCurPageDict->SetAt("MediaBox", pInheritable->Clone());
        } else {
          // Make the default size to be letter size (8.5'x11')
          CPDF_Array* pArray = new CPDF_Array;
          pArray->AddNumber(0);
          pArray->AddNumber(0);
          pArray->AddNumber(612);
          pArray->AddNumber(792);
          pCurPageDict->SetAt("MediaBox", pArray);
        }
      } else {
        pCurPageDict->SetAt("MediaBox", pInheritable->Clone());
      }
    }
    // 2 Resources //required
    if (!pCurPageDict->KeyExist("Resources")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "Resources");
      if (!pInheritable)
        return FALSE;
      pCurPageDict->SetAt("Resources", pInheritable->Clone());
    }
    // 3 CropBox  //Optional
    if (!pCurPageDict->KeyExist("CropBox")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "CropBox");
      if (pInheritable)
        pCurPageDict->SetAt("CropBox", pInheritable->Clone());
    }
    // 4 Rotate  //Optional
    if (!pCurPageDict->KeyExist("Rotate")) {
      pInheritable = PageDictGetInheritableTag(pSrcPageDict, "Rotate");
      if (pInheritable)
        pCurPageDict->SetAt("Rotate", pInheritable->Clone());
    }

    // Update the reference
    FX_DWORD dwOldPageObj = pSrcPageDict->GetObjNum();
    FX_DWORD dwNewPageObj = pCurPageDict->GetObjNum();

    (*pObjNumberMap)[dwOldPageObj] = dwNewPageObj;

    UpdateReference(pCurPageDict, pDestPDFDoc, pObjNumberMap.get());
    ++curpage;
  }

  return TRUE;
}

CPDF_Object* CPDF_PageOrganizer::PageDictGetInheritableTag(
    CPDF_Dictionary* pDict,
    CFX_ByteString nSrctag) {
  if (!pDict || nSrctag.IsEmpty())
    return nullptr;
  if (!pDict->KeyExist("Parent") || !pDict->KeyExist("Type"))
    return nullptr;

  CPDF_Object* pType = pDict->GetElement("Type")->GetDirect();
  if (!ToName(pType))
    return nullptr;
  if (pType->GetString().Compare("Page"))
    return nullptr;

  CPDF_Dictionary* pp = ToDictionary(pDict->GetElement("Parent")->GetDirect());
  if (!pp)
    return nullptr;

  if (pDict->KeyExist((const char*)nSrctag))
    return pDict->GetElement((const char*)nSrctag);

  while (pp) {
    if (pp->KeyExist((const char*)nSrctag))
      return pp->GetElement((const char*)nSrctag);
    if (!pp->KeyExist("Parent"))
      break;
    pp = ToDictionary(pp->GetElement("Parent")->GetDirect());
  }
  return nullptr;
}

FX_BOOL CPDF_PageOrganizer::UpdateReference(CPDF_Object* pObj,
                                            CPDF_Document* pDoc,
                                            ObjectNumberMap* pObjNumberMap) {
  switch (pObj->GetType()) {
    case PDFOBJ_REFERENCE: {
      CPDF_Reference* pReference = pObj->AsReference();
      FX_DWORD newobjnum = GetNewObjId(pDoc, pObjNumberMap, pReference);
      if (newobjnum == 0)
        return FALSE;
      pReference->SetRef(pDoc, newobjnum);
      break;
    }
    case PDFOBJ_DICTIONARY: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      auto it = pDict->begin();
      while (it != pDict->end()) {
        const CFX_ByteString& key = it->first;
        CPDF_Object* pNextObj = it->second;
        ++it;
        if (!FXSYS_strcmp(key, "Parent") || !FXSYS_strcmp(key, "Prev") ||
            !FXSYS_strcmp(key, "First")) {
          continue;
        }
        if (pNextObj) {
          if (!UpdateReference(pNextObj, pDoc, pObjNumberMap))
            pDict->RemoveAt(key);
        } else {
          return FALSE;
        }
      }
      break;
    }
    case PDFOBJ_ARRAY: {
      CPDF_Array* pArray = pObj->AsArray();
      FX_DWORD count = pArray->GetCount();
      for (FX_DWORD i = 0; i < count; ++i) {
        CPDF_Object* pNextObj = pArray->GetElement(i);
        if (!pNextObj)
          return FALSE;
        if (!UpdateReference(pNextObj, pDoc, pObjNumberMap))
          return FALSE;
      }
      break;
    }
    case PDFOBJ_STREAM: {
      CPDF_Stream* pStream = pObj->AsStream();
      CPDF_Dictionary* pDict = pStream->GetDict();
      if (pDict) {
        if (!UpdateReference(pDict, pDoc, pObjNumberMap))
          return FALSE;
      } else {
        return FALSE;
      }
      break;
    }
    default:
      break;
  }

  return TRUE;
}

FX_DWORD CPDF_PageOrganizer::GetNewObjId(CPDF_Document* pDoc,
                                         ObjectNumberMap* pObjNumberMap,
                                         CPDF_Reference* pRef) {
  if (!pRef)
    return 0;

  FX_DWORD dwObjnum = pRef->GetRefObjNum();
  FX_DWORD dwNewObjNum = 0;
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
      CFX_ByteString strType = pDictClone->GetString("Type");
      if (!FXSYS_stricmp(strType, "Pages")) {
        pDictClone->Release();
        return 4;
      }
      if (!FXSYS_stricmp(strType, "Page")) {
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
                                CFX_WordArray* pageArray,
                                int nCount) {
  if (rangstring.GetLength() != 0) {
    rangstring.Remove(' ');
    int nLength = rangstring.GetLength();
    CFX_ByteString cbCompareString("0123456789-,");
    for (int i = 0; i < nLength; ++i) {
      if (cbCompareString.Find(rangstring[i]) == -1)
        return FALSE;
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
        long lPageNum = atol(cbMidRange);
        if (lPageNum <= 0 || lPageNum > nCount)
          return FALSE;
        pageArray->Add((FX_WORD)lPageNum);
      } else {
        int nStartPageNum = atol(cbMidRange.Mid(0, nMid));
        if (nStartPageNum == 0)
          return FALSE;

        ++nMid;
        int nEnd = cbMidRange.GetLength() - nMid;
        if (nEnd == 0)
          return FALSE;

        int nEndPageNum = atol(cbMidRange.Mid(nMid, nEnd));
        if (nStartPageNum < 0 || nStartPageNum > nEndPageNum ||
            nEndPageNum > nCount) {
          return FALSE;
        }
        for (int i = nStartPageNum; i <= nEndPageNum; ++i) {
          pageArray->Add(i);
        }
      }
      nStringFrom = nStringTo + 1;
    }
  }
  return TRUE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_ImportPages(FPDF_DOCUMENT dest_doc,
                                             FPDF_DOCUMENT src_doc,
                                             FPDF_BYTESTRING pagerange,
                                             int index) {
  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest_doc)
    return FALSE;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return FALSE;

  CFX_WordArray pageArray;
  int nCount = pSrcDoc->GetPageCount();
  if (pagerange) {
    if (!ParserPageRangeString(pagerange, &pageArray, nCount))
      return FALSE;
  } else {
    for (int i = 1; i <= nCount; ++i) {
      pageArray.Add(i);
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
    return FALSE;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return FALSE;

  CPDF_Dictionary* pSrcDict = pSrcDoc->GetRoot();
  pSrcDict = pSrcDict->GetDict("ViewerPreferences");
  if (!pSrcDict)
    return FALSE;

  CPDF_Dictionary* pDstDict = pDstDoc->GetRoot();
  if (!pDstDict)
    return FALSE;

  pDstDict->SetAt("ViewerPreferences", pSrcDict->Clone(TRUE));
  return TRUE;
}
