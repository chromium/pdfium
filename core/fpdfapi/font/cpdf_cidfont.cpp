// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_cidfont.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/cmaps/fpdf_cmaps.h"
#include "core/fpdfapi/font/cfx_cttgsubtable.h"
#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"
#include "core/fpdfapi/font/cpdf_cmap.h"
#include "core/fpdfapi/font/cpdf_cmapparser.h"
#include "core/fpdfapi/font/cpdf_fontencoding.h"
#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_unicode.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/fx_font.h"

namespace {

struct LowHighVal {
  int low;
  int high;
  int val;
};

struct LowHighValXY : LowHighVal {
  int x;
  int y;
};

bool IsMetricForCID(const LowHighVal& val, uint16_t cid) {
  return val.low <= cid && cid <= val.high;
}

constexpr std::array<FX_CodePage, CIDSET_NUM_SETS> kCharsetCodePages = {
    FX_CodePage::kDefANSI,
    FX_CodePage::kChineseSimplified,
    FX_CodePage::kChineseTraditional,
    FX_CodePage::kShiftJIS,
    FX_CodePage::kHangul,
    FX_CodePage::kUTF16LE,
};

constexpr CIDTransform kJapan1VerticalCIDs[] = {
    {97, 129, 0, 0, 127, 55, 0},     {7887, 127, 0, 0, 127, 76, 89},
    {7888, 127, 0, 0, 127, 79, 94},  {7889, 0, 129, 127, 0, 17, 127},
    {7890, 0, 129, 127, 0, 17, 127}, {7891, 0, 129, 127, 0, 17, 127},
    {7892, 0, 129, 127, 0, 17, 127}, {7893, 0, 129, 127, 0, 17, 127},
    {7894, 0, 129, 127, 0, 17, 127}, {7895, 0, 129, 127, 0, 17, 127},
    {7896, 0, 129, 127, 0, 17, 127}, {7897, 0, 129, 127, 0, 17, 127},
    {7898, 0, 129, 127, 0, 17, 127}, {7899, 0, 129, 127, 0, 17, 104},
    {7900, 0, 129, 127, 0, 17, 127}, {7901, 0, 129, 127, 0, 17, 104},
    {7902, 0, 129, 127, 0, 17, 127}, {7903, 0, 129, 127, 0, 17, 127},
    {7904, 0, 129, 127, 0, 17, 127}, {7905, 0, 129, 127, 0, 17, 114},
    {7906, 0, 129, 127, 0, 17, 127}, {7907, 0, 129, 127, 0, 17, 127},
    {7908, 0, 129, 127, 0, 17, 127}, {7909, 0, 129, 127, 0, 17, 127},
    {7910, 0, 129, 127, 0, 17, 127}, {7911, 0, 129, 127, 0, 17, 127},
    {7912, 0, 129, 127, 0, 17, 127}, {7913, 0, 129, 127, 0, 17, 127},
    {7914, 0, 129, 127, 0, 17, 127}, {7915, 0, 129, 127, 0, 17, 114},
    {7916, 0, 129, 127, 0, 17, 127}, {7917, 0, 129, 127, 0, 17, 127},
    {7918, 127, 0, 0, 127, 18, 25},  {7919, 127, 0, 0, 127, 18, 25},
    {7920, 127, 0, 0, 127, 18, 25},  {7921, 127, 0, 0, 127, 18, 25},
    {7922, 127, 0, 0, 127, 18, 25},  {7923, 127, 0, 0, 127, 18, 25},
    {7924, 127, 0, 0, 127, 18, 25},  {7925, 127, 0, 0, 127, 18, 25},
    {7926, 127, 0, 0, 127, 18, 25},  {7927, 127, 0, 0, 127, 18, 25},
    {7928, 127, 0, 0, 127, 18, 25},  {7929, 127, 0, 0, 127, 18, 25},
    {7930, 127, 0, 0, 127, 18, 25},  {7931, 127, 0, 0, 127, 18, 25},
    {7932, 127, 0, 0, 127, 18, 25},  {7933, 127, 0, 0, 127, 18, 25},
    {7934, 127, 0, 0, 127, 18, 25},  {7935, 127, 0, 0, 127, 18, 25},
    {7936, 127, 0, 0, 127, 18, 25},  {7937, 127, 0, 0, 127, 18, 25},
    {7938, 127, 0, 0, 127, 18, 25},  {7939, 127, 0, 0, 127, 18, 25},
    {8720, 0, 129, 127, 0, 19, 102}, {8721, 0, 129, 127, 0, 13, 127},
    {8722, 0, 129, 127, 0, 19, 108}, {8723, 0, 129, 127, 0, 19, 102},
    {8724, 0, 129, 127, 0, 19, 102}, {8725, 0, 129, 127, 0, 19, 102},
    {8726, 0, 129, 127, 0, 19, 102}, {8727, 0, 129, 127, 0, 19, 102},
    {8728, 0, 129, 127, 0, 19, 114}, {8729, 0, 129, 127, 0, 19, 114},
    {8730, 0, 129, 127, 0, 38, 108}, {8731, 0, 129, 127, 0, 13, 108},
    {8732, 0, 129, 127, 0, 19, 108}, {8733, 0, 129, 127, 0, 19, 108},
    {8734, 0, 129, 127, 0, 19, 108}, {8735, 0, 129, 127, 0, 19, 108},
    {8736, 0, 129, 127, 0, 19, 102}, {8737, 0, 129, 127, 0, 19, 102},
    {8738, 0, 129, 127, 0, 19, 102}, {8739, 0, 129, 127, 0, 19, 102},
    {8740, 0, 129, 127, 0, 19, 102}, {8741, 0, 129, 127, 0, 19, 102},
    {8742, 0, 129, 127, 0, 19, 102}, {8743, 0, 129, 127, 0, 19, 102},
    {8744, 0, 129, 127, 0, 19, 102}, {8745, 0, 129, 127, 0, 19, 102},
    {8746, 0, 129, 127, 0, 19, 114}, {8747, 0, 129, 127, 0, 19, 114},
    {8748, 0, 129, 127, 0, 19, 102}, {8749, 0, 129, 127, 0, 19, 102},
    {8750, 0, 129, 127, 0, 19, 102}, {8751, 0, 129, 127, 0, 19, 102},
    {8752, 0, 129, 127, 0, 19, 102}, {8753, 0, 129, 127, 0, 19, 102},
    {8754, 0, 129, 127, 0, 19, 102}, {8755, 0, 129, 127, 0, 19, 102},
    {8756, 0, 129, 127, 0, 19, 102}, {8757, 0, 129, 127, 0, 19, 102},
    {8758, 0, 129, 127, 0, 19, 102}, {8759, 0, 129, 127, 0, 19, 102},
    {8760, 0, 129, 127, 0, 19, 102}, {8761, 0, 129, 127, 0, 19, 102},
    {8762, 0, 129, 127, 0, 19, 102}, {8763, 0, 129, 127, 0, 19, 102},
    {8764, 0, 129, 127, 0, 19, 102}, {8765, 0, 129, 127, 0, 19, 102},
    {8766, 0, 129, 127, 0, 19, 102}, {8767, 0, 129, 127, 0, 19, 102},
    {8768, 0, 129, 127, 0, 19, 102}, {8769, 0, 129, 127, 0, 19, 102},
    {8770, 0, 129, 127, 0, 19, 102}, {8771, 0, 129, 127, 0, 19, 102},
    {8772, 0, 129, 127, 0, 19, 102}, {8773, 0, 129, 127, 0, 19, 102},
    {8774, 0, 129, 127, 0, 19, 102}, {8775, 0, 129, 127, 0, 19, 102},
    {8776, 0, 129, 127, 0, 19, 102}, {8777, 0, 129, 127, 0, 19, 102},
    {8778, 0, 129, 127, 0, 19, 102}, {8779, 0, 129, 127, 0, 19, 114},
    {8780, 0, 129, 127, 0, 19, 108}, {8781, 0, 129, 127, 0, 19, 114},
    {8782, 0, 129, 127, 0, 13, 114}, {8783, 0, 129, 127, 0, 19, 108},
    {8784, 0, 129, 127, 0, 13, 114}, {8785, 0, 129, 127, 0, 19, 108},
    {8786, 0, 129, 127, 0, 19, 108}, {8787, 0, 129, 127, 0, 19, 108},
    {8788, 0, 129, 127, 0, 19, 108}, {8789, 0, 129, 127, 0, 19, 108},
    {8790, 0, 129, 127, 0, 19, 108}, {8791, 0, 129, 127, 0, 19, 108},
    {8792, 0, 129, 127, 0, 19, 108}, {8793, 0, 129, 127, 0, 19, 108},
    {8794, 0, 129, 127, 0, 19, 108}, {8795, 0, 129, 127, 0, 19, 108},
    {8796, 0, 129, 127, 0, 19, 108}, {8797, 0, 129, 127, 0, 19, 108},
    {8798, 0, 129, 127, 0, 19, 108}, {8799, 0, 129, 127, 0, 19, 108},
    {8800, 0, 129, 127, 0, 19, 108}, {8801, 0, 129, 127, 0, 19, 108},
    {8802, 0, 129, 127, 0, 19, 108}, {8803, 0, 129, 127, 0, 19, 108},
    {8804, 0, 129, 127, 0, 19, 108}, {8805, 0, 129, 127, 0, 19, 108},
    {8806, 0, 129, 127, 0, 19, 108}, {8807, 0, 129, 127, 0, 19, 108},
    {8808, 0, 129, 127, 0, 19, 108}, {8809, 0, 129, 127, 0, 19, 108},
    {8810, 0, 129, 127, 0, 19, 108}, {8811, 0, 129, 127, 0, 19, 114},
    {8812, 0, 129, 127, 0, 19, 102}, {8813, 0, 129, 127, 0, 19, 114},
    {8814, 0, 129, 127, 0, 76, 102}, {8815, 0, 129, 127, 0, 13, 121},
    {8816, 0, 129, 127, 0, 19, 114}, {8817, 0, 129, 127, 0, 19, 127},
    {8818, 0, 129, 127, 0, 19, 114}, {8819, 0, 129, 127, 0, 218, 108},
};

#if !BUILDFLAG(IS_WIN)

bool IsValidEmbeddedCharcodeFromUnicodeCharset(CIDSet charset) {
  switch (charset) {
    case CIDSET_GB1:
    case CIDSET_CNS1:
    case CIDSET_JAPAN1:
    case CIDSET_KOREA1:
      return true;

    default:
      return false;
  }
}

wchar_t EmbeddedUnicodeFromCharcode(const fxcmap::CMap* pEmbedMap,
                                    CIDSet charset,
                                    uint32_t charcode) {
  if (!IsValidEmbeddedCharcodeFromUnicodeCharset(charset)) {
    return 0;
  }

  uint16_t cid = fxcmap::CIDFromCharCode(pEmbedMap, charcode);
  if (!cid) {
    return 0;
  }

  pdfium::span<const uint16_t> map =
      CPDF_FontGlobals::GetInstance()->GetEmbeddedToUnicode(charset);
  return cid < map.size() ? map[cid] : 0;
}

uint32_t EmbeddedCharcodeFromUnicode(const fxcmap::CMap* pEmbedMap,
                                     CIDSet charset,
                                     wchar_t unicode) {
  if (!IsValidEmbeddedCharcodeFromUnicodeCharset(charset)) {
    return 0;
  }

  pdfium::span<const uint16_t> map =
      CPDF_FontGlobals::GetInstance()->GetEmbeddedToUnicode(charset);
  for (uint32_t i = 0; i < map.size(); ++i) {
    if (map[i] == unicode) {
      uint32_t charCode = fxcmap::CharCodeFromCID(pEmbedMap, i);
      if (charCode) {
        return charCode;
      }
    }
  }
  return 0;
}

#endif  // !BUILDFLAG(IS_WIN)

void UseCIDCharmap(const RetainPtr<CFX_Face>& face, CIDCoding coding) {
  fxge::FontEncoding encoding;
  switch (coding) {
    case CIDCoding::kGB:
      encoding = fxge::FontEncoding::kGB2312;
      break;
    case CIDCoding::kBIG5:
      encoding = fxge::FontEncoding::kBig5;
      break;
    case CIDCoding::kJIS:
      encoding = fxge::FontEncoding::kSjis;
      break;
    case CIDCoding::kKOREA:
      encoding = fxge::FontEncoding::kJohab;
      break;
    default:
      encoding = fxge::FontEncoding::kUnicode;
  }
  bool result = face->SelectCharMap(encoding);
  if (!result) {
    result = face->SelectCharMap(fxge::FontEncoding::kUnicode);
  }
  if (!result && face->GetCharMapCount()) {
    face->SetCharMapByIndex(0);
  }
}

void LoadMetricsArray(RetainPtr<const CPDF_Array> pArray,
                      std::vector<int>* result,
                      int nElements) {
  int width_status = 0;
  int iCurElement = 0;
  int first_code = 0;
  int last_code = 0;
  for (size_t i = 0; i < pArray->size(); i++) {
    RetainPtr<const CPDF_Object> pObj = pArray->GetDirectObjectAt(i);
    if (!pObj) {
      continue;
    }

    const CPDF_Array* pObjArray = pObj->AsArray();
    if (pObjArray) {
      if (width_status != 1) {
        return;
      }
      if (first_code > std::numeric_limits<int>::max() -
                           fxcrt::CollectionSize<int>(*pObjArray)) {
        width_status = 0;
        continue;
      }

      for (size_t j = 0; j < pObjArray->size(); j += nElements) {
        result->push_back(first_code);
        result->push_back(first_code);
        for (int k = 0; k < nElements; k++) {
          result->push_back(pObjArray->GetIntegerAt(j + k));
        }
        first_code++;
      }
      width_status = 0;
    } else {
      if (width_status == 0) {
        first_code = pObj->GetInteger();
        width_status = 1;
      } else if (width_status == 1) {
        last_code = pObj->GetInteger();
        width_status = 2;
        iCurElement = 0;
      } else {
        if (!iCurElement) {
          result->push_back(first_code);
          result->push_back(last_code);
        }
        result->push_back(pObj->GetInteger());
        iCurElement++;
        if (iCurElement == nElements) {
          width_status = 0;
        }
      }
    }
  }
}

}  // namespace

