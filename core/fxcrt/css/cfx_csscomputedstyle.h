// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSCOMPUTEDSTYLE_H_
#define CORE_FXCRT_CSS_CFX_CSSCOMPUTEDSTYLE_H_

#include <optional>
#include <vector>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_csscustomproperty.h"
#include "core/fxcrt/mask.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_CSSValueList;

class CFX_CSSComputedStyle final : public Retainable {
 public:
  class InheritedData {
   public:
    InheritedData();
    ~InheritedData();

    CFX_CSSLength letter_spacing_{CFX_CSSLengthUnit::Normal, 0};
    CFX_CSSLength word_spacing_{CFX_CSSLengthUnit::Normal, 0};
    CFX_CSSLength text_indent_{CFX_CSSLengthUnit::Point, 0};
    RetainPtr<CFX_CSSValueList> font_family_;
    float ffont_size_ = 12.0f;
    float fline_height_ = 14.0f;
    FX_ARGB font_color_ = 0xFF000000;
    uint16_t wfont_weight_ = 400;
    CFX_CSSFontVariant font_variant_ = CFX_CSSFontVariant::Normal;
    CFX_CSSFontStyle font_style_ = CFX_CSSFontStyle::Normal;
    CFX_CSSTextAlign text_align_ = CFX_CSSTextAlign::Left;
  };

  class NonInheritedData {
   public:
    NonInheritedData();

    CFX_CSSRect margin_width_{CFX_CSSLengthUnit::Point, 0};
    CFX_CSSRect border_width_{CFX_CSSLengthUnit::Point, 0};
    CFX_CSSRect padding_width_{CFX_CSSLengthUnit::Point, 0};
    CFX_CSSLength top_;
    CFX_CSSLength bottom_;
    CFX_CSSLength left_;
    CFX_CSSLength right_;
    float vertical_align_ = 0.0f;
    CFX_CSSDisplay display_ = CFX_CSSDisplay::Inline;
    CFX_CSSVerticalAlign vertical_align_type_ = CFX_CSSVerticalAlign::Baseline;
    Mask<CFX_CSSTEXTDECORATION> text_decoration_;
    bool has_margin_ = false;
    bool has_border_ = false;
    bool has_padding_ = false;
  };

  CONSTRUCT_VIA_MAKE_RETAIN;

  std::optional<WideString> GetLastFontFamily() const;
  uint16_t GetFontWeight() const;
  CFX_CSSFontVariant GetFontVariant() const;
  CFX_CSSFontStyle GetFontStyle() const;
  float GetFontSize() const;
  FX_ARGB GetColor() const;
  void SetFontWeight(uint16_t wFontWeight);
  void SetFontVariant(CFX_CSSFontVariant eFontVariant);
  void SetFontStyle(CFX_CSSFontStyle eFontStyle);
  void SetFontSize(float fFontSize);
  void SetColor(FX_ARGB dwFontColor);

  const CFX_CSSRect* GetBorderWidth() const;
  const CFX_CSSRect* GetMarginWidth() const;
  const CFX_CSSRect* GetPaddingWidth() const;
  void SetMarginWidth(const CFX_CSSRect& rect);
  void SetPaddingWidth(const CFX_CSSRect& rect);

  CFX_CSSDisplay GetDisplay() const;

  float GetLineHeight() const;
  const CFX_CSSLength& GetTextIndent() const;
  CFX_CSSTextAlign GetTextAlign() const;
  CFX_CSSVerticalAlign GetVerticalAlign() const;
  float GetNumberVerticalAlign() const;
  Mask<CFX_CSSTEXTDECORATION> GetTextDecoration() const;
  const CFX_CSSLength& GetLetterSpacing() const;
  void SetLineHeight(float fLineHeight);
  void SetTextIndent(const CFX_CSSLength& textIndent);
  void SetTextAlign(CFX_CSSTextAlign eTextAlign);
  void SetNumberVerticalAlign(float fAlign);
  void SetTextDecoration(Mask<CFX_CSSTEXTDECORATION> dwTextDecoration);
  void SetLetterSpacing(const CFX_CSSLength& letterSpacing);
  void AddCustomStyle(const CFX_CSSCustomProperty& prop);

  bool GetCustomStyle(const WideString& wsName, WideString* pValue) const;

  InheritedData inherited_data_;
  NonInheritedData non_inherited_data_;

 private:
  CFX_CSSComputedStyle();
  ~CFX_CSSComputedStyle() override;

  std::vector<CFX_CSSCustomProperty> custom_properties_;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSCOMPUTEDSTYLE_H_
