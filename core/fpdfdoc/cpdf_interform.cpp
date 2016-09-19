// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <vector>

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_font/include/cpdf_fontencoding.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cfdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_string.h"
#include "core/fpdfdoc/include/cpdf_filespec.h"
#include "core/fpdfdoc/include/cpdf_formcontrol.h"
#include "core/fpdfdoc/include/cpdf_interform.h"
#include "core/fxge/include/cfx_substfont.h"
#include "core/fxge/include/fx_font.h"
#include "third_party/base/stl_util.h"

namespace {

const int nMaxRecursion = 32;

const struct SupportFieldEncoding {
  const FX_CHAR* m_name;
  uint16_t m_codePage;
} g_fieldEncoding[] = {
    {"BigFive", 950},
    {"GBK", 936},
    {"Shift-JIS", 932},
    {"UHC", 949},
};

CFX_WideString GetFieldValue(const CPDF_Dictionary& pFieldDict,
                             const CFX_ByteString& bsEncoding) {
  const CFX_ByteString csBValue = pFieldDict.GetStringFor("V");
  for (const auto& encoding : g_fieldEncoding) {
    if (bsEncoding == encoding.m_name)
      return CFX_WideString::FromCodePage(csBValue.AsStringC(),
                                          encoding.m_codePage);
  }
  CFX_ByteString csTemp = csBValue.Left(2);
  if (csTemp == "\xFF\xFE" || csTemp == "\xFE\xFF")
    return PDF_DecodeText(csBValue);
  return CFX_WideString::FromLocal(csBValue.AsStringC());
}

void AddFont(CPDF_Dictionary*& pFormDict,
             CPDF_Document* pDocument,
             const CPDF_Font* pFont,
             CFX_ByteString& csNameTag);

void InitDict(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument) {
  if (!pDocument)
    return;

  if (!pFormDict) {
    pFormDict = new CPDF_Dictionary;
    uint32_t dwObjNum = pDocument->AddIndirectObject(pFormDict);
    CPDF_Dictionary* pRoot = pDocument->GetRoot();
    pRoot->SetReferenceFor("AcroForm", pDocument, dwObjNum);
  }

  CFX_ByteString csDA;
  if (!pFormDict->KeyExist("DR")) {
    CFX_ByteString csBaseName;
    CFX_ByteString csDefault;
    uint8_t charSet = CPDF_InterForm::GetNativeCharSet();
    CPDF_Font* pFont = CPDF_InterForm::AddStandardFont(pDocument, "Helvetica");
    if (pFont) {
      AddFont(pFormDict, pDocument, pFont, csBaseName);
      csDefault = csBaseName;
    }
    if (charSet != FXFONT_ANSI_CHARSET) {
      CFX_ByteString csFontName =
          CPDF_InterForm::GetNativeFont(charSet, nullptr);
      if (!pFont || csFontName != "Helvetica") {
        pFont = CPDF_InterForm::AddNativeFont(pDocument);
        if (pFont) {
          csBaseName = "";
          AddFont(pFormDict, pDocument, pFont, csBaseName);
          csDefault = csBaseName;
        }
      }
    }
    if (pFont)
      csDA = "/" + PDF_NameEncode(csDefault) + " 0 Tf";
  }
  if (!csDA.IsEmpty())
    csDA += " ";

  csDA += "0 g";
  if (!pFormDict->KeyExist("DA"))
    pFormDict->SetStringFor("DA", csDA);
}

uint32_t CountFonts(CPDF_Dictionary* pFormDict) {
  if (!pFormDict)
    return 0;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return 0;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return 0;

  uint32_t dwCount = 0;
  for (const auto& it : *pFonts) {
    CPDF_Object* pObj = it.second;
    if (!pObj)
      continue;

    if (CPDF_Dictionary* pDirect = ToDictionary(pObj->GetDirect())) {
      if (pDirect->GetStringFor("Type") == "Font")
        dwCount++;
    }
  }
  return dwCount;
}

CPDF_Font* GetFont(CPDF_Dictionary* pFormDict,
                   CPDF_Document* pDocument,
                   uint32_t index,
                   CFX_ByteString& csNameTag) {
  if (!pFormDict)
    return nullptr;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return nullptr;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return nullptr;

  uint32_t dwCount = 0;
  for (const auto& it : *pFonts) {
    const CFX_ByteString& csKey = it.first;
    CPDF_Object* pObj = it.second;
    if (!pObj)
      continue;

    CPDF_Dictionary* pElement = ToDictionary(pObj->GetDirect());
    if (!pElement)
      continue;
    if (pElement->GetStringFor("Type") != "Font")
      continue;
    if (dwCount == index) {
      csNameTag = csKey;
      return pDocument->LoadFont(pElement);
    }
    dwCount++;
  }
  return nullptr;
}

CPDF_Font* GetFont(CPDF_Dictionary* pFormDict,
                   CPDF_Document* pDocument,
                   CFX_ByteString csNameTag) {
  CFX_ByteString csAlias = PDF_NameDecode(csNameTag);
  if (!pFormDict || csAlias.IsEmpty())
    return nullptr;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return nullptr;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return nullptr;

  CPDF_Dictionary* pElement = pFonts->GetDictFor(csAlias);
  if (!pElement)
    return nullptr;

  if (pElement->GetStringFor("Type") == "Font")
    return pDocument->LoadFont(pElement);
  return nullptr;
}

CPDF_Font* GetFont(CPDF_Dictionary* pFormDict,
                   CPDF_Document* pDocument,
                   CFX_ByteString csFontName,
                   CFX_ByteString& csNameTag) {
  if (!pFormDict || csFontName.IsEmpty())
    return nullptr;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return nullptr;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return nullptr;

  for (const auto& it : *pFonts) {
    const CFX_ByteString& csKey = it.first;
    CPDF_Object* pObj = it.second;
    if (!pObj)
      continue;

    CPDF_Dictionary* pElement = ToDictionary(pObj->GetDirect());
    if (!pElement)
      continue;
    if (pElement->GetStringFor("Type") != "Font")
      continue;

    CPDF_Font* pFind = pDocument->LoadFont(pElement);
    if (!pFind)
      continue;

    CFX_ByteString csBaseFont;
    csBaseFont = pFind->GetBaseFont();
    csBaseFont.Remove(' ');
    if (csBaseFont == csFontName) {
      csNameTag = csKey;
      return pFind;
    }
  }
  return nullptr;
}

CPDF_Font* GetNativeFont(CPDF_Dictionary* pFormDict,
                         CPDF_Document* pDocument,
                         uint8_t charSet,
                         CFX_ByteString& csNameTag) {
  if (!pFormDict)
    return nullptr;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return nullptr;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return nullptr;

  for (const auto& it : *pFonts) {
    const CFX_ByteString& csKey = it.first;
    CPDF_Object* pObj = it.second;
    if (!pObj)
      continue;

    CPDF_Dictionary* pElement = ToDictionary(pObj->GetDirect());
    if (!pElement)
      continue;
    if (pElement->GetStringFor("Type") != "Font")
      continue;
    CPDF_Font* pFind = pDocument->LoadFont(pElement);
    if (!pFind)
      continue;

    CFX_SubstFont* pSubst = pFind->GetSubstFont();
    if (!pSubst)
      continue;

    if (pSubst->m_Charset == (int)charSet) {
      csNameTag = csKey;
      return pFind;
    }
  }
  return nullptr;
}

CPDF_Font* GetDefaultFont(CPDF_Dictionary* pFormDict,
                          CPDF_Document* pDocument) {
  if (!pFormDict)
    return nullptr;

  CPDF_DefaultAppearance cDA(pFormDict->GetStringFor("DA"));
  CFX_ByteString csFontNameTag;
  FX_FLOAT fFontSize;
  cDA.GetFont(csFontNameTag, fFontSize);
  return GetFont(pFormDict, pDocument, csFontNameTag);
}

FX_BOOL FindFont(CPDF_Dictionary* pFormDict,
                 const CPDF_Font* pFont,
                 CFX_ByteString& csNameTag) {
  if (!pFormDict || !pFont)
    return FALSE;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return FALSE;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return FALSE;

  for (const auto& it : *pFonts) {
    const CFX_ByteString& csKey = it.first;
    CPDF_Object* pObj = it.second;
    if (!pObj)
      continue;

    CPDF_Dictionary* pElement = ToDictionary(pObj->GetDirect());
    if (!pElement)
      continue;
    if (pElement->GetStringFor("Type") != "Font")
      continue;
    if (pFont->GetFontDict() == pElement) {
      csNameTag = csKey;
      return TRUE;
    }
  }
  return FALSE;
}

CPDF_Font* GetNativeFont(CPDF_Dictionary* pFormDict,
                         CPDF_Document* pDocument,
                         CFX_ByteString& csNameTag) {
  csNameTag.clear();
  uint8_t charSet = CPDF_InterForm::GetNativeCharSet();
  CPDF_Font* pFont = GetDefaultFont(pFormDict, pDocument);
  if (pFont) {
    CFX_SubstFont* pSubst = pFont->GetSubstFont();
    if (pSubst && pSubst->m_Charset == (int)charSet) {
      FindFont(pFormDict, pFont, csNameTag);
      return pFont;
    }
  }
  return GetNativeFont(pFormDict, pDocument, charSet, csNameTag);
}

FX_BOOL FindFont(CPDF_Dictionary* pFormDict,
                 CPDF_Document* pDocument,
                 CFX_ByteString csFontName,
                 CPDF_Font*& pFont,
                 CFX_ByteString& csNameTag) {
  if (!pFormDict)
    return FALSE;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return FALSE;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return FALSE;
  if (csFontName.GetLength() > 0)
    csFontName.Remove(' ');

  for (const auto& it : *pFonts) {
    const CFX_ByteString& csKey = it.first;
    CPDF_Object* pObj = it.second;
    if (!pObj)
      continue;

    CPDF_Dictionary* pElement = ToDictionary(pObj->GetDirect());
    if (!pElement)
      continue;
    if (pElement->GetStringFor("Type") != "Font")
      continue;

    pFont = pDocument->LoadFont(pElement);
    if (!pFont)
      continue;

    CFX_ByteString csBaseFont;
    csBaseFont = pFont->GetBaseFont();
    csBaseFont.Remove(' ');
    if (csBaseFont == csFontName) {
      csNameTag = csKey;
      return TRUE;
    }
  }
  return FALSE;
}

void AddFont(CPDF_Dictionary*& pFormDict,
             CPDF_Document* pDocument,
             const CPDF_Font* pFont,
             CFX_ByteString& csNameTag) {
  if (!pFont)
    return;
  if (!pFormDict)
    InitDict(pFormDict, pDocument);

  CFX_ByteString csTag;
  if (FindFont(pFormDict, pFont, csTag)) {
    csNameTag = csTag;
    return;
  }
  if (!pFormDict)
    InitDict(pFormDict, pDocument);

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR) {
    pDR = new CPDF_Dictionary;
    pFormDict->SetFor("DR", pDR);
  }
  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts) {
    pFonts = new CPDF_Dictionary;
    pDR->SetFor("Font", pFonts);
  }
  if (csNameTag.IsEmpty())
    csNameTag = pFont->GetBaseFont();

  csNameTag.Remove(' ');
  csNameTag = CPDF_InterForm::GenerateNewResourceName(pDR, "Font", 4,
                                                      csNameTag.c_str());
  pFonts->SetReferenceFor(csNameTag, pDocument, pFont->GetFontDict());
}

