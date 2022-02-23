// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_interactiveform.h"

#include <utility>
#include <vector>

#include "build/build_config.h"
#include "constants/form_fields.h"
#include "constants/form_flags.h"
#include "constants/stream_dict_common.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_fontencoding.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cfdf_document.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fx_font.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/numerics/safe_conversions.h"

namespace {

const int nMaxRecursion = 32;

#if BUILDFLAG(IS_WIN)
struct PDF_FONTDATA {
  bool bFind;
  LOGFONTA lf;
};

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXA* lpelfe,
                               NEWTEXTMETRICEX* lpntme,
                               DWORD FontType,
                               LPARAM lParam) {
  if (FontType != 0x004 || strchr(lpelfe->elfLogFont.lfFaceName, '@'))
    return 1;

  PDF_FONTDATA* pData = (PDF_FONTDATA*)lParam;
  memcpy(&pData->lf, &lpelfe->elfLogFont, sizeof(LOGFONTA));
  pData->bFind = true;
  return 0;
}

bool RetrieveSpecificFont(FX_Charset charSet,
                          LPCSTR pcsFontName,
                          LOGFONTA& lf) {
  memset(&lf, 0, sizeof(LOGFONTA));
  lf.lfCharSet = static_cast<int>(charSet);
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  if (pcsFontName) {
    // TODO(dsinclair): Should this be strncpy?
    // NOLINTNEXTLINE(runtime/printf)
    strcpy(lf.lfFaceName, pcsFontName);
  }

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
#endif  // BUILDFLAG(IS_WIN)

ByteString GetNativeFontName(FX_Charset charSet, void* pLogFont) {
  ByteString csFontName;
#if BUILDFLAG(IS_WIN)
  LOGFONTA lf = {};
  if (charSet == FX_Charset::kANSI) {
    csFontName = CFX_Font::kDefaultAnsiFontName;
    return csFontName;
  }
  bool bRet = false;
  const ByteString default_font_name =
      CFX_Font::GetDefaultFontNameByCharset(charSet);
  if (!default_font_name.IsEmpty())
    bRet = RetrieveSpecificFont(charSet, default_font_name.c_str(), lf);
  if (!bRet) {
    bRet =
        RetrieveSpecificFont(charSet, CFX_Font::kUniversalDefaultFontName, lf);
  }
  if (!bRet)
    bRet = RetrieveSpecificFont(charSet, "Microsoft Sans Serif", lf);
  if (!bRet)
    bRet = RetrieveSpecificFont(charSet, nullptr, lf);
  if (bRet) {
    if (pLogFont)
      memcpy(pLogFont, &lf, sizeof(LOGFONTA));
    csFontName = lf.lfFaceName;
  }
#endif
  return csFontName;
}

ByteString GenerateNewFontResourceName(const CPDF_Dictionary* pResDict,
                                       const ByteString& csPrefix) {
  static const char kDummyFontName[] = "ZiTi";
  ByteString csStr = csPrefix;
  if (csStr.IsEmpty())
    csStr = kDummyFontName;

  const size_t szCount = csStr.GetLength();
  size_t m = 0;
  ByteString csTmp;
  while (m < strlen(kDummyFontName) && m < szCount)
    csTmp += csStr[m++];
  while (m < strlen(kDummyFontName)) {
    csTmp += '0' + m % 10;
    m++;
  }

  const CPDF_Dictionary* pDict = pResDict->GetDictFor("Font");
  DCHECK(pDict);

  int num = 0;
  ByteString bsNum;
  while (true) {
    ByteString csKey = csTmp + bsNum;
    if (!pDict->KeyExist(csKey))
      return csKey;
    if (m < szCount)
      csTmp += csStr[m++];
    else
      bsNum = ByteString::Format("%d", num++);

    m++;
  }
}

RetainPtr<CPDF_Font> AddStandardFont(CPDF_Document* pDocument) {
  auto* pPageData = CPDF_DocPageData::FromDocument(pDocument);
  static const CPDF_FontEncoding encoding(PDFFONT_ENCODING_WINANSI);
  return pPageData->AddStandardFont(CFX_Font::kDefaultAnsiFontName, &encoding);
}

RetainPtr<CPDF_Font> AddNativeFont(FX_Charset charSet,
                                   CPDF_Document* pDocument) {
  DCHECK(pDocument);

#if BUILDFLAG(IS_WIN)
  LOGFONTA lf;
  ByteString csFontName = GetNativeFontName(charSet, &lf);
  if (!csFontName.IsEmpty()) {
    if (csFontName == CFX_Font::kDefaultAnsiFontName)
      return AddStandardFont(pDocument);
    return CPDF_DocPageData::FromDocument(pDocument)->AddWindowsFont(&lf);
  }
#endif
  return nullptr;
}

bool FindFont(CPDF_Dictionary* pFormDict,
              const CPDF_Font* pFont,
              ByteString* csNameTag) {
  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return false;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!ValidateFontResourceDict(pFonts))
    return false;

  CPDF_DictionaryLocker locker(pFonts);
  for (const auto& it : locker) {
    const ByteString& csKey = it.first;
    const CPDF_Dictionary* pElement = ToDictionary(it.second->GetDirect());
    if (!ValidateDictType(pElement, "Font"))
      continue;
    if (pFont->GetFontDict() == pElement) {
      *csNameTag = csKey;
      return true;
    }
  }
  return false;
}

