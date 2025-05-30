// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_CFGAS_FONTMGR_H_
#define XFA_FGAS_FONT_CFGAS_FONTMGR_H_

#include <array>
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/cfx_face.h"

class CFGAS_GEFont;
class IFX_SeekableReadStream;

#if BUILDFLAG(IS_WIN)
struct FX_FONTSIGNATURE {
  std::array<uint32_t, 4> fsUsb;
  std::array<uint32_t, 2> fsCsb;
};

inline bool operator==(const FX_FONTSIGNATURE& left,
                       const FX_FONTSIGNATURE& right) {
  return left.fsUsb[0] == right.fsUsb[0] && left.fsUsb[1] == right.fsUsb[1] &&
         left.fsUsb[2] == right.fsUsb[2] && left.fsUsb[3] == right.fsUsb[3] &&
         left.fsCsb[0] == right.fsCsb[0] && left.fsCsb[1] == right.fsCsb[1];
}

struct FX_FONTDESCRIPTOR {
  wchar_t wsFontFace[32];
  uint32_t dwFontStyles;
  FX_Charset uCharSet;
  FX_FONTSIGNATURE FontSignature;
};

inline bool operator==(const FX_FONTDESCRIPTOR& left,
                       const FX_FONTDESCRIPTOR& right) {
  return left.uCharSet == right.uCharSet &&
         left.dwFontStyles == right.dwFontStyles &&
         left.FontSignature == right.FontSignature &&
         UNSAFE_TODO(wcscmp(left.wsFontFace, right.wsFontFace)) == 0;
}

#else  // BUILDFLAG(IS_WIN)

class CFGAS_FontDescriptor {
 public:
  CFGAS_FontDescriptor();
  ~CFGAS_FontDescriptor();

  int32_t face_index_ = 0;
  uint32_t font_styles_ = 0;
  WideString face_name_;
  RetainPtr<CFX_Face> face_;
  std::vector<WideString> family_names_;
  std::array<uint32_t, 4> usb_ = {};
  std::array<uint32_t, 2> csb_ = {};
};

struct CFGAS_FontDescriptorInfo {
 public:
  UNOWNED_PTR_EXCLUSION CFGAS_FontDescriptor* font;  // POD struct.
  int32_t nPenalty;

  bool operator>(const CFGAS_FontDescriptorInfo& other) const {
    return nPenalty > other.nPenalty;
  }
  bool operator<(const CFGAS_FontDescriptorInfo& other) const {
    return nPenalty < other.nPenalty;
  }
  friend inline bool operator==(const CFGAS_FontDescriptorInfo& lhs,
                                const CFGAS_FontDescriptorInfo& rhs) {
    return lhs.nPenalty == rhs.nPenalty;
  }
};

#endif  // BUILDFLAG(IS_WIN)

class CFGAS_FontMgr {
 public:
  CFGAS_FontMgr();
  ~CFGAS_FontMgr();

  bool EnumFonts();
  RetainPtr<CFGAS_GEFont> GetFontByCodePage(FX_CodePage wCodePage,
                                            uint32_t dwFontStyles,
                                            const wchar_t* pszFontFamily);
  RetainPtr<CFGAS_GEFont> GetFontByUnicode(wchar_t wUnicode,
                                           uint32_t dwFontStyles,
                                           const wchar_t* pszFontFamily);
  RetainPtr<CFGAS_GEFont> LoadFont(const wchar_t* pszFontFamily,
                                   uint32_t dwFontStyles,
                                   FX_CodePage wCodePage);

 private:
  RetainPtr<CFGAS_GEFont> GetFontByUnicodeImpl(wchar_t wUnicode,
                                               uint32_t dwFontStyles,
                                               const wchar_t* pszFontFamily,
                                               uint32_t dwHash,
                                               FX_CodePage wCodePage,
                                               uint16_t wBitField);

#if BUILDFLAG(IS_WIN)
  const FX_FONTDESCRIPTOR* FindFont(const wchar_t* pszFontFamily,
                                    uint32_t dwFontStyles,
                                    bool matchParagraphStyle,
                                    FX_CodePage wCodePage,
                                    uint32_t dwUSB,
                                    wchar_t wUnicode);

#else   // BUILDFLAG(IS_WIN)
  bool EnumFontsFromFontMapper();
  void RegisterFace(RetainPtr<CFX_Face> pFace, const WideString& wsFaceName);
  void RegisterFaces(const RetainPtr<IFX_SeekableReadStream>& font_stream,
                     const WideString& wsFaceName);
  std::vector<CFGAS_FontDescriptorInfo> MatchFonts(FX_CodePage wCodePage,
                                                   uint32_t dwFontStyles,
                                                   const WideString& FontName,
                                                   wchar_t wcUnicode);
  RetainPtr<CFGAS_GEFont> LoadFontInternal(const WideString& wsFaceName,
                                           int32_t iFaceIndex);
#endif  // BUILDFLAG(IS_WIN)

  std::map<uint32_t, std::vector<RetainPtr<CFGAS_GEFont>>> hash_2fonts_;
  std::set<wchar_t> failed_unicodes_set_;

#if BUILDFLAG(IS_WIN)
  std::deque<FX_FONTDESCRIPTOR> font_faces_;
#else
  std::vector<std::unique_ptr<CFGAS_FontDescriptor>> installed_fonts_;
  std::map<uint32_t, std::vector<CFGAS_FontDescriptorInfo>>
      hash_2candidate_list_;
#endif  // BUILDFLAG(IS_WIN)
};

#endif  // XFA_FGAS_FONT_CFGAS_FONTMGR_H_
