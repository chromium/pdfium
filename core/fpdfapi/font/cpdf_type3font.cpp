// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_type3font.h"

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>

#include "core/fpdfapi/font/cpdf_type3char.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_system.h"

namespace {

constexpr int kMaxType3FormLevel = 4;

}  // namespace

CPDF_Type3Font::CPDF_Type3Font(CPDF_Document* document,
                               RetainPtr<CPDF_Dictionary> font_dict,
                               FormFactoryIface* pFormFactory)
    : CPDF_SimpleFont(document, std::move(font_dict)),
      form_factory_(pFormFactory) {
  DCHECK(GetDocument());
}

CPDF_Type3Font::~CPDF_Type3Font() = default;

bool CPDF_Type3Font::IsType3Font() const {
  return true;
}

const CPDF_Type3Font* CPDF_Type3Font::AsType3Font() const {
  return this;
}

CPDF_Type3Font* CPDF_Type3Font::AsType3Font() {
  return this;
}

void CPDF_Type3Font::WillBeDestroyed() {
  will_be_destroyed_ = true;

  // Last reference to |this| may be through one of its CPDF_Type3Chars.
  RetainPtr<CPDF_Font> protector(this);
  for (const auto& item : cache_map_) {
    if (item.second) {
      item.second->WillBeDestroyed();
    }
  }
}

bool CPDF_Type3Font::Load() {
  font_resources_ = font_dict_->GetMutableDictFor("Resources");
  RetainPtr<const CPDF_Array> pMatrix = font_dict_->GetArrayFor("FontMatrix");
  float xscale = 1.0f;
  float yscale = 1.0f;
  if (pMatrix) {
    font_matrix_ = pMatrix->GetMatrix();
    xscale = font_matrix_.a;
    yscale = font_matrix_.d;
  }

  RetainPtr<const CPDF_Array> pBBox = font_dict_->GetArrayFor("FontBBox");
  if (pBBox) {
    CFX_FloatRect box(
        pBBox->GetFloatAt(0) * xscale, pBBox->GetFloatAt(1) * yscale,
        pBBox->GetFloatAt(2) * xscale, pBBox->GetFloatAt(3) * yscale);
    CPDF_Type3Char::TextUnitRectToGlyphUnitRect(&box);
    font_bbox_ = box.ToFxRect();
  }

  const size_t kCharLimit = char_width_l_.size();
  int StartChar = font_dict_->GetIntegerFor("FirstChar");
  if (StartChar >= 0 && static_cast<size_t>(StartChar) < kCharLimit) {
    RetainPtr<const CPDF_Array> pWidthArray = font_dict_->GetArrayFor("Widths");
    if (pWidthArray) {
      size_t count = std::min(pWidthArray->size(), kCharLimit);
      count = std::min(count, kCharLimit - StartChar);
      for (size_t i = 0; i < count; i++) {
        char_width_l_[StartChar + i] =
            FXSYS_roundf(CPDF_Type3Char::TextUnitToGlyphUnit(
                pWidthArray->GetFloatAt(i) * xscale));
      }
    }
  }
  char_procs_ = font_dict_->GetMutableDictFor("CharProcs");
  if (font_dict_->GetDirectObjectFor("Encoding")) {
    LoadPDFEncoding(false, false);
  }
  return true;
}

void CPDF_Type3Font::LoadGlyphMap() {}

void CPDF_Type3Font::CheckType3FontMetrics() {
  CheckFontMetrics();
}

CPDF_Type3Char* CPDF_Type3Font::LoadChar(uint32_t charcode) {
  if (char_loading_depth_ >= kMaxType3FormLevel) {
    return nullptr;
  }

  auto it = cache_map_.find(charcode);
  if (it != cache_map_.end()) {
    return it->second.get();
  }

  const char* name = GetAdobeCharName(base_encoding_, char_names_, charcode);
  if (!name) {
    return nullptr;
  }

  if (!char_procs_) {
    return nullptr;
  }

  RetainPtr<CPDF_Stream> pStream =
      ToStream(char_procs_->GetMutableDirectObjectFor(name));
  if (!pStream) {
    return nullptr;
  }

  std::unique_ptr<CPDF_Font::FormIface> pForm = form_factory_->CreateForm(
      document_, font_resources_ ? font_resources_ : page_resources_, pStream);

  auto pNewChar = std::make_unique<CPDF_Type3Char>();

  // This can trigger recursion into this method. The content of |cache_map_|
  // can change as a result. Thus after it returns, check the cache again for
  // a cache hit.
  {
    AutoRestorer<int> restorer(&char_loading_depth_);
    char_loading_depth_++;
    pForm->ParseContentForType3Char(pNewChar.get());
  }
  it = cache_map_.find(charcode);
  if (it != cache_map_.end()) {
    return it->second.get();
  }

  pNewChar->Transform(pForm.get(), font_matrix_);
  if (pForm->HasPageObjects()) {
    pNewChar->SetForm(std::move(pForm));
  }

  CPDF_Type3Char* pCachedChar = pNewChar.get();
  cache_map_[charcode] = std::move(pNewChar);
  return pCachedChar;
}

int CPDF_Type3Font::GetCharWidthF(uint32_t charcode) {
  if (charcode >= std::size(char_width_l_)) {
    charcode = 0;
  }

  if (char_width_l_[charcode]) {
    return char_width_l_[charcode];
  }

  const CPDF_Type3Char* pChar = LoadChar(charcode);
  return pChar ? pChar->width() : 0;
}

FX_RECT CPDF_Type3Font::GetCharBBox(uint32_t charcode) {
  FX_RECT ret;
  const CPDF_Type3Char* pChar = LoadChar(charcode);
  if (pChar) {
    ret = pChar->bbox();
  }
  return ret;
}
