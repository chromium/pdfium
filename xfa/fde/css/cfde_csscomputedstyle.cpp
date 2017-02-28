// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csscomputedstyle.h"

#include "third_party/base/stl_util.h"
#include "xfa/fde/css/cfde_cssstringvalue.h"
#include "xfa/fde/css/cfde_cssvaluelist.h"

CFDE_CSSComputedStyle::CFDE_CSSComputedStyle() {}

CFDE_CSSComputedStyle::~CFDE_CSSComputedStyle() {}

bool CFDE_CSSComputedStyle::GetCustomStyle(const CFX_WideString& wsName,
                                           CFX_WideString& wsValue) const {
  for (auto iter = m_CustomProperties.rbegin();
       iter != m_CustomProperties.rend(); iter++) {
    if (wsName == iter->name()) {
      wsValue = iter->value();
      return true;
    }
  }
  return false;
}

int32_t CFDE_CSSComputedStyle::CountFontFamilies() const {
  return m_InheritedData.m_pFontFamily
             ? m_InheritedData.m_pFontFamily->CountValues()
             : 0;
}

const CFX_WideString CFDE_CSSComputedStyle::GetFontFamily(int32_t index) const {
  return m_InheritedData.m_pFontFamily->GetValue(index)
      .As<CFDE_CSSStringValue>()
      ->Value();
}

uint16_t CFDE_CSSComputedStyle::GetFontWeight() const {
  return m_InheritedData.m_wFontWeight;
}

FDE_CSSFontVariant CFDE_CSSComputedStyle::GetFontVariant() const {
  return m_InheritedData.m_eFontVariant;
}

FDE_CSSFontStyle CFDE_CSSComputedStyle::GetFontStyle() const {
  return m_InheritedData.m_eFontStyle;
}

FX_FLOAT CFDE_CSSComputedStyle::GetFontSize() const {
  return m_InheritedData.m_fFontSize;
}

FX_ARGB CFDE_CSSComputedStyle::GetColor() const {
  return m_InheritedData.m_dwFontColor;
}

void CFDE_CSSComputedStyle::SetFontWeight(uint16_t wFontWeight) {
  m_InheritedData.m_wFontWeight = wFontWeight;
}

void CFDE_CSSComputedStyle::SetFontVariant(FDE_CSSFontVariant eFontVariant) {
  m_InheritedData.m_eFontVariant = eFontVariant;
}

void CFDE_CSSComputedStyle::SetFontStyle(FDE_CSSFontStyle eFontStyle) {
  m_InheritedData.m_eFontStyle = eFontStyle;
}

void CFDE_CSSComputedStyle::SetFontSize(FX_FLOAT fFontSize) {
  m_InheritedData.m_fFontSize = fFontSize;
}

void CFDE_CSSComputedStyle::SetColor(FX_ARGB dwFontColor) {
  m_InheritedData.m_dwFontColor = dwFontColor;
}

const FDE_CSSRect* CFDE_CSSComputedStyle::GetBorderWidth() const {
  return m_NonInheritedData.m_bHasBorder ? &(m_NonInheritedData.m_BorderWidth)
                                         : nullptr;
}

const FDE_CSSRect* CFDE_CSSComputedStyle::GetMarginWidth() const {
  return m_NonInheritedData.m_bHasMargin ? &(m_NonInheritedData.m_MarginWidth)
                                         : nullptr;
}

const FDE_CSSRect* CFDE_CSSComputedStyle::GetPaddingWidth() const {
  return m_NonInheritedData.m_bHasPadding ? &(m_NonInheritedData.m_PaddingWidth)
                                          : nullptr;
}

void CFDE_CSSComputedStyle::SetMarginWidth(const FDE_CSSRect& rect) {
  m_NonInheritedData.m_MarginWidth = rect;
  m_NonInheritedData.m_bHasMargin = true;
}

void CFDE_CSSComputedStyle::SetPaddingWidth(const FDE_CSSRect& rect) {
  m_NonInheritedData.m_PaddingWidth = rect;
  m_NonInheritedData.m_bHasPadding = true;
}

