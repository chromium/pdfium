// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/android/cfx_androidfontinfo.h"

#include "core/fxcrt/fx_system.h"
#include "core/fxge/android/cfpf_skiafont.h"
#include "core/fxge/android/cfpf_skiafontmgr.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/fx_font.h"

CFX_AndroidFontInfo::CFX_AndroidFontInfo() = default;

CFX_AndroidFontInfo::~CFX_AndroidFontInfo() = default;

bool CFX_AndroidFontInfo::Init(CFPF_SkiaFontMgr* font_mgr,
                               const char** user_paths) {
  if (!font_mgr) {
    return false;
  }

  font_mgr_ = font_mgr;
  font_mgr_->LoadFonts(user_paths);
  return true;
}

void CFX_AndroidFontInfo::EnumFontList(CFX_FontMapper* pMapper) {}

void* CFX_AndroidFontInfo::MapFont(int weight,
                                   bool bItalic,
                                   FX_Charset charset,
                                   int pitch_family,
                                   const ByteString& face) {
  if (!font_mgr_) {
    return nullptr;
  }

  uint32_t dwStyle = 0;
  if (weight >= 700) {
    dwStyle |= pdfium::kFontStyleForceBold;
  }
  if (bItalic) {
    dwStyle |= pdfium::kFontStyleItalic;
  }
  if (FontFamilyIsFixedPitch(pitch_family)) {
    dwStyle |= pdfium::kFontStyleFixedPitch;
  }
  if (FontFamilyIsScript(pitch_family)) {
    dwStyle |= pdfium::kFontStyleScript;
  }
  if (FontFamilyIsRoman(pitch_family)) {
    dwStyle |= pdfium::kFontStyleSerif;
  }
  return font_mgr_->CreateFont(face.AsStringView(), charset, dwStyle);
}

void* CFX_AndroidFontInfo::GetFont(const ByteString& face) {
  return nullptr;
}

size_t CFX_AndroidFontInfo::GetFontData(void* hFont,
                                        uint32_t table,
                                        pdfium::span<uint8_t> buffer) {
  if (!hFont) {
    return 0;
  }
  return static_cast<CFPF_SkiaFont*>(hFont)->GetFontData(table, buffer);
}

bool CFX_AndroidFontInfo::GetFaceName(void* hFont, ByteString* name) {
  if (!hFont) {
    return false;
  }

  *name = static_cast<CFPF_SkiaFont*>(hFont)->GetFamilyName();
  return true;
}

bool CFX_AndroidFontInfo::GetFontCharset(void* hFont, FX_Charset* charset) {
  if (!hFont) {
    return false;
  }

  *charset = static_cast<CFPF_SkiaFont*>(hFont)->GetCharset();
  return false;
}

void CFX_AndroidFontInfo::DeleteFont(void* hFont) {}