CPDF_CIDFont::CPDF_CIDFont(CPDF_Document* pDocument,
                           RetainPtr<CPDF_Dictionary> pFontDict)
    : CPDF_Font(pDocument, std::move(pFontDict)) {
  for (size_t i = 0; i < std::size(char_bbox_); ++i) {
    char_bbox_[i] = FX_RECT(-1, -1, -1, -1);
  }
}

CPDF_CIDFont::~CPDF_CIDFont() = default;

bool CPDF_CIDFont::IsCIDFont() const {
  return true;
}

const CPDF_CIDFont* CPDF_CIDFont::AsCIDFont() const {
  return this;
}

CPDF_CIDFont* CPDF_CIDFont::AsCIDFont() {
  return this;
}

uint16_t CPDF_CIDFont::CIDFromCharCode(uint32_t charcode) const {
  return cmap_ ? cmap_->CIDFromCharCode(charcode)
               : static_cast<uint16_t>(charcode);
}

bool CPDF_CIDFont::IsVertWriting() const {
  return cmap_ && cmap_->IsVertWriting();
}

WideString CPDF_CIDFont::UnicodeFromCharCode(uint32_t charcode) const {
  WideString str = CPDF_Font::UnicodeFromCharCode(charcode);
  if (!str.IsEmpty()) {
    return str;
  }
  wchar_t ret = GetUnicodeFromCharCode(charcode);
  return ret ? WideString(ret) : WideString();
}

