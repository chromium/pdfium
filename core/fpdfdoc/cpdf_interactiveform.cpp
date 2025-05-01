// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_interactiveform.h"

#include <optional>
#include <type_traits>
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
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/fx_font.h"

#if BUILDFLAG(IS_WIN)
#include "core/fxcrt/win/win_util.h"
#endif

namespace {

constexpr int kMaxRecursion = 32;

#if BUILDFLAG(IS_WIN)
struct PDF_FONTDATA {
  bool bFind;
  LOGFONTA lf;
};

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXA* lpelfe,
                               NEWTEXTMETRICEX* lpntme,
                               DWORD FontType,
                               LPARAM lParam) {
  if (FontType != 0x004 ||
      UNSAFE_TODO(strchr(lpelfe->elfLogFont.lfFaceName, '@'))) {
    return 1;
  }

  PDF_FONTDATA* pData = (PDF_FONTDATA*)lParam;
  pData->lf = lpelfe->elfLogFont;
  pData->bFind = true;
  return 0;
}

bool RetrieveSpecificFont(FX_Charset charset,
                          LPCSTR pcsFontName,
                          LOGFONTA& lf) {
  lf = {};  // Aggregate initialization, not construction.
  static_assert(std::is_aggregate_v<std::remove_reference_t<decltype(lf)>>);
  lf.lfCharSet = static_cast<int>(charset);
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  if (pcsFontName) {
    // TODO(dsinclair): Should this be strncpy?
    // NOLINTNEXTLINE(runtime/printf)
    UNSAFE_TODO(strcpy(lf.lfFaceName, pcsFontName));
  }

  PDF_FONTDATA fd = {};  // Aggregate initialization, not construction.
  static_assert(std::is_aggregate_v<decltype(fd)>);
  HDC hDC = ::GetDC(nullptr);
  EnumFontFamiliesExA(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc, (LPARAM)&fd,
                      0);
  ::ReleaseDC(nullptr, hDC);
  if (fd.bFind) {
    UNSAFE_TODO(FXSYS_memcpy(&lf, &fd.lf, sizeof(LOGFONTA)));
  }
  return fd.bFind;
}
#endif  // BUILDFLAG(IS_WIN)

ByteString GetNativeFontName(FX_Charset charset, void* log_font) {
  ByteString font_name;
#if BUILDFLAG(IS_WIN)
  LOGFONTA lf = {};
  if (charset == FX_Charset::kANSI) {
    return CFX_Font::kDefaultAnsiFontName;
  }

  if (!pdfium::IsUser32AndGdi32Available()) {
    // Without GDI32 and User32, GetDC / EnumFontFamiliesExW / ReleaseDC all
    // fail, which is called by RetrieveSpecificFont. We won't be able to look
    // up native fonts without GDI.
    return ByteString();
  }

  bool result = false;
  const ByteString default_font_name =
      CFX_Font::GetDefaultFontNameByCharset(charset);
  if (!default_font_name.IsEmpty()) {
    result = RetrieveSpecificFont(charset, default_font_name.c_str(), lf);
  }
  if (!result) {
    result =
        RetrieveSpecificFont(charset, CFX_Font::kUniversalDefaultFontName, lf);
  }
  if (!result) {
    result = RetrieveSpecificFont(charset, "Microsoft Sans Serif", lf);
  }
  if (!result) {
    result = RetrieveSpecificFont(charset, nullptr, lf);
  }
  if (result) {
    if (log_font) {
      UNSAFE_TODO(FXSYS_memcpy(log_font, &lf, sizeof(LOGFONTA)));
    }
    font_name = lf.lfFaceName;
  }
#endif
  return font_name;
}

ByteString GenerateNewFontResourceName(const CPDF_Dictionary* resource_dict,
                                       ByteString prefix) {
  static constexpr auto kDummyFontName = pdfium::span_from_cstring("ZiTi");
  if (prefix.IsEmpty()) {
    prefix = ByteStringView(kDummyFontName);
  }

  const size_t prefix_length = prefix.GetLength();
  size_t m = 0;
  ByteString actual_prefix;
  while (m < kDummyFontName.size() && m < prefix_length) {
    actual_prefix += prefix[m++];
  }
  while (m < kDummyFontName.size()) {
    actual_prefix += '0' + m % 10;
    m++;
  }

  RetainPtr<const CPDF_Dictionary> pDict = resource_dict->GetDictFor("Font");
  DCHECK(pDict);

  int num = 0;
  ByteString key_number;
  while (true) {
    ByteString key = actual_prefix + key_number;
    if (!pDict->KeyExist(key)) {
      return key;
    }

    if (m < prefix_length) {
      actual_prefix += prefix[m++];
    } else {
      key_number = ByteString::FormatInteger(num++);
    }
    m++;
  }
}

