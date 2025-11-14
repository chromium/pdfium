// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_FONTMAPPER_H_
#define CORE_FXGE_CFX_FONTMAPPER_H_

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_face.h"

#ifdef PDF_ENABLE_XFA
#include "core/fxcrt/fixed_size_data_vector.h"
#endif

class CFX_FontMgr;
class CFX_SubstFont;
class SystemFontInfoIface;

class CFX_FontMapper {
 public:
  enum StandardFont : uint8_t {
    kCourier = 0,
    kCourierBold,
    kCourierBoldOblique,
    kCourierOblique,
    kHelvetica,
    kHelveticaBold,
    kHelveticaBoldOblique,
    kHelveticaOblique,
    kTimes,
    kTimesBold,
    kTimesBoldOblique,
    kTimesOblique,
    kSymbol,
    kDingbats,
    kLast = kDingbats
  };
  static constexpr int kNumStandardFonts = 14;

  explicit CFX_FontMapper(CFX_FontMgr* mgr);
  ~CFX_FontMapper();

  static std::optional<StandardFont> GetStandardFontName(ByteString* name);
  static bool IsStandardFontName(const ByteString& name);
  static bool IsSymbolicFont(StandardFont font);
  static bool IsFixedFont(StandardFont font);
  static constexpr uint32_t MakeTag(char c1, char c2, char c3, char c4) {
    return static_cast<uint8_t>(c1) << 24 | static_cast<uint8_t>(c2) << 16 |
           static_cast<uint8_t>(c3) << 8 | static_cast<uint8_t>(c4);
  }

  void SetSystemFontInfo(std::unique_ptr<SystemFontInfoIface> font_info);
  std::unique_ptr<SystemFontInfoIface> TakeSystemFontInfo();
  void SetSkipFontEnumeration(bool skip) { skip_font_enumeration_ = skip; }
  void AddInstalledFont(const ByteString& name, FX_Charset charset);
  void LoadInstalledFonts();

  RetainPtr<CFX_Face> FindSubstFont(const ByteString& face_name,
                                    bool is_truetype,
                                    uint32_t flags,
                                    int weight,
                                    int italic_angle,
                                    FX_CodePage code_page,
                                    CFX_SubstFont* subst_font);

  size_t GetFaceSize() const;
  // `index` must be less than GetFaceSize().
  ByteString GetFaceName(size_t index) const;
  bool HasInstalledFont(ByteStringView name) const;
  bool HasLocalizedFont(ByteStringView name) const;

#if BUILDFLAG(IS_WIN)
  std::optional<ByteString> InstalledFontNameStartingWith(
      const ByteString& name) const;
  std::optional<ByteString> LocalizedFontNameStartingWith(
      const ByteString& name) const;
#endif  // BUILDFLAG(IS_WIN)

#ifdef PDF_ENABLE_XFA
  // `index` must be less than GetFaceSize().
  FixedSizeDataVector<uint8_t> RawBytesForIndex(size_t index);
#endif  // PDF_ENABLE_XFA

 private:
  friend class TestFontMapper;

  uint32_t GetChecksumFromTT(void* font_handle);
  ByteString GetPSNameFromTT(void* font_handle);
  ByteString MatchInstalledFonts(const ByteString& norm_name);
  RetainPtr<CFX_Face> UseInternalSubst(int base_font,
                                       int weight,
                                       int italic_angle,
                                       int pitch_family,
                                       CFX_SubstFont* subst_font);
  RetainPtr<CFX_Face> UseExternalSubst(void* font_handle,
                                       ByteString face_name,
                                       int weight,
                                       bool is_italic,
                                       int italic_angle,
                                       FX_Charset charset,
                                       CFX_SubstFont* subst_font);
  RetainPtr<CFX_Face> GetCachedTTCFace(void* font_handle,
                                       size_t ttc_size,
                                       size_t data_size);
  RetainPtr<CFX_Face> GetCachedFace(void* font_handle,
                                    ByteString subst_name,
                                    int weight,
                                    bool is_italic,
                                    size_t data_size);

  struct FaceData {
    ByteString name;
    uint32_t charset;
  };

  bool list_loaded_ = false;
  bool skip_font_enumeration_ = false;
  ByteString last_family_;
  std::vector<FaceData> face_array_;
  std::unique_ptr<SystemFontInfoIface> font_info_;
  UnownedPtr<CFX_FontMgr> const font_mgr_;
  std::vector<ByteString> installed_ttfonts_;
  std::vector<std::pair<ByteString, ByteString>> localized_ttfonts_;
  std::array<RetainPtr<CFX_Face>, kNumStandardFonts> standard_faces_;
  RetainPtr<CFX_Face> generic_sans_face_;
  RetainPtr<CFX_Face> generic_serif_face_;
};

#endif  // CORE_FXGE_CFX_FONTMAPPER_H_