wchar_t CPDF_CIDFont::GetUnicodeFromCharCode(uint32_t charcode) const {
  switch (cmap_->GetCoding()) {
    case CIDCoding::kUCS2:
    case CIDCoding::kUTF16:
      return static_cast<wchar_t>(charcode);
    case CIDCoding::kCID:
      if (!cid2unicode_map_ || !cid2unicode_map_->IsLoaded()) {
        return 0;
      }
      return cid2unicode_map_->UnicodeFromCID(static_cast<uint16_t>(charcode));
    default:
      break;
  }
  if (cid2unicode_map_ && cid2unicode_map_->IsLoaded() && cmap_->IsLoaded()) {
    return cid2unicode_map_->UnicodeFromCID(CIDFromCharCode(charcode));
  }

#if BUILDFLAG(IS_WIN)
  uint8_t sequence[2] = {};
  const int charsize = charcode < 256 ? 1 : 2;
  if (charsize == 1) {
    sequence[0] = charcode;
  } else {
    sequence[0] = charcode / 256;
    sequence[1] = charcode % 256;
  }
  wchar_t unicode;
  size_t ret = FX_MultiByteToWideChar(
      kCharsetCodePages[static_cast<size_t>(cmap_->GetCoding())],
      ByteStringView(
          pdfium::span(sequence).first(static_cast<size_t>(charsize))),
      pdfium::span_from_ref(unicode));
  return ret == 1 ? unicode : 0;
#else
  if (!cmap_->GetEmbedMap()) {
    return 0;
  }
  return EmbeddedUnicodeFromCharcode(cmap_->GetEmbedMap(), cmap_->GetCharset(),
                                     charcode);
#endif
}