RetainPtr<CPDF_Font> AddStandardFont(CPDF_Document* document) {
  auto* page_data = CPDF_DocPageData::FromDocument(document);
  static const CPDF_FontEncoding encoding(FontEncoding::kWinAnsi);
  return page_data->AddStandardFont(CFX_Font::kDefaultAnsiFontName, &encoding);
}

RetainPtr<CPDF_Font> AddNativeFont(FX_Charset charset,
                                   CPDF_Document* document) {
  DCHECK(document);

#if BUILDFLAG(IS_WIN)
  LOGFONTA lf;
  ByteString font_name = GetNativeFontName(charset, &lf);
  if (!font_name.IsEmpty()) {
    if (font_name == CFX_Font::kDefaultAnsiFontName) {
      return AddStandardFont(document);
    }
    return CPDF_DocPageData::FromDocument(document)->AddWindowsFont(&lf);
  }
#endif
  return nullptr;
}

bool FindFont(const CPDF_Dictionary* form_dict,
              const CPDF_Font* font,
              ByteString* name_tag) {
  RetainPtr<const CPDF_Dictionary> pDR = form_dict->GetDictFor("DR");
  if (!pDR) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> font_dict = pDR->GetDictFor("Font");
  // TODO(tsepez): this eventually locks the dict, pass locker instead.
  if (!ValidateFontResourceDict(font_dict.Get())) {
    return false;
  }

  CPDF_DictionaryLocker locker(std::move(font_dict));
  for (const auto& it : locker) {
    const ByteString& key = it.first;
    RetainPtr<const CPDF_Dictionary> element =
        ToDictionary(it.second->GetDirect());
    if (!ValidateDictType(element.Get(), "Font")) {
      continue;
    }
    if (font->FontDictIs(element)) {
      *name_tag = key;
      return true;
    }
  }
  return false;
}

bool FindFontFromDoc(const CPDF_Dictionary* form_dict,
                     CPDF_Document* document,
                     ByteString font_name,
                     RetainPtr<CPDF_Font>& font,
                     ByteString* name_tag) {
  if (font_name.IsEmpty()) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> pDR = form_dict->GetDictFor("DR");
  if (!pDR) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> font_dict = pDR->GetDictFor("Font");
  if (!ValidateFontResourceDict(font_dict.Get())) {
    return false;
  }

  font_name.Remove(' ');
  CPDF_DictionaryLocker locker(font_dict);
  for (const auto& it : locker) {
    const ByteString& key = it.first;
    RetainPtr<CPDF_Dictionary> element =
        ToDictionary(it.second->GetMutableDirect());
    if (!ValidateDictType(element.Get(), "Font")) {
      continue;
    }

    auto* pData = CPDF_DocPageData::FromDocument(document);
    font = pData->GetFont(std::move(element));
    if (!font) {
      continue;
    }

    ByteString base_font = font->GetBaseFontName();
    base_font.Remove(' ');
    if (base_font == font_name) {
      *name_tag = key;
      return true;
    }
  }
  return false;
}

void AddFont(CPDF_Dictionary* form_dict,
             CPDF_Document* document,
             const RetainPtr<CPDF_Font>& font,
             ByteString* name_tag) {
  DCHECK(form_dict);
  DCHECK(font);

  ByteString tag;
  if (FindFont(form_dict, font.Get(), &tag)) {
    *name_tag = std::move(tag);
    return;
  }

  RetainPtr<CPDF_Dictionary> pDR = form_dict->GetOrCreateDictFor("DR");
  RetainPtr<CPDF_Dictionary> font_dict = pDR->GetOrCreateDictFor("Font");

  if (name_tag->IsEmpty()) {
    *name_tag = font->GetBaseFontName();
  }

  name_tag->Remove(' ');
  *name_tag = GenerateNewFontResourceName(pDR.Get(), *name_tag);
  font_dict->SetNewFor<CPDF_Reference>(*name_tag, document,
                                       font->GetFontDictObjNum());
}

FX_Charset GetNativeCharSet() {
  return FX_GetCharsetFromCodePage(FX_GetACP());
}

