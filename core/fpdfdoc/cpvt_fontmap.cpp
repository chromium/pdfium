// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpvt_fontmap.h"

#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/notreached.h"

CPVT_FontMap::CPVT_FontMap(CPDF_Document* doc,
                           RetainPtr<CPDF_Dictionary> pResDict,
                           RetainPtr<CPDF_Font> pDefFont,
                           const ByteString& sDefFontAlias)
    : document_(doc),
      res_dict_(std::move(pResDict)),
      def_font_(std::move(pDefFont)),
      def_font_alias_(sDefFontAlias) {}

CPVT_FontMap::~CPVT_FontMap() = default;

void CPVT_FontMap::SetupAnnotSysPDFFont() {
  if (!document_ || !res_dict_) {
    return;
  }

  RetainPtr<CPDF_Font> pPDFFont =
      CPDF_InteractiveForm::AddNativeInteractiveFormFont(document_,
                                                         &sys_font_alias_);
  if (!pPDFFont) {
    return;
  }

  RetainPtr<CPDF_Dictionary> font_list = res_dict_->GetMutableDictFor("Font");
  if (ValidateFontResourceDict(font_list.Get()) &&
      !font_list->KeyExist(sys_font_alias_.AsStringView())) {
    font_list->SetNewFor<CPDF_Reference>(sys_font_alias_, document_,
                                         pPDFFont->GetFontDictObjNum());
  }
  sys_font_ = std::move(pPDFFont);
}

RetainPtr<CPDF_Font> CPVT_FontMap::GetPDFFont(int32_t nFontIndex) {
  switch (nFontIndex) {
    case 0:
      return def_font_;
    case 1:
      if (!sys_font_) {
        SetupAnnotSysPDFFont();
      }
      return sys_font_;
    default:
      return nullptr;
  }
}

ByteString CPVT_FontMap::GetPDFFontAlias(int32_t nFontIndex) {
  switch (nFontIndex) {
    case 0:
      return def_font_alias_;
    case 1:
      if (!sys_font_) {
        SetupAnnotSysPDFFont();
      }
      return sys_font_alias_;
    default:
      return ByteString();
  }
}

int32_t CPVT_FontMap::GetWordFontIndex(uint16_t word,
                                       FX_Charset charset,
                                       int32_t nFontIndex) {
  NOTREACHED();
}

int32_t CPVT_FontMap::CharCodeFromUnicode(int32_t nFontIndex, uint16_t word) {
  NOTREACHED();
}

FX_Charset CPVT_FontMap::CharSetFromUnicode(uint16_t word,
                                            FX_Charset nOldCharset) {
  NOTREACHED();
}