uint32_t CPDF_CIDFont::CharCodeFromUnicode(wchar_t unicode) const {
  uint32_t charcode = CPDF_Font::CharCodeFromUnicode(unicode);
  if (charcode) {
    return charcode;
  }

  switch (cmap_->GetCoding()) {
    case CIDCoding::kUNKNOWN:
      return 0;
    case CIDCoding::kUCS2:
    case CIDCoding::kUTF16:
      return unicode;
    case CIDCoding::kCID: {
      if (!cid2unicode_map_ || !cid2unicode_map_->IsLoaded()) {
        return 0;
      }
      uint32_t cid = 0;
      while (cid < 65536) {
        wchar_t this_unicode =
            cid2unicode_map_->UnicodeFromCID(static_cast<uint16_t>(cid));
        if (this_unicode == unicode) {
          return cid;
        }
        cid++;
      }
      break;
    }
    default:
      break;
  }

  if (unicode < 0x80) {
    return static_cast<uint32_t>(unicode);
  }
  if (cmap_->GetCoding() == CIDCoding::kCID) {
    return 0;
  }
#if BUILDFLAG(IS_WIN)
  uint8_t buffer[32];
  size_t ret = FX_WideCharToMultiByte(
      kCharsetCodePages[static_cast<size_t>(cmap_->GetCoding())],
      WideStringView(unicode),
      pdfium::as_writable_chars(pdfium::span(buffer).first(4u)));
  if (ret == 1) {
    return buffer[0];
  }
  if (ret == 2) {
    return buffer[0] * 256 + buffer[1];
  }
#else
  if (cmap_->GetEmbedMap()) {
    return EmbeddedCharcodeFromUnicode(cmap_->GetEmbedMap(),
                                       cmap_->GetCharset(), unicode);
  }
#endif
  return 0;
}

