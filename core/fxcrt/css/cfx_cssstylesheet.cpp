// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssstylesheet.h"

#include <utility>

#include "core/fxcrt/css/cfx_cssdata.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/css/cfx_cssstylerule.h"
#include "core/fxcrt/fx_codepage.h"
#include "third_party/base/ptr_util.h"

CFX_CSSStyleSheet::CFX_CSSStyleSheet() {}

CFX_CSSStyleSheet::~CFX_CSSStyleSheet() = default;

size_t CFX_CSSStyleSheet::CountRules() const {
  return m_RuleArray.size();
}

CFX_CSSStyleRule* CFX_CSSStyleSheet::GetRule(size_t index) const {
  return m_RuleArray[index].get();
}

bool CFX_CSSStyleSheet::LoadBuffer(WideStringView buffer) {
  m_RuleArray.clear();
  auto pSyntax = pdfium::MakeUnique<CFX_CSSSyntaxParser>(buffer);
  while (1) {
    CFX_CSSSyntaxStatus eStatus = pSyntax->DoSyntaxParse();
    if (eStatus == CFX_CSSSyntaxStatus::kStyleRule)
      eStatus = LoadStyleRule(pSyntax.get());
    if (eStatus == CFX_CSSSyntaxStatus::kEOS)
      return true;
    if (eStatus == CFX_CSSSyntaxStatus::kError)
      return false;
  }
}

CFX_CSSSyntaxStatus CFX_CSSStyleSheet::LoadStyleRule(
    CFX_CSSSyntaxParser* pSyntax) {
  std::vector<std::unique_ptr<CFX_CSSSelector>> selectors;

  CFX_CSSStyleRule* pStyleRule = nullptr;
  int32_t iValueLen = 0;
  const CFX_CSSData::Property* property = nullptr;
  WideString wsName;
  while (1) {
    switch (pSyntax->DoSyntaxParse()) {
      case CFX_CSSSyntaxStatus::kSelector: {
        WideStringView strValue = pSyntax->GetCurrentString();
        auto pSelector = CFX_CSSSelector::FromString(strValue);
        if (pSelector)
          selectors.push_back(std::move(pSelector));
        break;
      }
      case CFX_CSSSyntaxStatus::kPropertyName: {
        WideStringView strValue = pSyntax->GetCurrentString();
        property = CFX_CSSData::GetPropertyByName(strValue);
        if (!property)
          wsName = WideString(strValue);
        break;
      }
      case CFX_CSSSyntaxStatus::kPropertyValue: {
        if (property || iValueLen > 0) {
          WideStringView strValue = pSyntax->GetCurrentString();
          auto* decl = pStyleRule->GetDeclaration();
          if (!strValue.IsEmpty()) {
            if (property) {
              decl->AddProperty(property, strValue);
            } else {
              decl->AddProperty(wsName, WideString(strValue));
            }
          }
        }
        break;
      }
      case CFX_CSSSyntaxStatus::kDeclOpen: {
        if (!pStyleRule && !selectors.empty()) {
          auto rule = pdfium::MakeUnique<CFX_CSSStyleRule>();
          pStyleRule = rule.get();
          pStyleRule->SetSelector(&selectors);
          m_RuleArray.push_back(std::move(rule));
        } else {
          SkipRuleSet(pSyntax);
          return CFX_CSSSyntaxStatus::kNone;
        }
        break;
      }
      case CFX_CSSSyntaxStatus::kDeclClose: {
        if (pStyleRule && pStyleRule->GetDeclaration()->empty()) {
          m_RuleArray.pop_back();
          pStyleRule = nullptr;
        }
        return CFX_CSSSyntaxStatus::kNone;
      }
      case CFX_CSSSyntaxStatus::kEOS:
        return CFX_CSSSyntaxStatus::kEOS;
      case CFX_CSSSyntaxStatus::kError:
      default:
        return CFX_CSSSyntaxStatus::kError;
    }
  }
}

void CFX_CSSStyleSheet::SkipRuleSet(CFX_CSSSyntaxParser* pSyntax) {
  while (1) {
    switch (pSyntax->DoSyntaxParse()) {
      case CFX_CSSSyntaxStatus::kSelector:
      case CFX_CSSSyntaxStatus::kDeclOpen:
      case CFX_CSSSyntaxStatus::kPropertyName:
      case CFX_CSSSyntaxStatus::kPropertyValue:
        break;
      case CFX_CSSSyntaxStatus::kDeclClose:
      case CFX_CSSSyntaxStatus::kEOS:
      case CFX_CSSSyntaxStatus::kError:
      default:
        return;
    }
  }
}
