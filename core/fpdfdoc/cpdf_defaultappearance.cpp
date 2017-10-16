// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_defaultappearance.h"

#include <algorithm>

#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxge/cfx_color.h"

bool CPDF_DefaultAppearance::HasFont() {
  if (m_csDA.IsEmpty())
    return false;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  return syntax.FindTagParamFromStart("Tf", 2);
}

ByteString CPDF_DefaultAppearance::GetFontString() {
  ByteString csFont;
  if (m_csDA.IsEmpty())
    return csFont;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart("Tf", 2)) {
    csFont += syntax.GetWord();
    csFont += " ";
    csFont += syntax.GetWord();
    csFont += " ";
    csFont += syntax.GetWord();
  }
  return csFont;
}

ByteString CPDF_DefaultAppearance::GetFont(float* fFontSize) {
  *fFontSize = 0.0f;
  if (m_csDA.IsEmpty())
    return ByteString();

  ByteString csFontNameTag;
  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart("Tf", 2)) {
    csFontNameTag = ByteString(syntax.GetWord());
    csFontNameTag.Delete(0, 1);
    *fFontSize = FX_atof(syntax.GetWord());
  }
  return PDF_NameDecode(csFontNameTag);
}

bool CPDF_DefaultAppearance::HasColor(PaintOperation nOperation) {
  if (m_csDA.IsEmpty())
    return false;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "G" : "g"), 1)) {
    return true;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "RG" : "rg"), 3)) {
    return true;
  }
  return syntax.FindTagParamFromStart(
      (nOperation == PaintOperation::STROKE ? "K" : "k"), 4);
}

ByteString CPDF_DefaultAppearance::GetColorString(PaintOperation nOperation) {
  ByteString csColor;
  if (m_csDA.IsEmpty())
    return csColor;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "G" : "g"), 1)) {
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    return csColor;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "RG" : "rg"), 3)) {
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    return csColor;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "K" : "k"), 4)) {
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
    csColor += " ";
    csColor += syntax.GetWord();
  }
  return csColor;
}

void CPDF_DefaultAppearance::GetColor(int& iColorType,
                                      float fc[4],
                                      PaintOperation nOperation) {
  iColorType = CFX_Color::kTransparent;
  for (int c = 0; c < 4; c++)
    fc[c] = 0;

  if (m_csDA.IsEmpty())
    return;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "G" : "g"), 1)) {
    iColorType = CFX_Color::kGray;
    fc[0] = FX_atof(syntax.GetWord());
    return;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "RG" : "rg"), 3)) {
    iColorType = CFX_Color::kRGB;
    fc[0] = FX_atof(syntax.GetWord());
    fc[1] = FX_atof(syntax.GetWord());
    fc[2] = FX_atof(syntax.GetWord());
    return;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "K" : "k"), 4)) {
    iColorType = CFX_Color::kCMYK;
    fc[0] = FX_atof(syntax.GetWord());
    fc[1] = FX_atof(syntax.GetWord());
    fc[2] = FX_atof(syntax.GetWord());
    fc[3] = FX_atof(syntax.GetWord());
  }
}

void CPDF_DefaultAppearance::GetColor(FX_ARGB& color,
                                      int& iColorType,
                                      PaintOperation nOperation) {
  color = 0;
  iColorType = CFX_Color::kTransparent;
  if (m_csDA.IsEmpty())
    return;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "G" : "g"), 1)) {
    iColorType = CFX_Color::kGray;
    float g = FX_atof(syntax.GetWord()) * 255 + 0.5f;
    color = ArgbEncode(255, (int)g, (int)g, (int)g);
    return;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "RG" : "rg"), 3)) {
    iColorType = CFX_Color::kRGB;
    float r = FX_atof(syntax.GetWord()) * 255 + 0.5f;
    float g = FX_atof(syntax.GetWord()) * 255 + 0.5f;
    float b = FX_atof(syntax.GetWord()) * 255 + 0.5f;
    color = ArgbEncode(255, (int)r, (int)g, (int)b);
    return;
  }
  if (syntax.FindTagParamFromStart(
          (nOperation == PaintOperation::STROKE ? "K" : "k"), 4)) {
    iColorType = CFX_Color::kCMYK;
    float c = FX_atof(syntax.GetWord());
    float m = FX_atof(syntax.GetWord());
    float y = FX_atof(syntax.GetWord());
    float k = FX_atof(syntax.GetWord());
    float r = 1.0f - std::min(1.0f, c + k);
    float g = 1.0f - std::min(1.0f, m + k);
    float b = 1.0f - std::min(1.0f, y + k);
    color = ArgbEncode(255, (int)(r * 255 + 0.5f), (int)(g * 255 + 0.5f),
                       (int)(b * 255 + 0.5f));
  }
}

bool CPDF_DefaultAppearance::HasTextMatrix() {
  if (m_csDA.IsEmpty())
    return false;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  return syntax.FindTagParamFromStart("Tm", 6);
}

ByteString CPDF_DefaultAppearance::GetTextMatrixString() {
  ByteString csTM;
  if (m_csDA.IsEmpty())
    return csTM;

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (syntax.FindTagParamFromStart("Tm", 6)) {
    for (int i = 0; i < 6; i++) {
      csTM += syntax.GetWord();
      csTM += " ";
    }
    csTM += syntax.GetWord();
  }
  return csTM;
}

CFX_Matrix CPDF_DefaultAppearance::GetTextMatrix() {
  if (m_csDA.IsEmpty())
    return CFX_Matrix();

  CPDF_SimpleParser syntax(m_csDA.AsStringView());
  if (!syntax.FindTagParamFromStart("Tm", 6))
    return CFX_Matrix();

  float f[6];
  for (int i = 0; i < 6; i++)
    f[i] = FX_atof(syntax.GetWord());
  return CFX_Matrix(f);
}
