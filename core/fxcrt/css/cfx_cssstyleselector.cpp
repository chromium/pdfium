// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssstyleselector.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/css/cfx_csscolorvalue.h"
#include "core/fxcrt/css/cfx_csscomputedstyle.h"
#include "core/fxcrt/css/cfx_csscustomproperty.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/css/cfx_cssenumvalue.h"
#include "core/fxcrt/css/cfx_csspropertyholder.h"
#include "core/fxcrt/css/cfx_cssselector.h"
#include "core/fxcrt/css/cfx_cssstylesheet.h"
#include "core/fxcrt/css/cfx_csssyntaxparser.h"
#include "core/fxcrt/css/cfx_cssvaluelist.h"

CFX_CSSStyleSelector::CFX_CSSStyleSelector() = default;

CFX_CSSStyleSelector::~CFX_CSSStyleSelector() = default;

void CFX_CSSStyleSelector::SetDefaultFontSize(float fFontSize) {
  DCHECK(fFontSize > 0);
  default_font_size_ = fFontSize;
}

RetainPtr<CFX_CSSComputedStyle> CFX_CSSStyleSelector::CreateComputedStyle(
    const CFX_CSSComputedStyle* pParentStyle) {
  auto pStyle = pdfium::MakeRetain<CFX_CSSComputedStyle>();
  if (pParentStyle) {
    pStyle->inherited_data_ = pParentStyle->inherited_data_;
  }
  return pStyle;
}

void CFX_CSSStyleSelector::SetUAStyleSheet(
    std::unique_ptr<CFX_CSSStyleSheet> pSheet) {
  ua_styles_ = std::move(pSheet);
}

void CFX_CSSStyleSelector::UpdateStyleIndex() {
  ua_rules_.SetRulesFromSheet(ua_styles_.get());
}

std::vector<const CFX_CSSDeclaration*> CFX_CSSStyleSelector::MatchDeclarations(
    const WideString& tagname) {
  std::vector<const CFX_CSSDeclaration*> matchedDecls;
  if (tagname.IsEmpty()) {
    return matchedDecls;
  }

  auto* rules = ua_rules_.GetTagRuleData(tagname);
  if (!rules) {
    return matchedDecls;
  }

  for (const auto& d : *rules) {
    if (MatchSelector(tagname, d->pSelector)) {
      matchedDecls.push_back(d->pDeclaration);
    }
  }
  return matchedDecls;
}

bool CFX_CSSStyleSelector::MatchSelector(const WideString& tagname,
                                         CFX_CSSSelector* pSel) {
  // TODO(dsinclair): The code only supports a single level of selector at this
  // point. None of the code using selectors required the complexity so lets
  // just say we don't support them to simplify the code for now.
  if (!pSel || pSel->next_selector() || pSel->is_descendant()) {
    return false;
  }
  return pSel->name_hash() == FX_HashCode_GetLoweredW(tagname.AsStringView());
}

void CFX_CSSStyleSelector::ComputeStyle(
    const std::vector<const CFX_CSSDeclaration*>& declArray,
    const WideString& styleString,
    const WideString& alignString,
    CFX_CSSComputedStyle* pDest) {
  std::unique_ptr<CFX_CSSDeclaration> pDecl;
  if (!styleString.IsEmpty() || !alignString.IsEmpty()) {
    pDecl = std::make_unique<CFX_CSSDeclaration>();

    if (!styleString.IsEmpty()) {
      AppendInlineStyle(pDecl.get(), styleString);
    }
    if (!alignString.IsEmpty()) {
      pDecl->AddProperty(
          CFX_CSSData::GetPropertyByEnum(CFX_CSSProperty::TextAlign),
          alignString.AsStringView());
    }
  }
  ApplyDeclarations(declArray, pDecl.get(), pDest);
}

void CFX_CSSStyleSelector::ApplyDeclarations(
    const std::vector<const CFX_CSSDeclaration*>& declArray,
    const CFX_CSSDeclaration* extraDecl,
    CFX_CSSComputedStyle* pComputedStyle) {
  std::vector<const CFX_CSSPropertyHolder*> importants;
  std::vector<const CFX_CSSPropertyHolder*> normals;
  std::vector<const CFX_CSSCustomProperty*> customs;

  for (auto* decl : declArray) {
    ExtractValues(decl, &importants, &normals, &customs);
  }

  if (extraDecl) {
    ExtractValues(extraDecl, &importants, &normals, &customs);
  }

  for (auto* prop : normals) {
    ApplyProperty(prop->eProperty, prop->pValue, pComputedStyle);
  }

  for (auto* prop : customs) {
    pComputedStyle->AddCustomStyle(*prop);
  }

  for (auto* prop : importants) {
    ApplyProperty(prop->eProperty, prop->pValue, pComputedStyle);
  }
}

