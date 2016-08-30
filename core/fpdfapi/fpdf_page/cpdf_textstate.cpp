// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_textstate.h"

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

CPDF_TextState::CPDF_TextState() {}
CPDF_TextState::~CPDF_TextState() {}

void CPDF_TextState::Emplace() {
  m_Ref.Emplace();
}

CPDF_Font* CPDF_TextState::GetFont() const {
  return m_Ref.GetObject()->m_pFont;
}

void CPDF_TextState::SetFont(CPDF_Font* pFont) {
  m_Ref.GetPrivateCopy()->SetFont(pFont);
}

FX_FLOAT CPDF_TextState::GetFontSize() const {
  return m_Ref.GetObject()->m_FontSize;
}

void CPDF_TextState::SetFontSize(FX_FLOAT size) {
  m_Ref.GetPrivateCopy()->m_FontSize = size;
}

const FX_FLOAT* CPDF_TextState::GetMatrix() const {
  return m_Ref.GetObject()->m_Matrix;
}

FX_FLOAT* CPDF_TextState::GetMutableMatrix() {
  return m_Ref.GetPrivateCopy()->m_Matrix;
}

FX_FLOAT CPDF_TextState::GetCharSpace() const {
  return m_Ref.GetObject()->m_CharSpace;
}

void CPDF_TextState::SetCharSpace(FX_FLOAT sp) {
  m_Ref.GetPrivateCopy()->m_CharSpace = sp;
}

FX_FLOAT CPDF_TextState::GetWordSpace() const {
  return m_Ref.GetObject()->m_WordSpace;
}

void CPDF_TextState::SetWordSpace(FX_FLOAT sp) {
  m_Ref.GetPrivateCopy()->m_WordSpace = sp;
}

FX_FLOAT CPDF_TextState::GetFontSizeV() const {
  return m_Ref.GetObject()->GetFontSizeV();
}

FX_FLOAT CPDF_TextState::GetFontSizeH() const {
  return m_Ref.GetObject()->GetFontSizeH();
}

FX_FLOAT CPDF_TextState::GetBaselineAngle() const {
  return m_Ref.GetObject()->GetBaselineAngle();
}

FX_FLOAT CPDF_TextState::GetShearAngle() const {
  return m_Ref.GetObject()->GetShearAngle();
}

TextRenderingMode CPDF_TextState::GetTextMode() const {
  return m_Ref.GetObject()->m_TextMode;
}

void CPDF_TextState::SetTextMode(TextRenderingMode mode) {
  m_Ref.GetPrivateCopy()->m_TextMode = mode;
}

const FX_FLOAT* CPDF_TextState::GetCTM() const {
  return m_Ref.GetObject()->m_CTM;
}

FX_FLOAT* CPDF_TextState::GetMutableCTM() {
  return m_Ref.GetPrivateCopy()->m_CTM;
}
