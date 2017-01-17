// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssstylesheet.h"

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

class CFDE_CSSStyleSheetTest : public testing::Test {
 public:
  void SetUp() override {
    sheet_ = pdfium::MakeUnique<CFDE_CSSStyleSheet>();
    decl_ = nullptr;
  }

  void TearDown() override { decl_ = nullptr; }

  void LoadAndVerifyDecl(const FX_WCHAR* buf, size_t decl_count) {
    ASSERT(sheet_);

    EXPECT_TRUE(sheet_->LoadFromBuffer(buf, FXSYS_wcslen(buf)));
    EXPECT_EQ(1, sheet_->CountRules());

    CFDE_CSSRule* rule = sheet_->GetRule(0);
    EXPECT_EQ(FDE_CSSRuleType::Style, rule->GetType());

    CFDE_CSSStyleRule* style = static_cast<CFDE_CSSStyleRule*>(rule);
    decl_ = style->GetDeclaration();
    EXPECT_EQ(decl_count, decl_->PropertyCountForTesting());
  }

  void VerifyFloat(FDE_CSSProperty prop, float val, FDE_CSSPrimitiveType type) {
    ASSERT(decl_);

    bool important;
    CFDE_CSSValue* v = decl_->GetProperty(prop, important);
    CFDE_CSSPrimitiveValue* pval = static_cast<CFDE_CSSPrimitiveValue*>(v);
    EXPECT_EQ(type, pval->GetPrimitiveType());
    EXPECT_EQ(val, pval->GetFloat());
  }

  void VerifyEnum(FDE_CSSProperty prop, FDE_CSSPropertyValue val) {
    ASSERT(decl_);

    bool important;
    CFDE_CSSValue* v = decl_->GetProperty(prop, important);
    CFDE_CSSPrimitiveValue* pval = static_cast<CFDE_CSSPrimitiveValue*>(v);
    EXPECT_EQ(FDE_CSSPrimitiveType::Enum, pval->GetPrimitiveType());
    EXPECT_EQ(val, pval->GetEnum());
  }

  std::unique_ptr<CFDE_CSSStyleSheet> sheet_;
  CFDE_CSSDeclaration* decl_;
};

TEST_F(CFDE_CSSStyleSheetTest, ParseBorder) {
  LoadAndVerifyDecl(L"a { border: 5px; }", 4);
  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderFull) {
  LoadAndVerifyDecl(L"a { border: 5px solid red; }", 4);
  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 5.0,
              FDE_CSSPrimitiveType::Pixels);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderLeft) {
  LoadAndVerifyDecl(L"a { border-left: 2.5pc; }", 1);
  VerifyFloat(FDE_CSSProperty::BorderLeftWidth, 2.5,
              FDE_CSSPrimitiveType::Picas);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderLeftThick) {
  LoadAndVerifyDecl(L"a { border-left: thick; }", 1);
  VerifyEnum(FDE_CSSProperty::BorderLeftWidth, FDE_CSSPropertyValue::Thick);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderRight) {
  LoadAndVerifyDecl(L"a { border-right: 2.5pc; }", 1);
  VerifyFloat(FDE_CSSProperty::BorderRightWidth, 2.5,
              FDE_CSSPrimitiveType::Picas);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderTop) {
  LoadAndVerifyDecl(L"a { border-top: 2.5pc; }", 1);
  VerifyFloat(FDE_CSSProperty::BorderTopWidth, 2.5,
              FDE_CSSPrimitiveType::Picas);
}

TEST_F(CFDE_CSSStyleSheetTest, ParseBorderBottom) {
  LoadAndVerifyDecl(L"a { border-bottom: 2.5pc; }", 1);
  VerifyFloat(FDE_CSSProperty::BorderBottomWidth, 2.5,
              FDE_CSSPrimitiveType::Picas);
}