CPDF_Font* AddNativeFont(CPDF_Dictionary*& pFormDict,
                         CPDF_Document* pDocument,
                         uint8_t charSet,
                         CFX_ByteString& csNameTag) {
  if (!pFormDict)
    InitDict(pFormDict, pDocument);

  CFX_ByteString csTemp;
  CPDF_Font* pFont = GetNativeFont(pFormDict, pDocument, charSet, csTemp);
  if (pFont) {
    csNameTag = csTemp;
    return pFont;
  }
  CFX_ByteString csFontName = CPDF_InterForm::GetNativeFont(charSet);
  if (!csFontName.IsEmpty() &&
      FindFont(pFormDict, pDocument, csFontName, pFont, csNameTag)) {
    return pFont;
  }
  pFont = CPDF_InterForm::AddNativeFont(charSet, pDocument);
  if (pFont)
    AddFont(pFormDict, pDocument, pFont, csNameTag);

  return pFont;
}

void RemoveFont(CPDF_Dictionary* pFormDict, const CPDF_Font* pFont) {
  if (!pFormDict || !pFont)
    return;

  CFX_ByteString csTag;
  if (!FindFont(pFormDict, pFont, csTag))
    return;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  pFonts->RemoveFor(csTag);
}

void RemoveFont(CPDF_Dictionary* pFormDict, CFX_ByteString csNameTag) {
  if (!pFormDict || csNameTag.IsEmpty())
    return;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    return;

  pFonts->RemoveFor(csNameTag);
}

