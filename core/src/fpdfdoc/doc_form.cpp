// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfdoc/fpdf_doc.h"
#include "doc_utils.h"
#include "third_party/base/stl_util.h"

const int nMaxRecursion = 32;

class CFieldNameExtractor {
 public:
  explicit CFieldNameExtractor(const CFX_WideString& full_name) {
    m_pStart = full_name.c_str();
    m_pEnd = m_pStart + full_name.GetLength();
    m_pCur = m_pStart;
  }
  void GetNext(const FX_WCHAR*& pSubName, FX_STRSIZE& size) {
    pSubName = m_pCur;
    while (m_pCur < m_pEnd && m_pCur[0] != L'.') {
      m_pCur++;
    }
    size = (FX_STRSIZE)(m_pCur - pSubName);
    if (m_pCur < m_pEnd && m_pCur[0] == L'.') {
      m_pCur++;
    }
  }

 protected:
  const FX_WCHAR* m_pStart;
  const FX_WCHAR* m_pEnd;
  const FX_WCHAR* m_pCur;
};
class CFieldTree {
 public:
  struct _Node {
    _Node* parent;
    CFX_ArrayTemplate<_Node*> children;
    CFX_WideString short_name;
    CPDF_FormField* field_ptr;
    int CountFields(int nLevel = 0) {
      if (nLevel > nMaxRecursion) {
        return 0;
      }
      if (field_ptr) {
        return 1;
      }
      int count = 0;
      for (int i = 0; i < children.GetSize(); i++) {
        count += children.GetAt(i)->CountFields(nLevel + 1);
      }
      return count;
    }
    CPDF_FormField* GetField(int* fields_to_go) {
      if (field_ptr) {
        if (*fields_to_go == 0) {
          return field_ptr;
        }
        --*fields_to_go;
        return NULL;
      }
      for (int i = 0; i < children.GetSize(); i++) {
        if (CPDF_FormField* pField = children.GetAt(i)->GetField(fields_to_go))
          return pField;
      }
      return NULL;
    }
    CPDF_FormField* GetField(int index) {
      int fields_to_go = index;
      return GetField(&fields_to_go);
    }
  };
  CFieldTree();
  ~CFieldTree();
  void SetField(const CFX_WideString& full_name, CPDF_FormField* field_ptr);
  CPDF_FormField* GetField(const CFX_WideString& full_name);
  CPDF_FormField* RemoveField(const CFX_WideString& full_name);
  void RemoveAll();
  _Node* FindNode(const CFX_WideString& full_name);
  _Node* AddChild(_Node* pParent,
                  const CFX_WideString& short_name,
                  CPDF_FormField* field_ptr);
  void RemoveNode(_Node* pNode, int nLevel = 0);
  _Node* _Lookup(_Node* pParent, const CFX_WideString& short_name);
  _Node m_Root;
};
CFieldTree::CFieldTree() {
  m_Root.parent = NULL;
  m_Root.field_ptr = NULL;
}
CFieldTree::~CFieldTree() {
  RemoveAll();
}
CFieldTree::_Node* CFieldTree::AddChild(_Node* pParent,
                                        const CFX_WideString& short_name,
                                        CPDF_FormField* field_ptr) {
  if (!pParent) {
    return NULL;
  }
  _Node* pNode = new _Node;
  pNode->parent = pParent;
  pNode->short_name = short_name;
  pNode->field_ptr = field_ptr;
  pParent->children.Add(pNode);
  return pNode;
}
void CFieldTree::RemoveNode(_Node* pNode, int nLevel) {
  if (!pNode) {
    return;
  }
  if (nLevel <= nMaxRecursion) {
    for (int i = 0; i < pNode->children.GetSize(); i++) {
      RemoveNode(pNode->children[i], nLevel + 1);
    }
  }
  delete pNode;
}
CFieldTree::_Node* CFieldTree::_Lookup(_Node* pParent,
                                       const CFX_WideString& short_name) {
  if (!pParent) {
    return NULL;
  }
  for (int i = 0; i < pParent->children.GetSize(); i++) {
    _Node* pNode = pParent->children[i];
    if (pNode->short_name.GetLength() == short_name.GetLength() &&
        FXSYS_memcmp(pNode->short_name.c_str(), short_name.c_str(),
                     short_name.GetLength() * sizeof(FX_WCHAR)) == 0) {
      return pNode;
    }
  }
  return NULL;
}
void CFieldTree::RemoveAll() {
  for (int i = 0; i < m_Root.children.GetSize(); i++) {
    RemoveNode(m_Root.children[i]);
  }
}
void CFieldTree::SetField(const CFX_WideString& full_name,
                          CPDF_FormField* field_ptr) {
  if (full_name == L"") {
    return;
  }
  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  _Node *pNode = &m_Root, *pLast = NULL;
  while (nLength > 0) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = _Lookup(pLast, name);
    if (!pNode) {
      pNode = AddChild(pLast, name, NULL);
    }
    name_extractor.GetNext(pName, nLength);
  }
  if (pNode != &m_Root) {
    pNode->field_ptr = field_ptr;
  }
}
CPDF_FormField* CFieldTree::GetField(const CFX_WideString& full_name) {
  if (full_name == L"") {
    return NULL;
  }
  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  _Node *pNode = &m_Root, *pLast = NULL;
  while (nLength > 0 && pNode) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = _Lookup(pLast, name);
    name_extractor.GetNext(pName, nLength);
  }
  return pNode ? pNode->field_ptr : NULL;
}
CPDF_FormField* CFieldTree::RemoveField(const CFX_WideString& full_name) {
  if (full_name == L"") {
    return NULL;
  }
  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  _Node *pNode = &m_Root, *pLast = NULL;
  while (nLength > 0 && pNode) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = _Lookup(pLast, name);
    name_extractor.GetNext(pName, nLength);
  }
  if (pNode && pNode != &m_Root) {
    for (int i = 0; i < pLast->children.GetSize(); i++) {
      if (pNode == pLast->children[i]) {
        pLast->children.RemoveAt(i);
        break;
      }
    }
    CPDF_FormField* pField = pNode->field_ptr;
    RemoveNode(pNode);
    return pField;
  }
  return NULL;
}
CFieldTree::_Node* CFieldTree::FindNode(const CFX_WideString& full_name) {
  if (full_name == L"") {
    return NULL;
  }
  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  _Node *pNode = &m_Root, *pLast = NULL;
  while (nLength > 0 && pNode) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = _Lookup(pLast, name);
    name_extractor.GetNext(pName, nLength);
  }
  return pNode;
}
CPDF_InterForm::CPDF_InterForm(CPDF_Document* pDocument, FX_BOOL bGenerateAP)
    : CFX_PrivateData(),
      m_pDocument(pDocument),
      m_bGenerateAP(bGenerateAP),
      m_pFormDict(nullptr),
      m_pFieldTree(new CFieldTree),
      m_pFormNotify(nullptr),
      m_bUpdated(FALSE) {
  CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  if (!pRoot)
    return;

  m_pFormDict = pRoot->GetDict("AcroForm");
  if (!m_pFormDict)
    return;

  CPDF_Array* pFields = m_pFormDict->GetArray("Fields");
  if (!pFields)
    return;

  int count = pFields->GetCount();
  for (int i = 0; i < count; i++) {
    LoadField(pFields->GetDict(i));
  }
}

