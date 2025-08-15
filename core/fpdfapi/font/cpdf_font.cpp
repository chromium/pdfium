// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_font.h"

#include <algorithm>
#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "constants/font_encodings.h"
#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_fontencoding.h"
#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/font/cpdf_tounicodemap.h"
#include "core/fpdfapi/font/cpdf_truetypefont.h"
#include "core/fpdfapi/font/cpdf_type1font.h"
#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/fx_fontencoding.h"

namespace {

constexpr std::array<const char*, 5> kChineseFontNames = {{
    "\xCB\xCE\xCC\xE5",
    "\xBF\xAC\xCC\xE5",
    "\xBA\xDA\xCC\xE5",
    "\xB7\xC2\xCB\xCE",
    "\xD0\xC2\xCB\xCE",
}};

}  // namespace

CPDF_Font::CPDF_Font(CPDF_Document* document,
                     RetainPtr<CPDF_Dictionary> font_dict)
    : document_(document),
      font_dict_(std::move(font_dict)),
      base_font_name_(font_dict_->GetByteStringFor("BaseFont")) {}

CPDF_Font::~CPDF_Font() {
  if (!will_be_destroyed_ && font_file_) {
    document_->MaybePurgeFontFileStreamAcc(std::move(font_file_));
  }
}

bool CPDF_Font::IsType1Font() const {
  return false;
}

bool CPDF_Font::IsTrueTypeFont() const {
  return false;
}

bool CPDF_Font::IsType3Font() const {
  return false;
}

bool CPDF_Font::IsCIDFont() const {
  return false;
}

const CPDF_Type1Font* CPDF_Font::AsType1Font() const {
  return nullptr;
}

CPDF_Type1Font* CPDF_Font::AsType1Font() {
  return nullptr;
}

const CPDF_TrueTypeFont* CPDF_Font::AsTrueTypeFont() const {
  return nullptr;
}

CPDF_TrueTypeFont* CPDF_Font::AsTrueTypeFont() {
  return nullptr;
}

const CPDF_Type3Font* CPDF_Font::AsType3Font() const {
  return nullptr;
}

CPDF_Type3Font* CPDF_Font::AsType3Font() {
  return nullptr;
}

const CPDF_CIDFont* CPDF_Font::AsCIDFont() const {
  return nullptr;
}

CPDF_CIDFont* CPDF_Font::AsCIDFont() {
  return nullptr;
}

size_t CPDF_Font::CountChar(ByteStringView pString) const {
  return pString.GetLength();
}

#if BUILDFLAG(IS_APPLE)
int CPDF_Font::GlyphFromCharCodeExt(uint32_t charcode) {
  return GlyphFromCharCode(charcode, nullptr);
}
#endif

void CPDF_Font::WillBeDestroyed() {
  will_be_destroyed_ = true;
}

bool CPDF_Font::IsVertWriting() const {
  const CPDF_CIDFont* pCIDFont = AsCIDFont();
  return pCIDFont ? pCIDFont->IsVertWriting() : font_.IsVertical();
}

void CPDF_Font::AppendChar(ByteString* str, uint32_t charcode) const {
  *str += static_cast<char>(charcode);
}

WideString CPDF_Font::UnicodeFromCharCode(uint32_t charcode) const {
  if (!to_unicode_loaded_) {
    LoadUnicodeMap();
  }

  return to_unicode_map_ ? to_unicode_map_->Lookup(charcode) : WideString();
}

uint32_t CPDF_Font::CharCodeFromUnicode(wchar_t unicode) const {
  if (!to_unicode_loaded_) {
    LoadUnicodeMap();
  }

  return to_unicode_map_ ? to_unicode_map_->ReverseLookup(unicode) : 0;
}

bool CPDF_Font::HasFontWidths() const {
  return true;
}