class CFieldNameExtractor {
 public:
  explicit CFieldNameExtractor(const CFX_WideString& full_name)
      : m_FullName(full_name) {
    m_pCur = m_FullName.c_str();
    m_pEnd = m_pCur + m_FullName.GetLength();
  }

  void GetNext(const FX_WCHAR*& pSubName, FX_STRSIZE& size) {
    pSubName = m_pCur;
    while (m_pCur < m_pEnd && m_pCur[0] != L'.')
      m_pCur++;

    size = (FX_STRSIZE)(m_pCur - pSubName);
    if (m_pCur < m_pEnd && m_pCur[0] == L'.')
      m_pCur++;
  }

 protected:
  CFX_WideString m_FullName;
  const FX_WCHAR* m_pCur;
  const FX_WCHAR* m_pEnd;
};

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
typedef struct {
  FX_BOOL bFind;
  LOGFONTA lf;
} PDF_FONTDATA;

static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXA* lpelfe,
                                      NEWTEXTMETRICEX* lpntme,
                                      DWORD FontType,
                                      LPARAM lParam) {
  if (FontType != 0x004 || strchr(lpelfe->elfLogFont.lfFaceName, '@'))
    return 1;

  PDF_FONTDATA* pData = (PDF_FONTDATA*)lParam;
  memcpy(&pData->lf, &lpelfe->elfLogFont, sizeof(LOGFONTA));
  pData->bFind = TRUE;
  return 0;
}

FX_BOOL RetrieveSpecificFont(LOGFONTA& lf) {
  PDF_FONTDATA fd;
  memset(&fd, 0, sizeof(PDF_FONTDATA));
  HDC hDC = ::GetDC(nullptr);
  EnumFontFamiliesExA(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc, (LPARAM)&fd,
                      0);
  ::ReleaseDC(nullptr, hDC);
  if (fd.bFind)
    memcpy(&lf, &fd.lf, sizeof(LOGFONTA));

  return fd.bFind;
}

FX_BOOL RetrieveSpecificFont(uint8_t charSet,
                             uint8_t pitchAndFamily,
                             LPCSTR pcsFontName,
                             LOGFONTA& lf) {
  memset(&lf, 0, sizeof(LOGFONTA));
  lf.lfCharSet = charSet;
  lf.lfPitchAndFamily = pitchAndFamily;
  if (pcsFontName) {
    // TODO(dsinclair): Should this be strncpy?
    strcpy(lf.lfFaceName, pcsFontName);
  }
  return RetrieveSpecificFont(lf);
}
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

}  // namespace

class CFieldTree {
 public:
  struct Node {
    Node* parent;
    CFX_ArrayTemplate<Node*> children;
    CFX_WideString short_name;
    CPDF_FormField* field_ptr;
    int CountFields(int nLevel = 0) {
      if (nLevel > nMaxRecursion)
        return 0;
      if (field_ptr)
        return 1;

      int count = 0;
      for (int i = 0; i < children.GetSize(); i++)
        count += children.GetAt(i)->CountFields(nLevel + 1);
      return count;
    }

    CPDF_FormField* GetField(int* fields_to_go) {
      if (field_ptr) {
        if (*fields_to_go == 0)
          return field_ptr;

        --*fields_to_go;
        return nullptr;
      }
      for (int i = 0; i < children.GetSize(); i++) {
        if (CPDF_FormField* pField = children.GetAt(i)->GetField(fields_to_go))
          return pField;
      }
      return nullptr;
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

  Node* FindNode(const CFX_WideString& full_name);
  Node* AddChild(Node* pParent,
                 const CFX_WideString& short_name,
                 CPDF_FormField* field_ptr);
  void RemoveNode(Node* pNode, int nLevel = 0);

  Node* Lookup(Node* pParent, const CFX_WideString& short_name);

  Node m_Root;
};

CFieldTree::CFieldTree() {
  m_Root.parent = nullptr;
  m_Root.field_ptr = nullptr;
}

CFieldTree::~CFieldTree() {
  RemoveAll();
}

CFieldTree::Node* CFieldTree::AddChild(Node* pParent,
                                       const CFX_WideString& short_name,
                                       CPDF_FormField* field_ptr) {
  if (!pParent)
    return nullptr;

  Node* pNode = new Node;
  pNode->parent = pParent;
  pNode->short_name = short_name;
  pNode->field_ptr = field_ptr;
  pParent->children.Add(pNode);
  return pNode;
}

void CFieldTree::RemoveNode(Node* pNode, int nLevel) {
  if (!pNode)
    return;
  if (nLevel <= nMaxRecursion) {
    for (int i = 0; i < pNode->children.GetSize(); i++)
      RemoveNode(pNode->children[i], nLevel + 1);
  }
  delete pNode;
}

CFieldTree::Node* CFieldTree::Lookup(Node* pParent,
                                     const CFX_WideString& short_name) {
  if (!pParent)
    return nullptr;

  for (int i = 0; i < pParent->children.GetSize(); i++) {
    Node* pNode = pParent->children[i];
    if (pNode->short_name == short_name)
      return pNode;
  }
  return nullptr;
}

void CFieldTree::RemoveAll() {
  for (int i = 0; i < m_Root.children.GetSize(); i++)
    RemoveNode(m_Root.children[i]);
}

void CFieldTree::SetField(const CFX_WideString& full_name,
                          CPDF_FormField* field_ptr) {
  if (full_name == L"")
    return;

  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  Node* pNode = &m_Root;
  Node* pLast = nullptr;
  while (nLength > 0) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = Lookup(pLast, name);
    if (!pNode)
      pNode = AddChild(pLast, name, nullptr);

    name_extractor.GetNext(pName, nLength);
  }
  if (pNode != &m_Root)
    pNode->field_ptr = field_ptr;
}

CPDF_FormField* CFieldTree::GetField(const CFX_WideString& full_name) {
  if (full_name == L"")
    return nullptr;

  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  Node* pNode = &m_Root;
  Node* pLast = nullptr;
  while (nLength > 0 && pNode) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = Lookup(pLast, name);
    name_extractor.GetNext(pName, nLength);
  }
  return pNode ? pNode->field_ptr : nullptr;
}