FDE_CSSDisplay CFDE_CSSComputedStyle::GetDisplay() const {
  return m_NonInheritedData.m_eDisplay;
}

FX_FLOAT CFDE_CSSComputedStyle::GetLineHeight() const {
  return m_InheritedData.m_fLineHeight;
}

const FDE_CSSLength& CFDE_CSSComputedStyle::GetTextIndent() const {
  return m_InheritedData.m_TextIndent;
}

FDE_CSSTextAlign CFDE_CSSComputedStyle::GetTextAlign() const {
  return m_InheritedData.m_eTextAlign;
}

FDE_CSSVerticalAlign CFDE_CSSComputedStyle::GetVerticalAlign() const {
  return m_NonInheritedData.m_eVerticalAlign;
}

FX_FLOAT CFDE_CSSComputedStyle::GetNumberVerticalAlign() const {
  return m_NonInheritedData.m_fVerticalAlign;
}

uint32_t CFDE_CSSComputedStyle::GetTextDecoration() const {
  return m_NonInheritedData.m_dwTextDecoration;
}

const FDE_CSSLength& CFDE_CSSComputedStyle::GetLetterSpacing() const {
  return m_InheritedData.m_LetterSpacing;
}

void CFDE_CSSComputedStyle::SetLineHeight(FX_FLOAT fLineHeight) {
  m_InheritedData.m_fLineHeight = fLineHeight;
}

void CFDE_CSSComputedStyle::SetTextIndent(const FDE_CSSLength& textIndent) {
  m_InheritedData.m_TextIndent = textIndent;
}

void CFDE_CSSComputedStyle::SetTextAlign(FDE_CSSTextAlign eTextAlign) {
  m_InheritedData.m_eTextAlign = eTextAlign;
}

void CFDE_CSSComputedStyle::SetNumberVerticalAlign(FX_FLOAT fAlign) {
  m_NonInheritedData.m_eVerticalAlign = FDE_CSSVerticalAlign::Number,
  m_NonInheritedData.m_fVerticalAlign = fAlign;
}

void CFDE_CSSComputedStyle::SetTextDecoration(uint32_t dwTextDecoration) {
  m_NonInheritedData.m_dwTextDecoration = dwTextDecoration;
}

void CFDE_CSSComputedStyle::SetLetterSpacing(
    const FDE_CSSLength& letterSpacing) {
  m_InheritedData.m_LetterSpacing = letterSpacing;
}

void CFDE_CSSComputedStyle::AddCustomStyle(const CFDE_CSSCustomProperty& prop) {
  // Force the property to be copied so we aren't dependent on the lifetime
  // of whatever currently owns it.
  m_CustomProperties.push_back(prop);
}

CFDE_CSSComputedStyle::InheritedData::InheritedData()
    : m_LetterSpacing(FDE_CSSLengthUnit::Normal),
      m_WordSpacing(FDE_CSSLengthUnit::Normal),
      m_TextIndent(FDE_CSSLengthUnit::Point, 0),
      m_pFontFamily(nullptr),
      m_fFontSize(12.0f),
      m_fLineHeight(14.0f),
      m_dwFontColor(0xFF000000),
      m_wFontWeight(400),
      m_eFontVariant(FDE_CSSFontVariant::Normal),
      m_eFontStyle(FDE_CSSFontStyle::Normal),
      m_eTextAlign(FDE_CSSTextAlign::Left) {}

CFDE_CSSComputedStyle::InheritedData::~InheritedData() {}

CFDE_CSSComputedStyle::NonInheritedData::NonInheritedData()
    : m_MarginWidth(FDE_CSSLengthUnit::Point, 0),
      m_BorderWidth(FDE_CSSLengthUnit::Point, 0),
      m_PaddingWidth(FDE_CSSLengthUnit::Point, 0),
      m_fVerticalAlign(0.0f),
      m_eDisplay(FDE_CSSDisplay::Inline),
      m_eVerticalAlign(FDE_CSSVerticalAlign::Baseline),
      m_dwTextDecoration(0),
      m_bHasMargin(false),
      m_bHasBorder(false),
      m_bHasPadding(false) {}