bool FindFontFromDoc(CPDF_Dictionary* pFormDict,
                     CPDF_Document* pDocument,
                     ByteString csFontName,
                     RetainPtr<CPDF_Font>& pFont,
                     ByteString* csNameTag) {
  if (csFontName.IsEmpty())
    return false;

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return false;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!ValidateFontResourceDict(pFonts))
    return false;

  csFontName.Remove(' ');
  CPDF_DictionaryLocker locker(pFonts);
  for (const auto& it : locker) {
    const ByteString& csKey = it.first;
    CPDF_Dictionary* pElement = ToDictionary(it.second->GetDirect());
    if (!ValidateDictType(pElement, "Font"))
      continue;

    pFont = CPDF_DocPageData::FromDocument(pDocument)->GetFont(pElement);
    if (!pFont)
      continue;

    ByteString csBaseFont = pFont->GetBaseFontName();
    csBaseFont.Remove(' ');
    if (csBaseFont == csFontName) {
      *csNameTag = csKey;
      return true;
    }
  }
  return false;
}

void AddFont(CPDF_Dictionary*& pFormDict,
             CPDF_Document* pDocument,
             const RetainPtr<CPDF_Font>& pFont,
             ByteString* csNameTag) {
  DCHECK(pFormDict);
  DCHECK(pFont);

  ByteString csTag;
  if (FindFont(pFormDict, pFont.Get(), &csTag)) {
    *csNameTag = std::move(csTag);
    return;
  }

  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    pDR = pFormDict->SetNewFor<CPDF_Dictionary>("DR");

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!pFonts)
    pFonts = pDR->SetNewFor<CPDF_Dictionary>("Font");

  if (csNameTag->IsEmpty())
    *csNameTag = pFont->GetBaseFontName();

  csNameTag->Remove(' ');
  *csNameTag = GenerateNewFontResourceName(pDR, *csNameTag);
  pFonts->SetNewFor<CPDF_Reference>(*csNameTag, pDocument,
                                    pFont->GetFontDict()->GetObjNum());
}

FX_Charset GetNativeCharSet() {
  return FX_GetCharsetFromCodePage(FX_GetACP());
}

void InitDict(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument) {
  DCHECK(pDocument);

  if (!pFormDict) {
    pFormDict = pDocument->NewIndirect<CPDF_Dictionary>();
    pDocument->GetRoot()->SetNewFor<CPDF_Reference>("AcroForm", pDocument,
                                                    pFormDict->GetObjNum());
  }

  ByteString csDA;
  if (!pFormDict->KeyExist("DR")) {
    ByteString csBaseName;
    FX_Charset charSet = GetNativeCharSet();
    RetainPtr<CPDF_Font> pFont = AddStandardFont(pDocument);
    if (pFont)
      AddFont(pFormDict, pDocument, pFont, &csBaseName);

    if (charSet != FX_Charset::kANSI) {
      ByteString csFontName = GetNativeFontName(charSet, nullptr);
      if (!pFont || csFontName != CFX_Font::kDefaultAnsiFontName) {
        pFont = AddNativeFont(charSet, pDocument);
        if (pFont) {
          csBaseName.clear();
          AddFont(pFormDict, pDocument, pFont, &csBaseName);
        }
      }
    }
    if (pFont)
      csDA = "/" + PDF_NameEncode(csBaseName) + " 0 Tf";
  }
  if (!csDA.IsEmpty())
    csDA += " ";

  csDA += "0 g";
  if (!pFormDict->KeyExist("DA"))
    pFormDict->SetNewFor<CPDF_String>("DA", csDA, false);
}