RetainPtr<CPDF_Dictionary> InitDict(CPDF_Document* document) {
  auto form_dict = document->NewIndirect<CPDF_Dictionary>();
  document->GetMutableRoot()->SetNewFor<CPDF_Reference>("AcroForm", document,
                                                        form_dict->GetObjNum());

  ByteString base_name;
  FX_Charset charset = GetNativeCharSet();
  RetainPtr<CPDF_Font> font = AddStandardFont(document);
  if (font) {
    AddFont(form_dict.Get(), document, font, &base_name);
  }
  if (charset != FX_Charset::kANSI) {
    ByteString font_name = GetNativeFontName(charset, nullptr);
    if (!font || font_name != CFX_Font::kDefaultAnsiFontName) {
      RetainPtr<CPDF_Font> native_font = AddNativeFont(charset, document);
      if (native_font) {
        base_name.clear();
        AddFont(form_dict.Get(), document, native_font, &base_name);
        font = std::move(native_font);
      }
    }
  }
  ByteString default_appearance;
  if (font) {
    default_appearance = "/" + PDF_NameEncode(base_name) + " 12 Tf ";
  }
  default_appearance += "0 g";
  form_dict->SetNewFor<CPDF_String>("DA", std::move(default_appearance));
  return form_dict;
}

RetainPtr<CPDF_Font> GetNativeFont(const CPDF_Dictionary* form_dict,
                                   CPDF_Document* document,
                                   FX_Charset charset,
                                   ByteString* name_tag) {
  RetainPtr<const CPDF_Dictionary> pDR = form_dict->GetDictFor("DR");
  if (!pDR) {
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> font_dict = pDR->GetDictFor("Font");
  if (!ValidateFontResourceDict(font_dict.Get())) {
    return nullptr;
  }

  CPDF_DictionaryLocker locker(font_dict);
  for (const auto& it : locker) {
    const ByteString& key = it.first;
    RetainPtr<CPDF_Dictionary> element =
        ToDictionary(it.second->GetMutableDirect());
    if (!ValidateDictType(element.Get(), "Font")) {
      continue;
    }

    auto* pData = CPDF_DocPageData::FromDocument(document);
    RetainPtr<CPDF_Font> pFind = pData->GetFont(std::move(element));
    if (!pFind) {
      continue;
    }

    auto maybe_charset = pFind->GetSubstFontCharset();
    if (maybe_charset.has_value() && maybe_charset.value() == charset) {
      *name_tag = key;
      return pFind;
    }
  }
  return nullptr;
}

class CFieldNameExtractor {
 public:
  explicit CFieldNameExtractor(const WideString& full_name)
      : full_name_(full_name) {}

  WideStringView GetNext() {
    size_t start_pos = cur_;
    while (cur_ < full_name_.GetLength() && full_name_[cur_] != L'.') {
      ++cur_;
    }

    size_t length = cur_ - start_pos;
    if (cur_ < full_name_.GetLength() && full_name_[cur_] == L'.') {
      ++cur_;
    }

    return full_name_.AsStringView().Substr(start_pos, length);
  }

 protected:
  const WideString full_name_;
  size_t cur_ = 0;
};

}  // namespace

class CFieldTree {
 public:
  class Node {
   public:
    Node() : level_(0) {}
    Node(const WideString& short_name, int level)
        : short_name_(short_name), level_(level) {}
    ~Node() = default;

    void AddChildNode(std::unique_ptr<Node> node) {
      children_.push_back(std::move(node));
    }

    size_t GetChildrenCount() const { return children_.size(); }

    Node* GetChildAt(size_t i) { return children_[i].get(); }
    const Node* GetChildAt(size_t i) const { return children_[i].get(); }

    CPDF_FormField* GetFieldAtIndex(size_t index) {
      size_t nFieldsToGo = index;
      return GetFieldInternal(&nFieldsToGo);
    }

    size_t CountFields() const { return CountFieldsInternal(); }

    void SetField(std::unique_ptr<CPDF_FormField> field) {
      field_ = std::move(field);
    }

    CPDF_FormField* GetField() const { return field_.get(); }
    WideString GetShortName() const { return short_name_; }
    int GetLevel() const { return level_; }

   private:
    CPDF_FormField* GetFieldInternal(size_t* pFieldsToGo) {
      if (field_) {
        if (*pFieldsToGo == 0) {
          return field_.get();
        }

        --*pFieldsToGo;
      }
      for (size_t i = 0; i < GetChildrenCount(); ++i) {
        CPDF_FormField* field = GetChildAt(i)->GetFieldInternal(pFieldsToGo);
        if (field) {
          return field;
        }
      }
      return nullptr;
    }

    size_t CountFieldsInternal() const {
      size_t count = 0;
      if (field_) {
        ++count;
      }

      for (size_t i = 0; i < GetChildrenCount(); ++i) {
        count += GetChildAt(i)->CountFieldsInternal();
      }
      return count;
    }

