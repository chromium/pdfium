// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSCOMPUTEDSTYLE_H_
#define XFA_FDE_CSS_CFDE_CSSCOMPUTEDSTYLE_H_

#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/cfde_csscustomproperty.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSValueList;

class CFDE_CSSComputedStyle : public CFX_Retainable {
 public:
  class InheritedData {
   public:
    InheritedData();
    ~InheritedData();

    FDE_CSSLength m_LetterSpacing;
    FDE_CSSLength m_WordSpacing;
    FDE_CSSLength m_TextIndent;
    CFX_RetainPtr<CFDE_CSSValueList> m_pFontFamily;
    FX_FLOAT m_fFontSize;
    FX_FLOAT m_fLineHeight;
    FX_ARGB m_dwFontColor;
    uint16_t m_wFontWeight;
    FDE_CSSFontVariant m_eFontVariant;
    FDE_CSSFontStyle m_eFontStyle;
    FDE_CSSTextAlign m_eTextAlign;
  };

  class NonInheritedData {
   public:
    NonInheritedData();

    FDE_CSSRect m_MarginWidth;
    FDE_CSSRect m_BorderWidth;
    FDE_CSSRect m_PaddingWidth;
    FDE_CSSLength m_Top;
    FDE_CSSLength m_Bottom;
    FDE_CSSLength m_Left;
    FDE_CSSLength m_Right;
    FX_FLOAT m_fVerticalAlign;
    FDE_CSSDisplay m_eDisplay;
    FDE_CSSVerticalAlign m_eVerticalAlign;
    uint8_t m_dwTextDecoration;
    bool m_bHasMargin;
    bool m_bHasBorder;
    bool m_bHasPadding;
  };

  int32_t CountFontFamilies() const;
  const CFX_WideString GetFontFamily(int32_t index) const;
  uint16_t GetFontWeight() const;
  FDE_CSSFontVariant GetFontVariant() const;
  FDE_CSSFontStyle GetFontStyle() const;
  FX_FLOAT GetFontSize() const;
  FX_ARGB GetColor() const;
  void SetFontWeight(uint16_t wFontWeight);
  void SetFontVariant(FDE_CSSFontVariant eFontVariant);
  void SetFontStyle(FDE_CSSFontStyle eFontStyle);
  void SetFontSize(FX_FLOAT fFontSize);
  void SetColor(FX_ARGB dwFontColor);

  const FDE_CSSRect* GetBorderWidth() const;
  const FDE_CSSRect* GetMarginWidth() const;
  const FDE_CSSRect* GetPaddingWidth() const;
  void SetMarginWidth(const FDE_CSSRect& rect);
  void SetPaddingWidth(const FDE_CSSRect& rect);

  FDE_CSSDisplay GetDisplay() const;

  FX_FLOAT GetLineHeight() const;
  const FDE_CSSLength& GetTextIndent() const;
  FDE_CSSTextAlign GetTextAlign() const;
  FDE_CSSVerticalAlign GetVerticalAlign() const;
  FX_FLOAT GetNumberVerticalAlign() const;
  uint32_t GetTextDecoration() const;
  const FDE_CSSLength& GetLetterSpacing() const;
  void SetLineHeight(FX_FLOAT fLineHeight);
  void SetTextIndent(const FDE_CSSLength& textIndent);
  void SetTextAlign(FDE_CSSTextAlign eTextAlign);
  void SetNumberVerticalAlign(FX_FLOAT fAlign);
  void SetTextDecoration(uint32_t dwTextDecoration);
  void SetLetterSpacing(const FDE_CSSLength& letterSpacing);
  void AddCustomStyle(const CFDE_CSSCustomProperty& prop);

  bool GetCustomStyle(const CFX_WideString& wsName,
                      CFX_WideString& wsValue) const;

  InheritedData m_InheritedData;
  NonInheritedData m_NonInheritedData;

 private:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  CFDE_CSSComputedStyle();
  ~CFDE_CSSComputedStyle() override;

  std::vector<CFDE_CSSCustomProperty> m_CustomProperties;
};

#endif  // XFA_FDE_CSS_CFDE_CSSCOMPUTEDSTYLE_H_