RetainPtr<CPDF_Font> GetNativeFont(CPDF_Dictionary* pFormDict,
                                   CPDF_Document* pDocument,
                                   FX_Charset charSet,
                                   ByteString* csNameTag) {
  CPDF_Dictionary* pDR = pFormDict->GetDictFor("DR");
  if (!pDR)
    return nullptr;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!ValidateFontResourceDict(pFonts))
    return nullptr;

  CPDF_DictionaryLocker locker(pFonts);
  for (const auto& it : locker) {
    const ByteString& csKey = it.first;
    CPDF_Dictionary* pElement = ToDictionary(it.second->GetDirect());
    if (!ValidateDictType(pElement, "Font"))
      continue;

    auto* pData = CPDF_DocPageData::FromDocument(pDocument);
    RetainPtr<CPDF_Font> pFind = pData->GetFont(pElement);
    if (!pFind)
      continue;

    CFX_SubstFont* pSubst = pFind->GetSubstFont();
    if (!pSubst)
      continue;

    if (pSubst->m_Charset == charSet) {
      *csNameTag = csKey;
      return pFind;
    }
  }
  return nullptr;
}

class CFieldNameExtractor {
 public:
  explicit CFieldNameExtractor(const WideString& full_name)
      : m_FullName(full_name) {}

  WideStringView GetNext() {
    size_t start_pos = m_iCur;
    while (m_iCur < m_FullName.GetLength() && m_FullName[m_iCur] != L'.')
      ++m_iCur;

    size_t length = m_iCur - start_pos;
    if (m_iCur < m_FullName.GetLength() && m_FullName[m_iCur] == L'.')
      ++m_iCur;

    return m_FullName.AsStringView().Substr(start_pos, length);
  }

 protected:
  const WideString m_FullName;
  size_t m_iCur = 0;
};

}  // namespace

class CFieldTree {
 public:
  class Node {
   public:
    Node() : m_level(0) {}
    Node(const WideString& short_name, int level)
        : m_ShortName(short_name), m_level(level) {}
    ~Node() = default;

    void AddChildNode(std::unique_ptr<Node> pNode) {
      m_Children.push_back(std::move(pNode));
    }

    size_t GetChildrenCount() const { return m_Children.size(); }

    Node* GetChildAt(size_t i) { return m_Children[i].get(); }
    const Node* GetChildAt(size_t i) const { return m_Children[i].get(); }

    CPDF_FormField* GetFieldAtIndex(size_t index) {
      size_t nFieldsToGo = index;
      return GetFieldInternal(&nFieldsToGo);
    }

    size_t CountFields() const { return CountFieldsInternal(); }

    void SetField(std::unique_ptr<CPDF_FormField> pField) {
      m_pField = std::move(pField);
    }

    CPDF_FormField* GetField() const { return m_pField.get(); }
    WideString GetShortName() const { return m_ShortName; }
    int GetLevel() const { return m_level; }

   private:
    CPDF_FormField* GetFieldInternal(size_t* pFieldsToGo) {
      if (m_pField) {
        if (*pFieldsToGo == 0)
          return m_pField.get();

        --*pFieldsToGo;
      }
      for (size_t i = 0; i < GetChildrenCount(); ++i) {
        CPDF_FormField* pField = GetChildAt(i)->GetFieldInternal(pFieldsToGo);
        if (pField)
          return pField;
      }
      return nullptr;
    }

    size_t CountFieldsInternal() const {
      size_t count = 0;
      if (m_pField)
        ++count;

      for (size_t i = 0; i < GetChildrenCount(); ++i)
        count += GetChildAt(i)->CountFieldsInternal();
      return count;
    }