    std::vector<std::unique_ptr<Node>> children_;
    WideString short_name_;
    std::unique_ptr<CPDF_FormField> field_;
    const int level_;
  };

  CFieldTree();
  ~CFieldTree();

  bool SetField(const WideString& full_name,
                std::unique_ptr<CPDF_FormField> field);
  CPDF_FormField* GetField(const WideString& full_name);

  Node* GetRoot() { return root_.get(); }
  Node* FindNode(const WideString& full_name);
  Node* AddChild(Node* pParent, const WideString& short_name);
  Node* Lookup(Node* pParent, WideStringView short_name);

 private:
  std::unique_ptr<Node> root_;
};

CFieldTree::CFieldTree() : root_(std::make_unique<Node>()) {}

CFieldTree::~CFieldTree() = default;

CFieldTree::Node* CFieldTree::AddChild(Node* pParent,
                                       const WideString& short_name) {
  if (!pParent) {
    return nullptr;
  }

  int level = pParent->GetLevel() + 1;
  if (level > kMaxRecursion) {
    return nullptr;
  }

  auto new_node = std::make_unique<Node>(short_name, pParent->GetLevel() + 1);
  Node* pChild = new_node.get();
  pParent->AddChildNode(std::move(new_node));
  return pChild;
}

CFieldTree::Node* CFieldTree::Lookup(Node* pParent, WideStringView short_name) {
  if (!pParent) {
    return nullptr;
  }

  for (size_t i = 0; i < pParent->GetChildrenCount(); ++i) {
    Node* node = pParent->GetChildAt(i);
    if (node->GetShortName() == short_name) {
      return node;
    }
  }
  return nullptr;
}

bool CFieldTree::SetField(const WideString& full_name,
                          std::unique_ptr<CPDF_FormField> field) {
  if (full_name.IsEmpty()) {
    return false;
  }

  Node* node = GetRoot();
  Node* last_node = nullptr;
  CFieldNameExtractor name_extractor(full_name);
  while (true) {
    WideStringView name_view = name_extractor.GetNext();
    if (name_view.IsEmpty()) {
      break;
    }
    last_node = node;
    node = Lookup(last_node, name_view);
    if (node) {
      continue;
    }
    node = AddChild(last_node, WideString(name_view));
    if (!node) {
      return false;
    }
  }
  if (node == GetRoot()) {
    return false;
  }

  node->SetField(std::move(field));
  return true;
}

CPDF_FormField* CFieldTree::GetField(const WideString& full_name) {
  if (full_name.IsEmpty()) {
    return nullptr;
  }

  Node* node = GetRoot();
  Node* last_node = nullptr;
  CFieldNameExtractor name_extractor(full_name);
  while (node) {
    WideStringView name_view = name_extractor.GetNext();
    if (name_view.IsEmpty()) {
      break;
    }
    last_node = node;
    node = Lookup(last_node, name_view);
  }
  return node ? node->GetField() : nullptr;
}

CFieldTree::Node* CFieldTree::FindNode(const WideString& full_name) {
  if (full_name.IsEmpty()) {
    return nullptr;
  }

  Node* node = GetRoot();
  Node* last_node = nullptr;
  CFieldNameExtractor name_extractor(full_name);
  while (node) {
    WideStringView name_view = name_extractor.GetNext();
    if (name_view.IsEmpty()) {
      break;
    }
    last_node = node;
    node = Lookup(last_node, name_view);
  }
  return node;
}

CPDF_InteractiveForm::CPDF_InteractiveForm(CPDF_Document* document)
    : document_(document), field_tree_(std::make_unique<CFieldTree>()) {
  RetainPtr<CPDF_Dictionary> pRoot = document_->GetMutableRoot();
  if (!pRoot) {
    return;
  }

  form_dict_ = pRoot->GetMutableDictFor("AcroForm");
  if (!form_dict_) {
    return;
  }

  RetainPtr<CPDF_Array> fields = form_dict_->GetMutableArrayFor("Fields");
  if (!fields) {
    return;
  }

  for (size_t i = 0; i < fields->size(); ++i) {
    LoadField(fields->GetMutableDictAt(i), 0);
  }
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
    CPDF_Document* document,
    ByteString* name_tag) {
  DCHECK(document);
  DCHECK(name_tag);

  RetainPtr<CPDF_Dictionary> form_dict =
      document->GetMutableRoot()->GetMutableDictFor("AcroForm");
  if (!form_dict) {
    form_dict = InitDict(document);
  }

  FX_Charset charset = GetNativeCharSet();
  ByteString tag;
  RetainPtr<CPDF_Font> font =
      GetNativeFont(form_dict.Get(), document, charset, &tag);
  if (font) {
    *name_tag = std::move(tag);
    return font;
  }
  ByteString font_name = GetNativeFontName(charset, nullptr);
  if (FindFontFromDoc(form_dict.Get(), document, font_name, font, name_tag)) {
    return font;
  }

  font = AddNativeFont(charset, document);
  if (!font) {
    return nullptr;
  }

  AddFont(form_dict.Get(), document, font, name_tag);
  return font;
}