void CFX_CSSStyleSelector::ExtractValues(
    const CFX_CSSDeclaration* decl,
    std::vector<const CFX_CSSPropertyHolder*>* importants,
    std::vector<const CFX_CSSPropertyHolder*>* normals,
    std::vector<const CFX_CSSCustomProperty*>* custom) {
  for (const auto& holder : *decl) {
    if (holder->bImportant) {
      importants->push_back(holder.get());
    } else {
      normals->push_back(holder.get());
    }
  }
  for (auto it = decl->custom_begin(); it != decl->custom_end(); it++) {
    custom->push_back(it->get());
  }
}

void CFX_CSSStyleSelector::AppendInlineStyle(CFX_CSSDeclaration* pDecl,
                                             const WideString& style) {
  DCHECK(pDecl);
  DCHECK(!style.IsEmpty());

  auto pSyntax = std::make_unique<CFX_CSSSyntaxParser>(style.AsStringView());
  pSyntax->SetParseOnlyDeclarations();

  int32_t iLen2 = 0;
  const CFX_CSSData::Property* property = nullptr;
  WideString wsName;
  while (true) {
    CFX_CSSSyntaxParser::Status eStatus = pSyntax->DoSyntaxParse();
    if (eStatus == CFX_CSSSyntaxParser::Status::kPropertyName) {
      WideStringView strValue = pSyntax->GetCurrentString();
      property = CFX_CSSData::GetPropertyByName(strValue);
      if (!property) {
        wsName = WideString(strValue);
      }
    } else if (eStatus == CFX_CSSSyntaxParser::Status::kPropertyValue) {
      if (property || iLen2 > 0) {
        WideStringView strValue = pSyntax->GetCurrentString();
        if (!strValue.IsEmpty()) {
          if (property) {
            pDecl->AddProperty(property, strValue);
          } else if (iLen2 > 0) {
            pDecl->AddProperty(wsName, WideString(strValue));
          }
        }
      }
    } else {
      break;
    }
  }
}