    std::vector<std::unique_ptr<Node>> m_Children;
    WideString m_ShortName;
    std::unique_ptr<CPDF_FormField> m_pField;
    const int m_level;
  };

  CFieldTree();
  ~CFieldTree();

  bool SetField(const WideString& full_name,
                std::unique_ptr<CPDF_FormField> pField);
  CPDF_FormField* GetField(const WideString& full_name);

  Node* GetRoot() { return m_pRoot.get(); }
  Node* FindNode(const WideString& full_name);
  Node* AddChild(Node* pParent, const WideString& short_name);
  Node* Lookup(Node* pParent, WideStringView short_name);

 private:
  std::unique_ptr<Node> m_pRoot;
};

CFieldTree::CFieldTree() : m_pRoot(std::make_unique<Node>()) {}

CFieldTree::~CFieldTree() = default;

CFieldTree::Node* CFieldTree::AddChild(Node* pParent,
                                       const WideString& short_name) {
  if (!pParent)
    return nullptr;

  int level = pParent->GetLevel() + 1;
  if (level > nMaxRecursion)
    return nullptr;

  auto pNew = std::make_unique<Node>(short_name, pParent->GetLevel() + 1);
  Node* pChild = pNew.get();
  pParent->AddChildNode(std::move(pNew));
  return pChild;
}

CFieldTree::Node* CFieldTree::Lookup(Node* pParent, WideStringView short_name) {
  if (!pParent)
    return nullptr;

  for (size_t i = 0; i < pParent->GetChildrenCount(); ++i) {
    Node* pNode = pParent->GetChildAt(i);
    if (pNode->GetShortName() == short_name)
      return pNode;
  }
  return nullptr;
}

bool CFieldTree::SetField(const WideString& full_name,
                          std::unique_ptr<CPDF_FormField> pField) {
  if (full_name.IsEmpty())
    return false;

  Node* pNode = GetRoot();
  Node* pLast = nullptr;
  CFieldNameExtractor name_extractor(full_name);
  while (true) {
    WideStringView name_view = name_extractor.GetNext();
    if (name_view.IsEmpty())
      break;
    pLast = pNode;
    pNode = Lookup(pLast, name_view);
    if (pNode)
      continue;
    pNode = AddChild(pLast, WideString(name_view));
    if (!pNode)
      return false;
  }
  if (pNode == GetRoot())
    return false;

  pNode->SetField(std::move(pField));
  return true;
}

CPDF_FormField* CFieldTree::GetField(const WideString& full_name) {
  if (full_name.IsEmpty())
    return nullptr;

  Node* pNode = GetRoot();
  Node* pLast = nullptr;
  CFieldNameExtractor name_extractor(full_name);
  while (pNode) {
    WideStringView name_view = name_extractor.GetNext();
    if (name_view.IsEmpty())
      break;
    pLast = pNode;
    pNode = Lookup(pLast, name_view);
  }
  return pNode ? pNode->GetField() : nullptr;
}

CFieldTree::Node* CFieldTree::FindNode(const WideString& full_name) {
  if (full_name.IsEmpty())
    return nullptr;

  Node* pNode = GetRoot();
  Node* pLast = nullptr;
  CFieldNameExtractor name_extractor(full_name);
  while (pNode) {
    WideStringView name_view = name_extractor.GetNext();
    if (name_view.IsEmpty())
      break;
    pLast = pNode;
    pNode = Lookup(pLast, name_view);
  }
  return pNode;
}

CPDF_InteractiveForm::CPDF_InteractiveForm(CPDF_Document* pDocument)
    : m_pDocument(pDocument), m_pFieldTree(std::make_unique<CFieldTree>()) {
  CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  if (!pRoot)
    return;

  m_pFormDict.Reset(pRoot->GetDictFor("AcroForm"));
  if (!m_pFormDict)
    return;

  CPDF_Array* pFields = m_pFormDict->GetArrayFor("Fields");
  if (!pFields)
    return;

  for (size_t i = 0; i < pFields->size(); ++i)
    LoadField(pFields->GetDictAt(i), 0);
}