// static
RetainPtr<CPDF_Dictionary> CPDF_InteractiveForm::InitAcroFormDict(
    CPDF_Document* document) {
  return InitDict(document);
}

size_t CPDF_InteractiveForm::CountFields(const WideString& field_name) const {
  if (field_name.IsEmpty()) {
    return field_tree_->GetRoot()->CountFields();
  }

  CFieldTree::Node* node = field_tree_->FindNode(field_name);
  return node ? node->CountFields() : 0;
}

CPDF_FormField* CPDF_InteractiveForm::GetField(
    size_t index,
    const WideString& field_name) const {
  if (field_name.IsEmpty()) {
    return field_tree_->GetRoot()->GetFieldAtIndex(index);
  }

  CFieldTree::Node* node = field_tree_->FindNode(field_name);
  return node ? node->GetFieldAtIndex(index) : nullptr;
}

CPDF_FormField* CPDF_InteractiveForm::GetFieldByDict(
    const CPDF_Dictionary* field_dict) const {
  if (!field_dict) {
    return nullptr;
  }

  return field_tree_->GetField(CPDF_FormField::GetFullNameForDict(field_dict));
}

const CPDF_FormControl* CPDF_InteractiveForm::GetControlAtPoint(
    const CPDF_Page* page,
    const CFX_PointF& point,
    int* z_order) const {
  RetainPtr<const CPDF_Array> annots = page->GetAnnotsArray();
  if (!annots) {
    return nullptr;
  }

  for (size_t i = annots->size(); i > 0; --i) {
    size_t annot_index = i - 1;
    RetainPtr<const CPDF_Dictionary> annot = annots->GetDictAt(annot_index);
    if (!annot) {
      continue;
    }

    const auto it = control_map_.find(annot.Get());
    if (it == control_map_.end()) {
      continue;
    }

    const CPDF_FormControl* control = it->second.get();
    if (!control->GetRect().Contains(point)) {
      continue;
    }

    if (z_order) {
      *z_order = static_cast<int>(annot_index);
    }
    return control;
  }
  return nullptr;
}

CPDF_FormControl* CPDF_InteractiveForm::GetControlByDict(
    const CPDF_Dictionary* widget_dict) const {
  const auto it = control_map_.find(widget_dict);
  return it != control_map_.end() ? it->second.get() : nullptr;
}

bool CPDF_InteractiveForm::NeedConstructAP() const {
  return form_dict_ && form_dict_->GetBooleanFor("NeedAppearances", false);
}

int CPDF_InteractiveForm::CountFieldsInCalculationOrder() {
  if (!form_dict_) {
    return 0;
  }

  RetainPtr<const CPDF_Array> pArray = form_dict_->GetArrayFor("CO");
  return pArray ? fxcrt::CollectionSize<int>(*pArray) : 0;
}

