// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fontmgr.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "xfa/fgas/font/cfgas_defaultfontmanager.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/fgas_fontutils.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

CXFA_FontMgr::CXFA_FontMgr() = default;

CXFA_FontMgr::~CXFA_FontMgr() = default;

void CXFA_FontMgr::Trace(cppgc::Visitor* visitor) const {}

RetainPtr<CFGAS_GEFont> CXFA_FontMgr::GetFont(CXFA_FFDoc* doc,
                                              const WideString& wsFontFamily,
                                              uint32_t dwFontStyles) {
  auto key = std::make_pair(wsFontFamily, dwFontStyles);
  auto iter = font_map_.find(key);
  if (iter != font_map_.end()) {
    return iter->second;
  }

  WideString wsEnglishName = FGAS_FontNameToEnglishName(wsFontFamily);
  RetainPtr<CFGAS_GEFont> font =
      doc->GetPDFFont(wsEnglishName, dwFontStyles, true);
  if (font) {
    return font;
  }

  font = CFGAS_DefaultFontManager::GetFont(wsFontFamily, dwFontStyles);
  if (!font) {
    font = doc->GetPDFFont(wsEnglishName, dwFontStyles, false);
    if (font) {
      return font;
    }
  }
  if (!font) {
    font = CFGAS_DefaultFontManager::GetDefaultFont(dwFontStyles);
  }
  if (!font) {
    font = CFGAS_GEFont::LoadStockFont(
        doc->GetPDFDoc(), ByteString::Format("%ls", wsFontFamily.c_str()));
  }
  if (!font) {
    return nullptr;
  }

  font_map_[key] = font;
  return font;
}