bool CPDF_CIDFont::Load() {
  if (font_dict_->GetByteStringFor("Subtype") == "TrueType") {
    LoadGB2312();
    return true;
  }

  RetainPtr<const CPDF_Array> pFonts =
      font_dict_->GetArrayFor("DescendantFonts");
  if (!pFonts || pFonts->size() != 1) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> pCIDFontDict = pFonts->GetDictAt(0);
  if (!pCIDFontDict) {
    return false;
  }

  base_font_name_ = pCIDFontDict->GetByteStringFor("BaseFont");
  if ((base_font_name_ == "CourierStd" ||
       base_font_name_ == "CourierStd-Bold" ||
       base_font_name_ == "CourierStd-BoldOblique" ||
       base_font_name_ == "CourierStd-Oblique") &&
      !IsEmbedded()) {
    adobe_courier_std_ = true;
  }

  RetainPtr<const CPDF_Object> pEncoding =
      font_dict_->GetDirectObjectFor("Encoding");
  if (!pEncoding) {
    return false;
  }

  ByteString subtype = pCIDFontDict->GetByteStringFor("Subtype");
  font_type_ =
      subtype == "CIDFontType0" ? CIDFontType::kType1 : CIDFontType::kTrueType;

  if (!pEncoding->IsName() && !pEncoding->IsStream()) {
    return false;
  }

  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  const CPDF_Stream* pEncodingStream = pEncoding->AsStream();
  if (pEncodingStream) {
    auto pAcc =
        pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(pEncodingStream));
    pAcc->LoadAllDataFiltered();
    pdfium::span<const uint8_t> span = pAcc->GetSpan();
    cmap_ = pdfium::MakeRetain<CPDF_CMap>(span);
  } else {
    DCHECK(pEncoding->IsName());
    ByteString cmap = pEncoding->GetString();
    cmap_ = pFontGlobals->GetPredefinedCMap(cmap);
  }

  RetainPtr<const CPDF_Dictionary> pFontDesc =
      pCIDFontDict->GetDictFor("FontDescriptor");
  if (pFontDesc) {
    LoadFontDescriptor(pFontDesc.Get());
  }

  charset_ = cmap_->GetCharset();
  if (charset_ == CIDSET_UNKNOWN) {
    RetainPtr<const CPDF_Dictionary> pCIDInfo =
        pCIDFontDict->GetDictFor("CIDSystemInfo");
    if (pCIDInfo) {
      charset_ = CPDF_CMapParser::CharsetFromOrdering(
          pCIDInfo->GetByteStringFor("Ordering").AsStringView());
    }
  }
  if (charset_ != CIDSET_UNKNOWN) {
    cid2unicode_map_ = pFontGlobals->GetCID2UnicodeMap(charset_);
  }
  RetainPtr<CFX_Face> face = font_.GetFace();
  if (face) {
    if (font_type_ == CIDFontType::kType1) {
      face->SelectCharMap(fxge::FontEncoding::kUnicode);
    } else {
      UseCIDCharmap(face, cmap_->GetCoding());
    }
  }
  default_width_ = pCIDFontDict->GetIntegerFor("DW", 1000);
  RetainPtr<const CPDF_Array> pWidthArray = pCIDFontDict->GetArrayFor("W");
  if (pWidthArray) {
    LoadMetricsArray(std::move(pWidthArray), &width_list_, 1);
  }

  if (!IsEmbedded()) {
    LoadSubstFont();
  }

  RetainPtr<const CPDF_Object> pmap =
      pCIDFontDict->GetDirectObjectFor("CIDToGIDMap");
  if (pmap) {
    RetainPtr<const CPDF_Stream> pMapStream(pmap->AsStream());
    if (pMapStream) {
      stream_acc_ = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pMapStream));
      stream_acc_->LoadAllDataFiltered();
    } else if (font_file_ && pmap->IsName() &&
               pmap->GetString() == "Identity") {
      cid_is_gid_ = true;
    }
  }

  CheckFontMetrics();
  if (IsVertWriting()) {
    RetainPtr<const CPDF_Array> pWidth2Array = pCIDFontDict->GetArrayFor("W2");
    if (pWidth2Array) {
      LoadMetricsArray(std::move(pWidth2Array), &vert_metrics_, 3);
    }

    RetainPtr<const CPDF_Array> pDefaultArray =
        pCIDFontDict->GetArrayFor("DW2");
    if (pDefaultArray) {
      default_vy_ = pDefaultArray->GetIntegerAt(0);
      default_w1_ = pDefaultArray->GetIntegerAt(1);
    }
  }

  // TODO(thestig): Better identify font types and identify more font types.
  if (font_type_ == CIDFontType::kTrueType && IsEmbedded()) {
    font_.SetFontType(CFX_Font::FontType::kCIDTrueType);
  }

  return true;
}