CPDF_InterForm::~CPDF_InterForm() {
  for (auto it : m_ControlMap)
    delete it.second;

  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; ++i) {
    delete m_pFieldTree->m_Root.GetField(i);
  }
}

FX_BOOL CPDF_InterForm::m_bUpdateAP = TRUE;
FX_BOOL CPDF_InterForm::UpdatingAPEnabled() {
  return m_bUpdateAP;
}
void CPDF_InterForm::EnableUpdateAP(FX_BOOL bUpdateAP) {
  m_bUpdateAP = bUpdateAP;
}
CFX_ByteString CPDF_InterForm::GenerateNewResourceName(
    const CPDF_Dictionary* pResDict,
    const FX_CHAR* csType,
    int iMinLen,
    const FX_CHAR* csPrefix) {
  CFX_ByteString csStr = csPrefix;
  CFX_ByteString csBType = csType;
  if (csStr.IsEmpty()) {
    if (csBType == "ExtGState") {
      csStr = "GS";
    } else if (csBType == "ColorSpace") {
      csStr = "CS";
    } else if (csBType == "Font") {
      csStr = "ZiTi";
    } else {
      csStr = "Res";
    }
  }
  CFX_ByteString csTmp = csStr;
  int iCount = csStr.GetLength();
  int m = 0;
  if (iMinLen > 0) {
    csTmp = "";
    while (m < iMinLen && m < iCount) {
      csTmp += csStr[m++];
    }
    while (m < iMinLen) {
      csTmp += '0' + m % 10;
      m++;
    }
  } else {
    m = iCount;
  }
  if (!pResDict) {
    return csTmp;
  }
  CPDF_Dictionary* pDict = pResDict->GetDict(csType);
  if (!pDict) {
    return csTmp;
  }
  int num = 0;
  CFX_ByteString bsNum;
  while (TRUE) {
    if (!pDict->KeyExist(csTmp + bsNum)) {
      return csTmp + bsNum;
    }
    if (m < iCount) {
      csTmp += csStr[m++];
    } else {
      bsNum.Format("%d", num++);
    }
    m++;
  }
  return csTmp;
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
typedef struct _PDF_FONTDATA {
  FX_BOOL bFind;
  LOGFONTA lf;
} PDF_FONTDATA, FAR* LPDF_FONTDATA;
static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXA* lpelfe,
                                      NEWTEXTMETRICEX* lpntme,
                                      DWORD FontType,
                                      LPARAM lParam) {
  if (FontType != 0x004 || strchr(lpelfe->elfLogFont.lfFaceName, '@')) {
    return 1;
  }
  LPDF_FONTDATA pData = (LPDF_FONTDATA)lParam;
  memcpy(&pData->lf, &lpelfe->elfLogFont, sizeof(LOGFONTA));
  pData->bFind = TRUE;
  return 0;
}
static FX_BOOL RetrieveSpecificFont(LOGFONTA& lf) {
  PDF_FONTDATA fd;
  memset(&fd, 0, sizeof(PDF_FONTDATA));
  HDC hDC = ::GetDC(NULL);
  EnumFontFamiliesExA(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc, (LPARAM)&fd,
                      0);
  ::ReleaseDC(NULL, hDC);
  if (fd.bFind) {
    memcpy(&lf, &fd.lf, sizeof(LOGFONTA));
  }
  return fd.bFind;
}
static FX_BOOL RetrieveSpecificFont(uint8_t charSet,
                                    uint8_t pitchAndFamily,
                                    LPCSTR pcsFontName,
                                    LOGFONTA& lf) {
  memset(&lf, 0, sizeof(LOGFONTA));
  lf.lfCharSet = charSet;
  lf.lfPitchAndFamily = pitchAndFamily;
  if (pcsFontName) {
    strcpy(lf.lfFaceName, pcsFontName);
  }
  return RetrieveSpecificFont(lf);
}
#ifdef PDF_ENABLE_XFA
static FX_BOOL RetrieveStockFont(int iFontObject,
                                 uint8_t charSet,
                                 LOGFONTA& lf) {
  HFONT hFont = (HFONT)::GetStockObject(iFontObject);
  if (hFont != NULL) {
    memset(&lf, 0, sizeof(LOGFONTA));
    int iRet = ::GetObject(hFont, sizeof(LOGFONTA), &lf);
    if (iRet > 0 && (lf.lfCharSet == charSet || charSet == 255)) {
      return RetrieveSpecificFont(lf);
    }
  }
  return FALSE;
}
#endif  // PDF_ENABLE_XFA
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