CPDF_InteractiveForm::~CPDF_InteractiveForm() = default;

bool CPDF_InteractiveForm::s_bUpdateAP = true;

// static
bool CPDF_InteractiveForm::IsUpdateAPEnabled() {
  return s_bUpdateAP;
}

// static
void CPDF_InteractiveForm::SetUpdateAP(bool bUpdateAP) {
  s_bUpdateAP = bUpdateAP;
}

// static
RetainPtr<CPDF_Font> CPDF_InteractiveForm::AddNativeInteractiveFormFont(
    CPDF_Dictionary*& pFormDict,
    CPDF_Document* pDocument,
    ByteString* csNameTag) {
  DCHECK(pDocument);
  DCHECK(csNameTag);

  if (!pFormDict)
    InitDict(pFormDict, pDocument);
  DCHECK(pFormDict);

  FX_Charset charSet = GetNativeCharSet();
  ByteString csTemp;
  RetainPtr<CPDF_Font> pFont =
      GetNativeFont(pFormDict, pDocument, charSet, &csTemp);
  if (pFont) {
    *csNameTag = std::move(csTemp);
    return pFont;
  }
  ByteString csFontName = GetNativeFontName(charSet, nullptr);
  if (FindFontFromDoc(pFormDict, pDocument, csFontName, pFont, csNameTag))
    return pFont;

  pFont = AddNativeFont(charSet, pDocument);
  if (!pFont)
    return nullptr;

  AddFont(pFormDict, pDocument, pFont, csNameTag);
  return pFont;
}

size_t CPDF_InteractiveForm::CountFields(const WideString& csFieldName) const {
  if (csFieldName.IsEmpty())
    return m_pFieldTree->GetRoot()->CountFields();

  CFieldTree::Node* pNode = m_pFieldTree->FindNode(csFieldName);
  return pNode ? pNode->CountFields() : 0;
}

CPDF_FormField* CPDF_InteractiveForm::GetField(
    size_t index,
    const WideString& csFieldName) const {
  if (csFieldName.IsEmpty())
    return m_pFieldTree->GetRoot()->GetFieldAtIndex(index);

  CFieldTree::Node* pNode = m_pFieldTree->FindNode(csFieldName);
  return pNode ? pNode->GetFieldAtIndex(index) : nullptr;
}

CPDF_FormField* CPDF_InteractiveForm::GetFieldByDict(
    CPDF_Dictionary* pFieldDict) const {
  if (!pFieldDict)
    return nullptr;

  WideString csWName = CPDF_FormField::GetFullNameForDict(pFieldDict);
  return m_pFieldTree->GetField(csWName);
}

const CPDF_FormControl* CPDF_InteractiveForm::GetControlAtPoint(
    const CPDF_Page* pPage,
    const CFX_PointF& point,
    int* z_order) const {
  const CPDF_Array* pAnnotList = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnotList)
    return nullptr;

  for (size_t i = pAnnotList->size(); i > 0; --i) {
    size_t annot_index = i - 1;
    const CPDF_Dictionary* pAnnot = pAnnotList->GetDictAt(annot_index);
    if (!pAnnot)
      continue;

    const auto it = m_ControlMap.find(pAnnot);
    if (it == m_ControlMap.end())
      continue;

    const CPDF_FormControl* pControl = it->second.get();
    if (!pControl->GetRect().Contains(point))
      continue;

    if (z_order)
      *z_order = static_cast<int>(annot_index);
    return pControl;
  }
  return nullptr;
}

CPDF_FormControl* CPDF_InteractiveForm::GetControlByDict(
    const CPDF_Dictionary* pWidgetDict) const {
  const auto it = m_ControlMap.find(pWidgetDict);
  return it != m_ControlMap.end() ? it->second.get() : nullptr;
}

bool CPDF_InteractiveForm::NeedConstructAP() const {
  return m_pFormDict && m_pFormDict->GetBooleanFor("NeedAppearances", false);
}

int CPDF_InteractiveForm::CountFieldsInCalculationOrder() {
  if (!m_pFormDict)
    return 0;

  CPDF_Array* pArray = m_pFormDict->GetArrayFor("CO");
  return pArray ? fxcrt::CollectionSize<int>(*pArray) : 0;
}