FX_RECT CPDF_CIDFont::GetCharBBox(uint32_t charcode) {
  if (charcode < 256 && char_bbox_[charcode].right != -1) {
    return char_bbox_[charcode];
  }

  FX_RECT rect;
  bool bVert = false;
  int glyph_index = GlyphFromCharCode(charcode, &bVert);
  RetainPtr<CFX_Face> face = font_.GetFace();
  if (face) {
    rect = face->GetCharBBox(charcode, glyph_index);
  }
  if (!font_file_ && charset_ == CIDSET_JAPAN1) {
    uint16_t cid = CIDFromCharCode(charcode);
    const CIDTransform* pTransform = GetCIDTransform(cid);
    if (pTransform && !bVert) {
      CFX_Matrix matrix(CIDTransformToFloat(pTransform->a),
                        CIDTransformToFloat(pTransform->b),
                        CIDTransformToFloat(pTransform->c),
                        CIDTransformToFloat(pTransform->d),
                        CIDTransformToFloat(pTransform->e) * 1000,
                        CIDTransformToFloat(pTransform->f) * 1000);
      rect = matrix.TransformRect(CFX_FloatRect(rect)).GetOuterRect();
    }
  }
  if (charcode < 256) {
    char_bbox_[charcode] = rect;
  }

  return rect;
}

int CPDF_CIDFont::GetCharWidthF(uint32_t charcode) {
  if (charcode < 0x80 && ansi_widths_fixed_) {
    return (charcode >= 32 && charcode < 127) ? 500 : 0;
  }
  uint16_t cid = CIDFromCharCode(charcode);
  auto lhv_span =
      fxcrt::reinterpret_span<const LowHighVal>(pdfium::span(width_list_));
  for (const auto& lhv : lhv_span) {
    if (IsMetricForCID(lhv, cid)) {
      return lhv.val;
    }
  }
  return default_width_;
}

int16_t CPDF_CIDFont::GetVertWidth(uint16_t cid) const {
  auto lhvxy_span =
      fxcrt::reinterpret_span<const LowHighValXY>(pdfium::span(vert_metrics_));
  for (const auto& lhvxy : lhvxy_span) {
    if (IsMetricForCID(lhvxy, cid)) {
      return lhvxy.val;
    }
  }
  return default_w1_;
}

CFX_Point16 CPDF_CIDFont::GetVertOrigin(uint16_t cid) const {
  auto lhvxy_span =
      fxcrt::reinterpret_span<const LowHighValXY>(pdfium::span(vert_metrics_));
  for (const auto& lhvxy : lhvxy_span) {
    if (IsMetricForCID(lhvxy, cid)) {
      return {static_cast<int16_t>(lhvxy.x), static_cast<int16_t>(lhvxy.y)};
    }
  }
  int width = default_width_;
  auto lhv_span =
      fxcrt::reinterpret_span<const LowHighVal>(pdfium::span(width_list_));
  for (const auto& lhv : lhv_span) {
    if (IsMetricForCID(lhv, cid)) {
      width = lhv.val;
      break;
    }
  }
  return {static_cast<int16_t>(width / 2), default_vy_};
}

