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

namespace {

constexpr std::array<const char*, 5> kChineseFontNames = {{
    "\xCB\xCE\xCC\xE5",
    "\xBF\xAC\xCC\xE5",
    "\xBA\xDA\xCC\xE5",
    "\xB7\xC2\xCB\xCE",
    "\xD0\xC2\xCB\xCE",
}};

}  // namespace

CPDF_Font::CPDF_Font(CPDF_Document* pDocument,
                     RetainPtr<CPDF_Dictionary> pFontDict)
    : document_(pDocument),
      font_dict_(std::move(pFontDict)),
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

void CPDF_Font::LoadFontDescriptor(const CPDF_Dictionary* pFontDesc) {
  flags_ = pFontDesc->GetIntegerFor("Flags", pdfium::kFontStyleNonSymbolic);
  int ItalicAngle = 0;
  bool bExistItalicAngle = false;
  if (pFontDesc->KeyExist("ItalicAngle")) {
    ItalicAngle = pFontDesc->GetIntegerFor("ItalicAngle");
    bExistItalicAngle = true;
  }
  if (ItalicAngle < 0) {
    flags_ |= pdfium::kFontStyleItalic;
    italic_angle_ = ItalicAngle;
  }
  bool bExistStemV = false;
  if (pFontDesc->KeyExist("StemV")) {
    stem_v_ = pFontDesc->GetIntegerFor("StemV");
    bExistStemV = true;
  }
  bool bExistAscent = false;
  if (pFontDesc->KeyExist("Ascent")) {
    ascent_ = pFontDesc->GetIntegerFor("Ascent");
    bExistAscent = true;
  }
  bool bExistDescent = false;
  if (pFontDesc->KeyExist("Descent")) {
    descent_ = pFontDesc->GetIntegerFor("Descent");
    bExistDescent = true;
  }
  bool bExistCapHeight = false;
  if (pFontDesc->KeyExist("CapHeight")) {
    bExistCapHeight = true;
  }
  if (bExistItalicAngle && bExistAscent && bExistCapHeight && bExistDescent &&
      bExistStemV) {
    flags_ |= FXFONT_USEEXTERNATTR;
  }
  if (descent_ > 10) {
    descent_ = -descent_;
  }
  RetainPtr<const CPDF_Array> pBBox = pFontDesc->GetArrayFor("FontBBox");
  if (pBBox) {
    font_bbox_.left = pBBox->GetIntegerAt(0);
    font_bbox_.bottom = pBBox->GetIntegerAt(1);
    font_bbox_.right = pBBox->GetIntegerAt(2);
    font_bbox_.top = pBBox->GetIntegerAt(3);
  }

  RetainPtr<const CPDF_Stream> pFontFile = pFontDesc->GetStreamFor("FontFile");
  if (!pFontFile) {
    pFontFile = pFontDesc->GetStreamFor("FontFile2");
  }
  if (!pFontFile) {
    pFontFile = pFontDesc->GetStreamFor("FontFile3");
  }
  if (!pFontFile) {
    return;
  }

  const uint64_t key = pFontFile->KeyForCache();
  font_file_ = document_->GetFontFileStreamAcc(std::move(pFontFile));
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
RetainPtr<CPDF_Font> CPDF_Font::GetStockFont(CPDF_Document* pDoc,
                                             ByteStringView name) {
  ByteString fontname(name);
  std::optional<CFX_FontMapper::StandardFont> font_id =
      CFX_FontMapper::GetStandardFontName(&fontname);
  if (!font_id.has_value()) {
    return nullptr;
  }

  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  RetainPtr<CPDF_Font> pFont = pFontGlobals->Find(pDoc, font_id.value());
  if (pFont) {
    return pFont;
  }

  auto pDict = pDoc->New<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Name>("Type", "Font");
  pDict->SetNewFor<CPDF_Name>("Subtype", "Type1");
  pDict->SetNewFor<CPDF_Name>("BaseFont", fontname);
  pDict->SetNewFor<CPDF_Name>("Encoding",
                              pdfium::font_encodings::kWinAnsiEncoding);
  pFont = CPDF_Font::Create(nullptr, std::move(pDict), nullptr);
  pFontGlobals->Set(pDoc, font_id.value(), pFont);
  return pFont;
}

// static
RetainPtr<CPDF_Font> CPDF_Font::Create(CPDF_Document* pDoc,
                                       RetainPtr<CPDF_Dictionary> pFontDict,
                                       FormFactoryIface* pFactory) {
  ByteString type = pFontDict->GetByteStringFor("Subtype");
  RetainPtr<CPDF_Font> pFont;
  if (type == "TrueType") {
    ByteString tag = pFontDict->GetByteStringFor("BaseFont").First(4);
    for (const char* chinese_font_name : kChineseFontNames) {
      if (tag == chinese_font_name) {
        RetainPtr<const CPDF_Dictionary> pFontDesc =
            pFontDict->GetDictFor("FontDescriptor");
        if (!pFontDesc || !pFontDesc->KeyExist("FontFile2")) {
          pFont = pdfium::MakeRetain<CPDF_CIDFont>(pDoc, std::move(pFontDict));
        }
        break;
      }
    }
    if (!pFont) {
      pFont = pdfium::MakeRetain<CPDF_TrueTypeFont>(pDoc, std::move(pFontDict));
    }
  } else if (type == "Type3") {
    pFont = pdfium::MakeRetain<CPDF_Type3Font>(pDoc, std::move(pFontDict),
                                               pFactory);
  } else if (type == "Type0") {
    pFont = pdfium::MakeRetain<CPDF_CIDFont>(pDoc, std::move(pFontDict));
  } else {
    pFont = pdfium::MakeRetain<CPDF_Type1Font>(pDoc, std::move(pFontDict));
  }
  if (!pFont->Load()) {
    return nullptr;
  }

  return pFont;
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
  CFX_SubstFont* pFont = font_.GetSubstFont();
  if (!pFont) {
    return std::nullopt;
  }
  return pFont->charset_;
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

// static
bool CPDF_Font::UseTTCharmap(const RetainPtr<CFX_Face>& face,
                             int platform_id,
                             int encoding_id) {
  for (size_t i = 0; i < face->GetCharMapCount(); i++) {
    if (face->GetCharMapPlatformIdByIndex(i) == platform_id &&
        face->GetCharMapEncodingIdByIndex(i) == encoding_id) {
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