void CPDF_Font::LoadFontDescriptor(const CPDF_Dictionary* font_desc) {
  flags_ = font_desc->GetIntegerFor("Flags", pdfium::kFontStyleNonSymbolic);
  int ItalicAngle = 0;
  bool bExistItalicAngle = false;
  if (font_desc->KeyExist("ItalicAngle")) {
    ItalicAngle = font_desc->GetIntegerFor("ItalicAngle");
    bExistItalicAngle = true;
  }
  if (ItalicAngle < 0) {
    flags_ |= pdfium::kFontStyleItalic;
    italic_angle_ = ItalicAngle;
  }
  bool bExistStemV = false;
  if (font_desc->KeyExist("StemV")) {
    stem_v_ = font_desc->GetIntegerFor("StemV");
    bExistStemV = true;
  }
  bool bExistAscent = false;
  if (font_desc->KeyExist("Ascent")) {
    ascent_ = font_desc->GetIntegerFor("Ascent");
    bExistAscent = true;
  }
  bool bExistDescent = false;
  if (font_desc->KeyExist("Descent")) {
    descent_ = font_desc->GetIntegerFor("Descent");
    bExistDescent = true;
  }
  bool bExistCapHeight = false;
  if (font_desc->KeyExist("CapHeight")) {
    bExistCapHeight = true;
  }
  if (bExistItalicAngle && bExistAscent && bExistCapHeight && bExistDescent &&
      bExistStemV) {
    flags_ |= FXFONT_USEEXTERNATTR;
  }
  if (descent_ > 10) {
    descent_ = -descent_;
  }
  RetainPtr<const CPDF_Array> pBBox = font_desc->GetArrayFor("FontBBox");
  if (pBBox) {
    font_bbox_.left = pBBox->GetIntegerAt(0);
    font_bbox_.bottom = pBBox->GetIntegerAt(1);
    font_bbox_.right = pBBox->GetIntegerAt(2);
    font_bbox_.top = pBBox->GetIntegerAt(3);
  }

  RetainPtr<const CPDF_Stream> font_file = font_desc->GetStreamFor("FontFile");
  if (!font_file) {
    font_file = font_desc->GetStreamFor("FontFile2");
  }
  if (!font_file) {
    font_file = font_desc->GetStreamFor("FontFile3");
  }
  if (!font_file) {
    return;
  }

  const uint64_t key = font_file->KeyForCache();
  font_file_ = document_->GetFontFileStreamAcc(std::move(font_file));
  if (!font_file_) {
    return;
  }

  if (!font_.LoadEmbedded(font_file_->GetSpan(), IsVertWriting(), key)) {
    document_->MaybePurgeFontFileStreamAcc(std::move(font_file_));
  }
}

void CPDF_Font::CheckFontMetrics() {
  if (font_bbox_.top == 0 && font_bbox_.bottom == 0 && font_bbox_.left == 0 &&
      font_bbox_.right == 0) {
    RetainPtr<CFX_Face> face = font_.GetFace();
    if (face) {
      // Note that `font_bbox_` is deliberately flipped.
      const FX_RECT raw_bbox = face->GetBBox();
      const uint16_t upem = face->GetUnitsPerEm();
      font_bbox_.left = NormalizeFontMetric(raw_bbox.left, upem);
      font_bbox_.bottom = NormalizeFontMetric(raw_bbox.top, upem);
      font_bbox_.right = NormalizeFontMetric(raw_bbox.right, upem);
      font_bbox_.top = NormalizeFontMetric(raw_bbox.bottom, upem);
      ascent_ = NormalizeFontMetric(face->GetAscender(), upem);
      descent_ = NormalizeFontMetric(face->GetDescender(), upem);
    } else {
      bool bFirst = true;
      for (int i = 0; i < 256; i++) {
        FX_RECT rect = GetCharBBox(i);
        if (rect.left == rect.right) {
          continue;
        }
        if (bFirst) {
          font_bbox_ = rect;
          bFirst = false;
        } else {
          font_bbox_.left = std::min(font_bbox_.left, rect.left);
          font_bbox_.top = std::max(font_bbox_.top, rect.top);
          font_bbox_.right = std::max(font_bbox_.right, rect.right);
          font_bbox_.bottom = std::min(font_bbox_.bottom, rect.bottom);
        }
      }
    }
  }
  if (ascent_ == 0 && descent_ == 0) {
    FX_RECT rect = GetCharBBox('A');
    ascent_ = rect.bottom == rect.top ? font_bbox_.top : rect.top;
    rect = GetCharBBox('g');
    descent_ = rect.bottom == rect.top ? font_bbox_.bottom : rect.bottom;
  }
}