int CPDF_CIDFont::GetGlyphIndex(uint32_t unicode, bool* pVertGlyph) {
  if (pVertGlyph) {
    *pVertGlyph = false;
  }

  int index = font_.GetFace()->GetCharIndex(unicode);
  if (unicode == pdfium::unicode::kBoxDrawingsLightVerical) {
    return index;
  }

  if (!index || !IsVertWriting()) {
    return index;
  }

  if (ttg_subtable_) {
    return GetVerticalGlyph(index, pVertGlyph);
  }

  static constexpr uint32_t kGsubTag =
      CFX_FontMapper::MakeTag('G', 'S', 'U', 'B');
  RetainPtr<CFX_Face> face = font_.GetFace();
  size_t length = face->GetSfntTable(kGsubTag, {});
  if (!length) {
    return index;
  }

  auto sub_data = FixedSizeDataVector<uint8_t>::Uninit(length);
  if (!face->GetSfntTable(kGsubTag, sub_data.span())) {
    return index;
  }

  // CFX_CTTGSUBTable parses the data and stores all the values in its structs.
  // It does not store pointers into `sub_data`.
  ttg_subtable_ = std::make_unique<CFX_CTTGSUBTable>(sub_data.span());
  return GetVerticalGlyph(index, pVertGlyph);
}

int CPDF_CIDFont::GetVerticalGlyph(int index, bool* pVertGlyph) {
  uint32_t vindex = ttg_subtable_->GetVerticalGlyph(index);
  if (!vindex) {
    return index;
  }

  index = vindex;
  if (pVertGlyph) {
    *pVertGlyph = true;
  }
  return index;
}

int CPDF_CIDFont::GlyphFromCharCode(uint32_t charcode, bool* pVertGlyph) {
  if (pVertGlyph) {
    *pVertGlyph = false;
  }

  if (!font_file_ && (!stream_acc_ || cid2unicode_map_)) {
    uint16_t cid = CIDFromCharCode(charcode);
    wchar_t unicode = 0;
    if (cid_is_gid_) {
#if BUILDFLAG(IS_APPLE)
      if (FontStyleIsSymbolic(flags_)) {
        return cid;
      }

      WideString uni_str = UnicodeFromCharCode(charcode);
      if (uni_str.IsEmpty()) {
        return cid;
      }

      unicode = uni_str[0];
#else
      return cid;
#endif
    } else {
      if (cid && cid2unicode_map_ && cid2unicode_map_->IsLoaded()) {
        unicode = cid2unicode_map_->UnicodeFromCID(cid);
      }
      if (unicode == 0) {
        unicode = GetUnicodeFromCharCode(charcode);
      }
      if (unicode == 0) {
        WideString unicode_str = UnicodeFromCharCode(charcode);
        if (!unicode_str.IsEmpty()) {
          unicode = unicode_str[0];
        }
      }
    }
    if (unicode == 0) {
      if (!adobe_courier_std_) {
        return charcode ? static_cast<int>(charcode) : -1;
      }

      charcode += 31;
      RetainPtr<CFX_Face> face = font_.GetFace();
      bool bMSUnicode = UseTTCharmapMSUnicode(face);
      bool bMacRoman = !bMSUnicode && UseTTCharmapMacRoman(face);
      FontEncoding base_encoding = FontEncoding::kStandard;
      if (bMSUnicode) {
        base_encoding = FontEncoding::kWinAnsi;
      } else if (bMacRoman) {
        base_encoding = FontEncoding::kMacRoman;
      }
      const char* name =
          GetAdobeCharName(base_encoding, std::vector<ByteString>(), charcode);
      if (!name) {
        return charcode ? static_cast<int>(charcode) : -1;
      }

      int index = 0;
      uint16_t name_unicode = UnicodeFromAdobeName(name);
      if (!name_unicode) {
        return charcode ? static_cast<int>(charcode) : -1;
      }

      if (base_encoding == FontEncoding::kStandard) {
        return face->GetCharIndex(name_unicode);
      }

      if (base_encoding == FontEncoding::kWinAnsi) {
        index = face->GetCharIndex(name_unicode);
      } else {
        DCHECK_EQ(base_encoding, FontEncoding::kMacRoman);
        uint32_t maccode = CharCodeFromUnicodeForEncoding(
            fxge::FontEncoding::kAppleRoman, name_unicode);
        index =
            maccode ? face->GetCharIndex(maccode) : face->GetNameIndex(name);
      }
      if (index == 0 || index == 0xffff) {
        return charcode ? static_cast<int>(charcode) : -1;
      }
      return index;
    }
    if (charset_ == CIDSET_JAPAN1) {
      if (unicode == '\\') {
        unicode = '/';
#if !BUILDFLAG(IS_APPLE)
      } else if (unicode == 0xa5) {
        unicode = 0x5c;
#endif
      }
    }

    RetainPtr<CFX_Face> face = font_.GetFace();
    if (!face) {
      return unicode;
    }

    size_t num_charmaps = face->GetCharMapCount();
    if (!face->SelectCharMap(fxge::FontEncoding::kUnicode)) {
      size_t i;
      for (i = 0; i < num_charmaps; i++) {
        uint32_t ret = CharCodeFromUnicodeForEncoding(
            face->GetCharMapEncodingByIndex(i), static_cast<wchar_t>(charcode));
        if (ret == 0) {
          continue;
        }
        face->SetCharMapByIndex(i);
        unicode = static_cast<wchar_t>(ret);
        break;
      }
      if (i == num_charmaps && i) {
        face->SetCharMapByIndex(0);
        unicode = static_cast<wchar_t>(charcode);
      }
    }
    if (num_charmaps) {
      int index = GetGlyphIndex(unicode, pVertGlyph);
      return index != 0 ? index : -1;
    }
    return unicode;
  }

  RetainPtr<CFX_Face> face = font_.GetFace();
  if (!face) {
    return -1;
  }

  uint16_t cid = CIDFromCharCode(charcode);
  if (!stream_acc_) {
    if (font_type_ == CIDFontType::kType1) {
      return cid;
    }
    if (font_file_ && cmap_->IsDirectCharcodeToCIDTableIsEmpty()) {
      return cid;
    }
    if (cmap_->GetCoding() == CIDCoding::kUNKNOWN) {
      return cid;
    }

    std::optional<fxge::FontEncoding> charmap =
        face->GetCurrentCharMapEncoding();
    if (!charmap.has_value()) {
      return cid;
    }

    if (charmap.value() == fxge::FontEncoding::kUnicode) {
      WideString unicode_str = UnicodeFromCharCode(charcode);
      if (unicode_str.IsEmpty()) {
        return -1;
      }

      charcode = unicode_str[0];
    }
    return GetGlyphIndex(charcode, pVertGlyph);
  }
  uint32_t byte_pos = cid * 2;
  if (byte_pos + 2 > stream_acc_->GetSize()) {
    return -1;
  }

  pdfium::span<const uint8_t> span = stream_acc_->GetSpan().subspan(byte_pos);
  return span[0] * 256 + span[1];
}

