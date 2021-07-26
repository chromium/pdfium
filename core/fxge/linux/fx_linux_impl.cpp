// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

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
#include "third_party/base/cxx17_backports.h"

#if !defined(OS_LINUX) && !defined(OS_CHROMEOS) && !defined(OS_ASMJS)
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

const char* const g_LinuxJpFontList[][JpFontFamily::kCount] = {
    {"TakaoPGothic", "VL PGothic", "IPAPGothic", "VL Gothic"},
    {"TakaoGothic", "VL Gothic", "IPAGothic", "Kochi Gothic"},
    {"TakaoPMincho", "IPAPMincho", "VL Gothic", "Kochi Mincho"},
    {"TakaoMincho", "IPAMincho", "VL Gothic", "Kochi Mincho"},
};

const char* const g_LinuxGbFontList[] = {
    "AR PL UMing CN Light",
    "WenQuanYi Micro Hei",
    "AR PL UKai CN",
};

const char* const g_LinuxB5FontList[] = {
    "AR PL UMing TW Light",
    "WenQuanYi Micro Hei",
    "AR PL UKai TW",
};

const char* const g_LinuxHGFontList[] = {
    "UnDotum",
};

uint8_t GetJapanesePreference(const char* facearr,
                              int weight,
                              int pitch_family) {
  ByteString face = facearr;
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
                const char* family) override;

  bool ParseFontCfg(const char** pUserPaths);
};

void* CFX_LinuxFontInfo::MapFont(int weight,
                                 bool bItalic,
                                 FX_Charset charset,
                                 int pitch_family,
                                 const char* family) {
  void* font = GetSubstFont(family);
  if (font)
    return font;

  bool bCJK = true;
  switch (charset) {
    case FX_Charset::kShiftJIS: {
      uint8_t index = GetJapanesePreference(family, weight, pitch_family);
      DCHECK(index < pdfium::size(g_LinuxJpFontList));
      for (const char* name : g_LinuxJpFontList[index]) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    case FX_Charset::kChineseSimplified: {
      for (const char* name : g_LinuxGbFontList) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    case FX_Charset::kChineseTraditional: {
      for (const char* name : g_LinuxB5FontList) {
        auto it = m_FontList.find(name);
        if (it != m_FontList.end())
          return it->second.get();
      }
      break;
    }
    case FX_Charset::kHangul: {
      for (const char* name : g_LinuxHGFontList) {
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
  return FindFont(weight, bItalic, charset, pitch_family, family, !bCJK);
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