CPDF_FormField* CFieldTree::RemoveField(const CFX_WideString& full_name) {
  if (full_name == L"")
    return nullptr;

  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  Node* pNode = &m_Root;
  Node* pLast = nullptr;
  while (nLength > 0 && pNode) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = Lookup(pLast, name);
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
  return nullptr;
}

CFieldTree::Node* CFieldTree::FindNode(const CFX_WideString& full_name) {
  if (full_name == L"")
    return nullptr;

  CFieldNameExtractor name_extractor(full_name);
  const FX_WCHAR* pName;
  FX_STRSIZE nLength;
  name_extractor.GetNext(pName, nLength);
  Node* pNode = &m_Root;
  Node* pLast = nullptr;
  while (nLength > 0 && pNode) {
    pLast = pNode;
    CFX_WideString name = CFX_WideString(pName, nLength);
    pNode = Lookup(pLast, name);
    name_extractor.GetNext(pName, nLength);
  }
  return pNode;
}

CPDF_Font* AddNativeInterFormFont(CPDF_Dictionary*& pFormDict,
                                  CPDF_Document* pDocument,
                                  CFX_ByteString& csNameTag) {
  uint8_t charSet = CPDF_InterForm::GetNativeCharSet();
  return AddNativeFont(pFormDict, pDocument, charSet, csNameTag);
}

// static
uint8_t CPDF_InterForm::GetNativeCharSet() {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  uint8_t charSet = FXFONT_ANSI_CHARSET;
  UINT iCodePage = ::GetACP();
  switch (iCodePage) {
    case 932:
      charSet = FXFONT_SHIFTJIS_CHARSET;
      break;
    case 936:
      charSet = FXFONT_GB2312_CHARSET;
      break;
    case 950:
      charSet = FXFONT_CHINESEBIG5_CHARSET;
      break;
    case 1252:
      charSet = FXFONT_ANSI_CHARSET;
      break;
    case 874:
      charSet = FXFONT_THAI_CHARSET;
      break;
    case 949:
      charSet = FXFONT_HANGUL_CHARSET;
      break;
    case 1200:
      charSet = FXFONT_ANSI_CHARSET;
      break;
    case 1250:
      charSet = FXFONT_EASTEUROPE_CHARSET;
      break;
    case 1251:
      charSet = FXFONT_RUSSIAN_CHARSET;
      break;
    case 1253:
      charSet = FXFONT_GREEK_CHARSET;
      break;
    case 1254:
      charSet = FXFONT_TURKISH_CHARSET;
      break;
    case 1255:
      charSet = FXFONT_HEBREW_CHARSET;
      break;
    case 1256:
      charSet = FXFONT_ARABIC_CHARSET;
      break;
    case 1257:
      charSet = FXFONT_BALTIC_CHARSET;
      break;
    case 1258:
      charSet = FXFONT_VIETNAMESE_CHARSET;
      break;
    case 1361:
      charSet = FXFONT_JOHAB_CHARSET;
      break;
  }
  return charSet;
#else
  return 0;
#endif
}

CPDF_InterForm::CPDF_InterForm(CPDF_Document* pDocument)
    : m_pDocument(pDocument),
      m_pFormDict(nullptr),
      m_pFieldTree(new CFieldTree),
      m_pFormNotify(nullptr) {
  CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  if (!pRoot)
    return;

  m_pFormDict = pRoot->GetDictFor("AcroForm");
  if (!m_pFormDict)
    return;

  CPDF_Array* pFields = m_pFormDict->GetArrayFor("Fields");
  if (!pFields)
    return;

  for (size_t i = 0; i < pFields->GetCount(); i++)
    LoadField(pFields->GetDictAt(i));
}

CPDF_InterForm::~CPDF_InterForm() {
  for (auto it : m_ControlMap)
    delete it.second;

  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; ++i)
    delete m_pFieldTree->m_Root.GetField(i);
}

FX_BOOL CPDF_InterForm::s_bUpdateAP = TRUE;

FX_BOOL CPDF_InterForm::IsUpdateAPEnabled() {
  return s_bUpdateAP;
}

void CPDF_InterForm::SetUpdateAP(FX_BOOL bUpdateAP) {
  s_bUpdateAP = bUpdateAP;
}

