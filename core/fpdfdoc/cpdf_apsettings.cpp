// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_apsettings.h"

#include <algorithm>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"

CPDF_ApSettings::CPDF_ApSettings(CPDF_Dictionary* pDict) : m_pDict(pDict) {}

CPDF_ApSettings::CPDF_ApSettings(const CPDF_ApSettings& that) = default;

CPDF_ApSettings::~CPDF_ApSettings() = default;

bool CPDF_ApSettings::HasMKEntry(const ByteString& csEntry) const {
  return m_pDict && m_pDict->KeyExist(csEntry);
}

int CPDF_ApSettings::GetRotation() const {
  return m_pDict ? m_pDict->GetIntegerFor("R") : 0;
}

CFX_Color::TypeAndARGB CPDF_ApSettings::GetColorARGB(
    const ByteString& csEntry) const {
  if (!m_pDict)
    return {CFX_Color::Type::kTransparent, 0};

  CPDF_Array* pEntry = m_pDict->GetArrayFor(csEntry);
  if (!pEntry)
    return {CFX_Color::Type::kTransparent, 0};

  const size_t dwCount = pEntry->size();
  if (dwCount == 1) {
    const float g = pEntry->GetNumberAt(0) * 255;
    return {CFX_Color::Type::kGray, ArgbEncode(255, (int)g, (int)g, (int)g)};
  }
  if (dwCount == 3) {
    float r = pEntry->GetNumberAt(0) * 255;
    float g = pEntry->GetNumberAt(1) * 255;
    float b = pEntry->GetNumberAt(2) * 255;
    return {CFX_Color::Type::kRGB, ArgbEncode(255, (int)r, (int)g, (int)b)};
  }
  if (dwCount == 4) {
    float c = pEntry->GetNumberAt(0);
    float m = pEntry->GetNumberAt(1);
    float y = pEntry->GetNumberAt(2);
    float k = pEntry->GetNumberAt(3);
    float r = (1.0f - std::min(1.0f, c + k)) * 255;
    float g = (1.0f - std::min(1.0f, m + k)) * 255;
    float b = (1.0f - std::min(1.0f, y + k)) * 255;
    return {CFX_Color::Type::kCMYK, ArgbEncode(255, (int)r, (int)g, (int)b)};
  }
  return {CFX_Color::Type::kTransparent, 0};
}

float CPDF_ApSettings::GetOriginalColorComponent(
    int index,
    const ByteString& csEntry) const {
  if (!m_pDict)
    return 0;

  CPDF_Array* pEntry = m_pDict->GetArrayFor(csEntry);
  return pEntry ? pEntry->GetNumberAt(index) : 0;
}

CFX_Color CPDF_ApSettings::GetOriginalColor(const ByteString& csEntry) const {
  if (!m_pDict)
    return CFX_Color();

  CPDF_Array* pEntry = m_pDict->GetArrayFor(csEntry);
  if (!pEntry)
    return CFX_Color();

  size_t dwCount = pEntry->size();
  if (dwCount == 1) {
    return CFX_Color(CFX_Color::Type::kGray, pEntry->GetNumberAt(0));
  }
  if (dwCount == 3) {
    return CFX_Color(CFX_Color::Type::kRGB, pEntry->GetNumberAt(0),
                     pEntry->GetNumberAt(1), pEntry->GetNumberAt(2));
  }
  if (dwCount == 4) {
    return CFX_Color(CFX_Color::Type::kCMYK, pEntry->GetNumberAt(0),
                     pEntry->GetNumberAt(1), pEntry->GetNumberAt(2),
                     pEntry->GetNumberAt(3));
  }
  return CFX_Color();
}

WideString CPDF_ApSettings::GetCaption(const ByteString& csEntry) const {
  return m_pDict ? m_pDict->GetUnicodeTextFor(csEntry) : WideString();
}

CPDF_Stream* CPDF_ApSettings::GetIcon(const ByteString& csEntry) const {
  return m_pDict ? m_pDict->GetStreamFor(csEntry) : nullptr;
}

CPDF_IconFit CPDF_ApSettings::GetIconFit() const {
  return CPDF_IconFit(m_pDict ? m_pDict->GetDictFor("IF") : nullptr);
}

int CPDF_ApSettings::GetTextPosition() const {
  return m_pDict ? m_pDict->GetIntegerFor("TP", TEXTPOS_CAPTION)
                 : TEXTPOS_CAPTION;
}