CPDF_FormField* CPDF_InteractiveForm::GetFieldInCalculationOrder(int index) {
  if (!m_pFormDict || index < 0)
    return nullptr;

  CPDF_Array* pArray = m_pFormDict->GetArrayFor("CO");
  if (!pArray)
    return nullptr;

  CPDF_Dictionary* pElement = ToDictionary(pArray->GetDirectObjectAt(index));
  return pElement ? GetFieldByDict(pElement) : nullptr;
}

int CPDF_InteractiveForm::FindFieldInCalculationOrder(
    const CPDF_FormField* pField) {
  if (!m_pFormDict)
    return -1;

  CPDF_Array* pArray = m_pFormDict->GetArrayFor("CO");
  if (!pArray)
    return -1;

  absl::optional<size_t> maybe_found = pArray->Find(pField->GetDict());
  if (!maybe_found.has_value())
    return -1;

  return pdfium::base::checked_cast<int>(maybe_found.value());
}

RetainPtr<CPDF_Font> CPDF_InteractiveForm::GetFormFont(
    ByteString csNameTag) const {
  ByteString csAlias = PDF_NameDecode(csNameTag.AsStringView());
  if (!m_pFormDict || csAlias.IsEmpty())
    return nullptr;

  CPDF_Dictionary* pDR = m_pFormDict->GetDictFor("DR");
  if (!pDR)
    return nullptr;

  CPDF_Dictionary* pFonts = pDR->GetDictFor("Font");
  if (!ValidateFontResourceDict(pFonts))
    return nullptr;

  CPDF_Dictionary* pElement = pFonts->GetDictFor(csAlias);
  if (!ValidateDictType(pElement, "Font"))
    return nullptr;

  return CPDF_DocPageData::FromDocument(m_pDocument)->GetFont(pElement);
}

CPDF_DefaultAppearance CPDF_InteractiveForm::GetDefaultAppearance() const {
  if (!m_pFormDict)
    return CPDF_DefaultAppearance();
  return CPDF_DefaultAppearance(m_pFormDict->GetStringFor("DA"));
}

int CPDF_InteractiveForm::GetFormAlignment() const {
  return m_pFormDict ? m_pFormDict->GetIntegerFor("Q", 0) : 0;
}

void CPDF_InteractiveForm::ResetForm(pdfium::span<CPDF_FormField*> fields,
                                     bool bIncludeOrExclude) {
  CFieldTree::Node* pRoot = m_pFieldTree->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_FormField* pField = pRoot->GetFieldAtIndex(i);
    if (!pField)
      continue;

    if (bIncludeOrExclude == pdfium::Contains(fields, pField))
      pField->ResetField();
  }
  if (m_pFormNotify)
    m_pFormNotify->AfterFormReset(this);
}

void CPDF_InteractiveForm::ResetForm() {
  ResetForm(/*fields=*/{}, /*bIncludeOrExclude=*/false);
}

const std::vector<UnownedPtr<CPDF_FormControl>>&
CPDF_InteractiveForm::GetControlsForField(const CPDF_FormField* pField) {
  return m_ControlLists[pField];
}

void CPDF_InteractiveForm::LoadField(CPDF_Dictionary* pFieldDict, int nLevel) {
  if (nLevel > nMaxRecursion)
    return;
  if (!pFieldDict)
    return;

  uint32_t dwParentObjNum = pFieldDict->GetObjNum();
  CPDF_Array* pKids = pFieldDict->GetArrayFor(pdfium::form_fields::kKids);
  if (!pKids) {
    AddTerminalField(pFieldDict);
    return;
  }

  CPDF_Dictionary* pFirstKid = pKids->GetDictAt(0);
  if (!pFirstKid)
    return;

  if (pFirstKid->KeyExist(pdfium::form_fields::kT) ||
      pFirstKid->KeyExist(pdfium::form_fields::kKids)) {
    for (size_t i = 0; i < pKids->size(); i++) {
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

void CPDF_InteractiveForm::FixPageFields(CPDF_Page* pPage) {
  CPDF_Array* pAnnots = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnots)
    return;

  for (size_t i = 0; i < pAnnots->size(); i++) {
    CPDF_Dictionary* pAnnot = pAnnots->GetDictAt(i);
    if (pAnnot && pAnnot->GetNameFor("Subtype") == "Widget")
      LoadField(pAnnot, 0);
  }
}

void CPDF_InteractiveForm::AddTerminalField(CPDF_Dictionary* pFieldDict) {
  if (!pFieldDict->KeyExist(pdfium::form_fields::kFT)) {
    // Key "FT" is required for terminal fields, it is also inheritable.
    CPDF_Dictionary* pParentDict =
        pFieldDict->GetDictFor(pdfium::form_fields::kParent);
    if (!pParentDict || !pParentDict->KeyExist(pdfium::form_fields::kFT))
      return;
  }

  CPDF_Dictionary* pDict = pFieldDict;
  WideString csWName = CPDF_FormField::GetFullNameForDict(pFieldDict);
  if (csWName.IsEmpty())
    return;

  CPDF_FormField* pField = nullptr;
  pField = m_pFieldTree->GetField(csWName);
  if (!pField) {
    CPDF_Dictionary* pParent = pFieldDict;
    if (!pFieldDict->KeyExist(pdfium::form_fields::kT) &&
        pFieldDict->GetNameFor("Subtype") == "Widget") {
      pParent = pFieldDict->GetDictFor(pdfium::form_fields::kParent);
      if (!pParent)
        pParent = pFieldDict;
    }

    if (pParent && pParent != pFieldDict &&
        !pParent->KeyExist(pdfium::form_fields::kFT)) {
      if (pFieldDict->KeyExist(pdfium::form_fields::kFT)) {
        CPDF_Object* pFTValue =
            pFieldDict->GetDirectObjectFor(pdfium::form_fields::kFT);
        if (pFTValue)
          pParent->SetFor(pdfium::form_fields::kFT, pFTValue->Clone());
      }

      if (pFieldDict->KeyExist(pdfium::form_fields::kFf)) {
        CPDF_Object* pFfValue =
            pFieldDict->GetDirectObjectFor(pdfium::form_fields::kFf);
        if (pFfValue)
          pParent->SetFor(pdfium::form_fields::kFf, pFfValue->Clone());
      }
    }

    auto newField = std::make_unique<CPDF_FormField>(this, pParent);
    pField = newField.get();
    CPDF_Object* pTObj = pDict->GetObjectFor(pdfium::form_fields::kT);
    if (ToReference(pTObj)) {
      RetainPtr<CPDF_Object> pClone = pTObj->CloneDirectObject();
      if (pClone)
        pDict->SetFor(pdfium::form_fields::kT, std::move(pClone));
      else
        pDict->SetNewFor<CPDF_Name>(pdfium::form_fields::kT, ByteString());
    }
    if (!m_pFieldTree->SetField(csWName, std::move(newField)))
      return;
  }

  CPDF_Array* pKids = pFieldDict->GetArrayFor(pdfium::form_fields::kKids);
  if (pKids) {
    for (size_t i = 0; i < pKids->size(); i++) {
      CPDF_Dictionary* pKid = pKids->GetDictAt(i);
      if (pKid && pKid->GetNameFor("Subtype") == "Widget")
        AddControl(pField, pKid);
    }
  } else {
    if (pFieldDict->GetNameFor("Subtype") == "Widget")
      AddControl(pField, pFieldDict);
  }
}

CPDF_FormControl* CPDF_InteractiveForm::AddControl(
    CPDF_FormField* pField,
    CPDF_Dictionary* pWidgetDict) {
  DCHECK(pWidgetDict);
  const auto it = m_ControlMap.find(pWidgetDict);
  if (it != m_ControlMap.end())
    return it->second.get();

  auto pNew = std::make_unique<CPDF_FormControl>(pField, pWidgetDict);
  CPDF_FormControl* pControl = pNew.get();
  m_ControlMap[pWidgetDict] = std::move(pNew);
  m_ControlLists[pField].emplace_back(pControl);
  return pControl;
}

bool CPDF_InteractiveForm::CheckRequiredFields(
    const std::vector<CPDF_FormField*>* fields,
    bool bIncludeOrExclude) const {
  CFieldTree::Node* pRoot = m_pFieldTree->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_FormField* pField = pRoot->GetFieldAtIndex(i);
    if (!pField)
      continue;

    int32_t iType = pField->GetType();
    if (iType == CPDF_FormField::kPushButton ||
        iType == CPDF_FormField::kCheckBox ||
        iType == CPDF_FormField::kListBox) {
      continue;
    }
    if (pField->IsNoExport())
      continue;

    bool bFind = true;
    if (fields)
      bFind = pdfium::Contains(*fields, pField);
    if (bIncludeOrExclude == bFind) {
      const CPDF_Dictionary* pFieldDict = pField->GetDict();
      if (pField->IsRequired() &&
          pFieldDict->GetStringFor(pdfium::form_fields::kV).IsEmpty()) {
        return false;
      }
    }
  }
  return true;
}

std::unique_ptr<CFDF_Document> CPDF_InteractiveForm::ExportToFDF(
    const WideString& pdf_path) const {
  std::vector<CPDF_FormField*> fields;
  CFieldTree::Node* pRoot = m_pFieldTree->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i)
    fields.push_back(pRoot->GetFieldAtIndex(i));
  return ExportToFDF(pdf_path, fields, true);
}

