// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <iterator>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_folderfontinfo.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "third_party/base/check.h"

#if !BUILDFLAG(IS_LINUX) && !BUILDFLAG(IS_CHROMEOS) && !defined(OS_FUCHSIA) && \
    !defined(OS_ASMJS)
#error "Included on the wrong platform"
#endif

namespace {

enum JpFontFamily : uint8_t {
  kJpFontPGothic,
  kJpFontGothic,
  kJpFontPMincho,
  kJpFontMincho,
  kCount
};

const char* const kLinuxJpFontList[][JpFontFamily::kCount] = {
    {"TakaoPGothic", "VL PGothic", "IPAPGothic", "VL Gothic"},
    {"TakaoGothic", "VL Gothic", "IPAGothic", "Kochi Gothic"},
    {"TakaoPMincho", "IPAPMincho", "VL Gothic", "Kochi Mincho"},
    {"TakaoMincho", "IPAMincho", "VL Gothic", "Kochi Mincho"},
};

const char* const kLinuxGbFontList[] = {
    "AR PL UMing CN Light",
    "WenQuanYi Micro Hei",
    "AR PL UKai CN",
};

const char* const kLinuxB5FontList[] = {
    "AR PL UMing TW Light",
    "WenQuanYi Micro Hei",
    "AR PL UKai TW",
};

const char* const kLinuxHGFontList[] = {
    "UnDotum",
};

JpFontFamily GetJapanesePreference(const ByteString& face,
                                   int weight,
                                   int pitch_family) {
  if (face.Contains("Gothic") ||
      face.Contains("\x83\x53\x83\x56\x83\x62\x83\x4e")) {
    if (face.Contains("PGothic") ||
        face.Contains("\x82\x6f\x83\x53\x83\x56\x83\x62\x83\x4e")) {
      return kJpFontPGothic;
    }
    return kJpFontGothic;
  }
  if (face.Contains("Mincho") || face.Contains("\x96\xbe\x92\xa9")) {
    if (face.Contains("PMincho") || face.Contains("\x82\x6f\x96\xbe\x92\xa9")) {
      return kJpFontPMincho;
    }
    return kJpFontMincho;
  }
  if (!FontFamilyIsRoman(pitch_family) && weight > 400)
    return kJpFontPGothic;

  return kJpFontPMincho;
}

class CFX_LinuxFontInfo final : public CFX_FolderFontInfo {
 public:
  CFX_LinuxFontInfo() = default;
  ~CFX_LinuxFontInfo() override = default;

  // CFX_FolderFontInfo:
  void* MapFont(int weight,
                bool bItalic,
                FX_Charset charset,
                int pitch_family,
                const ByteString& face) override;

  bool ParseFontCfg(const char** pUserPaths);
};

void* CFX_LinuxFontInfo::MapFont(int weight,
                                 bool bItalic,
                                 FX_Charset charset,
                                 int pitch_family,
                                 const ByteString& face) {
  void* font = GetSubstFont(face);
  if (font)
    return font;

  bool bCJK = true;
  switch (charset) {
    case FX_Charset::kShiftJIS: {
      JpFontFamily index = GetJapanesePreference(face, weight, pitch_family);
      DCHECK(index < std::size(kLinuxJpFontList));
      for (const char* name : kLinuxJpFontList[index]) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    case FX_Charset::kChineseSimplified: {
      for (const char* name : kLinuxGbFontList) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    case FX_Charset::kChineseTraditional: {
      for (const char* name : kLinuxB5FontList) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    case FX_Charset::kHangul: {
      for (const char* name : kLinuxHGFontList) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    default:
      bCJK = false;
      break;
  }
  return FindFont(weight, bItalic, charset, pitch_family, face, !bCJK);
}

bool CFX_LinuxFontInfo::ParseFontCfg(const char** pUserPaths) {
  if (!pUserPaths)
    return false;

  for (const char** pPath = pUserPaths; *pPath; ++pPath)
    AddPath(*pPath);
  return true;
}

}  // namespace

class CLinuxPlatform : public CFX_GEModule::PlatformIface {
 public:
  CLinuxPlatform() = default;
  ~CLinuxPlatform() override = default;

  void Init() override {}

  std::unique_ptr<SystemFontInfoIface> CreateDefaultSystemFontInfo() override {
    auto pInfo = std::make_unique<CFX_LinuxFontInfo>();
    if (!pInfo->ParseFontCfg(CFX_GEModule::Get()->GetUserFontPaths())) {
      pInfo->AddPath("/usr/share/fonts");
      pInfo->AddPath("/usr/share/X11/fonts/Type1");
      pInfo->AddPath("/usr/share/X11/fonts/TTF");
      pInfo->AddPath("/usr/local/share/fonts");
    }
    return pInfo;
  }
};

// static
std::unique_ptr<CFX_GEModule::PlatformIface>
CFX_GEModule::PlatformIface::Create() {
  return std::make_unique<CLinuxPlatform>();
}