CPDF_Font* CPDF_InterForm::AddStandardFont(CPDF_Document* pDocument,
                                           CFX_ByteString csFontName) {
  if (!pDocument || csFontName.IsEmpty())
    return nullptr;

  if (csFontName == "ZapfDingbats")
    return pDocument->AddStandardFont(csFontName.c_str(), nullptr);

  CPDF_FontEncoding encoding(PDFFONT_ENCODING_WINANSI);
  return pDocument->AddStandardFont(csFontName.c_str(), &encoding);
}

CFX_ByteString CPDF_InterForm::GetNativeFont(uint8_t charSet, void* pLogFont) {
  CFX_ByteString csFontName;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  LOGFONTA lf;
  FX_BOOL bRet;
  if (charSet == ANSI_CHARSET) {
    csFontName = "Helvetica";
    return csFontName;
  }
  bRet = FALSE;
  if (charSet == SHIFTJIS_CHARSET) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE,
                                "MS Mincho", lf);
  } else if (charSet == GB2312_CHARSET) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "SimSun",
                                lf);
  } else if (charSet == CHINESEBIG5_CHARSET) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "MingLiU",
                                lf);
  }
  if (!bRet) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE,
                                "Arial Unicode MS", lf);
  }
  if (!bRet) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE,
                                "Microsoft Sans Serif", lf);
  }
  if (!bRet) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, NULL, lf);
  }
  if (bRet) {
    if (pLogFont) {
      memcpy(pLogFont, &lf, sizeof(LOGFONTA));
    }
    csFontName = lf.lfFaceName;
    return csFontName;
  }
#endif
  return csFontName;
}
CFX_ByteString CPDF_InterForm::GetNativeFont(void* pLogFont) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  uint8_t charSet = GetNativeCharSet();
  return GetNativeFont(charSet, pLogFont);
#else
  return CFX_ByteString();
#endif
}
uint8_t CPDF_InterForm::GetNativeCharSet() {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  uint8_t charSet = ANSI_CHARSET;
  UINT iCodePage = ::GetACP();
  switch (iCodePage) {
    case 932:
      charSet = SHIFTJIS_CHARSET;
      break;
    case 936:
      charSet = GB2312_CHARSET;
      break;
    case 950:
      charSet = CHINESEBIG5_CHARSET;
      break;
    case 1252:
      charSet = ANSI_CHARSET;
      break;
    case 874:
      charSet = THAI_CHARSET;
      break;
    case 949:
      charSet = HANGUL_CHARSET;
      break;
    case 1200:
      charSet = ANSI_CHARSET;
      break;
    case 1250:
      charSet = EASTEUROPE_CHARSET;
      break;
    case 1251:
      charSet = RUSSIAN_CHARSET;
      break;
    case 1253:
      charSet = GREEK_CHARSET;
      break;
    case 1254:
      charSet = TURKISH_CHARSET;
      break;
    case 1255:
      charSet = HEBREW_CHARSET;
      break;
    case 1256:
      charSet = ARABIC_CHARSET;
      break;
    case 1257:
      charSet = BALTIC_CHARSET;
      break;
    case 1258:
      charSet = VIETNAMESE_CHARSET;
      break;
    case 1361:
      charSet = JOHAB_CHARSET;
      break;
  }
  return charSet;
#else
  return 0;
#endif
}

CPDF_Font* CPDF_InterForm::AddNativeFont(uint8_t charSet,
                                         CPDF_Document* pDocument) {
  if (!pDocument)
    return nullptr;

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  LOGFONTA lf;
  CFX_ByteString csFontName = GetNativeFont(charSet, &lf);
  if (!csFontName.IsEmpty()) {
    if (csFontName == "Helvetica")
      return AddStandardFont(pDocument, csFontName);
    return pDocument->AddWindowsFont(&lf, FALSE, TRUE);
  }
#endif
  return nullptr;
}

CPDF_Font* CPDF_InterForm::AddNativeFont(CPDF_Document* pDocument) {
  return pDocument ? AddNativeFont(GetNativeCharSet(), pDocument) : nullptr;
}