std::unique_ptr<CFDF_Document> CPDF_InteractiveForm::ExportToFDF(
    const WideString& pdf_path,
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude) const {
  std::unique_ptr<CFDF_Document> pDoc = CFDF_Document::CreateNewDoc();
  if (!pDoc)
    return nullptr;

  CPDF_Dictionary* pMainDict = pDoc->GetRoot()->GetDictFor("FDF");
  if (!pdf_path.IsEmpty()) {
    auto pNewDict = pDoc->New<CPDF_Dictionary>();
    pNewDict->SetNewFor<CPDF_Name>("Type", "Filespec");
    CPDF_FileSpec filespec(pNewDict.Get());
    filespec.SetFileName(pdf_path);
    pMainDict->SetFor("F", pNewDict);
  }

  CPDF_Array* pFields = pMainDict->SetNewFor<CPDF_Array>("Fields");
  CFieldTree::Node* pRoot = m_pFieldTree->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_FormField* pField = pRoot->GetFieldAtIndex(i);
    if (!pField || pField->GetType() == CPDF_FormField::kPushButton)
      continue;

    uint32_t dwFlags = pField->GetFieldFlags();
    if (dwFlags & pdfium::form_flags::kNoExport)
      continue;

    if (bIncludeOrExclude != pdfium::Contains(fields, pField))
      continue;

    if ((dwFlags & pdfium::form_flags::kRequired) != 0 &&
        pField->GetDict()->GetStringFor(pdfium::form_fields::kV).IsEmpty()) {
      continue;
    }

    WideString fullname =
        CPDF_FormField::GetFullNameForDict(pField->GetFieldDict());
    auto pFieldDict = pDoc->New<CPDF_Dictionary>();
    pFieldDict->SetNewFor<CPDF_String>(pdfium::form_fields::kT, fullname);
    if (pField->GetType() == CPDF_FormField::kCheckBox ||
        pField->GetType() == CPDF_FormField::kRadioButton) {
      WideString csExport = pField->GetCheckValue(false);
      ByteString csBExport = PDF_EncodeText(csExport);
      CPDF_Object* pOpt =
          CPDF_FormField::GetFieldAttr(pField->GetDict(), "Opt");
      if (pOpt) {
        pFieldDict->SetNewFor<CPDF_String>(pdfium::form_fields::kV, csBExport,
                                           false);
      } else {
        pFieldDict->SetNewFor<CPDF_Name>(pdfium::form_fields::kV, csBExport);
      }
    } else {
      CPDF_Object* pV = CPDF_FormField::GetFieldAttr(pField->GetDict(),
                                                     pdfium::form_fields::kV);
      if (pV)
        pFieldDict->SetFor(pdfium::form_fields::kV, pV->CloneDirectObject());
    }
    pFields->Append(pFieldDict);
  }
  return pDoc;
}

void CPDF_InteractiveForm::SetNotifierIface(NotifierIface* pNotify) {
  m_pFormNotify = pNotify;
}
