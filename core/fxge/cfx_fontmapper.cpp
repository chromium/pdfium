// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_fontmapper.h"

#include <stdint.h>

#include <algorithm>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/unowned_ptr_exclusion.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/systemfontinfo_iface.h"

namespace {

static_assert(CFX_FontMapper::kLast + 1 == CFX_FontMapper::kNumStandardFonts,
              "StandardFont enum count mismatch");

constexpr std::array<const char*, CFX_FontMapper::kNumStandardFonts>
    kBase14FontNames = {{
        "Courier",
        "Courier-Bold",
        "Courier-BoldOblique",
        "Courier-Oblique",
        "Helvetica",
        "Helvetica-Bold",
        "Helvetica-BoldOblique",
        "Helvetica-Oblique",
        "Times-Roman",
        "Times-Bold",
        "Times-BoldItalic",
        "Times-Italic",
        "Symbol",
        "ZapfDingbats",
    }};

struct AltFontName {
  const char* name_;  // Raw, POD struct.
  CFX_FontMapper::StandardFont index_;
};

constexpr AltFontName kAltFontNames[] = {
    {"Arial", CFX_FontMapper::kHelvetica},
    {"Arial,Bold", CFX_FontMapper::kHelveticaBold},
    {"Arial,BoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"Arial,Italic", CFX_FontMapper::kHelveticaOblique},
    {"Arial-Bold", CFX_FontMapper::kHelveticaBold},
    {"Arial-BoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"Arial-BoldItalicMT", CFX_FontMapper::kHelveticaBoldOblique},
    {"Arial-BoldMT", CFX_FontMapper::kHelveticaBold},
    {"Arial-Italic", CFX_FontMapper::kHelveticaOblique},
    {"Arial-ItalicMT", CFX_FontMapper::kHelveticaOblique},
    {"ArialBold", CFX_FontMapper::kHelveticaBold},
    {"ArialBoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"ArialItalic", CFX_FontMapper::kHelveticaOblique},
    {"ArialMT", CFX_FontMapper::kHelvetica},
    {"ArialMT,Bold", CFX_FontMapper::kHelveticaBold},
    {"ArialMT,BoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"ArialMT,Italic", CFX_FontMapper::kHelveticaOblique},
    {"ArialRoundedMTBold", CFX_FontMapper::kHelveticaBold},
    {"Courier", CFX_FontMapper::kCourier},
    {"Courier,Bold", CFX_FontMapper::kCourierBold},
    {"Courier,BoldItalic", CFX_FontMapper::kCourierBoldOblique},
    {"Courier,Italic", CFX_FontMapper::kCourierOblique},
    {"Courier-Bold", CFX_FontMapper::kCourierBold},
    {"Courier-BoldOblique", CFX_FontMapper::kCourierBoldOblique},
    {"Courier-Oblique", CFX_FontMapper::kCourierOblique},
    {"CourierBold", CFX_FontMapper::kCourierBold},
    {"CourierBoldItalic", CFX_FontMapper::kCourierBoldOblique},
    {"CourierItalic", CFX_FontMapper::kCourierOblique},
    {"CourierNew", CFX_FontMapper::kCourier},
    {"CourierNew,Bold", CFX_FontMapper::kCourierBold},
    {"CourierNew,BoldItalic", CFX_FontMapper::kCourierBoldOblique},
    {"CourierNew,Italic", CFX_FontMapper::kCourierOblique},
    {"CourierNew-Bold", CFX_FontMapper::kCourierBold},
    {"CourierNew-BoldItalic", CFX_FontMapper::kCourierBoldOblique},
    {"CourierNew-Italic", CFX_FontMapper::kCourierOblique},
    {"CourierNewBold", CFX_FontMapper::kCourierBold},
    {"CourierNewBoldItalic", CFX_FontMapper::kCourierBoldOblique},
    {"CourierNewItalic", CFX_FontMapper::kCourierOblique},
    {"CourierNewPS-BoldItalicMT", CFX_FontMapper::kCourierBoldOblique},
    {"CourierNewPS-BoldMT", CFX_FontMapper::kCourierBold},
    {"CourierNewPS-ItalicMT", CFX_FontMapper::kCourierOblique},
    {"CourierNewPSMT", CFX_FontMapper::kCourier},
    {"CourierStd", CFX_FontMapper::kCourier},
    {"CourierStd-Bold", CFX_FontMapper::kCourierBold},
    {"CourierStd-BoldOblique", CFX_FontMapper::kCourierBoldOblique},
    {"CourierStd-Oblique", CFX_FontMapper::kCourierOblique},
    {"Helvetica", CFX_FontMapper::kHelvetica},
    {"Helvetica,Bold", CFX_FontMapper::kHelveticaBold},
    {"Helvetica,BoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"Helvetica,Italic", CFX_FontMapper::kHelveticaOblique},
    {"Helvetica-Bold", CFX_FontMapper::kHelveticaBold},
    {"Helvetica-BoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"Helvetica-BoldOblique", CFX_FontMapper::kHelveticaBoldOblique},
    {"Helvetica-Italic", CFX_FontMapper::kHelveticaOblique},
    {"Helvetica-Oblique", CFX_FontMapper::kHelveticaOblique},
    {"HelveticaBold", CFX_FontMapper::kHelveticaBold},
    {"HelveticaBoldItalic", CFX_FontMapper::kHelveticaBoldOblique},
    {"HelveticaItalic", CFX_FontMapper::kHelveticaOblique},
    {"Symbol", CFX_FontMapper::kSymbol},
    {"SymbolMT", CFX_FontMapper::kSymbol},
    {"Times-Bold", CFX_FontMapper::kTimesBold},
    {"Times-BoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"Times-Italic", CFX_FontMapper::kTimesOblique},
    {"Times-Roman", CFX_FontMapper::kTimes},
    {"TimesBold", CFX_FontMapper::kTimesBold},
    {"TimesBoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"TimesItalic", CFX_FontMapper::kTimesOblique},
    {"TimesNewRoman", CFX_FontMapper::kTimes},
    {"TimesNewRoman,Bold", CFX_FontMapper::kTimesBold},
    {"TimesNewRoman,BoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"TimesNewRoman,Italic", CFX_FontMapper::kTimesOblique},
    {"TimesNewRoman-Bold", CFX_FontMapper::kTimesBold},
    {"TimesNewRoman-BoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"TimesNewRoman-Italic", CFX_FontMapper::kTimesOblique},
    {"TimesNewRomanBold", CFX_FontMapper::kTimesBold},
    {"TimesNewRomanBoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"TimesNewRomanItalic", CFX_FontMapper::kTimesOblique},
    {"TimesNewRomanPS", CFX_FontMapper::kTimes},
    {"TimesNewRomanPS-Bold", CFX_FontMapper::kTimesBold},
    {"TimesNewRomanPS-BoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"TimesNewRomanPS-BoldItalicMT", CFX_FontMapper::kTimesBoldOblique},
    {"TimesNewRomanPS-BoldMT", CFX_FontMapper::kTimesBold},
    {"TimesNewRomanPS-Italic", CFX_FontMapper::kTimesOblique},
    {"TimesNewRomanPS-ItalicMT", CFX_FontMapper::kTimesOblique},
    {"TimesNewRomanPSMT", CFX_FontMapper::kTimes},
    {"TimesNewRomanPSMT,Bold", CFX_FontMapper::kTimesBold},
    {"TimesNewRomanPSMT,BoldItalic", CFX_FontMapper::kTimesBoldOblique},
    {"TimesNewRomanPSMT,Italic", CFX_FontMapper::kTimesOblique},
    {"ZapfDingbats", CFX_FontMapper::kDingbats},
};

struct AltFontFamily {
  const char* font_name_;    // Raw, POD struct.
  const char* font_family_;  // Raw, POD struct.
};

constexpr AltFontFamily kAltFontFamilies[] = {
    {"AGaramondPro", "Adobe Garamond Pro"},
    {"BankGothicBT-Medium", "BankGothic Md BT"},
    {"ForteMT", "Forte"},
};

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || defined(OS_ASMJS)
const char kNarrowFamily[] = "LiberationSansNarrow";
#elif BUILDFLAG(IS_ANDROID)
const char kNarrowFamily[] = "RobotoCondensed";
#else
const char kNarrowFamily[] = "ArialNarrow";
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || defined(OS_ASMJS)

ByteString TT_NormalizeName(ByteString norm) {
  norm.Remove(' ');
  norm.Remove('-');
  norm.Remove(',');
  auto pos = norm.Find('+');
  if (pos.has_value() && pos.value() != 0) {
    norm = norm.First(pos.value());
  }
  norm.MakeLower();
  return norm;
}

const char* GetFontFamily(uint32_t nStyle, const ByteString& fontname) {
  if (fontname.Contains("Script")) {
    if (FontStyleIsForceBold(nStyle)) {
      return "ScriptMTBold";
    }
    if (fontname.Contains("Palace")) {
      return "PalaceScriptMT";
    } else if (fontname.Contains("French")) {
      return "FrenchScriptMT";
    } else if (fontname.Contains("FreeStyle")) {
      return "FreeStyleScript";
    }
    return nullptr;
  }
  for (const auto& alternate : kAltFontFamilies) {
    if (fontname.Contains(alternate.font_name_)) {
      return alternate.font_family_;
    }
  }
  return nullptr;
}

ByteString ParseStyle(const ByteString& bsStyle, size_t iStart) {
  ByteStringView bsRegion = bsStyle.AsStringView().Substr(iStart);
  size_t iIndex = bsRegion.Find(',').value_or(bsRegion.GetLength());
  return ByteString(bsRegion.First(iIndex));
}

struct FX_FontStyle {
  const char* name;
  size_t len;
  uint32_t style;
};

constexpr FX_FontStyle kFontStyles[] = {
    {"Regular", 7, pdfium::kFontStyleNormal},
    {"Reg", 3, pdfium::kFontStyleNormal},
    {"BoldItalic", 10, pdfium::kFontStyleForceBold | pdfium::kFontStyleItalic},
    {"Italic", 6, pdfium::kFontStyleItalic},
    {"Bold", 4, pdfium::kFontStyleForceBold},
};

const FX_FontStyle* GetStyleType(ByteStringView font_name,
                                 bool reverse_search) {
  if (font_name.IsEmpty()) {
    return nullptr;
  }

  for (const FX_FontStyle& style : kFontStyles) {
    if (style.len > font_name.GetLength()) {
      continue;
    }

    ByteStringView style_view =
        reverse_search ? font_name.Last(style.len) : font_name.First(style.len);
    if (style_view == style.name) {
      return &style;
    }
  }
  return nullptr;
}

bool ParseStyles(const ByteString& style_str,
                 bool* is_style_available,
                 int* weight,
                 uint32_t* style) {
  if (style_str.IsEmpty()) {
    return false;
  }

  size_t i = 0;
  bool is_first_item = true;
  while (i < style_str.GetLength()) {
    ByteString buf = ParseStyle(style_str, i);
    const FX_FontStyle* style_result =
        GetStyleType(buf.AsStringView(), /*reverse_search=*/false);
    if ((i && !*is_style_available) || (!i && !style_result)) {
      return true;
    }

    uint32_t parsed_style;
    if (style_result) {
      *is_style_available = true;
      parsed_style = style_result->style;
    } else {
      parsed_style = pdfium::kFontStyleNormal;
    }

    if (FontStyleIsForceBold(parsed_style)) {
      // If we're already bold, then we're double bold, use special weight.
      if (FontStyleIsForceBold(*style)) {
        *weight = pdfium::kFontWeightExtraBold;
      } else {
        *weight = pdfium::kFontWeightBold;
        *style |= pdfium::kFontStyleForceBold;
      }

      is_first_item = false;
    }
    if (FontStyleIsItalic(parsed_style) && FontStyleIsForceBold(parsed_style)) {
      *style |= pdfium::kFontStyleItalic;
    } else if (FontStyleIsItalic(parsed_style)) {
      if (!is_first_item) {
        return true;
      }

      *style |= pdfium::kFontStyleItalic;
      break;
    }
    i += buf.GetLength() + 1;
  }
  return false;
}

bool CheckSupportThirdPartFont(const ByteString& name, int* pitch_family) {
  if (name != "MyriadPro") {
    return false;
  }
  *pitch_family &= ~pdfium::kFontPitchFamilyRoman;
  return true;
}

uint32_t GetStyleFromBaseFont(int base_font) {
  int pos = base_font % 4;
  uint32_t style = pdfium::kFontStyleNormal;
  if (pos == 1 || pos == 2) {
    style |= pdfium::kFontStyleForceBold;
  }
  if (pos / 2) {
    style |= pdfium::kFontStyleItalic;
  }
  return style;
}

int GetPitchFamilyFromBaseFont(int base_font) {
  if (base_font < 4) {
    return pdfium::kFontPitchFamilyFixed;
  }
  if (base_font >= 8) {
    return pdfium::kFontPitchFamilyRoman;
  }
  return 0;
}

int GetPitchFamilyFromFlags(uint32_t flags) {
  int pitch_family = 0;
  if (FontStyleIsSerif(flags)) {
    pitch_family |= pdfium::kFontPitchFamilyRoman;
  }
  if (FontStyleIsScript(flags)) {
    pitch_family |= pdfium::kFontPitchFamilyScript;
  }
  if (FontStyleIsFixedPitch(flags)) {
    pitch_family |= pdfium::kFontPitchFamilyFixed;
  }
  return pitch_family;
}

int AdjustBaseFontForStyle(int base_font, uint32_t style) {
  if (!style || (base_font % 4)) {
    return base_font;
  }

  if (FontStyleIsForceBold(style) && FontStyleIsItalic(style)) {
    base_font += 2;
  } else if (FontStyleIsForceBold(style)) {
    base_font += 1;
  } else if (FontStyleIsItalic(style)) {
    base_font += 3;
  }
  return base_font;
}

FX_Charset GetCharset(FX_CodePage code_page, int base_font, uint32_t flags) {
  if (code_page != FX_CodePage::kDefANSI) {
    return FX_GetCharsetFromCodePage(code_page);
  }
  if (FontStyleIsSymbolic(flags) &&
      base_font == CFX_FontMapper::kNumStandardFonts) {
    return FX_Charset::kSymbol;
  }
  return FX_Charset::kANSI;
}

bool IsStrUpper(const ByteString& str) {
  for (size_t i = 0; i < str.GetLength(); ++i) {
    if (!FXSYS_IsUpperASCII(str[i])) {
      return false;
    }
  }
  return true;
}

void RemoveSubsettedFontPrefix(ByteString* subst_name) {
  static constexpr size_t kPrefixLength = 6;
  if (subst_name->GetLength() > kPrefixLength &&
      (*subst_name)[kPrefixLength] == '+' &&
      IsStrUpper(subst_name->First(kPrefixLength))) {
    *subst_name =
        subst_name->Last(subst_name->GetLength() - (kPrefixLength + 1));
  }
}

ByteString GetSubstName(const ByteString& name, bool is_truetype) {
  ByteString subst_name = name;
  if (is_truetype && name.Front() == '@') {
    subst_name.Delete(0);
  } else {
    subst_name.Remove(' ');
  }
  RemoveSubsettedFontPrefix(&subst_name);
  CFX_FontMapper::GetStandardFontName(&subst_name);
  return subst_name;
}

bool IsNarrowFontName(const ByteString& name) {
  static const char kNarrowFonts[][10] = {"Narrow", "Condensed"};
  for (const char* font : kNarrowFonts) {
    std::optional<size_t> pos = name.Find(font);
    if (pos.has_value() && pos.value() != 0) {
      return true;
    }
  }
  return false;
}

class ScopedFontDeleter {
 public:
  FX_STACK_ALLOCATED();

  ScopedFontDeleter(SystemFontInfoIface* font_info, void* font)
      : font_info_(font_info), font_(font) {}
  ~ScopedFontDeleter() { font_info_->DeleteFont(font_); }

 private:
  UnownedPtr<SystemFontInfoIface> const font_info_;
  UNOWNED_PTR_EXCLUSION void* const font_;  // void type incompatible.
};

}  // namespace

CFX_FontMapper::CFX_FontMapper(CFX_FontMgr* mgr) : font_mgr_(mgr) {}

CFX_FontMapper::~CFX_FontMapper() = default;

void CFX_FontMapper::SetSystemFontInfo(
    std::unique_ptr<SystemFontInfoIface> font_info) {
  if (!font_info) {
    return;
  }

  list_loaded_ = false;
  font_info_ = std::move(font_info);
}

std::unique_ptr<SystemFontInfoIface> CFX_FontMapper::TakeSystemFontInfo() {
  return std::move(font_info_);
}

uint32_t CFX_FontMapper::GetChecksumFromTT(void* font_handle) {
  uint32_t buffer[256];
  font_info_->GetFontData(font_handle, kTableTTCF,
                          pdfium::as_writable_byte_span(buffer));

  uint32_t checksum = 0;
  for (auto x : buffer) {
    checksum += x;
  }
  return checksum;
}

ByteString CFX_FontMapper::GetPSNameFromTT(void* font_handle) {
  size_t size = font_info_->GetFontData(font_handle, kTableNAME, {});
  if (!size) {
    return ByteString();
  }

  DataVector<uint8_t> buffer(size);
  size_t bytes_read = font_info_->GetFontData(font_handle, kTableNAME, buffer);
  return bytes_read == size ? GetNameFromTT(buffer, 6) : ByteString();
}

void CFX_FontMapper::AddInstalledFont(const ByteString& name,
                                      FX_Charset charset) {
  if (!font_info_) {
    return;
  }

  face_array_.push_back({name, static_cast<uint32_t>(charset)});
  if (name == last_family_) {
    return;
  }

  bool is_localized = std::any_of(name.begin(), name.end(), [](const char& c) {
    return static_cast<uint8_t>(c) > 0x80;
  });

  if (is_localized) {
    void* font_handle = font_info_->GetFont(name);
    if (!font_handle) {
      font_handle =
          font_info_->MapFont(0, false, FX_Charset::kDefault, 0, name);
      if (!font_handle) {
        return;
      }
    }

    ScopedFontDeleter scoped_font(font_info_.get(), font_handle);
    ByteString new_name = GetPSNameFromTT(font_handle);
    if (!new_name.IsEmpty()) {
      localized_ttfonts_.emplace_back(new_name, name);
    }
  }
  installed_ttfonts_.push_back(name);
  last_family_ = name;
}

void CFX_FontMapper::LoadInstalledFonts() {
  if (!font_info_ || list_loaded_) {
    return;
  }

  font_info_->EnumFontList(this);
  list_loaded_ = true;
}

ByteString CFX_FontMapper::MatchInstalledFonts(const ByteString& norm_name) {
  LoadInstalledFonts();
  for (const ByteString& font : pdfium::Reversed(installed_ttfonts_)) {
    if (TT_NormalizeName(font) == norm_name) {
      return font;
    }
  }
  for (const auto& font_data : pdfium::Reversed(localized_ttfonts_)) {
    if (TT_NormalizeName(font_data.first) == norm_name) {
      return font_data.second;
    }
  }
  return ByteString();
}

RetainPtr<CFX_Face> CFX_FontMapper::UseInternalSubst(
    int base_font,
    int weight,
    int italic_angle,
    int pitch_family,
    CFX_SubstFont* subst_font) {
  if (base_font < kNumStandardFonts) {
    if (!standard_faces_[base_font]) {
      standard_faces_[base_font] = font_mgr_->NewFixedFace(
          nullptr, font_mgr_->GetStandardFont(base_font), 0);
    }
    return standard_faces_[base_font];
  }

  subst_font->SetIsBuiltInGenericFont();
  subst_font->italic_angle_ = italic_angle;
  if (weight) {
    subst_font->weight_ = weight;
  }
  if (FontFamilyIsRoman(pitch_family)) {
    subst_font->UseChromeSerif();
    if (!generic_serif_face_) {
      generic_serif_face_ =
          font_mgr_->NewFixedFace(nullptr, font_mgr_->GetGenericSerifFont(), 0);
    }
    return generic_serif_face_;
  }
  subst_font->family_ = "Chrome Sans";
  if (!generic_sans_face_) {
    generic_sans_face_ =
        font_mgr_->NewFixedFace(nullptr, font_mgr_->GetGenericSansFont(), 0);
  }
  return generic_sans_face_;
}

RetainPtr<CFX_Face> CFX_FontMapper::UseExternalSubst(
    void* font_handle,
    ByteString face_name,
    int weight,
    bool is_italic,
    int italic_angle,
    FX_Charset charset,
    CFX_SubstFont* subst_font) {
  DCHECK(font_handle);

  ScopedFontDeleter scoped_font(font_info_.get(), font_handle);
  font_info_->GetFaceName(font_handle, &face_name);
  if (charset == FX_Charset::kDefault) {
    font_info_->GetFontCharset(font_handle, &charset);
  }
  size_t ttc_size = font_info_->GetFontData(font_handle, kTableTTCF, {});
  size_t font_size = font_info_->GetFontData(font_handle, 0, {});
  if (font_size == 0 && ttc_size == 0) {
    return nullptr;
  }

  RetainPtr<CFX_Face> face =
      ttc_size
          ? GetCachedTTCFace(font_handle, ttc_size, font_size)
          : GetCachedFace(font_handle, face_name, weight, is_italic, font_size);
  if (!face) {
    return nullptr;
  }

  subst_font->family_ = face_name;
  subst_font->charset_ = charset;
  int face_weight =
      face->IsBold() ? pdfium::kFontWeightBold : pdfium::kFontWeightNormal;
  if (weight != face_weight) {
    subst_font->weight_ = weight;
  }
  if (is_italic && !face->IsItalic()) {
    if (italic_angle == 0) {
      italic_angle = -12;
    } else if (abs(italic_angle) < 5) {
      italic_angle = 0;
    }
    subst_font->italic_angle_ = italic_angle;
  }
  return face;
}

RetainPtr<CFX_Face> CFX_FontMapper::FindSubstFont(const ByteString& name,
                                                  bool is_truetype,
                                                  uint32_t flags,
                                                  int weight,
                                                  int italic_angle,
                                                  FX_CodePage code_page,
                                                  CFX_SubstFont* subst_font) {
  if (weight == 0) {
    weight = pdfium::kFontWeightNormal;
  }

  if (!(flags & FXFONT_USEEXTERNATTR)) {
    weight = pdfium::kFontWeightNormal;
    italic_angle = 0;
  }
  const ByteString subst_name = GetSubstName(name, is_truetype);
  if (subst_name == "Symbol" && !is_truetype) {
    subst_font->family_ = "Chrome Symbol";
    subst_font->charset_ = FX_Charset::kSymbol;
    return UseInternalSubst(kSymbol, weight, italic_angle, 0, subst_font);
  }
  if (subst_name == "ZapfDingbats") {
    subst_font->family_ = "Chrome Dingbats";
    subst_font->charset_ = FX_Charset::kSymbol;
    return UseInternalSubst(kDingbats, weight, italic_angle, 0, subst_font);
  }
  int base_font = 0;
  ByteString family;
  ByteString style;
  bool has_comma = false;
  bool has_hyphen = false;
  {
    std::optional<size_t> pos = subst_name.Find(",");
    if (pos.has_value()) {
      family = subst_name.First(pos.value());
      GetStandardFontName(&family);
      style = subst_name.Last(subst_name.GetLength() - (pos.value() + 1));
      has_comma = true;
    } else {
      family = subst_name;
    }
  }
  for (; base_font < kSymbol; base_font++) {
    if (family == kBase14FontNames[base_font]) {
      break;
    }
  }
  int pitch_family;
  uint32_t nStyle;
  bool is_style_available = false;
  if (base_font < kSymbol) {
    nStyle = GetStyleFromBaseFont(base_font);
    pitch_family = GetPitchFamilyFromBaseFont(base_font);
  } else {
    base_font = kNumStandardFonts;
    nStyle = pdfium::kFontStyleNormal;
    if (!has_comma) {
      std::optional<size_t> pos = family.ReverseFind('-');
      if (pos.has_value()) {
        style = family.Last(family.GetLength() - (pos.value() + 1));
        family = family.First(pos.value());
        has_hyphen = true;
      }
    }
    if (!has_hyphen) {
      size_t nLen = family.GetLength();
      const FX_FontStyle* style_result =
          GetStyleType(family.AsStringView(), /*reverse_search=*/true);
      if (style_result) {
        family = family.First(nLen - style_result->len);
        nStyle |= style_result->style;
      }
    }
    pitch_family = GetPitchFamilyFromFlags(flags);
  }

  const int old_weight = weight;
  if (FontStyleIsForceBold(nStyle)) {
    weight = pdfium::kFontWeightBold;
  }

  if (ParseStyles(style, &is_style_available, &weight, &nStyle)) {
    family = subst_name;
    base_font = kNumStandardFonts;
  }

  if (!font_info_) {
    return UseInternalSubst(base_font, old_weight, italic_angle, pitch_family,
                            subst_font);
  }

  const FX_Charset Charset = GetCharset(code_page, base_font, flags);
  const bool is_cjk = FX_CharSetIsCJK(Charset);
  bool is_italic = FontStyleIsItalic(nStyle);

  const char* maybe_family = GetFontFamily(nStyle, family);
  if (maybe_family) {
    family = ByteString(maybe_family);
  }

  ByteString match = MatchInstalledFonts(TT_NormalizeName(family));
  if (match.IsEmpty() && family != subst_name &&
      (!has_comma && (!has_hyphen || (has_hyphen && !is_style_available)))) {
    match = MatchInstalledFonts(TT_NormalizeName(subst_name));
  }
  if (match.IsEmpty() && base_font >= kNumStandardFonts) {
    if (!is_cjk) {
      if (!CheckSupportThirdPartFont(family, &pitch_family)) {
        is_italic = italic_angle != 0;
        weight = old_weight;
      }
      if (IsNarrowFontName(subst_name)) {
        family = kNarrowFamily;
      }
    } else {
      subst_font->subst_cjk_ = true;
      if (nStyle) {
        subst_font->weight_cjk_ = nStyle ? weight : pdfium::kFontWeightNormal;
      }
      if (FontStyleIsItalic(nStyle)) {
        subst_font->italic_cjk_ = true;
      }
    }
  } else {
    italic_angle = 0;
    if (nStyle == pdfium::kFontStyleNormal) {
      weight = pdfium::kFontWeightNormal;
    }
  }

  if (!match.IsEmpty() || base_font < kNumStandardFonts) {
    if (!match.IsEmpty()) {
      family = match;
    }
    if (base_font < kNumStandardFonts) {
      base_font = AdjustBaseFontForStyle(base_font, nStyle);
      family = kBase14FontNames[base_font];
    }
  } else if (FontStyleIsItalic(flags)) {
    is_italic = true;
  }
  void* font_handle =
      font_info_->MapFont(weight, is_italic, Charset, pitch_family, family);
  if (font_handle) {
    return UseExternalSubst(font_handle, subst_name, weight, is_italic,
                            italic_angle, Charset, subst_font);
  }

  if (is_cjk) {
    is_italic = italic_angle != 0;
    weight = old_weight;
  }
  if (!match.IsEmpty()) {
    font_handle = font_info_->GetFont(match);
    if (!font_handle) {
      return UseInternalSubst(base_font, old_weight, italic_angle, pitch_family,
                              subst_font);
    }
    return UseExternalSubst(font_handle, subst_name, weight, is_italic,
                            italic_angle, Charset, subst_font);
  }

  if (Charset == FX_Charset::kSymbol) {
#if !BUILDFLAG(IS_WIN)
    if (subst_name == "Symbol") {
      subst_font->family_ = "Chrome Symbol";
      subst_font->charset_ = FX_Charset::kSymbol;
      return UseInternalSubst(kSymbol, old_weight, italic_angle, pitch_family,
                              subst_font);
    }
#endif
    return FindSubstFont(family, is_truetype,
                         flags & ~pdfium::kFontStyleSymbolic, weight,
                         italic_angle, FX_CodePage::kDefANSI, subst_font);
  }

  if (Charset == FX_Charset::kANSI) {
    return UseInternalSubst(base_font, old_weight, italic_angle, pitch_family,
                            subst_font);
  }

  auto it = std::ranges::find_if(face_array_, [Charset](const FaceData& face) {
    return face.charset == static_cast<uint32_t>(Charset);
  });
  if (it == face_array_.end()) {
    return UseInternalSubst(base_font, old_weight, italic_angle, pitch_family,
                            subst_font);
  }
  font_handle = font_info_->GetFont(it->name);
  if (!font_handle) {
    return nullptr;
  }
  return UseExternalSubst(font_handle, subst_name, weight, is_italic,
                          italic_angle, Charset, subst_font);
}

size_t CFX_FontMapper::GetFaceSize() const {
  return face_array_.size();
}

ByteString CFX_FontMapper::GetFaceName(size_t index) const {
  CHECK_LT(index, face_array_.size());
  return face_array_[index].name;
}

bool CFX_FontMapper::HasInstalledFont(ByteStringView name) const {
  for (const auto& font : installed_ttfonts_) {
    if (font == name) {
      return true;
    }
  }
  return false;
}

bool CFX_FontMapper::HasLocalizedFont(ByteStringView name) const {
  for (const auto& fontPair : localized_ttfonts_) {
    if (fontPair.first == name) {
      return true;
    }
  }
  return false;
}

#if BUILDFLAG(IS_WIN)
std::optional<ByteString> CFX_FontMapper::InstalledFontNameStartingWith(
    const ByteString& name) const {
  for (const auto& thisname : installed_ttfonts_) {
    if (thisname.First(name.GetLength()) == name) {
      return thisname;
    }
  }
  return std::nullopt;
}

std::optional<ByteString> CFX_FontMapper::LocalizedFontNameStartingWith(
    const ByteString& name) const {
  for (const auto& thispair : localized_ttfonts_) {
    if (thispair.first.First(name.GetLength()) == name) {
      return thispair.second;
    }
  }
  return std::nullopt;
}
#endif  // BUILDFLAG(IS_WIN)

#ifdef PDF_ENABLE_XFA
FixedSizeDataVector<uint8_t> CFX_FontMapper::RawBytesForIndex(size_t index) {
  CHECK_LT(index, face_array_.size());

  void* font_handle = font_info_->MapFont(0, false, FX_Charset::kDefault, 0,
                                          GetFaceName(index));
  if (!font_handle) {
    return FixedSizeDataVector<uint8_t>();
  }
  ScopedFontDeleter scoped_font(font_info_.get(), font_handle);
  size_t required_size = font_info_->GetFontData(font_handle, 0, {});
  if (required_size == 0) {
    return FixedSizeDataVector<uint8_t>();
  }
  auto result = FixedSizeDataVector<uint8_t>::Uninit(required_size);
  size_t actual_size = font_info_->GetFontData(font_handle, 0, result.span());
  if (actual_size != required_size) {
    return FixedSizeDataVector<uint8_t>();
  }
  return result;
}
#endif  // PDF_ENABLE_XFA

RetainPtr<CFX_Face> CFX_FontMapper::GetCachedTTCFace(void* font_handle,
                                                     size_t ttc_size,
                                                     size_t data_size) {
  CHECK_GE(ttc_size, data_size);
  uint32_t checksum = GetChecksumFromTT(font_handle);
  RetainPtr<CFX_FontMgr::FontDesc> font_desc =
      font_mgr_->GetCachedTTCFontDesc(ttc_size, checksum);
  if (!font_desc) {
    auto font_data = FixedSizeDataVector<uint8_t>::Uninit(ttc_size);
    size_t size =
        font_info_->GetFontData(font_handle, kTableTTCF, font_data.span());
    if (size != ttc_size) {
      return nullptr;
    }

    font_desc = font_mgr_->AddCachedTTCFontDesc(ttc_size, checksum,
                                                std::move(font_data));
  }
  size_t font_offset = ttc_size - data_size;
  size_t face_index =
      GetTTCIndex(font_desc->FontData().first(ttc_size), font_offset);
  RetainPtr<CFX_Face> pFace(font_desc->GetFace(face_index));
  if (pFace) {
    return pFace;
  }

  pFace = font_mgr_->NewFixedFace(
      font_desc, font_desc->FontData().first(ttc_size), face_index);
  if (!pFace) {
    return nullptr;
  }

  font_desc->SetFace(face_index, pFace.Get());
  return pFace;
}

RetainPtr<CFX_Face> CFX_FontMapper::GetCachedFace(void* font_handle,
                                                  ByteString subst_name,
                                                  int weight,
                                                  bool is_italic,
                                                  size_t data_size) {
  RetainPtr<CFX_FontMgr::FontDesc> font_desc =
      font_mgr_->GetCachedFontDesc(subst_name, weight, is_italic);
  if (!font_desc) {
    auto font_data = FixedSizeDataVector<uint8_t>::Uninit(data_size);
    size_t size = font_info_->GetFontData(font_handle, 0, font_data.span());
    if (size != data_size) {
      return nullptr;
    }

    font_desc = font_mgr_->AddCachedFontDesc(subst_name, weight, is_italic,
                                             std::move(font_data));
  }
  RetainPtr<CFX_Face> pFace(font_desc->GetFace(0));
  if (pFace) {
    return pFace;
  }

  pFace = font_mgr_->NewFixedFace(font_desc,
                                  font_desc->FontData().first(data_size), 0);
  if (!pFace) {
    return nullptr;
  }

  font_desc->SetFace(0, pFace.Get());
  return pFace;
}

// static
std::optional<CFX_FontMapper::StandardFont> CFX_FontMapper::GetStandardFontName(
    ByteString* name) {
  const auto* end = std::end(kAltFontNames);
  const auto* found =
      std::lower_bound(std::begin(kAltFontNames), end, name->c_str(),
                       [](const AltFontName& element, const char* name) {
                         return FXSYS_stricmp(element.name_, name) < 0;
                       });
  if (found == end || FXSYS_stricmp(found->name_, name->c_str())) {
    return std::nullopt;
  }

  *name = kBase14FontNames[static_cast<size_t>(found->index_)];
  return found->index_;
}

// static
bool CFX_FontMapper::IsStandardFontName(const ByteString& name) {
  return pdfium::Contains(kBase14FontNames, name);
}

// static
bool CFX_FontMapper::IsSymbolicFont(StandardFont font) {
  return font == StandardFont::kSymbol || font == StandardFont::kDingbats;
}

// static
bool CFX_FontMapper::IsFixedFont(StandardFont font) {
  return font == StandardFont::kCourier || font == StandardFont::kCourierBold ||
         font == StandardFont::kCourierBoldOblique ||
         font == StandardFont::kCourierOblique;
}