void CFX_CSSStyleSelector::ApplyProperty(CFX_CSSProperty eProperty,
                                         const RetainPtr<CFX_CSSValue>& pValue,
                                         CFX_CSSComputedStyle* pComputedStyle) {
  const CFX_CSSValue::PrimitiveType eType = pValue->GetType();
  if (eType == CFX_CSSValue::PrimitiveType::kList) {
    RetainPtr<CFX_CSSValueList> value_list = pValue.As<CFX_CSSValueList>();
    if (!value_list->values().empty()) {
      switch (eProperty) {
        case CFX_CSSProperty::FontFamily:
          pComputedStyle->inherited_data_.font_family_ = std::move(value_list);
          break;
        case CFX_CSSProperty::TextDecoration:
          pComputedStyle->non_inherited_data_.text_decoration_ =
              ToTextDecoration(value_list);
          break;
        default:
          break;
      }
    }
    return;
  }

  switch (eProperty) {
    case CFX_CSSProperty::Display:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->non_inherited_data_.display_ =
            ToDisplay(pValue.AsRaw<CFX_CSSEnumValue>()->Value());
      }
      break;
    case CFX_CSSProperty::FontSize: {
      float& fFontSize = pComputedStyle->inherited_data_.ffont_size_;
      if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
        fFontSize = pValue.AsRaw<CFX_CSSNumberValue>()->Apply(fFontSize);
      } else if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        fFontSize =
            ToFontSize(pValue.AsRaw<CFX_CSSEnumValue>()->Value(), fFontSize);
      }
    } break;
    case CFX_CSSProperty::LineHeight:
      if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
        RetainPtr<CFX_CSSNumberValue> v = pValue.As<CFX_CSSNumberValue>();
        if (v->unit() == CFX_CSSNumber::Unit::kNumber) {
          pComputedStyle->inherited_data_.fline_height_ =
              v->value() * pComputedStyle->inherited_data_.ffont_size_;
        } else {
          pComputedStyle->inherited_data_.fline_height_ =
              v->Apply(pComputedStyle->inherited_data_.ffont_size_);
        }
      }
      break;
    case CFX_CSSProperty::TextAlign:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->inherited_data_.text_align_ =
            ToTextAlign(pValue.AsRaw<CFX_CSSEnumValue>()->Value());
      }
      break;
    case CFX_CSSProperty::TextIndent:
      SetLengthWithPercent(pComputedStyle->inherited_data_.text_indent_, eType,
                           pValue, pComputedStyle->inherited_data_.ffont_size_);
      break;
    case CFX_CSSProperty::FontWeight:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->inherited_data_.wfont_weight_ =
            ToFontWeight(pValue.AsRaw<CFX_CSSEnumValue>()->Value());
      } else if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
        int32_t iValue =
            static_cast<int32_t>(pValue.AsRaw<CFX_CSSNumberValue>()->value()) /
            100;
        if (iValue >= 1 && iValue <= 9) {
          pComputedStyle->inherited_data_.wfont_weight_ = iValue * 100;
        }
      }
      break;
    case CFX_CSSProperty::FontStyle:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->inherited_data_.font_style_ =
            ToFontStyle(pValue.AsRaw<CFX_CSSEnumValue>()->Value());
      }
      break;
    case CFX_CSSProperty::Color:
      if (eType == CFX_CSSValue::PrimitiveType::kRGB) {
        pComputedStyle->inherited_data_.font_color_ =
            pValue.AsRaw<CFX_CSSColorValue>()->Value();
      }
      break;
    case CFX_CSSProperty::MarginLeft:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.margin_width_.left, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_margin_ = true;
      }
      break;
    case CFX_CSSProperty::MarginTop:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.margin_width_.top, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_margin_ = true;
      }
      break;
    case CFX_CSSProperty::MarginRight:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.margin_width_.right, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_margin_ = true;
      }
      break;
    case CFX_CSSProperty::MarginBottom:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.margin_width_.bottom, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_margin_ = true;
      }
      break;
    case CFX_CSSProperty::PaddingLeft:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.padding_width_.left, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_padding_ = true;
      }
      break;
    case CFX_CSSProperty::PaddingTop:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.padding_width_.top, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_padding_ = true;
      }
      break;
    case CFX_CSSProperty::PaddingRight:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.padding_width_.right, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_padding_ = true;
      }
      break;
    case CFX_CSSProperty::PaddingBottom:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.padding_width_.bottom, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_padding_ = true;
      }
      break;
    case CFX_CSSProperty::BorderLeftWidth:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.border_width_.left, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_border_ = true;
      }
      break;
    case CFX_CSSProperty::BorderTopWidth:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.border_width_.top, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_border_ = true;
      }
      break;
    case CFX_CSSProperty::BorderRightWidth:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.border_width_.right, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_border_ = true;
      }
      break;
    case CFX_CSSProperty::BorderBottomWidth:
      if (SetLengthWithPercent(
              pComputedStyle->non_inherited_data_.border_width_.bottom, eType,
              pValue, pComputedStyle->inherited_data_.ffont_size_)) {
        pComputedStyle->non_inherited_data_.has_border_ = true;
      }
      break;
    case CFX_CSSProperty::VerticalAlign:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->non_inherited_data_.vertical_align_type_ =
            ToVerticalAlign(pValue.AsRaw<CFX_CSSEnumValue>()->Value());
      } else if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
        pComputedStyle->non_inherited_data_.vertical_align_type_ =
            CFX_CSSVerticalAlign::Number;
        pComputedStyle->non_inherited_data_.vertical_align_ =
            pValue.AsRaw<CFX_CSSNumberValue>()->Apply(
                pComputedStyle->inherited_data_.ffont_size_);
      }
      break;
    case CFX_CSSProperty::FontVariant:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->inherited_data_.font_variant_ =
            ToFontVariant(pValue.AsRaw<CFX_CSSEnumValue>()->Value());
      }
      break;
    case CFX_CSSProperty::LetterSpacing:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->inherited_data_.letter_spacing_.Set(
            CFX_CSSLengthUnit::Normal);
      } else if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
        if (pValue.AsRaw<CFX_CSSNumberValue>()->unit() ==
            CFX_CSSNumber::Unit::kPercent) {
          break;
        }

        SetLengthWithPercent(pComputedStyle->inherited_data_.letter_spacing_,
                             eType, pValue,
                             pComputedStyle->inherited_data_.ffont_size_);
      }
      break;
    case CFX_CSSProperty::WordSpacing:
      if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
        pComputedStyle->inherited_data_.word_spacing_.Set(
            CFX_CSSLengthUnit::Normal);
      } else if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
        if (pValue.AsRaw<CFX_CSSNumberValue>()->unit() ==
            CFX_CSSNumber::Unit::kPercent) {
          break;
        }
        SetLengthWithPercent(pComputedStyle->inherited_data_.word_spacing_,
                             eType, pValue,
                             pComputedStyle->inherited_data_.ffont_size_);
      }
      break;
    case CFX_CSSProperty::Top:
      SetLengthWithPercent(pComputedStyle->non_inherited_data_.top_, eType,
                           pValue, pComputedStyle->inherited_data_.ffont_size_);
      break;
    case CFX_CSSProperty::Bottom:
      SetLengthWithPercent(pComputedStyle->non_inherited_data_.bottom_, eType,
                           pValue, pComputedStyle->inherited_data_.ffont_size_);
      break;
    case CFX_CSSProperty::Left:
      SetLengthWithPercent(pComputedStyle->non_inherited_data_.left_, eType,
                           pValue, pComputedStyle->inherited_data_.ffont_size_);
      break;
    case CFX_CSSProperty::Right:
      SetLengthWithPercent(pComputedStyle->non_inherited_data_.right_, eType,
                           pValue, pComputedStyle->inherited_data_.ffont_size_);
      break;
    default:
      break;
  }
}