uint32_t CPDF_CIDFont::GetNextChar(ByteStringView pString,
                                   size_t* pOffset) const {
  return cmap_->GetNextChar(pString, pOffset);
}

int CPDF_CIDFont::GetCharSize(uint32_t charcode) const {
  return cmap_->GetCharSize(charcode);
}

size_t CPDF_CIDFont::CountChar(ByteStringView pString) const {
  return cmap_->CountChar(pString);
}

void CPDF_CIDFont::AppendChar(ByteString* str, uint32_t charcode) const {
  cmap_->AppendChar(str, charcode);
}

bool CPDF_CIDFont::IsUnicodeCompatible() const {
  if (cid2unicode_map_ && cid2unicode_map_->IsLoaded() && cmap_->IsLoaded()) {
    return true;
  }
  return cmap_->GetCoding() != CIDCoding::kUNKNOWN;
}

void CPDF_CIDFont::LoadSubstFont() {
  FX_SAFE_INT32 safe_stem_v(stem_v_);
  safe_stem_v *= 5;
  font_.LoadSubst(base_font_name_, font_type_ == CIDFontType::kTrueType, flags_,
                  safe_stem_v.ValueOrDefault(pdfium::kFontWeightNormal),
                  italic_angle_, kCharsetCodePages[charset_], IsVertWriting());
}

// static
float CPDF_CIDFont::CIDTransformToFloat(uint8_t ch) {
  return (ch < 128 ? ch : ch - 255) * (1.0f / 127);
}

void CPDF_CIDFont::LoadGB2312() {
  base_font_name_ = font_dict_->GetByteStringFor("BaseFont");
  charset_ = CIDSET_GB1;

  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  cmap_ = pFontGlobals->GetPredefinedCMap("GBK-EUC-H");
  cid2unicode_map_ = pFontGlobals->GetCID2UnicodeMap(charset_);
  RetainPtr<const CPDF_Dictionary> pFontDesc =
      font_dict_->GetDictFor("FontDescriptor");
  if (pFontDesc) {
    LoadFontDescriptor(pFontDesc.Get());
  }

  if (!IsEmbedded()) {
    LoadSubstFont();
  }
  CheckFontMetrics();
  ansi_widths_fixed_ = true;
}

const CIDTransform* CPDF_CIDFont::GetCIDTransform(uint16_t cid) const {
  if (charset_ != CIDSET_JAPAN1 || font_file_) {
    return nullptr;
  }

  const auto* pBegin = std::begin(kJapan1VerticalCIDs);
  const auto* pEnd = std::end(kJapan1VerticalCIDs);
  const auto* pTransform = std::lower_bound(
      pBegin, pEnd, cid,
      [](const CIDTransform& entry, uint16_t cid) { return entry.cid < cid; });

  return pTransform < pEnd && cid == pTransform->cid ? pTransform : nullptr;
}