FX_BOOL CPDF_InterForm::ValidateFieldName(
    CFX_WideString& csNewFieldName,
    int iType,
    const CPDF_FormField* pExcludedField,
    const CPDF_FormControl* pExcludedControl) {
  if (csNewFieldName.IsEmpty()) {
    return FALSE;
  }
  int iPos = 0;
  int iLength = csNewFieldName.GetLength();
  CFX_WideString csSub;
  while (TRUE) {
    while (iPos < iLength &&
           (csNewFieldName[iPos] == L'.' || csNewFieldName[iPos] == L' ')) {
      iPos++;
    }
    if (iPos < iLength && !csSub.IsEmpty()) {
      csSub += L'.';
    }
    while (iPos < iLength && csNewFieldName[iPos] != L'.') {
      csSub += csNewFieldName[iPos++];
    }
    for (int i = csSub.GetLength() - 1; i > -1; i--) {
      if (csSub[i] == L' ' || csSub[i] == L'.') {
        csSub.SetAt(i, L'\0');
      } else {
        break;
      }
    }
    FX_DWORD dwCount = m_pFieldTree->m_Root.CountFields();
    for (FX_DWORD m = 0; m < dwCount; m++) {
      CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(m);
      if (!pField) {
        continue;
      }
      if (pField == pExcludedField) {
        if (pExcludedControl) {
          if (pField->CountControls() < 2) {
            continue;
          }
        } else {
          continue;
        }
      }
      CFX_WideString csFullName = pField->GetFullName();
      int iRet = CompareFieldName(csSub, csFullName);
      if (iRet == 1) {
        if (pField->GetFieldType() != iType) {
          return FALSE;
        }
      } else if (iRet == 2 && csSub == csNewFieldName) {
        if (csFullName[iPos] == L'.') {
          return FALSE;
        }
      } else if (iRet == 3 && csSub == csNewFieldName) {
        if (csNewFieldName[csFullName.GetLength()] == L'.') {
          return FALSE;
        }
      }
    }
    if (iPos >= iLength) {
      break;
    }
  }
  if (csSub.IsEmpty()) {
    return FALSE;
  }
  csNewFieldName = csSub;
  return TRUE;
}
FX_BOOL CPDF_InterForm::ValidateFieldName(CFX_WideString& csNewFieldName,
                                          int iType) {
  return ValidateFieldName(csNewFieldName, iType, NULL, NULL);
}
FX_BOOL CPDF_InterForm::ValidateFieldName(const CPDF_FormField* pField,
                                          CFX_WideString& csNewFieldName) {
  return pField && !csNewFieldName.IsEmpty() &&
         ValidateFieldName(csNewFieldName,
                           ((CPDF_FormField*)pField)->GetFieldType(), pField,
                           NULL);
}
FX_BOOL CPDF_InterForm::ValidateFieldName(const CPDF_FormControl* pControl,
                                          CFX_WideString& csNewFieldName) {
  if (!pControl || csNewFieldName.IsEmpty()) {
    return FALSE;
  }
  CPDF_FormField* pField = ((CPDF_FormControl*)pControl)->GetField();
  return ValidateFieldName(csNewFieldName, pField->GetFieldType(), pField,
                           pControl);
}
int CPDF_InterForm::CompareFieldName(const CFX_ByteString& name1,
                                     const CFX_ByteString& name2) {
  const FX_CHAR* ptr1 = name1;
  const FX_CHAR* ptr2 = name2;
  if (name1.GetLength() == name2.GetLength()) {
    return name1 == name2 ? 1 : 0;
  }
  int i = 0;
  while (ptr1[i] == ptr2[i]) {
    i++;
  }
  if (i == name1.GetLength()) {
    return 2;
  }
  if (i == name2.GetLength()) {
    return 3;
  }
  return 0;
}
int CPDF_InterForm::CompareFieldName(const CFX_WideString& name1,
                                     const CFX_WideString& name2) {
  const FX_WCHAR* ptr1 = name1.c_str();
  const FX_WCHAR* ptr2 = name2.c_str();
  if (name1.GetLength() == name2.GetLength()) {
    return name1 == name2 ? 1 : 0;
  }
  int i = 0;
  while (ptr1[i] == ptr2[i]) {
    i++;
  }
  if (i == name1.GetLength()) {
    return 2;
  }
  if (i == name2.GetLength()) {
    return 3;
  }
  return 0;
}
FX_DWORD CPDF_InterForm::CountFields(const CFX_WideString& csFieldName) {
  if (csFieldName.IsEmpty()) {
    return (FX_DWORD)m_pFieldTree->m_Root.CountFields();
  }
  CFieldTree::_Node* pNode = m_pFieldTree->FindNode(csFieldName);
  return pNode ? pNode->CountFields() : 0;
}
CPDF_FormField* CPDF_InterForm::GetField(FX_DWORD index,
                                         const CFX_WideString& csFieldName) {
  if (csFieldName == L"") {
    return m_pFieldTree->m_Root.GetField(index);
  }
  CFieldTree::_Node* pNode = m_pFieldTree->FindNode(csFieldName);
  return pNode ? pNode->GetField(index) : nullptr;
}
void CPDF_InterForm::GetAllFieldNames(CFX_WideStringArray& allFieldNames) {
  allFieldNames.RemoveAll();
  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; i++) {
    CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
    if (pField) {
      CFX_WideString full_name = GetFullName(pField->GetFieldDict());
      allFieldNames.Add(full_name);
    }
  }
}