CFX_CSSDisplay CFX_CSSStyleSelector::ToDisplay(CFX_CSSPropertyValue eValue) {
  switch (eValue) {
    case CFX_CSSPropertyValue::Block:
      return CFX_CSSDisplay::Block;
    case CFX_CSSPropertyValue::None:
      return CFX_CSSDisplay::None;
    case CFX_CSSPropertyValue::ListItem:
      return CFX_CSSDisplay::ListItem;
    case CFX_CSSPropertyValue::InlineTable:
      return CFX_CSSDisplay::InlineTable;
    case CFX_CSSPropertyValue::InlineBlock:
      return CFX_CSSDisplay::InlineBlock;
    case CFX_CSSPropertyValue::Inline:
    default:
      return CFX_CSSDisplay::Inline;
  }
}

CFX_CSSTextAlign CFX_CSSStyleSelector::ToTextAlign(
    CFX_CSSPropertyValue eValue) {
  switch (eValue) {
    case CFX_CSSPropertyValue::Center:
      return CFX_CSSTextAlign::Center;
    case CFX_CSSPropertyValue::Right:
      return CFX_CSSTextAlign::Right;
    case CFX_CSSPropertyValue::Justify:
      return CFX_CSSTextAlign::Justify;
    case CFX_CSSPropertyValue::Left:
    default:
      return CFX_CSSTextAlign::Left;
  }
}

uint16_t CFX_CSSStyleSelector::ToFontWeight(CFX_CSSPropertyValue eValue) {
  switch (eValue) {
    case CFX_CSSPropertyValue::Bold:
      return 700;
    case CFX_CSSPropertyValue::Bolder:
      return 900;
    case CFX_CSSPropertyValue::Lighter:
      return 200;
    case CFX_CSSPropertyValue::Normal:
    default:
      return 400;
  }
}

CFX_CSSFontStyle CFX_CSSStyleSelector::ToFontStyle(
    CFX_CSSPropertyValue eValue) {
  switch (eValue) {
    case CFX_CSSPropertyValue::Italic:
    case CFX_CSSPropertyValue::Oblique:
      return CFX_CSSFontStyle::Italic;
    default:
      return CFX_CSSFontStyle::Normal;
  }
}

bool CFX_CSSStyleSelector::SetLengthWithPercent(
    CFX_CSSLength& width,
    CFX_CSSValue::PrimitiveType eType,
    const RetainPtr<CFX_CSSValue>& pValue,
    float fFontSize) {
  if (eType == CFX_CSSValue::PrimitiveType::kNumber) {
    RetainPtr<CFX_CSSNumberValue> v = pValue.As<CFX_CSSNumberValue>();
    if (v->unit() == CFX_CSSNumber::Unit::kPercent) {
      width.Set(CFX_CSSLengthUnit::Percent,
                pValue.AsRaw<CFX_CSSNumberValue>()->value() / 100.0f);
      return width.NonZero();
    }

    float fValue = v->Apply(fFontSize);
    width.Set(CFX_CSSLengthUnit::Point, fValue);
    return width.NonZero();
  } else if (eType == CFX_CSSValue::PrimitiveType::kEnum) {
    switch (pValue.AsRaw<CFX_CSSEnumValue>()->Value()) {
      case CFX_CSSPropertyValue::Auto:
        width.Set(CFX_CSSLengthUnit::Auto);
        return true;
      case CFX_CSSPropertyValue::None:
        width.Set(CFX_CSSLengthUnit::None);
        return true;
      case CFX_CSSPropertyValue::Thin:
        width.Set(CFX_CSSLengthUnit::Point, 2);
        return true;
      case CFX_CSSPropertyValue::Medium:
        width.Set(CFX_CSSLengthUnit::Point, 3);
        return true;
      case CFX_CSSPropertyValue::Thick:
        width.Set(CFX_CSSLengthUnit::Point, 4);
        return true;
      default:
        return false;
    }
  }
  return false;
}