void CPDF_Font::LoadUnicodeMap() const {
  to_unicode_loaded_ = true;
  RetainPtr<const CPDF_Stream> pStream = font_dict_->GetStreamFor("ToUnicode");
  if (!pStream) {
    return;
  }

  to_unicode_map_ = std::make_unique<CPDF_ToUnicodeMap>(std::move(pStream));
}

int CPDF_Font::GetStringWidth(ByteStringView pString) {
  size_t offset = 0;
  int width = 0;
  while (offset < pString.GetLength()) {
    width += GetCharWidthF(GetNextChar(pString, &offset));
  }
  return width;
}

// static
RetainPtr<CPDF_Font> CPDF_Font::GetStockFont(CPDF_Document* doc,
                                             ByteStringView name) {
  ByteString fontname(name);
  std::optional<CFX_FontMapper::StandardFont> font_id =
      CFX_FontMapper::GetStandardFontName(&fontname);
  if (!font_id.has_value()) {
    return nullptr;
  }

  auto* font_globals = CPDF_FontGlobals::GetInstance();
  RetainPtr<CPDF_Font> font = font_globals->Find(doc, font_id.value());
  if (font) {
    return font;
  }

  auto dict = doc->New<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "Font");
  dict->SetNewFor<CPDF_Name>("Subtype", "Type1");
  dict->SetNewFor<CPDF_Name>("BaseFont", fontname);
  dict->SetNewFor<CPDF_Name>("Encoding",
                             pdfium::font_encodings::kWinAnsiEncoding);
  font = CPDF_Font::Create(nullptr, std::move(dict), nullptr);
  font_globals->Set(doc, font_id.value(), font);
  return font;
}

// static
RetainPtr<CPDF_Font> CPDF_Font::Create(CPDF_Document* doc,
                                       RetainPtr<CPDF_Dictionary> font_dict,
                                       FormFactoryIface* pFactory) {
  ByteString type = font_dict->GetByteStringFor("Subtype");
  RetainPtr<CPDF_Font> font;
  if (type == "TrueType") {
    ByteString tag = font_dict->GetByteStringFor("BaseFont").First(4);
    for (const char* chinese_font_name : kChineseFontNames) {
      if (tag == chinese_font_name) {
        RetainPtr<const CPDF_Dictionary> font_desc =
            font_dict->GetDictFor("FontDescriptor");
        if (!font_desc || !font_desc->KeyExist("FontFile2")) {
          font = pdfium::MakeRetain<CPDF_CIDFont>(doc, std::move(font_dict));
        }
        break;
      }
    }
    if (!font) {
      font = pdfium::MakeRetain<CPDF_TrueTypeFont>(doc, std::move(font_dict));
    }
  } else if (type == "Type3") {
    font =
        pdfium::MakeRetain<CPDF_Type3Font>(doc, std::move(font_dict), pFactory);
  } else if (type == "Type0") {
    font = pdfium::MakeRetain<CPDF_CIDFont>(doc, std::move(font_dict));
  } else {
    font = pdfium::MakeRetain<CPDF_Type1Font>(doc, std::move(font_dict));
  }
  if (!font->Load()) {
    return nullptr;
  }

  return font;
}

uint32_t CPDF_Font::GetNextChar(ByteStringView pString, size_t* pOffset) const {
  if (pString.IsEmpty()) {
    return 0;
  }

  size_t& offset = *pOffset;
  return offset < pString.GetLength() ? pString[offset++] : pString.Back();
}

bool CPDF_Font::IsStandardFont() const {
  if (!IsType1Font()) {
    return false;
  }
  if (font_file_) {
    return false;
  }
  return AsType1Font()->IsBase14Font();
}

std::optional<FX_Charset> CPDF_Font::GetSubstFontCharset() const {
  CFX_SubstFont* font = font_.GetSubstFont();
  if (!font) {
    return std::nullopt;
  }
  return font->charset_;
}

