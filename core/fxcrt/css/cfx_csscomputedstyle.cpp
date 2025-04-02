// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_csscomputedstyle.h"

#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/css/cfx_cssstringvalue.h"
#include "core/fxcrt/css/cfx_cssvaluelist.h"

CFX_CSSComputedStyle::CFX_CSSComputedStyle() = default;

CFX_CSSComputedStyle::~CFX_CSSComputedStyle() = default;

bool CFX_CSSComputedStyle::GetCustomStyle(const WideString& wsName,
                                          WideString* pValue) const {
  for (const auto& prop : pdfium::Reversed(custom_properties_)) {
    if (wsName == prop.name()) {
      *pValue = prop.value();
      return true;
    }
  }
  return false;
}

std::optional<WideString> CFX_CSSComputedStyle::GetLastFontFamily() const {
  if (!inherited_data_.font_family_ ||
      inherited_data_.font_family_->values().empty()) {
    return std::nullopt;
  }

  return inherited_data_.font_family_->values()
      .back()
      .AsRaw<CFX_CSSStringValue>()
      ->Value();
}

uint16_t CFX_CSSComputedStyle::GetFontWeight() const {
  return inherited_data_.wfont_weight_;
}

CFX_CSSFontVariant CFX_CSSComputedStyle::GetFontVariant() const {
  return inherited_data_.font_variant_;
}

CFX_CSSFontStyle CFX_CSSComputedStyle::GetFontStyle() const {
  return inherited_data_.font_style_;
}

float CFX_CSSComputedStyle::GetFontSize() const {
  return inherited_data_.ffont_size_;
}

FX_ARGB CFX_CSSComputedStyle::GetColor() const {
  return inherited_data_.font_color_;
}

void CFX_CSSComputedStyle::SetFontWeight(uint16_t wFontWeight) {
  inherited_data_.wfont_weight_ = wFontWeight;
}

void CFX_CSSComputedStyle::SetFontVariant(CFX_CSSFontVariant eFontVariant) {
  inherited_data_.font_variant_ = eFontVariant;
}

void CFX_CSSComputedStyle::SetFontStyle(CFX_CSSFontStyle eFontStyle) {
  inherited_data_.font_style_ = eFontStyle;
}

void CFX_CSSComputedStyle::SetFontSize(float fFontSize) {
  inherited_data_.ffont_size_ = fFontSize;
}

void CFX_CSSComputedStyle::SetColor(FX_ARGB dwFontColor) {
  inherited_data_.font_color_ = dwFontColor;
}

const CFX_CSSRect* CFX_CSSComputedStyle::GetBorderWidth() const {
  return non_inherited_data_.has_border_ ? &(non_inherited_data_.border_width_)
                                         : nullptr;
}

const CFX_CSSRect* CFX_CSSComputedStyle::GetMarginWidth() const {
  return non_inherited_data_.has_margin_ ? &(non_inherited_data_.margin_width_)
                                         : nullptr;
}

const CFX_CSSRect* CFX_CSSComputedStyle::GetPaddingWidth() const {
  return non_inherited_data_.has_padding_
             ? &(non_inherited_data_.padding_width_)
             : nullptr;
}

void CFX_CSSComputedStyle::SetMarginWidth(const CFX_CSSRect& rect) {
  non_inherited_data_.margin_width_ = rect;
  non_inherited_data_.has_margin_ = true;
}

void CFX_CSSComputedStyle::SetPaddingWidth(const CFX_CSSRect& rect) {
  non_inherited_data_.padding_width_ = rect;
  non_inherited_data_.has_padding_ = true;
}

CFX_CSSDisplay CFX_CSSComputedStyle::GetDisplay() const {
  return non_inherited_data_.display_;
}

float CFX_CSSComputedStyle::GetLineHeight() const {
  return inherited_data_.fline_height_;
}

const CFX_CSSLength& CFX_CSSComputedStyle::GetTextIndent() const {
  return inherited_data_.text_indent_;
}

CFX_CSSTextAlign CFX_CSSComputedStyle::GetTextAlign() const {
  return inherited_data_.text_align_;
}

CFX_CSSVerticalAlign CFX_CSSComputedStyle::GetVerticalAlign() const {
  return non_inherited_data_.vertical_align_type_;
}

float CFX_CSSComputedStyle::GetNumberVerticalAlign() const {
  return non_inherited_data_.vertical_align_;
}

Mask<CFX_CSSTEXTDECORATION> CFX_CSSComputedStyle::GetTextDecoration() const {
  return non_inherited_data_.text_decoration_;
}

const CFX_CSSLength& CFX_CSSComputedStyle::GetLetterSpacing() const {
  return inherited_data_.letter_spacing_;
}

void CFX_CSSComputedStyle::SetLineHeight(float fLineHeight) {
  inherited_data_.fline_height_ = fLineHeight;
}

void CFX_CSSComputedStyle::SetTextIndent(const CFX_CSSLength& textIndent) {
  inherited_data_.text_indent_ = textIndent;
}

void CFX_CSSComputedStyle::SetTextAlign(CFX_CSSTextAlign eTextAlign) {
  inherited_data_.text_align_ = eTextAlign;
}

void CFX_CSSComputedStyle::SetNumberVerticalAlign(float fAlign) {
  non_inherited_data_.vertical_align_type_ = CFX_CSSVerticalAlign::Number,
  non_inherited_data_.vertical_align_ = fAlign;
}

void CFX_CSSComputedStyle::SetTextDecoration(
    Mask<CFX_CSSTEXTDECORATION> dwTextDecoration) {
  non_inherited_data_.text_decoration_ = dwTextDecoration;
}

void CFX_CSSComputedStyle::SetLetterSpacing(
    const CFX_CSSLength& letterSpacing) {
  inherited_data_.letter_spacing_ = letterSpacing;
}

void CFX_CSSComputedStyle::AddCustomStyle(const CFX_CSSCustomProperty& prop) {
  // Force the property to be copied so we aren't dependent on the lifetime
  // of whatever currently owns it.
  custom_properties_.push_back(prop);
}

CFX_CSSComputedStyle::InheritedData::InheritedData() = default;

CFX_CSSComputedStyle::InheritedData::~InheritedData() = default;

CFX_CSSComputedStyle::NonInheritedData::NonInheritedData() = default;