CFX_ByteString CPDF_InterForm::GenerateNewResourceName(
    const CPDF_Dictionary* pResDict,
    const FX_CHAR* csType,
    int iMinLen,
    const FX_CHAR* csPrefix) {
  CFX_ByteString csStr = csPrefix;
  CFX_ByteString csBType = csType;
  if (csStr.IsEmpty()) {
    if (csBType == "ExtGState")
      csStr = "GS";
    else if (csBType == "ColorSpace")
      csStr = "CS";
    else if (csBType == "Font")
      csStr = "ZiTi";
    else
      csStr = "Res";
  }
  CFX_ByteString csTmp = csStr;
  int iCount = csStr.GetLength();
  int m = 0;
  if (iMinLen > 0) {
    csTmp = "";
    while (m < iMinLen && m < iCount)
      csTmp += csStr[m++];
    while (m < iMinLen) {
      csTmp += '0' + m % 10;
      m++;
    }
  } else {
    m = iCount;
  }
  if (!pResDict)
    return csTmp;

  CPDF_Dictionary* pDict = pResDict->GetDictFor(csType);
  if (!pDict)
    return csTmp;

  int num = 0;
  CFX_ByteString bsNum;
  while (TRUE) {
    CFX_ByteString csKey = csTmp + bsNum;
    if (!pDict->KeyExist(csKey))
      return csKey;
    if (m < iCount)
      csTmp += csStr[m++];
    else
      bsNum.Format("%d", num++);

    m++;
  }
  return csTmp;
}

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
  LOGFONTA lf = {};
  if (charSet == FXFONT_ANSI_CHARSET) {
    csFontName = "Helvetica";
    return csFontName;
  }
  FX_BOOL bRet = FALSE;
  if (charSet == FXFONT_SHIFTJIS_CHARSET) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE,
                                "MS Mincho", lf);
  } else if (charSet == FXFONT_GB2312_CHARSET) {
    bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "SimSun",
                                lf);
  } else if (charSet == FXFONT_CHINESEBIG5_CHARSET) {
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
    bRet =
        RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, nullptr, lf);
  }
  if (bRet) {
    if (pLogFont)
      memcpy(pLogFont, &lf, sizeof(LOGFONTA));

    csFontName = lf.lfFaceName;
    return csFontName;
  }
#endif
  return csFontName;
}

CFX_ByteString CPDF_InterForm::GetNativeFont(void* pLogFont) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  return GetNativeFont(GetNativeCharSet(), pLogFont);
#else
  return CFX_ByteString();
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
  if (csNewFieldName.IsEmpty())
    return FALSE;

  int iPos = 0;
  int iLength = csNewFieldName.GetLength();
  CFX_WideString csSub;
  while (TRUE) {
    while (iPos < iLength &&
           (csNewFieldName[iPos] == L'.' || csNewFieldName[iPos] == L' ')) {
      iPos++;
    }
    if (iPos < iLength && !csSub.IsEmpty())
      csSub += L'.';
    while (iPos < iLength && csNewFieldName[iPos] != L'.')
      csSub += csNewFieldName[iPos++];
    for (int i = csSub.GetLength() - 1; i > -1; i--) {
      if (csSub[i] == L' ' || csSub[i] == L'.')
        csSub.SetAt(i, L'\0');
      else
        break;
    }
    uint32_t dwCount = m_pFieldTree->m_Root.CountFields();
    for (uint32_t m = 0; m < dwCount; m++) {
      CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(m);
      if (!pField)
        continue;
      if (pField == pExcludedField) {
        if (pExcludedControl) {
          if (pField->CountControls() < 2)
            continue;
        } else {
          continue;
        }
      }
      CFX_WideString csFullName = pField->GetFullName();
      int iRet = CompareFieldName(csSub, csFullName);
      if (iRet == 1) {
        if (pField->GetFieldType() != iType)
          return FALSE;
      } else if (iRet == 2 && csSub == csNewFieldName) {
        if (csFullName[iPos] == L'.')
          return FALSE;
      } else if (iRet == 3 && csSub == csNewFieldName) {
        if (csNewFieldName[csFullName.GetLength()] == L'.')
          return FALSE;
      }
    }
    if (iPos >= iLength)
      break;
  }
  if (csSub.IsEmpty())
    return FALSE;

  csNewFieldName = csSub;
  return TRUE;
}

FX_BOOL CPDF_InterForm::ValidateFieldName(CFX_WideString& csNewFieldName,
                                          int iType) {
  return ValidateFieldName(csNewFieldName, iType, nullptr, nullptr);
}

FX_BOOL CPDF_InterForm::ValidateFieldName(const CPDF_FormField* pField,
                                          CFX_WideString& csNewFieldName) {
  return pField && !csNewFieldName.IsEmpty() &&
         ValidateFieldName(csNewFieldName, pField->GetFieldType(), pField,
                           nullptr);
}

FX_BOOL CPDF_InterForm::ValidateFieldName(const CPDF_FormControl* pControl,
                                          CFX_WideString& csNewFieldName) {
  if (!pControl || csNewFieldName.IsEmpty())
    return FALSE;

  CPDF_FormField* pField = pControl->GetField();
  return ValidateFieldName(csNewFieldName, pField->GetFieldType(), pField,
                           pControl);
}

int CPDF_InterForm::CompareFieldName(const CFX_ByteString& name1,
                                     const CFX_ByteString& name2) {
  if (name1.GetLength() == name2.GetLength())
    return name1 == name2 ? 1 : 0;

  const FX_CHAR* ptr1 = name1.c_str();
  const FX_CHAR* ptr2 = name2.c_str();
  int i = 0;
  while (ptr1[i] == ptr2[i])
    i++;
  if (i == name1.GetLength())
    return 2;
  if (i == name2.GetLength())
    return 3;
  return 0;
}

int CPDF_InterForm::CompareFieldName(const CFX_WideString& name1,
                                     const CFX_WideString& name2) {
  const FX_WCHAR* ptr1 = name1.c_str();
  const FX_WCHAR* ptr2 = name2.c_str();
  if (name1.GetLength() == name2.GetLength())
    return name1 == name2 ? 1 : 0;

  int i = 0;
  while (ptr1[i] == ptr2[i])
    i++;
  if (i == name1.GetLength())
    return 2;
  if (i == name2.GetLength())
    return 3;
  return 0;
}