CPDF_FormField* CPDF_InterForm::GetFieldByDict(
    CPDF_Dictionary* pFieldDict) const {
  if (!pFieldDict) {
    return NULL;
  }
  CFX_WideString csWName = GetFullName(pFieldDict);
  return m_pFieldTree->GetField(csWName);
}

CPDF_FormControl* CPDF_InterForm::GetControlAtPoint(CPDF_Page* pPage,
                                                    FX_FLOAT pdf_x,
                                                    FX_FLOAT pdf_y,
                                                    int* z_order) const {
  CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArray("Annots");
  if (!pAnnotList)
    return nullptr;

  for (FX_DWORD i = pAnnotList->GetCount(); i > 0; --i) {
    FX_DWORD annot_index = i - 1;
    CPDF_Dictionary* pAnnot = pAnnotList->GetDict(annot_index);
    if (!pAnnot)
      continue;

    const auto it = m_ControlMap.find(pAnnot);
    if (it == m_ControlMap.end())
      continue;

    CPDF_FormControl* pControl = it->second;
    CFX_FloatRect rect = pControl->GetRect();
    if (!rect.Contains(pdf_x, pdf_y))
      continue;

    if (z_order)
      *z_order = annot_index;
    return pControl;
  }
  return nullptr;
}

CPDF_FormControl* CPDF_InterForm::GetControlByDict(
    const CPDF_Dictionary* pWidgetDict) const {
  const auto it = m_ControlMap.find(pWidgetDict);
  return it != m_ControlMap.end() ? it->second : nullptr;
}

