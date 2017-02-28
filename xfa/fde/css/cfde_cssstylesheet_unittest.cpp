// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssstylesheet.h"

#include <memory>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssenumvalue.h"
#include "xfa/fde/css/cfde_cssnumbervalue.h"
#include "xfa/fde/css/cfde_cssstylerule.h"
#include "xfa/fde/css/cfde_cssvaluelist.h"

class CFDE_CSSStyleSheetTest : public testing::Test {
 public:
  void SetUp() override {
    sheet_ = pdfium::MakeUnique<CFDE_CSSStyleSheet>();
    decl_ = nullptr;
  }

  void TearDown() override { decl_ = nullptr; }

  void LoadAndVerifyDecl(const FX_WCHAR* buf,
                         const std::vector<CFX_WideString>& selectors,
                         size_t decl_count) {
    ASSERT(sheet_);

    EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
    EXPECT_EQ(sheet_->CountRules(), 1);

    CFDE_CSSStyleRule* style = sheet_->GetRule(0);
    EXPECT_EQ(selectors.size(), style->CountSelectorLists());

    for (size_t i = 0; i < selectors.size(); i++) {
      uint32_t hash = FX_HashCode_GetW(selectors[i].AsStringC(), true);
      EXPECT_EQ(hash, style->GetSelectorList(i)->GetNameHash());
    }

    decl_ = style->GetDeclaration();
    EXPECT_EQ(decl_->PropertyCountForTesting(), decl_count);
  }

  void VerifyFloat(FDE_CSSProperty prop, float val, FDE_CSSNumberType type) {
    ASSERT(decl_);

    bool important;
    CFX_RetainPtr<CFDE_CSSValue> v = decl_->GetProperty(prop, &important);
    EXPECT_EQ(v->GetType(), FDE_CSSPrimitiveType::Number);
    EXPECT_EQ(v.As<CFDE_CSSNumberValue>()->Kind(), type);
    EXPECT_EQ(v.As<CFDE_CSSNumberValue>()->Value(), val);
  }

  void VerifyEnum(FDE_CSSProperty prop, FDE_CSSPropertyValue val) {
    ASSERT(decl_);

    bool important;
    CFX_RetainPtr<CFDE_CSSValue> v = decl_->GetProperty(prop, &important);
    EXPECT_EQ(v->GetType(), FDE_CSSPrimitiveType::Enum);
    EXPECT_EQ(v.As<CFDE_CSSEnumValue>()->Value(), val);
  }

  void VerifyList(FDE_CSSProperty prop,
                  std::vector<FDE_CSSPropertyValue> values) {
    ASSERT(decl_);

    bool important;
    CFX_RetainPtr<CFDE_CSSValueList> list =
        decl_->GetProperty(prop, &important).As<CFDE_CSSValueList>();
    EXPECT_EQ(list->CountValues(), pdfium::CollectionSize<int32_t>(values));

    for (size_t i = 0; i < values.size(); i++) {
      CFX_RetainPtr<CFDE_CSSValue> val = list->GetValue(i);
      EXPECT_EQ(val->GetType(), FDE_CSSPrimitiveType::Enum);
      EXPECT_EQ(val.As<CFDE_CSSEnumValue>()->Value(), values[i]);
    }
  }

  std::unique_ptr<CFDE_CSSStyleSheet> sheet_;
  CFDE_CSSDeclaration* decl_;
};