uint32_t CPDF_InterForm::CountFields(const CFX_WideString& csFieldName) {
  if (csFieldName.IsEmpty())
    return (uint32_t)m_pFieldTree->m_Root.CountFields();

  CFieldTree::Node* pNode = m_pFieldTree->FindNode(csFieldName);
  return pNode ? pNode->CountFields() : 0;
}

CPDF_FormField* CPDF_InterForm::GetField(uint32_t index,
                                         const CFX_WideString& csFieldName) {
  if (csFieldName == L"")
    return m_pFieldTree->m_Root.GetField(index);

  CFieldTree::Node* pNode = m_pFieldTree->FindNode(csFieldName);
  return pNode ? pNode->GetField(index) : nullptr;
}

CPDF_FormField* CPDF_InterForm::GetFieldByDict(
    CPDF_Dictionary* pFieldDict) const {
  if (!pFieldDict)
    return nullptr;

  CFX_WideString csWName = FPDF_GetFullName(pFieldDict);
  return m_pFieldTree->GetField(csWName);
}

CPDF_FormControl* CPDF_InterForm::GetControlAtPoint(CPDF_Page* pPage,
                                                    FX_FLOAT pdf_x,
                                                    FX_FLOAT pdf_y,
                                                    int* z_order) const {
  CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnotList)
    return nullptr;

  for (size_t i = pAnnotList->GetCount(); i > 0; --i) {
    size_t annot_index = i - 1;
    CPDF_Dictionary* pAnnot = pAnnotList->GetDictAt(annot_index);
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
      *z_order = static_cast<int>(annot_index);
    return pControl;
  }
  return nullptr;
}

CPDF_FormControl* CPDF_InterForm::GetControlByDict(
    const CPDF_Dictionary* pWidgetDict) const {
  const auto it = m_ControlMap.find(pWidgetDict);
  return it != m_ControlMap.end() ? it->second : nullptr;
}

FX_BOOL CPDF_InterForm::NeedConstructAP() const {
  return m_pFormDict && m_pFormDict->GetBooleanFor("NeedAppearances");
}

int CPDF_InterForm::CountFieldsInCalculationOrder() {
  if (!m_pFormDict)
    return 0;

  CPDF_Array* pArray = m_pFormDict->GetArrayFor("CO");
  return pArray ? pArray->GetCount() : 0;
}

CPDF_FormField* CPDF_InterForm::GetFieldInCalculationOrder(int index) {
  if (!m_pFormDict || index < 0)
    return nullptr;

  CPDF_Array* pArray = m_pFormDict->GetArrayFor("CO");
  if (!pArray)
    return nullptr;

  CPDF_Dictionary* pElement = ToDictionary(pArray->GetDirectObjectAt(index));
  return pElement ? GetFieldByDict(pElement) : nullptr;
}

int CPDF_InterForm::FindFieldInCalculationOrder(const CPDF_FormField* pField) {
  if (!m_pFormDict || !pField)
    return -1;

  CPDF_Array* pArray = m_pFormDict->GetArrayFor("CO");
  if (!pArray)
    return -1;

  for (size_t i = 0; i < pArray->GetCount(); i++) {
    CPDF_Object* pElement = pArray->GetDirectObjectAt(i);
    if (pElement == pField->m_pDict)
      return i;
  }
  return -1;
}

uint32_t CPDF_InterForm::CountFormFonts() {
  return CountFonts(m_pFormDict);
}

CPDF_Font* CPDF_InterForm::GetFormFont(uint32_t index,
                                       CFX_ByteString& csNameTag) {
  return GetFont(m_pFormDict, m_pDocument, index, csNameTag);
}

CPDF_Font* CPDF_InterForm::GetFormFont(CFX_ByteString csNameTag) {
  return GetFont(m_pFormDict, m_pDocument, csNameTag);
}

CPDF_Font* CPDF_InterForm::GetFormFont(CFX_ByteString csFontName,
                                       CFX_ByteString& csNameTag) {
  return GetFont(m_pFormDict, m_pDocument, csFontName, csNameTag);
}

CPDF_Font* CPDF_InterForm::GetNativeFormFont(uint8_t charSet,
                                             CFX_ByteString& csNameTag) {
  return ::GetNativeFont(m_pFormDict, m_pDocument, charSet, csNameTag);
}

CPDF_Font* CPDF_InterForm::GetNativeFormFont(CFX_ByteString& csNameTag) {
  return ::GetNativeFont(m_pFormDict, m_pDocument, csNameTag);
}

FX_BOOL CPDF_InterForm::FindFormFont(const CPDF_Font* pFont,
                                     CFX_ByteString& csNameTag) {
  return FindFont(m_pFormDict, pFont, csNameTag);
}

FX_BOOL CPDF_InterForm::FindFormFont(CFX_ByteString csFontName,
                                     CPDF_Font*& pFont,
                                     CFX_ByteString& csNameTag) {
  return FindFont(m_pFormDict, m_pDocument, csFontName, pFont, csNameTag);
}

void CPDF_InterForm::AddFormFont(const CPDF_Font* pFont,
                                 CFX_ByteString& csNameTag) {
  AddFont(m_pFormDict, m_pDocument, pFont, csNameTag);
}

CPDF_Font* CPDF_InterForm::AddNativeFormFont(uint8_t charSet,
                                             CFX_ByteString& csNameTag) {
  return ::AddNativeFont(m_pFormDict, m_pDocument, charSet, csNameTag);
}

CPDF_Font* CPDF_InterForm::AddNativeFormFont(CFX_ByteString& csNameTag) {
  return AddNativeInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}

void CPDF_InterForm::RemoveFormFont(const CPDF_Font* pFont) {
  RemoveFont(m_pFormDict, pFont);
}

