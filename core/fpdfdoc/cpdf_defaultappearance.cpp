// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_defaultappearance.h"

#include <algorithm>
#include <vector>

#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxge/cfx_color.h"
#include "third_party/base/notreached.h"

namespace {

// Find the token and its |nParams| parameters from the start of data,
// and move the current position to the start of those parameters.
bool FindTagParamFromStart(CPDF_SimpleParser* parser,
                           ByteStringView token,
                           int nParams) {
  nParams++;

  std::vector<uint32_t> pBuf(nParams);
  int buf_index = 0;
  int buf_count = 0;

  parser->SetCurPos(0);
  while (true) {
    pBuf[buf_index++] = parser->GetCurPos();
    if (buf_index == nParams)
      buf_index = 0;

    buf_count++;
    if (buf_count > nParams)
      buf_count = nParams;

    ByteStringView word = parser->GetWord();
    if (word.IsEmpty())
      return false;

    if (word == token) {
      if (buf_count < nParams)
        continue;

      parser->SetCurPos(pBuf[buf_index]);
      return true;
    }
  }
}

}  // namespace

CPDF_DefaultAppearance::CPDF_DefaultAppearance() = default;

CPDF_DefaultAppearance::CPDF_DefaultAppearance(const ByteString& csDA)
    : m_csDA(csDA) {}

CPDF_DefaultAppearance::CPDF_DefaultAppearance(
    const CPDF_DefaultAppearance& cDA) = default;

CPDF_DefaultAppearance::~CPDF_DefaultAppearance() = default;

absl::optional<ByteString> CPDF_DefaultAppearance::GetFont(
    float* fFontSize) const {
  *fFontSize = 0.0f;
  if (m_csDA.IsEmpty())
    return absl::nullopt;

  ByteString csFontNameTag;
  CPDF_SimpleParser syntax(m_csDA.AsStringView().raw_span());
  if (FindTagParamFromStart(&syntax, "Tf", 2)) {
    csFontNameTag = ByteString(syntax.GetWord());
    csFontNameTag.Delete(0, 1);
    *fFontSize = StringToFloat(syntax.GetWord());
  }
  return PDF_NameDecode(csFontNameTag.AsStringView());
}

absl::optional<CFX_Color> CPDF_DefaultAppearance::GetColor() const {
  if (m_csDA.IsEmpty())
    return absl::nullopt;

  float fc[4];
  CPDF_SimpleParser syntax(m_csDA.AsStringView().raw_span());
  if (FindTagParamFromStart(&syntax, "g", 1)) {
    fc[0] = StringToFloat(syntax.GetWord());
    return CFX_Color(CFX_Color::Type::kGray, fc[0]);
  }
  if (FindTagParamFromStart(&syntax, "rg", 3)) {
    fc[0] = StringToFloat(syntax.GetWord());
    fc[1] = StringToFloat(syntax.GetWord());
    fc[2] = StringToFloat(syntax.GetWord());
    return CFX_Color(CFX_Color::Type::kRGB, fc[0], fc[1], fc[2]);
  }
  if (FindTagParamFromStart(&syntax, "k", 4)) {
    fc[0] = StringToFloat(syntax.GetWord());
    fc[1] = StringToFloat(syntax.GetWord());
    fc[2] = StringToFloat(syntax.GetWord());
    fc[3] = StringToFloat(syntax.GetWord());
    return CFX_Color(CFX_Color::Type::kCMYK, fc[0], fc[1], fc[2], fc[3]);
  }
  return absl::nullopt;
}

absl::optional<CFX_Color::TypeAndARGB> CPDF_DefaultAppearance::GetColorARGB()
    const {
  absl::optional<CFX_Color> maybe_color = GetColor();
  if (!maybe_color.has_value())
    return absl::nullopt;

  const CFX_Color& color = maybe_color.value();
  if (color.nColorType == CFX_Color::Type::kGray) {
    int g = static_cast<int>(color.fColor1 * 255 + 0.5f);
    return CFX_Color::TypeAndARGB(CFX_Color::Type::kGray,
                                  ArgbEncode(255, g, g, g));
  }
  if (color.nColorType == CFX_Color::Type::kRGB) {
    int r = static_cast<int>(color.fColor1 * 255 + 0.5f);
    int g = static_cast<int>(color.fColor2 * 255 + 0.5f);
    int b = static_cast<int>(color.fColor3 * 255 + 0.5f);
    return CFX_Color::TypeAndARGB(CFX_Color::Type::kRGB,
                                  ArgbEncode(255, r, g, b));
  }
  if (color.nColorType == CFX_Color::Type::kCMYK) {
    float r = 1.0f - std::min(1.0f, color.fColor1 + color.fColor4);
    float g = 1.0f - std::min(1.0f, color.fColor2 + color.fColor4);
    float b = 1.0f - std::min(1.0f, color.fColor3 + color.fColor4);
    return CFX_Color::TypeAndARGB(
        CFX_Color::Type::kCMYK,
        ArgbEncode(255, static_cast<int>(r * 255 + 0.5f),
                   static_cast<int>(g * 255 + 0.5f),
                   static_cast<int>(b * 255 + 0.5f)));
  }
  NOTREACHED();
  return absl::nullopt;
}

// static
bool CPDF_DefaultAppearance::FindTagParamFromStartForTesting(
    CPDF_SimpleParser* parser,
    ByteStringView token,
    int nParams) {
  return FindTagParamFromStart(parser, token, nParams);
}
