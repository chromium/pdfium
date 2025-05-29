// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_apsettings.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"

CPDF_ApSettings::CPDF_ApSettings(RetainPtr<CPDF_Dictionary> dict)
    : dict_(std::move(dict)) {}

CPDF_ApSettings::CPDF_ApSettings(const CPDF_ApSettings& that) = default;

CPDF_ApSettings::~CPDF_ApSettings() = default;

bool CPDF_ApSettings::HasMKEntry(ByteStringView entry) const {
  return dict_ && dict_->KeyExist(entry);
}

int CPDF_ApSettings::GetRotation() const {
  return dict_ ? dict_->GetIntegerFor("R") : 0;
}

CFX_Color::TypeAndARGB CPDF_ApSettings::GetColorARGB(
    ByteStringView entry) const {
  if (!dict_) {
    return {CFX_Color::Type::kTransparent, 0};
  }

  RetainPtr<const CPDF_Array> pEntry = dict_->GetArrayFor(entry);
  if (!pEntry) {
    return {CFX_Color::Type::kTransparent, 0};
  }

  const size_t dwCount = pEntry->size();
  if (dwCount == 1) {
    const float g = pEntry->GetFloatAt(0) * 255;
    return {CFX_Color::Type::kGray, ArgbEncode(255, (int)g, (int)g, (int)g)};
  }
  if (dwCount == 3) {
    float r = pEntry->GetFloatAt(0) * 255;
    float g = pEntry->GetFloatAt(1) * 255;
    float b = pEntry->GetFloatAt(2) * 255;
    return {CFX_Color::Type::kRGB, ArgbEncode(255, (int)r, (int)g, (int)b)};
  }
  if (dwCount == 4) {
    float c = pEntry->GetFloatAt(0);
    float m = pEntry->GetFloatAt(1);
    float y = pEntry->GetFloatAt(2);
    float k = pEntry->GetFloatAt(3);
    float r = (1.0f - std::min(1.0f, c + k)) * 255;
    float g = (1.0f - std::min(1.0f, m + k)) * 255;
    float b = (1.0f - std::min(1.0f, y + k)) * 255;
    return {CFX_Color::Type::kCMYK, ArgbEncode(255, (int)r, (int)g, (int)b)};
  }
  return {CFX_Color::Type::kTransparent, 0};
}

float CPDF_ApSettings::GetOriginalColorComponent(int index,
                                                 ByteStringView entry) const {
  if (!dict_) {
    return 0;
  }

  RetainPtr<const CPDF_Array> pEntry = dict_->GetArrayFor(entry);
  return pEntry ? pEntry->GetFloatAt(index) : 0;
}

CFX_Color CPDF_ApSettings::GetOriginalColor(ByteStringView entry) const {
  if (!dict_) {
    return CFX_Color();
  }

  RetainPtr<const CPDF_Array> pEntry = dict_->GetArrayFor(entry);
  if (!pEntry) {
    return CFX_Color();
  }

  size_t dwCount = pEntry->size();
  if (dwCount == 1) {
    return CFX_Color(CFX_Color::Type::kGray, pEntry->GetFloatAt(0));
  }
  if (dwCount == 3) {
    return CFX_Color(CFX_Color::Type::kRGB, pEntry->GetFloatAt(0),
                     pEntry->GetFloatAt(1), pEntry->GetFloatAt(2));
  }
  if (dwCount == 4) {
    return CFX_Color(CFX_Color::Type::kCMYK, pEntry->GetFloatAt(0),
                     pEntry->GetFloatAt(1), pEntry->GetFloatAt(2),
                     pEntry->GetFloatAt(3));
  }
  return CFX_Color();
}

WideString CPDF_ApSettings::GetCaption(ByteStringView entry) const {
  return dict_ ? dict_->GetUnicodeTextFor(entry) : WideString();
}

RetainPtr<CPDF_Stream> CPDF_ApSettings::GetIcon(ByteStringView entry) const {
  return dict_ ? dict_->GetMutableStreamFor(entry) : nullptr;
}

CPDF_IconFit CPDF_ApSettings::GetIconFit() const {
  return CPDF_IconFit(dict_ ? dict_->GetDictFor("IF") : nullptr);
}

int CPDF_ApSettings::GetTextPosition() const {
  return dict_ ? dict_->GetIntegerFor("TP", TEXTPOS_CAPTION) : TEXTPOS_CAPTION;
}
