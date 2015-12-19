// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfdoc/fpdf_doc.h"

const int nMaxRecursion = 32;
int CPDF_Dest::GetPageIndex(CPDF_Document* pDoc) {
  CPDF_Array* pArray = ToArray(m_pObj);
  if (!pArray)
    return 0;

  CPDF_Object* pPage = pArray->GetElementValue(0);
  if (!pPage)
    return 0;
  if (pPage->IsNumber())
    return pPage->GetInteger();
  if (!pPage->IsDictionary())
    return 0;
  return pDoc->GetPageIndex(pPage->GetObjNum());
}
FX_DWORD CPDF_Dest::GetPageObjNum() {
  CPDF_Array* pArray = ToArray(m_pObj);
  if (!pArray)
    return 0;

  CPDF_Object* pPage = pArray->GetElementValue(0);
  if (!pPage)
    return 0;
  if (pPage->IsNumber())
    return pPage->GetInteger();
  if (pPage->IsDictionary())
    return pPage->GetObjNum();
  return 0;
}
const FX_CHAR* g_sZoomModes[] = {"XYZ",  "Fit",   "FitH",  "FitV", "FitR",
                                 "FitB", "FitBH", "FitBV", ""};
int CPDF_Dest::GetZoomMode() {
  CPDF_Array* pArray = ToArray(m_pObj);
  if (!pArray)
    return 0;

  CFX_ByteString mode;
  CPDF_Object* pObj = pArray->GetElementValue(1);
  mode = pObj ? pObj->GetString() : CFX_ByteString();
  int i = 0;
  while (g_sZoomModes[i][0] != '\0') {
    if (mode == g_sZoomModes[i]) {
      return i + 1;
    }
    i++;
  }
  return 0;
}
FX_FLOAT CPDF_Dest::GetParam(int index) {
  CPDF_Array* pArray = ToArray(m_pObj);
  return pArray ? pArray->GetNumber(2 + index) : 0;
}
CFX_ByteString CPDF_Dest::GetRemoteName() {
  return m_pObj ? m_pObj->GetString() : CFX_ByteString();
}
CPDF_NameTree::CPDF_NameTree(CPDF_Document* pDoc,
                             const CFX_ByteStringC& category) {
  if (pDoc->GetRoot() && pDoc->GetRoot()->GetDict("Names"))
    m_pRoot = pDoc->GetRoot()->GetDict("Names")->GetDict(category);
  else
    m_pRoot = NULL;
}
static CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode,
                                   const CFX_ByteString& csName,
                                   int& nIndex,
                                   CPDF_Array** ppFind,
                                   int nLevel = 0) {
  if (nLevel > nMaxRecursion) {
    return NULL;
  }
  CPDF_Array* pLimits = pNode->GetArray("Limits");
  if (pLimits) {
    CFX_ByteString csLeft = pLimits->GetString(0);
    CFX_ByteString csRight = pLimits->GetString(1);
    if (csLeft.Compare(csRight) > 0) {
      CFX_ByteString csTmp = csRight;
      csRight = csLeft;
      csLeft = csTmp;
    }
    if (csName.Compare(csLeft) < 0 || csName.Compare(csRight) > 0) {
      return NULL;
    }
  }
  CPDF_Array* pNames = pNode->GetArray("Names");
  if (pNames) {
    FX_DWORD dwCount = pNames->GetCount() / 2;
    for (FX_DWORD i = 0; i < dwCount; i++) {
      CFX_ByteString csValue = pNames->GetString(i * 2);
      int32_t iCompare = csValue.Compare(csName);
      if (iCompare <= 0) {
        if (ppFind) {
          *ppFind = pNames;
        }
        if (iCompare < 0) {
          continue;
        }
      } else {
        break;
      }
      nIndex += i;
      return pNames->GetElementValue(i * 2 + 1);
    }
    nIndex += dwCount;
    return NULL;
  }
  CPDF_Array* pKids = pNode->GetArray("Kids");
  if (!pKids) {
    return NULL;
  }
  for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDict(i);
    if (!pKid) {
      continue;
    }
    CPDF_Object* pFound =
        SearchNameNode(pKid, csName, nIndex, ppFind, nLevel + 1);
    if (pFound) {
      return pFound;
    }
  }
  return NULL;
}
static CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode,
                                   int nIndex,
                                   int& nCurIndex,
                                   CFX_ByteString& csName,
                                   CPDF_Array** ppFind,
                                   int nLevel = 0) {
  if (nLevel > nMaxRecursion) {
    return NULL;
  }
  CPDF_Array* pNames = pNode->GetArray("Names");
  if (pNames) {
    int nCount = pNames->GetCount() / 2;
    if (nIndex >= nCurIndex + nCount) {
      nCurIndex += nCount;
      return NULL;
    }
    if (ppFind) {
      *ppFind = pNames;
    }
    csName = pNames->GetString((nIndex - nCurIndex) * 2);
    return pNames->GetElementValue((nIndex - nCurIndex) * 2 + 1);
  }
  CPDF_Array* pKids = pNode->GetArray("Kids");
  if (!pKids) {
    return NULL;
  }
  for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDict(i);
    if (!pKid) {
      continue;
    }
    CPDF_Object* pFound =
        SearchNameNode(pKid, nIndex, nCurIndex, csName, ppFind, nLevel + 1);
    if (pFound) {
      return pFound;
    }
  }
  return NULL;
}
static int CountNames(CPDF_Dictionary* pNode, int nLevel = 0) {
  if (nLevel > nMaxRecursion) {
    return 0;
  }
  CPDF_Array* pNames = pNode->GetArray("Names");
  if (pNames) {
    return pNames->GetCount() / 2;
  }
  CPDF_Array* pKids = pNode->GetArray("Kids");
  if (!pKids) {
    return 0;
  }
  int nCount = 0;
  for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDict(i);
    if (!pKid) {
      continue;
    }
    nCount += CountNames(pKid, nLevel + 1);
  }
  return nCount;
}
int CPDF_NameTree::GetCount() const {
  if (!m_pRoot) {
    return 0;
  }
  return ::CountNames(m_pRoot);
}
int CPDF_NameTree::GetIndex(const CFX_ByteString& csName) const {
  if (!m_pRoot) {
    return -1;
  }
  int nIndex = 0;
  if (!SearchNameNode(m_pRoot, csName, nIndex, NULL)) {
    return -1;
  }
  return nIndex;
}
CPDF_Object* CPDF_NameTree::LookupValue(int nIndex,
                                        CFX_ByteString& csName) const {
  if (!m_pRoot) {
    return NULL;
  }
  int nCurIndex = 0;
  return SearchNameNode(m_pRoot, nIndex, nCurIndex, csName, NULL);
}
CPDF_Object* CPDF_NameTree::LookupValue(const CFX_ByteString& csName) const {
  if (!m_pRoot) {
    return NULL;
  }
  int nIndex = 0;
  return SearchNameNode(m_pRoot, csName, nIndex, NULL);
}
CPDF_Array* CPDF_NameTree::LookupNamedDest(CPDF_Document* pDoc,
                                           const CFX_ByteStringC& sName) {
  CPDF_Object* pValue = LookupValue(sName);
  if (!pValue) {
    CPDF_Dictionary* pDests = pDoc->GetRoot()->GetDict("Dests");
    if (!pDests)
      return nullptr;
    pValue = pDests->GetElementValue(sName);
  }
  if (!pValue)
    return nullptr;
  if (CPDF_Array* pArray = pValue->AsArray())
    return pArray;
  if (CPDF_Dictionary* pDict = pValue->AsDictionary())
    return pDict->GetArray("D");
  return nullptr;
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_ || \
    _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
static CFX_WideString ChangeSlashToPlatform(const FX_WCHAR* str) {
  CFX_WideString result;
  while (*str) {
    if (*str == '/') {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      result += ':';
#else
      result += '\\';
#endif
    } else {
      result += *str;
    }
    str++;
  }
  return result;
}
static CFX_WideString ChangeSlashToPDF(const FX_WCHAR* str) {
  CFX_WideString result;
  while (*str) {
    if (*str == '\\' || *str == ':') {
      result += '/';
    } else {
      result += *str;
    }
    str++;
  }
  return result;
}
#endif
static CFX_WideString FILESPEC_DecodeFileName(const CFX_WideStringC& filepath) {
  if (filepath.GetLength() <= 1) {
    return CFX_WideString();
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  if (filepath.Left(sizeof("/Mac") - 1) == CFX_WideStringC(L"/Mac")) {
    return ChangeSlashToPlatform(filepath.GetPtr() + 1);
  }
  return ChangeSlashToPlatform(filepath.GetPtr());
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  if (filepath.GetAt(0) != '/') {
    return ChangeSlashToPlatform(filepath.GetPtr());
  }
  if (filepath.GetAt(1) == '/') {
    return ChangeSlashToPlatform(filepath.GetPtr() + 1);
  }
  if (filepath.GetAt(2) == '/') {
    CFX_WideString result;
    result += filepath.GetAt(1);
    result += ':';
    result += ChangeSlashToPlatform(filepath.GetPtr() + 2);
    return result;
  }
  CFX_WideString result;
  result += '\\';
  result += ChangeSlashToPlatform(filepath.GetPtr());
  return result;
#else
  return filepath;
#endif
}
FX_BOOL CPDF_FileSpec::GetFileName(CFX_WideString& csFileName) const {
  if (!m_pObj) {
    return FALSE;
  }
  if (CPDF_Dictionary* pDict = m_pObj->AsDictionary()) {
    csFileName = pDict->GetUnicodeText("UF");
    if (csFileName.IsEmpty()) {
      csFileName = CFX_WideString::FromLocal(pDict->GetString("F"));
    }
    if (pDict->GetString("FS") == "URL") {
      return TRUE;
    }
    if (csFileName.IsEmpty()) {
      if (pDict->KeyExist("DOS")) {
        csFileName = CFX_WideString::FromLocal(pDict->GetString("DOS"));
      } else if (pDict->KeyExist("Mac")) {
        csFileName = CFX_WideString::FromLocal(pDict->GetString("Mac"));
      } else if (pDict->KeyExist("Unix")) {
        csFileName = CFX_WideString::FromLocal(pDict->GetString("Unix"));
      } else {
        return FALSE;
      }
    }
  } else {
    csFileName = CFX_WideString::FromLocal(m_pObj->GetString());
  }
  csFileName = FILESPEC_DecodeFileName(csFileName);
  return TRUE;
}
CPDF_FileSpec::CPDF_FileSpec() {
  m_pObj = new CPDF_Dictionary;
  if (CPDF_Dictionary* pDict = ToDictionary(m_pObj)) {
    pDict->SetAtName("Type", "Filespec");
  }
}
FX_BOOL CPDF_FileSpec::IsURL() const {
  if (CPDF_Dictionary* pDict = ToDictionary(m_pObj)) {
    return pDict->GetString("FS") == "URL";
  }
  return FALSE;
}
CFX_WideString FILESPEC_EncodeFileName(const CFX_WideStringC& filepath) {
  if (filepath.GetLength() <= 1) {
    return CFX_WideString();
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  if (filepath.GetAt(1) == ':') {
    CFX_WideString result;
    result = '/';
    result += filepath.GetAt(0);
    if (filepath.GetAt(2) != '\\') {
      result += '/';
    }
    result += ChangeSlashToPDF(filepath.GetPtr() + 2);
    return result;
  }
  if (filepath.GetAt(0) == '\\' && filepath.GetAt(1) == '\\') {
    return ChangeSlashToPDF(filepath.GetPtr() + 1);
  }
  if (filepath.GetAt(0) == '\\') {
    CFX_WideString result;
    result = '/';
    result += ChangeSlashToPDF(filepath.GetPtr());
    return result;
  }
  return ChangeSlashToPDF(filepath.GetPtr());
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  if (filepath.Left(sizeof("Mac") - 1) == FX_WSTRC(L"Mac")) {
    CFX_WideString result;
    result = '/';
    result += ChangeSlashToPDF(filepath.GetPtr());
    return result;
  }
  return ChangeSlashToPDF(filepath.GetPtr());
#else
  return filepath;
#endif
}
CPDF_Stream* CPDF_FileSpec::GetFileStream() const {
  if (!m_pObj)
    return nullptr;
  if (CPDF_Stream* pStream = m_pObj->AsStream())
    return pStream;
  if (CPDF_Dictionary* pEF = m_pObj->AsDictionary()->GetDict("EF"))
    return pEF->GetStream("F");
  return nullptr;
}
static void FPDFDOC_FILESPEC_SetFileName(CPDF_Object* pObj,
                                         const CFX_WideStringC& wsFileName,
                                         FX_BOOL bURL) {
  CFX_WideString wsStr;
  if (bURL) {
    wsStr = wsFileName;
  } else {
    wsStr = FILESPEC_EncodeFileName(wsFileName);
  }
  if (pObj->IsString()) {
    pObj->SetString(CFX_ByteString::FromUnicode(wsStr));
  } else if (CPDF_Dictionary* pDict = pObj->AsDictionary()) {
    pDict->SetAtString("F", CFX_ByteString::FromUnicode(wsStr));
    pDict->SetAtString("UF", PDF_EncodeText(wsStr));
  }
}
void CPDF_FileSpec::SetFileName(const CFX_WideStringC& wsFileName,
                                FX_BOOL bURL) {
  if (bURL) {
    if (CPDF_Dictionary* pDict = m_pObj->AsDictionary()) {
      pDict->SetAtName("FS", "URL");
    }
  }
  FPDFDOC_FILESPEC_SetFileName(m_pObj, wsFileName, bURL);
}
static CFX_WideString _MakeRoman(int num) {
  const int arabic[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
  const CFX_WideString roman[] = {L"m",  L"cm", L"d",  L"cd", L"c",
                                  L"xc", L"l",  L"xl", L"x",  L"ix",
                                  L"v",  L"iv", L"i"};
  const int nMaxNum = 1000000;
  num %= nMaxNum;
  int i = 0;
  CFX_WideString wsRomanNumber;
  while (num > 0) {
    while (num >= arabic[i]) {
      num = num - arabic[i];
      wsRomanNumber += roman[i];
    }
    i = i + 1;
  }
  return wsRomanNumber;
}
static CFX_WideString _MakeLetters(int num) {
  if (num == 0) {
    return CFX_WideString();
  }
  CFX_WideString wsLetters;
  const int nMaxCount = 1000;
  const int nLetterCount = 26;
  num -= 1;
  int count = num / nLetterCount + 1;
  count %= nMaxCount;
  FX_WCHAR ch = L'a' + num % nLetterCount;
  for (int i = 0; i < count; i++) {
    wsLetters += ch;
  }
  return wsLetters;
}
static CFX_WideString _GetLabelNumPortion(int num,
                                          const CFX_ByteString& bsStyle) {
  CFX_WideString wsNumPortion;
  if (bsStyle.IsEmpty()) {
    return wsNumPortion;
  }
  if (bsStyle == "D") {
    wsNumPortion.Format(L"%d", num);
  } else if (bsStyle == "R") {
    wsNumPortion = _MakeRoman(num);
    wsNumPortion.MakeUpper();
  } else if (bsStyle == "r") {
    wsNumPortion = _MakeRoman(num);
  } else if (bsStyle == "A") {
    wsNumPortion = _MakeLetters(num);
    wsNumPortion.MakeUpper();
  } else if (bsStyle == "a") {
    wsNumPortion = _MakeLetters(num);
  }
  return wsNumPortion;
}
CFX_WideString CPDF_PageLabel::GetLabel(int nPage) const {
  CFX_WideString wsLabel;
  if (!m_pDocument) {
    return wsLabel;
  }
  CPDF_Dictionary* pPDFRoot = m_pDocument->GetRoot();
  if (!pPDFRoot) {
    return wsLabel;
  }
  CPDF_Dictionary* pLabels = pPDFRoot->GetDict("PageLabels");
  CPDF_NumberTree numberTree(pLabels);
  CPDF_Object* pValue = NULL;
  int n = nPage;
  while (n >= 0) {
    pValue = numberTree.LookupValue(n);
    if (pValue) {
      break;
    }
    n--;
  }
  if (pValue) {
    pValue = pValue->GetDirect();
    if (CPDF_Dictionary* pLabel = pValue->AsDictionary()) {
      if (pLabel->KeyExist("P")) {
        wsLabel += pLabel->GetUnicodeText("P");
      }
      CFX_ByteString bsNumberingStyle = pLabel->GetString("S", NULL);
      int nLabelNum = nPage - n + pLabel->GetInteger("St", 1);
      CFX_WideString wsNumPortion =
          _GetLabelNumPortion(nLabelNum, bsNumberingStyle);
      wsLabel += wsNumPortion;
      return wsLabel;
    }
  }
  wsLabel.Format(L"%d", nPage + 1);
  return wsLabel;
}
int32_t CPDF_PageLabel::GetPageByLabel(const CFX_ByteStringC& bsLabel) const {
  if (!m_pDocument) {
    return -1;
  }
  CPDF_Dictionary* pPDFRoot = m_pDocument->GetRoot();
  if (!pPDFRoot) {
    return -1;
  }
  int nPages = m_pDocument->GetPageCount();
  CFX_ByteString bsLbl;
  CFX_ByteString bsOrig = bsLabel;
  for (int i = 0; i < nPages; i++) {
    bsLbl = PDF_EncodeText(GetLabel(i));
    if (!bsLbl.Compare(bsOrig)) {
      return i;
    }
  }
  bsLbl = bsOrig;
  int nPage = FXSYS_atoi(bsLbl);
  if (nPage > 0 && nPage <= nPages) {
    return nPage;
  }
  return -1;
}
int32_t CPDF_PageLabel::GetPageByLabel(const CFX_WideStringC& wsLabel) const {
  CFX_ByteString bsLabel = PDF_EncodeText(wsLabel.GetPtr());
  return GetPageByLabel(bsLabel);
}