void CPDF_InterForm::RemoveFormFont(CFX_ByteString csNameTag) {
  RemoveFont(m_pFormDict, csNameTag);
}

CPDF_DefaultAppearance CPDF_InterForm::GetDefaultAppearance() {
  if (!m_pFormDict)
    return CPDF_DefaultAppearance();
  return CPDF_DefaultAppearance(m_pFormDict->GetStringFor("DA"));
}

CPDF_Font* CPDF_InterForm::GetDefaultFormFont() {
  return GetDefaultFont(m_pFormDict, m_pDocument);
}

int CPDF_InterForm::GetFormAlignment() {
  return m_pFormDict ? m_pFormDict->GetIntegerFor("Q", 0) : 0;
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
  if (nLevel > nMaxRecursion)
    return;
  if (!pFieldDict)
    return;

  uint32_t dwParentObjNum = pFieldDict->GetObjNum();
  CPDF_Array* pKids = pFieldDict->GetArrayFor("Kids");
  if (!pKids) {
    AddTerminalField(pFieldDict);
    return;
  }

  CPDF_Dictionary* pFirstKid = pKids->GetDictAt(0);
  if (!pFirstKid)
    return;

  if (pFirstKid->KeyExist("T") || pFirstKid->KeyExist("Kids")) {
    for (size_t i = 0; i < pKids->GetCount(); i++) {
      CPDF_Dictionary* pChildDict = pKids->GetDictAt(i);
      if (pChildDict) {
        if (pChildDict->GetObjNum() != dwParentObjNum)
          LoadField(pChildDict, nLevel + 1);
      }
    }
  } else {
    AddTerminalField(pFieldDict);
  }
}

FX_BOOL CPDF_InterForm::HasXFAForm() const {
  return m_pFormDict && m_pFormDict->GetArrayFor("XFA");
}

void CPDF_InterForm::FixPageFields(const CPDF_Page* pPage) {
  CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
  if (!pPageDict)
    return;

  CPDF_Array* pAnnots = pPageDict->GetArrayFor("Annots");
  if (!pAnnots)
    return;

  for (size_t i = 0; i < pAnnots->GetCount(); i++) {
    CPDF_Dictionary* pAnnot = pAnnots->GetDictAt(i);
    if (pAnnot && pAnnot->GetStringFor("Subtype") == "Widget")
      LoadField(pAnnot);
  }
}

CPDF_FormField* CPDF_InterForm::AddTerminalField(CPDF_Dictionary* pFieldDict) {
  if (!pFieldDict->KeyExist("T"))
    return nullptr;

  CPDF_Dictionary* pDict = pFieldDict;
  CFX_WideString csWName = FPDF_GetFullName(pFieldDict);
  if (csWName.IsEmpty())
    return nullptr;

  CPDF_FormField* pField = nullptr;
  pField = m_pFieldTree->GetField(csWName);
  if (!pField) {
    CPDF_Dictionary* pParent = pFieldDict;
    if (!pFieldDict->KeyExist("T") &&
        pFieldDict->GetStringFor("Subtype") == "Widget") {
      pParent = pFieldDict->GetDictFor("Parent");
      if (!pParent)
        pParent = pFieldDict;
    }

    if (pParent && pParent != pFieldDict && !pParent->KeyExist("FT")) {
      if (pFieldDict->KeyExist("FT")) {
        CPDF_Object* pFTValue = pFieldDict->GetDirectObjectFor("FT");
        if (pFTValue)
          pParent->SetFor("FT", pFTValue->Clone());
      }

      if (pFieldDict->KeyExist("Ff")) {
        CPDF_Object* pFfValue = pFieldDict->GetDirectObjectFor("Ff");
        if (pFfValue)
          pParent->SetFor("Ff", pFfValue->Clone());
      }
    }

    pField = new CPDF_FormField(this, pParent);
    CPDF_Object* pTObj = pDict->GetObjectFor("T");
    if (ToReference(pTObj)) {
      CPDF_Object* pClone = pTObj->CloneDirectObject();
      if (pClone)
        pDict->SetFor("T", pClone);
      else
        pDict->SetNameFor("T", "");
    }
    m_pFieldTree->SetField(csWName, pField);
  }

  CPDF_Array* pKids = pFieldDict->GetArrayFor("Kids");
  if (!pKids) {
    if (pFieldDict->GetStringFor("Subtype") == "Widget")
      AddControl(pField, pFieldDict);
  } else {
    for (size_t i = 0; i < pKids->GetCount(); i++) {
      CPDF_Dictionary* pKid = pKids->GetDictAt(i);
      if (!pKid)
        continue;
      if (pKid->GetStringFor("Subtype") != "Widget")
        continue;

      AddControl(pField, pKid);
    }
  }
  return pField;
}