CPDF_FormField* CPDF_InteractiveForm::GetFieldInCalculationOrder(int index) {
  if (!form_dict_ || index < 0) {
    return nullptr;
  }

  RetainPtr<const CPDF_Array> pArray = form_dict_->GetArrayFor("CO");
  if (!pArray) {
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> element =
      ToDictionary(pArray->GetDirectObjectAt(index));
  return element ? GetFieldByDict(element.Get()) : nullptr;
}

int CPDF_InteractiveForm::FindFieldInCalculationOrder(
    const CPDF_FormField* field) {
  if (!form_dict_) {
    return -1;
  }

  RetainPtr<const CPDF_Array> pArray = form_dict_->GetArrayFor("CO");
  if (!pArray) {
    return -1;
  }

  std::optional<size_t> maybe_found = pArray->Find(field->GetFieldDict());
  if (!maybe_found.has_value()) {
    return -1;
  }

  return pdfium::checked_cast<int>(maybe_found.value());
}

RetainPtr<CPDF_Font> CPDF_InteractiveForm::GetFormFont(
    ByteString name_tag) const {
  if (!form_dict_) {
    return nullptr;
  }
  ByteString alias = PDF_NameDecode(name_tag.AsStringView());
  if (alias.IsEmpty()) {
    return nullptr;
  }

  RetainPtr<CPDF_Dictionary> pDR = form_dict_->GetMutableDictFor("DR");
  if (!pDR) {
    return nullptr;
  }

  RetainPtr<CPDF_Dictionary> font_dict = pDR->GetMutableDictFor("Font");
  if (!ValidateFontResourceDict(font_dict.Get())) {
    return nullptr;
  }

  RetainPtr<CPDF_Dictionary> element = font_dict->GetMutableDictFor(alias);
  if (!ValidateDictType(element.Get(), "Font")) {
    return nullptr;
  }

  return GetFontForElement(std::move(element));
}

RetainPtr<CPDF_Font> CPDF_InteractiveForm::GetFontForElement(
    RetainPtr<CPDF_Dictionary> element) const {
  auto* pData = CPDF_DocPageData::FromDocument(document_);
  return pData->GetFont(std::move(element));
}

CPDF_DefaultAppearance CPDF_InteractiveForm::GetDefaultAppearance() const {
  return CPDF_DefaultAppearance(form_dict_ ? form_dict_->GetByteStringFor("DA")
                                           : "");
}

int CPDF_InteractiveForm::GetFormAlignment() const {
  return form_dict_ ? form_dict_->GetIntegerFor("Q", 0) : 0;
}

void CPDF_InteractiveForm::ResetForm(pdfium::span<CPDF_FormField*> fields,
                                     bool bIncludeOrExclude) {
  CFieldTree::Node* pRoot = field_tree_->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_FormField* field = pRoot->GetFieldAtIndex(i);
    if (!field) {
      continue;
    }

    if (bIncludeOrExclude == pdfium::Contains(fields, field)) {
      field->ResetField();
    }
  }
  if (form_notify_) {
    form_notify_->AfterFormReset(this);
  }
}

void CPDF_InteractiveForm::ResetForm() {
  ResetForm(/*fields=*/{}, /*bIncludeOrExclude=*/false);
}

const std::vector<UnownedPtr<CPDF_FormControl>>&
CPDF_InteractiveForm::GetControlsForField(const CPDF_FormField* field) {
  return control_lists_[pdfium::WrapUnowned(field)];
}

void CPDF_InteractiveForm::LoadField(RetainPtr<CPDF_Dictionary> field_dict,
                                     int nLevel) {
  if (nLevel > kMaxRecursion) {
    return;
  }
  if (!field_dict) {
    return;
  }

  uint32_t dwParentObjNum = field_dict->GetObjNum();
  RetainPtr<CPDF_Array> kids =
      field_dict->GetMutableArrayFor(pdfium::form_fields::kKids);
  if (!kids) {
    AddTerminalField(std::move(field_dict));
    return;
  }

  RetainPtr<const CPDF_Dictionary> pFirstKid = kids->GetDictAt(0);
  if (!pFirstKid) {
    return;
  }

  if (!pFirstKid->KeyExist(pdfium::form_fields::kT) &&
      !pFirstKid->KeyExist(pdfium::form_fields::kKids)) {
    AddTerminalField(std::move(field_dict));
    return;
  }
  for (size_t i = 0; i < kids->size(); i++) {
    RetainPtr<CPDF_Dictionary> pChildDict = kids->GetMutableDictAt(i);
    if (pChildDict && pChildDict->GetObjNum() != dwParentObjNum) {
      LoadField(std::move(pChildDict), nLevel + 1);
    }
  }
}

void CPDF_InteractiveForm::FixPageFields(CPDF_Page* page) {
  RetainPtr<CPDF_Array> annots = page->GetMutableAnnotsArray();
  if (!annots) {
    return;
  }

  for (size_t i = 0; i < annots->size(); i++) {
    RetainPtr<CPDF_Dictionary> annot = annots->GetMutableDictAt(i);
    if (annot && annot->GetNameFor("Subtype") == "Widget") {
      LoadField(std::move(annot), 0);
    }
  }
}

void CPDF_InteractiveForm::AddTerminalField(
    RetainPtr<CPDF_Dictionary> field_dict) {
  if (!field_dict->KeyExist(pdfium::form_fields::kFT)) {
    // Key "FT" is required for terminal fields, it is also inheritable.
    RetainPtr<const CPDF_Dictionary> pParentDict =
        field_dict->GetDictFor(pdfium::form_fields::kParent);
    if (!pParentDict || !pParentDict->KeyExist(pdfium::form_fields::kFT)) {
      return;
    }
  }

  WideString field_name = CPDF_FormField::GetFullNameForDict(field_dict.Get());
  if (field_name.IsEmpty()) {
    return;
  }

  CPDF_FormField* field = field_tree_->GetField(field_name);
  if (!field) {
    RetainPtr<CPDF_Dictionary> pParent(field_dict);
    if (!field_dict->KeyExist(pdfium::form_fields::kT) &&
        field_dict->GetNameFor("Subtype") == "Widget") {
      pParent = field_dict->GetMutableDictFor(pdfium::form_fields::kParent);
      if (!pParent) {
        pParent = field_dict;
      }
    }

    if (pParent && pParent != field_dict &&
        !pParent->KeyExist(pdfium::form_fields::kFT)) {
      if (field_dict->KeyExist(pdfium::form_fields::kFT)) {
        RetainPtr<const CPDF_Object> pFTValue =
            field_dict->GetDirectObjectFor(pdfium::form_fields::kFT);
        if (pFTValue) {
          pParent->SetFor(pdfium::form_fields::kFT, pFTValue->Clone());
        }
      }

      if (field_dict->KeyExist(pdfium::form_fields::kFf)) {
        RetainPtr<const CPDF_Object> pFfValue =
            field_dict->GetDirectObjectFor(pdfium::form_fields::kFf);
        if (pFfValue) {
          pParent->SetFor(pdfium::form_fields::kFf, pFfValue->Clone());
        }
      }
    }

    auto new_field = std::make_unique<CPDF_FormField>(this, std::move(pParent));
    field = new_field.get();
    RetainPtr<const CPDF_Object> t_obj =
        field_dict->GetObjectFor(pdfium::form_fields::kT);
    if (ToReference(t_obj)) {
      RetainPtr<CPDF_Object> t_obj_clone = t_obj->CloneDirectObject();
      if (t_obj_clone && t_obj_clone->IsString()) {
        field_dict->SetFor(pdfium::form_fields::kT, std::move(t_obj_clone));
      } else {
        field_dict->SetNewFor<CPDF_String>(pdfium::form_fields::kT,
                                           ByteString());
      }
    }
    if (!field_tree_->SetField(field_name, std::move(new_field))) {
      return;
    }
  }

  RetainPtr<CPDF_Array> kids =
      field_dict->GetMutableArrayFor(pdfium::form_fields::kKids);
  if (!kids) {
    if (field_dict->GetNameFor("Subtype") == "Widget") {
      AddControl(field, std::move(field_dict));
    }
    return;
  }
  for (size_t i = 0; i < kids->size(); i++) {
    RetainPtr<CPDF_Dictionary> kid = kids->GetMutableDictAt(i);
    if (kid && kid->GetNameFor("Subtype") == "Widget") {
      AddControl(field, std::move(kid));
    }
  }
}

CPDF_FormControl* CPDF_InteractiveForm::AddControl(
    CPDF_FormField* field,
    RetainPtr<CPDF_Dictionary> widget_dict) {
  DCHECK(widget_dict);
  const auto it = control_map_.find(widget_dict.Get());
  if (it != control_map_.end()) {
    return it->second.get();
  }

  auto new_control =
      std::make_unique<CPDF_FormControl>(field, widget_dict, this);
  CPDF_FormControl* control = new_control.get();
  control_map_[widget_dict] = std::move(new_control);
  control_lists_[pdfium::WrapUnowned(field)].emplace_back(control);
  return control;
}

bool CPDF_InteractiveForm::CheckRequiredFields(
    const std::vector<CPDF_FormField*>* fields,
    bool bIncludeOrExclude) const {
  CFieldTree::Node* pRoot = field_tree_->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_FormField* field = pRoot->GetFieldAtIndex(i);
    if (!field) {
      continue;
    }

    int32_t iType = field->GetType();
    if (iType == CPDF_FormField::kPushButton ||
        iType == CPDF_FormField::kCheckBox ||
        iType == CPDF_FormField::kListBox) {
      continue;
    }
    if (field->IsNoExport()) {
      continue;
    }

    bool bFind = true;
    if (fields) {
      bFind = pdfium::Contains(*fields, field);
    }
    if (bIncludeOrExclude == bFind) {
      RetainPtr<const CPDF_Dictionary> field_dict = field->GetFieldDict();
      if (field->IsRequired() &&
          field_dict->GetByteStringFor(pdfium::form_fields::kV).IsEmpty()) {
        return false;
      }
    }
  }
  return true;
}