// static
const char* CPDF_Font::GetAdobeCharName(
    FontEncoding base_encoding,
    const std::vector<ByteString>& charnames,
    uint32_t charcode) {
  if (charcode >= 256) {
    return nullptr;
  }

  if (!charnames.empty() && !charnames[charcode].IsEmpty()) {
    return charnames[charcode].c_str();
  }

  const char* name = nullptr;
  if (base_encoding != FontEncoding::kBuiltin) {
    name = CharNameFromPredefinedCharSet(base_encoding, charcode);
  }
  if (!name) {
    return nullptr;
  }

  DCHECK(name[0]);
  return name;
}

uint32_t CPDF_Font::FallbackFontFromCharcode(uint32_t charcode) {
  if (font_fallbacks_.empty()) {
    font_fallbacks_.push_back(std::make_unique<CFX_Font>());
    FX_SAFE_INT32 safe_weight = stem_v_;
    safe_weight *= 5;
    font_fallbacks_[0]->LoadSubst(
        "Arial", IsTrueTypeFont(), flags_,
        safe_weight.ValueOrDefault(pdfium::kFontWeightNormal), italic_angle_,
        FX_CodePage::kDefANSI, IsVertWriting());
  }
  return 0;
}

int CPDF_Font::FallbackGlyphFromCharcode(int fallbackFont, uint32_t charcode) {
  if (!fxcrt::IndexInBounds(font_fallbacks_, fallbackFont)) {
    return -1;
  }

  WideString str = UnicodeFromCharCode(charcode);
  uint32_t unicode = !str.IsEmpty() ? str[0] : charcode;
  int glyph = font_fallbacks_[fallbackFont]->GetFace()->GetCharIndex(unicode);
  if (glyph == 0) {
    return -1;
  }

  return glyph;
}

CFX_Font* CPDF_Font::GetFontFallback(int position) {
  if (position < 0 || static_cast<size_t>(position) >= font_fallbacks_.size()) {
    return nullptr;
  }
  return font_fallbacks_[position].get();
}

bool CPDF_Font::UseTTCharmapUnicode(const RetainPtr<CFX_Face>& face) {
  size_t charmap_unicode_index = 0;
  bool charmap_unicode_found = false;
  bool charmap_mssymbol_found = false;
  for (size_t i = 0; i < face->GetCharMapCount(); i++) {
    const CFX_Face::CharMapId charmap_id = face->GetCharMapIdByIndex(i);
    if (charmap_id == CFX_Face::kWindowsUnicodeCmapId) {
      face->SetCharMapByIndex(i);
      return true;
    }
    if (charmap_id == CFX_Face::kWindowsSymbolCmapId) {
      charmap_mssymbol_found = true;
      continue;
    }
    const fxge::FontEncoding encoding = face->GetCharMapEncodingByIndex(i);
    if (!charmap_unicode_found && encoding == fxge::FontEncoding::kUnicode) {
      charmap_unicode_found = true;
      charmap_unicode_index = i;
    }
  }
  if (charmap_unicode_found && !charmap_mssymbol_found) {
    face->SetCharMapByIndex(charmap_unicode_index);
    return true;
  }
  return false;
}

// static
bool CPDF_Font::UseTTCharmap(const RetainPtr<CFX_Face>& face,
                             const CFX_Face::CharMapId& cmap_id) {
  for (size_t i = 0; i < face->GetCharMapCount(); i++) {
    if (face->GetCharMapIdByIndex(i) == cmap_id) {
      face->SetCharMapByIndex(i);
      return true;
    }
  }
  return false;
}

std::optional<int> CPDF_Font::GetFontWeight() const {
  FX_SAFE_INT32 safe_stem_v(stem_v_);
  if (stem_v_ < 140) {
    safe_stem_v *= 5;
  } else {
    safe_stem_v = safe_stem_v * 4 + 140;
  }
  if (!safe_stem_v.IsValid()) {
    return std::nullopt;
  }
  return safe_stem_v.ValueOrDie();
}