FX_BOOL CPDF_InterForm::NeedConstructAP() {
  return m_pFormDict && m_pFormDict->GetBoolean("NeedAppearances");
}
void CPDF_InterForm::NeedConstructAP(FX_BOOL bNeedAP) {
  if (!m_pFormDict) {
    InitInterFormDict(m_pFormDict, m_pDocument);
  }
  m_pFormDict->SetAtBoolean("NeedAppearances", bNeedAP);
  m_bGenerateAP = bNeedAP;
}
int CPDF_InterForm::CountFieldsInCalculationOrder() {
  if (!m_pFormDict) {
    return 0;
  }
  CPDF_Array* pArray = m_pFormDict->GetArray("CO");
  return pArray ? pArray->GetCount() : 0;
}
CPDF_FormField* CPDF_InterForm::GetFieldInCalculationOrder(int index) {
  if (!m_pFormDict || index < 0) {
    return NULL;
  }
  CPDF_Array* pArray = m_pFormDict->GetArray("CO");
  if (!pArray) {
    return NULL;
  }
  if (CPDF_Dictionary* pElement =
          ToDictionary(pArray->GetElementValue(index))) {
    return GetFieldByDict(pElement);
  }
  return NULL;
}
int CPDF_InterForm::FindFieldInCalculationOrder(const CPDF_FormField* pField) {
  if (!m_pFormDict || !pField) {
    return -1;
  }
  CPDF_Array* pArray = m_pFormDict->GetArray("CO");
  if (!pArray) {
    return -1;
  }
  for (FX_DWORD i = 0; i < pArray->GetCount(); i++) {
    CPDF_Object* pElement = pArray->GetElementValue(i);
    if (pElement == pField->m_pDict) {
      return i;
    }
  }
  return -1;
}
FX_DWORD CPDF_InterForm::CountFormFonts() {
  return CountInterFormFonts(m_pFormDict);
}
CPDF_Font* CPDF_InterForm::GetFormFont(FX_DWORD index,
                                       CFX_ByteString& csNameTag) {
  return GetInterFormFont(m_pFormDict, m_pDocument, index, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetFormFont(CFX_ByteString csNameTag) {
  return GetInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetFormFont(CFX_ByteString csFontName,
                                       CFX_ByteString& csNameTag) {
  return GetInterFormFont(m_pFormDict, m_pDocument, csFontName, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetNativeFormFont(uint8_t charSet,
                                             CFX_ByteString& csNameTag) {
  return GetNativeInterFormFont(m_pFormDict, m_pDocument, charSet, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetNativeFormFont(CFX_ByteString& csNameTag) {
  return GetNativeInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}
FX_BOOL CPDF_InterForm::FindFormFont(const CPDF_Font* pFont,
                                     CFX_ByteString& csNameTag) {
  return FindInterFormFont(m_pFormDict, pFont, csNameTag);
}
FX_BOOL CPDF_InterForm::FindFormFont(CFX_ByteString csFontName,
                                     CPDF_Font*& pFont,
                                     CFX_ByteString& csNameTag) {
  return FindInterFormFont(m_pFormDict, m_pDocument, csFontName, pFont,
                           csNameTag);
}
void CPDF_InterForm::AddFormFont(const CPDF_Font* pFont,
                                 CFX_ByteString& csNameTag) {
  AddInterFormFont(m_pFormDict, m_pDocument, pFont, csNameTag);
  m_bUpdated = TRUE;
}
CPDF_Font* CPDF_InterForm::AddNativeFormFont(uint8_t charSet,
                                             CFX_ByteString& csNameTag) {
  m_bUpdated = TRUE;
  return AddNativeInterFormFont(m_pFormDict, m_pDocument, charSet, csNameTag);
}
CPDF_Font* CPDF_InterForm::AddNativeFormFont(CFX_ByteString& csNameTag) {
  m_bUpdated = TRUE;
  return AddNativeInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}
void CPDF_InterForm::RemoveFormFont(const CPDF_Font* pFont) {
  m_bUpdated = TRUE;
  RemoveInterFormFont(m_pFormDict, pFont);
}
void CPDF_InterForm::RemoveFormFont(CFX_ByteString csNameTag) {
  m_bUpdated = TRUE;
  RemoveInterFormFont(m_pFormDict, csNameTag);
}
CPDF_DefaultAppearance CPDF_InterForm::GetDefaultAppearance() {
  CFX_ByteString csDA;
  if (!m_pFormDict) {
    return csDA;
  }
  csDA = m_pFormDict->GetString("DA");
  return csDA;
}
CPDF_Font* CPDF_InterForm::GetDefaultFormFont() {
  return GetDefaultInterFormFont(m_pFormDict, m_pDocument);
}
int CPDF_InterForm::GetFormAlignment() {
  return m_pFormDict ? m_pFormDict->GetInteger("Q", 0) : 0;
}

bool CPDF_InterForm::ResetForm(const std::vector<CPDF_FormField*>& fields,
                               bool bIncludeOrExclude,
                               bool bNotify) {
  if (bNotify && m_pFormNotify && m_pFormNotify->BeforeFormReset(this) < 0)
    return false;

  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; ++i) {
    CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
    if (!pField)
      continue;

    if (bIncludeOrExclude == pdfium::ContainsValue(fields, pField))
      pField->ResetField(bNotify);
  }
  if (bNotify && m_pFormNotify)
    m_pFormNotify->AfterFormReset(this);
  return true;
}

bool CPDF_InterForm::ResetForm(bool bNotify) {
  if (bNotify && m_pFormNotify && m_pFormNotify->BeforeFormReset(this) < 0)
    return false;

  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; ++i) {
    CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
    if (!pField)
      continue;

    pField->ResetField(bNotify);
  }
  if (bNotify && m_pFormNotify)
    m_pFormNotify->AfterFormReset(this);
  return true;
}

void CPDF_InterForm::LoadField(CPDF_Dictionary* pFieldDict, int nLevel) {
  if (nLevel > nMaxRecursion) {
    return;
  }
  if (!pFieldDict) {
    return;
  }
  FX_DWORD dwParentObjNum = pFieldDict->GetObjNum();
  CPDF_Array* pKids = pFieldDict->GetArray("Kids");
  if (!pKids) {
    AddTerminalField(pFieldDict);
    return;
  }
  CPDF_Dictionary* pFirstKid = pKids->GetDict(0);
  if (!pFirstKid) {
    return;
  }
  if (pFirstKid->KeyExist("T") || pFirstKid->KeyExist("Kids")) {
    for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
      CPDF_Dictionary* pChildDict = pKids->GetDict(i);
      if (pChildDict) {
        if (pChildDict->GetObjNum() != dwParentObjNum) {
          LoadField(pChildDict, nLevel + 1);
        }
      }
    }
  } else {
    AddTerminalField(pFieldDict);
  }
}
FX_BOOL CPDF_InterForm::HasXFAForm() const {
  return m_pFormDict && m_pFormDict->GetArray("XFA");
}
void CPDF_InterForm::FixPageFields(const CPDF_Page* pPage) {
  CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
  if (!pPageDict) {
    return;
  }
  CPDF_Array* pAnnots = pPageDict->GetArray("Annots");
  if (!pAnnots) {
    return;
  }
  int iAnnotCount = pAnnots->GetCount();
  for (int i = 0; i < iAnnotCount; i++) {
    CPDF_Dictionary* pAnnot = pAnnots->GetDict(i);
    if (pAnnot && pAnnot->GetString("Subtype") == "Widget") {
      LoadField(pAnnot);
    }
  }
}
CPDF_FormField* CPDF_InterForm::AddTerminalField(CPDF_Dictionary* pFieldDict) {
  if (!pFieldDict->KeyExist("T")) {
    return NULL;
  }
  CPDF_Dictionary* pDict = pFieldDict;
  CFX_WideString csWName = GetFullName(pFieldDict);
  if (csWName.IsEmpty()) {
    return NULL;
  }
  CPDF_FormField* pField = NULL;
  pField = m_pFieldTree->GetField(csWName);
  if (!pField) {
    CPDF_Dictionary* pParent = pFieldDict;
    if (!pFieldDict->KeyExist("T") &&
        pFieldDict->GetString("Subtype") == "Widget") {
      pParent = pFieldDict->GetDict("Parent");
      if (!pParent) {
        pParent = pFieldDict;
      }
    }
    if (pParent && pParent != pFieldDict && !pParent->KeyExist("FT")) {
      if (pFieldDict->KeyExist("FT")) {
        CPDF_Object* pFTValue = pFieldDict->GetElementValue("FT");
        if (pFTValue) {
          pParent->SetAt("FT", pFTValue->Clone());
        }
      }
      if (pFieldDict->KeyExist("Ff")) {
        CPDF_Object* pFfValue = pFieldDict->GetElementValue("Ff");
        if (pFfValue) {
          pParent->SetAt("Ff", pFfValue->Clone());
        }
      }
    }
    pField = new CPDF_FormField(this, pParent);
    CPDF_Object* pTObj = pDict->GetElement("T");
    if (ToReference(pTObj)) {
      CPDF_Object* pClone = pTObj->Clone(TRUE);
      if (pClone)
        pDict->SetAt("T", pClone);
      else
        pDict->SetAtName("T", "");
    }
    m_pFieldTree->SetField(csWName, pField);
  }
  CPDF_Array* pKids = pFieldDict->GetArray("Kids");
  if (!pKids) {
    if (pFieldDict->GetString("Subtype") == "Widget") {
      AddControl(pField, pFieldDict);
    }
  } else {
    for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
      CPDF_Dictionary* pKid = pKids->GetDict(i);
      if (!pKid) {
        continue;
      }
      if (pKid->GetString("Subtype") != "Widget") {
        continue;
      }
      AddControl(pField, pKid);
    }
  }
  return pField;
}
CPDF_FormControl* CPDF_InterForm::AddControl(const CPDF_FormField* pField,
                                             CPDF_Dictionary* pWidgetDict) {
  const auto it = m_ControlMap.find(pWidgetDict);
  if (it != m_ControlMap.end())
    return it->second;

  CPDF_FormControl* pControl =
      new CPDF_FormControl((CPDF_FormField*)pField, pWidgetDict);
  m_ControlMap[pWidgetDict] = pControl;
  ((CPDF_FormField*)pField)->m_ControlList.Add(pControl);
  return pControl;
}

CPDF_FormField* CPDF_InterForm::CheckRequiredFields(
    const std::vector<CPDF_FormField*>* fields,
    bool bIncludeOrExclude) const {
  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; ++i) {
    CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
    if (!pField)
      continue;

    int32_t iType = pField->GetType();
    if (iType == CPDF_FormField::PushButton ||
        iType == CPDF_FormField::CheckBox || iType == CPDF_FormField::ListBox) {
      continue;
    }
    FX_DWORD dwFlags = pField->GetFieldFlags();
    // TODO(thestig): Look up these magic numbers and add constants for them.
    if (dwFlags & 0x04)
      continue;

    bool bFind = true;
    if (fields)
      bFind = pdfium::ContainsValue(*fields, pField);
    if (bIncludeOrExclude == bFind) {
      CPDF_Dictionary* pFieldDict = pField->m_pDict;
      if ((dwFlags & 0x02) != 0 && pFieldDict->GetString("V").IsEmpty()) {
        return pField;
      }
    }
  }
  return nullptr;
}

CFDF_Document* CPDF_InterForm::ExportToFDF(const CFX_WideStringC& pdf_path,
                                           bool bSimpleFileSpec) const {
  std::vector<CPDF_FormField*> fields;
  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; ++i)
    fields.push_back(m_pFieldTree->m_Root.GetField(i));
  return ExportToFDF(pdf_path, fields, true, bSimpleFileSpec);
}

CFDF_Document* CPDF_InterForm::ExportToFDF(
    const CFX_WideStringC& pdf_path,
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude,
    bool bSimpleFileSpec) const {
  CFDF_Document* pDoc = CFDF_Document::CreateNewDoc();
  if (!pDoc) {
    return NULL;
  }
  CPDF_Dictionary* pMainDict = pDoc->GetRoot()->GetDict("FDF");
  if (!pdf_path.IsEmpty()) {
    if (bSimpleFileSpec) {
      CFX_WideString wsFilePath = FILESPEC_EncodeFileName(pdf_path);
      pMainDict->SetAtString("F", CFX_ByteString::FromUnicode(wsFilePath));
      pMainDict->SetAtString("UF", PDF_EncodeText(wsFilePath));
    } else {
      CPDF_FileSpec filespec;
      filespec.SetFileName(pdf_path);
      pMainDict->SetAt("F", static_cast<CPDF_Object*>(filespec));
    }
  }
  CPDF_Array* pFields = new CPDF_Array;
  pMainDict->SetAt("Fields", pFields);
  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; i++) {
    CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
    if (!pField || pField->GetType() == CPDF_FormField::PushButton) {
      continue;
    }
    FX_DWORD dwFlags = pField->GetFieldFlags();
    if (dwFlags & 0x04)
      continue;

    if (bIncludeOrExclude == pdfium::ContainsValue(fields, pField)) {
      if ((dwFlags & 0x02) != 0 && pField->m_pDict->GetString("V").IsEmpty())
        continue;

      CFX_WideString fullname = GetFullName(pField->GetFieldDict());
      CPDF_Dictionary* pFieldDict = new CPDF_Dictionary;
      pFieldDict->SetAt("T", new CPDF_String(fullname));
      if (pField->GetType() == CPDF_FormField::CheckBox ||
          pField->GetType() == CPDF_FormField::RadioButton) {
        CFX_WideString csExport = pField->GetCheckValue(FALSE);
        CFX_ByteString csBExport = PDF_EncodeText(csExport);
        CPDF_Object* pOpt = FPDF_GetFieldAttr(pField->m_pDict, "Opt");
        if (pOpt)
          pFieldDict->SetAtString("V", csBExport);
        else
          pFieldDict->SetAtName("V", csBExport);
      } else {
        CPDF_Object* pV = FPDF_GetFieldAttr(pField->m_pDict, "V");
        if (pV)
          pFieldDict->SetAt("V", pV->Clone(TRUE));
      }
      pFields->Add(pFieldDict);
    }
  }
  return pDoc;
}
const struct _SupportFieldEncoding {
  const FX_CHAR* m_name;
  int32_t m_codePage;
} g_fieldEncoding[] = {
    {"BigFive", 950},
    {"GBK", 936},
    {"Shift-JIS", 932},
    {"UHC", 949},
};
static void FPDFDOC_FDF_GetFieldValue(CPDF_Dictionary* pFieldDict,
                                      CFX_WideString& csValue,
                                      CFX_ByteString& bsEncoding) {
  CFX_ByteString csBValue = pFieldDict->GetString("V");
  int32_t iCount = sizeof(g_fieldEncoding) / sizeof(g_fieldEncoding[0]);
  int32_t i = 0;
  for (; i < iCount; ++i)
    if (bsEncoding == g_fieldEncoding[i].m_name) {
      break;
    }
  if (i < iCount) {
    CFX_CharMap* pCharMap =
        CFX_CharMap::GetDefaultMapper(g_fieldEncoding[i].m_codePage);
    FXSYS_assert(pCharMap);
    csValue.ConvertFrom(csBValue, pCharMap);
    return;
  }
  CFX_ByteString csTemp = csBValue.Left(2);
  if (csTemp == "\xFF\xFE" || csTemp == "\xFE\xFF") {
    csValue = PDF_DecodeText(csBValue);
  } else {
    csValue = CFX_WideString::FromLocal(csBValue);
  }
}
void CPDF_InterForm::FDF_ImportField(CPDF_Dictionary* pFieldDict,
                                     const CFX_WideString& parent_name,
                                     FX_BOOL bNotify,
                                     int nLevel) {
  CFX_WideString name;
  if (!parent_name.IsEmpty()) {
    name = parent_name + L".";
  }
  name += pFieldDict->GetUnicodeText("T");
  CPDF_Array* pKids = pFieldDict->GetArray("Kids");
  if (pKids) {
    for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
      CPDF_Dictionary* pKid = pKids->GetDict(i);
      if (!pKid) {
        continue;
      }
      if (nLevel <= nMaxRecursion) {
        FDF_ImportField(pKid, name, bNotify, nLevel + 1);
      }
    }
    return;
  }
  if (!pFieldDict->KeyExist("V")) {
    return;
  }
  CPDF_FormField* pField = m_pFieldTree->GetField(name);
  if (!pField) {
    return;
  }
  CFX_WideString csWValue;
  FPDFDOC_FDF_GetFieldValue(pFieldDict, csWValue, m_bsEncoding);
  int iType = pField->GetFieldType();
  if (bNotify && m_pFormNotify) {
    int iRet = 0;
    if (iType == FIELDTYPE_LISTBOX) {
      iRet = m_pFormNotify->BeforeSelectionChange(pField, csWValue);
    } else if (iType == FIELDTYPE_COMBOBOX || iType == FIELDTYPE_TEXTFIELD) {
      iRet = m_pFormNotify->BeforeValueChange(pField, csWValue);
    }
    if (iRet < 0) {
      return;
    }
  }
  CFX_ByteArray statusArray;
  if (iType == FIELDTYPE_CHECKBOX || iType == FIELDTYPE_RADIOBUTTON) {
    SaveCheckedFieldStatus(pField, statusArray);
  }
  pField->SetValue(csWValue);
  CPDF_FormField::Type eType = pField->GetType();
  if ((eType == CPDF_FormField::ListBox || eType == CPDF_FormField::ComboBox) &&
      pFieldDict->KeyExist("Opt")) {
    pField->m_pDict->SetAt("Opt",
                           pFieldDict->GetElementValue("Opt")->Clone(TRUE));
  }
  if (bNotify && m_pFormNotify) {
    if (iType == FIELDTYPE_CHECKBOX || iType == FIELDTYPE_RADIOBUTTON) {
      m_pFormNotify->AfterCheckedStatusChange(pField, statusArray);
    } else if (iType == FIELDTYPE_LISTBOX) {
      m_pFormNotify->AfterSelectionChange(pField);
    } else if (iType == FIELDTYPE_COMBOBOX || iType == FIELDTYPE_TEXTFIELD) {
      m_pFormNotify->AfterValueChange(pField);
    }
  }
  if (CPDF_InterForm::m_bUpdateAP) {
    pField->UpdateAP(NULL);
  }
}
FX_BOOL CPDF_InterForm::ImportFromFDF(const CFDF_Document* pFDF,
                                      FX_BOOL bNotify) {
  if (!pFDF) {
    return FALSE;
  }
  CPDF_Dictionary* pMainDict = pFDF->GetRoot()->GetDict("FDF");
  if (!pMainDict) {
    return FALSE;
  }
  CPDF_Array* pFields = pMainDict->GetArray("Fields");
  if (!pFields) {
    return FALSE;
  }
  m_bsEncoding = pMainDict->GetString("Encoding");
  if (bNotify && m_pFormNotify) {
    int iRet = m_pFormNotify->BeforeFormImportData(this);
    if (iRet < 0) {
      return FALSE;
    }
  }
  for (FX_DWORD i = 0; i < pFields->GetCount(); i++) {
    CPDF_Dictionary* pField = pFields->GetDict(i);
    if (!pField) {
      continue;
    }
    FDF_ImportField(pField, L"", bNotify);
  }
  if (bNotify && m_pFormNotify) {
    m_pFormNotify->AfterFormImportData(this);
  }
  return TRUE;
}
void CPDF_InterForm::SetFormNotify(const CPDF_FormNotify* pNotify) {
  m_pFormNotify = (CPDF_FormNotify*)pNotify;
}