std::unique_ptr<CFDF_Document> CPDF_InteractiveForm::ExportToFDF(
    const WideString& pdf_path) const {
  std::vector<CPDF_FormField*> fields;
  CFieldTree::Node* pRoot = field_tree_->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    fields.push_back(pRoot->GetFieldAtIndex(i));
  }
  return ExportToFDF(pdf_path, fields, true);
}

std::unique_ptr<CFDF_Document> CPDF_InteractiveForm::ExportToFDF(
    const WideString& pdf_path,
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude) const {
  std::unique_ptr<CFDF_Document> pDoc = CFDF_Document::CreateNewDoc();
  if (!pDoc) {
    return nullptr;
  }

  RetainPtr<CPDF_Dictionary> pMainDict =
      pDoc->GetMutableRoot()->GetMutableDictFor("FDF");
  if (!pdf_path.IsEmpty()) {
    auto new_dict = pDoc->New<CPDF_Dictionary>();
    new_dict->SetNewFor<CPDF_Name>("Type", "Filespec");
    WideString wsStr = CPDF_FileSpec::EncodeFileName(pdf_path);
    new_dict->SetNewFor<CPDF_String>(pdfium::stream::kF, wsStr.ToDefANSI());
    new_dict->SetNewFor<CPDF_String>("UF", wsStr.AsStringView());
    pMainDict->SetFor("F", new_dict);
  }

  auto fields_array = pMainDict->SetNewFor<CPDF_Array>("Fields");
  CFieldTree::Node* pRoot = field_tree_->GetRoot();
  const size_t nCount = pRoot->CountFields();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_FormField* field = pRoot->GetFieldAtIndex(i);
    if (!field || field->GetType() == CPDF_FormField::kPushButton) {
      continue;
    }

    uint32_t dwFlags = field->GetFieldFlags();
    if (dwFlags & pdfium::form_flags::kNoExport) {
      continue;
    }

    if (bIncludeOrExclude != pdfium::Contains(fields, field)) {
      continue;
    }

    if ((dwFlags & pdfium::form_flags::kRequired) != 0 &&
        field->GetFieldDict()
            ->GetByteStringFor(pdfium::form_fields::kV)
            .IsEmpty()) {
      continue;
    }

    WideString fullname =
        CPDF_FormField::GetFullNameForDict(field->GetFieldDict());
    auto field_dict = pDoc->New<CPDF_Dictionary>();
    field_dict->SetNewFor<CPDF_String>(pdfium::form_fields::kT,
                                       fullname.AsStringView());
    if (field->GetType() == CPDF_FormField::kCheckBox ||
        field->GetType() == CPDF_FormField::kRadioButton) {
      ByteString export_value =
          PDF_EncodeText(field->GetCheckValue(false).AsStringView());
      RetainPtr<const CPDF_Object> opt = field->GetFieldAttr("Opt");
      if (opt) {
        field_dict->SetNewFor<CPDF_String>(pdfium::form_fields::kV,
                                           export_value);
      } else {
        field_dict->SetNewFor<CPDF_Name>(pdfium::form_fields::kV, export_value);
      }
    } else {
      RetainPtr<const CPDF_Object> value =
          field->GetFieldAttr(pdfium::form_fields::kV);
      if (value) {
        field_dict->SetFor(pdfium::form_fields::kV, value->CloneDirectObject());
      }
    }
    fields_array->Append(field_dict);
  }
  return pDoc;
}

void CPDF_InteractiveForm::SetNotifierIface(NotifierIface* notify) {
  form_notify_ = notify;
}

bool CPDF_InteractiveForm::NotifyBeforeValueChange(CPDF_FormField* field,
                                                   const WideString& value) {
  return !form_notify_ || form_notify_->BeforeValueChange(field, value);
}

void CPDF_InteractiveForm::NotifyAfterValueChange(CPDF_FormField* field) {
  if (form_notify_) {
    form_notify_->AfterValueChange(field);
  }
}

bool CPDF_InteractiveForm::NotifyBeforeSelectionChange(
    CPDF_FormField* field,
    const WideString& value) {
  return !form_notify_ || form_notify_->BeforeSelectionChange(field, value);
}

void CPDF_InteractiveForm::NotifyAfterSelectionChange(CPDF_FormField* field) {
  if (form_notify_) {
    form_notify_->AfterSelectionChange(field);
  }
}

void CPDF_InteractiveForm::NotifyAfterCheckedStatusChange(
    CPDF_FormField* field) {
  if (form_notify_) {
    form_notify_->AfterCheckedStatusChange(field);
  }
}