float CFX_CSSStyleSelector::ToFontSize(CFX_CSSPropertyValue eValue,
                                       float fCurFontSize) {
  switch (eValue) {
    case CFX_CSSPropertyValue::XxSmall:
      return default_font_size_ / 1.2f / 1.2f / 1.2f;
    case CFX_CSSPropertyValue::XSmall:
      return default_font_size_ / 1.2f / 1.2f;
    case CFX_CSSPropertyValue::Small:
      return default_font_size_ / 1.2f;
    case CFX_CSSPropertyValue::Medium:
      return default_font_size_;
    case CFX_CSSPropertyValue::Large:
      return default_font_size_ * 1.2f;
    case CFX_CSSPropertyValue::XLarge:
      return default_font_size_ * 1.2f * 1.2f;
    case CFX_CSSPropertyValue::XxLarge:
      return default_font_size_ * 1.2f * 1.2f * 1.2f;
    case CFX_CSSPropertyValue::Larger:
      return fCurFontSize * 1.2f;
    case CFX_CSSPropertyValue::Smaller:
      return fCurFontSize / 1.2f;
    default:
      return fCurFontSize;
  }
}

CFX_CSSVerticalAlign CFX_CSSStyleSelector::ToVerticalAlign(
    CFX_CSSPropertyValue eValue) {
  switch (eValue) {
    case CFX_CSSPropertyValue::Middle:
      return CFX_CSSVerticalAlign::Middle;
    case CFX_CSSPropertyValue::Bottom:
      return CFX_CSSVerticalAlign::Bottom;
    case CFX_CSSPropertyValue::Super:
      return CFX_CSSVerticalAlign::Super;
    case CFX_CSSPropertyValue::Sub:
      return CFX_CSSVerticalAlign::Sub;
    case CFX_CSSPropertyValue::Top:
      return CFX_CSSVerticalAlign::Top;
    case CFX_CSSPropertyValue::TextTop:
      return CFX_CSSVerticalAlign::TextTop;
    case CFX_CSSPropertyValue::TextBottom:
      return CFX_CSSVerticalAlign::TextBottom;
    case CFX_CSSPropertyValue::Baseline:
    default:
      return CFX_CSSVerticalAlign::Baseline;
  }
}

Mask<CFX_CSSTEXTDECORATION> CFX_CSSStyleSelector::ToTextDecoration(
    const RetainPtr<CFX_CSSValueList>& pValue) {
  Mask<CFX_CSSTEXTDECORATION> dwDecoration;
  for (const RetainPtr<CFX_CSSValue>& val :
       pdfium::Reversed(pValue->values())) {
    if (val->GetType() != CFX_CSSValue::PrimitiveType::kEnum) {
      continue;
    }

    switch (val.AsRaw<CFX_CSSEnumValue>()->Value()) {
      case CFX_CSSPropertyValue::Underline:
        dwDecoration |= CFX_CSSTEXTDECORATION::kUnderline;
        break;
      case CFX_CSSPropertyValue::LineThrough:
        dwDecoration |= CFX_CSSTEXTDECORATION::kLineThrough;
        break;
      case CFX_CSSPropertyValue::Overline:
        dwDecoration |= CFX_CSSTEXTDECORATION::kOverline;
        break;
      case CFX_CSSPropertyValue::Blink:
        dwDecoration |= CFX_CSSTEXTDECORATION::kBlink;
        break;
      case CFX_CSSPropertyValue::Double:
        dwDecoration |= CFX_CSSTEXTDECORATION::kDouble;
        break;
      default:
        break;
    }
  }
  return dwDecoration;
}

CFX_CSSFontVariant CFX_CSSStyleSelector::ToFontVariant(
    CFX_CSSPropertyValue eValue) {
  return eValue == CFX_CSSPropertyValue::SmallCaps
             ? CFX_CSSFontVariant::SmallCaps
             : CFX_CSSFontVariant::Normal;
}