TEST_F(CFDE_CSSStyleSheetTest, ParseMultipleSelectors) {
  const FX_WCHAR* buf =
      L"a { border: 10px; }\nb { text-decoration: underline; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
  EXPECT_EQ(2, sheet_->CountRules());

  CFDE_CSSStyleRule* style = sheet_->GetRule(0);
  EXPECT_EQ(1UL, style->CountSelectorLists());

  bool found_selector = false;
  uint32_t hash = FX_HashCode_GetW(L"a", true);
  for (size_t i = 0; i < style->CountSelectorLists(); i++) {
    if (style->GetSelectorList(i)->GetNameHash() == hash) {
      found_selector = true;
      break;
    }
  }
  EXPECT_TRUE(found_selector);

  decl_ = style->GetDeclaration();
  EXPECT_EQ(4UL, decl_->PropertyCountForTesting());

  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 10.0,
              FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 10.0,
              FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 10.0, FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 10.0,
              FDE_CSSNumberType::Pixels);

  style = sheet_->GetRule(1);
  EXPECT_EQ(1UL, style->CountSelectorLists());

  found_selector = false;
  hash = FX_HashCode_GetW(L"b", true);
  for (size_t i = 0; i < style->CountSelectorLists(); i++) {
    if (style->GetSelectorList(i)->GetNameHash() == hash) {
      found_selector = true;
      break;
    }
  }
  EXPECT_TRUE(found_selector);

  decl_ = style->GetDeclaration();
  EXPECT_EQ(1UL, decl_->PropertyCountForTesting());
  VerifyList(FDE_CSSProperty::TextDecoration,
             {FDE_CSSPropertyValue::Underline});
}

TEST_F(CFDE_CSSStyleSheetTest, ParseChildSelectors) {
  const FX_WCHAR* buf = L"a b c { border: 10px; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
  EXPECT_EQ(1, sheet_->CountRules());

  CFDE_CSSStyleRule* style = sheet_->GetRule(0);
  EXPECT_EQ(1UL, style->CountSelectorLists());

  auto sel = style->GetSelectorList(0);
  EXPECT_TRUE(sel != nullptr);
  EXPECT_EQ(FX_HashCode_GetW(L"c", true), sel->GetNameHash());

  sel = sel->GetNextSelector();
  EXPECT_TRUE(sel != nullptr);
  EXPECT_EQ(FX_HashCode_GetW(L"b", true), sel->GetNameHash());

  sel = sel->GetNextSelector();
  EXPECT_TRUE(sel != nullptr);
  EXPECT_EQ(FX_HashCode_GetW(L"a", true), sel->GetNameHash());

  sel = sel->GetNextSelector();
  EXPECT_TRUE(sel == nullptr);

  decl_ = style->GetDeclaration();
  EXPECT_EQ(4UL, decl_->PropertyCountForTesting());

  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 10.0,
              FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 10.0,
              FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 10.0, FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 10.0,
              FDE_CSSNumberType::Pixels);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseUnhandledSelectors) {
  const FX_WCHAR* buf = L"a > b { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
  EXPECT_EQ(0, sheet_->CountRules());

  buf = L"a[first] { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
  EXPECT_EQ(0, sheet_->CountRules());

  buf = L"a+b { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
  EXPECT_EQ(0, sheet_->CountRules());

  buf = L"a ^ b { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf, FXSYS_wcslen(buf)));
  EXPECT_EQ(0, sheet_->CountRules());
}

TEST_F(CFDE_CSSStyleSheetTest, ParseMultipleSelectorsCombined) {
  LoadAndVerifyDecl(L"a, b, c { border: 5px; }", {L"a", L"b", L"c"}, 4);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorder) {
  LoadAndVerifyDecl(L"a { border: 5px; }", {L"a"}, 4);
  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 5.0, FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 5.0,
              FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 5.0, FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 5.0,
              FDE_CSSNumberType::Pixels);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderFull) {
  LoadAndVerifyDecl(L"a { border: 5px solid red; }", {L"a"}, 4);
  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 5.0, FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 5.0,
              FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 5.0, FDE_CSSNumberType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 5.0,
              FDE_CSSNumberType::Pixels);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderLeft) {
  LoadAndVerifyDecl(L"a { border-left: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 2.5, FDE_CSSNumberType::Picas);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderLeftThick) {
  LoadAndVerifyDecl(L"a { border-left: thick; }", {L"a"}, 1);
  VerifyEnum(FDE_CSSProperty::BorderLeftWidth, FDE_CSSPropertyValue::Thick);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderRight) {
  LoadAndVerifyDecl(L"a { border-right: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 2.5, FDE_CSSNumberType::Picas);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderTop) {
  LoadAndVerifyDecl(L"a { border-top: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 2.5, FDE_CSSNumberType::Picas);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderBottom) {
  LoadAndVerifyDecl(L"a { border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 2.5,
              FDE_CSSNumberType::Picas);
}