CPDF_FormControl* CPDF_InterForm::AddControl(CPDF_FormField* pField,
                                             CPDF_Dictionary* pWidgetDict) {
  const auto it = m_ControlMap.find(pWidgetDict);
  if (it != m_ControlMap.end())
    return it->second;

  CPDF_FormControl* pControl = new CPDF_FormControl(pField, pWidgetDict);
  m_ControlMap[pWidgetDict] = pControl;
  pField->m_ControlList.Add(pControl);
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
    uint32_t dwFlags = pField->GetFieldFlags();
    // TODO(thestig): Look up these magic numbers and add constants for them.
    if (dwFlags & 0x04)
      continue;

    bool bFind = true;
    if (fields)
      bFind = pdfium::ContainsValue(*fields, pField);
    if (bIncludeOrExclude == bFind) {
      CPDF_Dictionary* pFieldDict = pField->m_pDict;
      if ((dwFlags & 0x02) != 0 && pFieldDict->GetStringFor("V").IsEmpty())
        return pField;
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
  if (!pDoc)
    return nullptr;

  CPDF_Dictionary* pMainDict = pDoc->GetRoot()->GetDictFor("FDF");
  if (!pdf_path.IsEmpty()) {
    if (bSimpleFileSpec) {
      CFX_WideString wsFilePath = CPDF_FileSpec::EncodeFileName(pdf_path);
      pMainDict->SetStringFor("F", CFX_ByteString::FromUnicode(wsFilePath));
      pMainDict->SetStringFor("UF", PDF_EncodeText(wsFilePath));
    } else {
      CPDF_FileSpec filespec;
      filespec.SetFileName(pdf_path);
      pMainDict->SetFor("F", filespec.GetObj());
    }
  }

  CPDF_Array* pFields = new CPDF_Array;
  pMainDict->SetFor("Fields", pFields);
  int nCount = m_pFieldTree->m_Root.CountFields();
  for (int i = 0; i < nCount; i++) {
    CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
    if (!pField || pField->GetType() == CPDF_FormField::PushButton)
      continue;

    uint32_t dwFlags = pField->GetFieldFlags();
    if (dwFlags & 0x04)
      continue;

    if (bIncludeOrExclude == pdfium::ContainsValue(fields, pField)) {
      if ((dwFlags & 0x02) != 0 && pField->m_pDict->GetStringFor("V").IsEmpty())
        continue;

      CFX_WideString fullname = FPDF_GetFullName(pField->GetFieldDict());
      CPDF_Dictionary* pFieldDict = new CPDF_Dictionary;
      pFieldDict->SetFor("T", new CPDF_String(fullname));
      if (pField->GetType() == CPDF_FormField::CheckBox ||
          pField->GetType() == CPDF_FormField::RadioButton) {
        CFX_WideString csExport = pField->GetCheckValue(FALSE);
        CFX_ByteString csBExport = PDF_EncodeText(csExport);
        CPDF_Object* pOpt = FPDF_GetFieldAttr(pField->m_pDict, "Opt");
        if (pOpt)
          pFieldDict->SetStringFor("V", csBExport);
        else
          pFieldDict->SetNameFor("V", csBExport);
      } else {
        CPDF_Object* pV = FPDF_GetFieldAttr(pField->m_pDict, "V");
        if (pV)
          pFieldDict->SetFor("V", pV->CloneDirectObject());
      }
      pFields->Add(pFieldDict);
    }
  }
  return pDoc;
}

void CPDF_InterForm::FDF_ImportField(CPDF_Dictionary* pFieldDict,
                                     const CFX_WideString& parent_name,
                                     FX_BOOL bNotify,
                                     int nLevel) {
  CFX_WideString name;
  if (!parent_name.IsEmpty())
    name = parent_name + L".";

  name += pFieldDict->GetUnicodeTextFor("T");
  CPDF_Array* pKids = pFieldDict->GetArrayFor("Kids");
  if (pKids) {
    for (size_t i = 0; i < pKids->GetCount(); i++) {
      CPDF_Dictionary* pKid = pKids->GetDictAt(i);
      if (!pKid)
        continue;
      if (nLevel <= nMaxRecursion)
        FDF_ImportField(pKid, name, bNotify, nLevel + 1);
    }
    return;
  }
  if (!pFieldDict->KeyExist("V"))
    return;

  CPDF_FormField* pField = m_pFieldTree->GetField(name);
  if (!pField)
    return;

  CFX_WideString csWValue = GetFieldValue(*pFieldDict, m_bsEncoding);
  int iType = pField->GetFieldType();
  if (bNotify && m_pFormNotify) {
    int iRet = 0;
    if (iType == FIELDTYPE_LISTBOX)
      iRet = m_pFormNotify->BeforeSelectionChange(pField, csWValue);
    else if (iType == FIELDTYPE_COMBOBOX || iType == FIELDTYPE_TEXTFIELD)
      iRet = m_pFormNotify->BeforeValueChange(pField, csWValue);

    if (iRet < 0)
      return;
  }

  pField->SetValue(csWValue);
  CPDF_FormField::Type eType = pField->GetType();
  if ((eType == CPDF_FormField::ListBox || eType == CPDF_FormField::ComboBox) &&
      pFieldDict->KeyExist("Opt")) {
    pField->m_pDict->SetFor(
        "Opt", pFieldDict->GetDirectObjectFor("Opt")->CloneDirectObject());
  }

  if (bNotify && m_pFormNotify) {
    if (iType == FIELDTYPE_CHECKBOX || iType == FIELDTYPE_RADIOBUTTON)
      m_pFormNotify->AfterCheckedStatusChange(pField);
    else if (iType == FIELDTYPE_LISTBOX)
      m_pFormNotify->AfterSelectionChange(pField);
    else if (iType == FIELDTYPE_COMBOBOX || iType == FIELDTYPE_TEXTFIELD)
      m_pFormNotify->AfterValueChange(pField);
  }
}

FX_BOOL CPDF_InterForm::ImportFromFDF(const CFDF_Document* pFDF,
                                      FX_BOOL bNotify) {
  if (!pFDF)
    return FALSE;

  CPDF_Dictionary* pMainDict = pFDF->GetRoot()->GetDictFor("FDF");
  if (!pMainDict)
    return FALSE;

  CPDF_Array* pFields = pMainDict->GetArrayFor("Fields");
  if (!pFields)
    return FALSE;

  m_bsEncoding = pMainDict->GetStringFor("Encoding");
  if (bNotify && m_pFormNotify && m_pFormNotify->BeforeFormImportData(this) < 0)
    return FALSE;

  for (size_t i = 0; i < pFields->GetCount(); i++) {
    CPDF_Dictionary* pField = pFields->GetDictAt(i);
    if (!pField)
      continue;

    FDF_ImportField(pField, L"", bNotify);
  }
  if (bNotify && m_pFormNotify)
    m_pFormNotify->AfterFormImportData(this);
  return TRUE;
}

void CPDF_InterForm::SetFormNotify(IPDF_FormNotify* pNotify) {
  m_pFormNotify = pNotify;
}
