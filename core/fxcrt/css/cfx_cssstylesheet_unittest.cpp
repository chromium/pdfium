// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssstylesheet.h"

#include <memory>
#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/css/cfx_cssenumvalue.h"
#include "core/fxcrt/css/cfx_cssnumbervalue.h"
#include "core/fxcrt/css/cfx_cssstylerule.h"
#include "core/fxcrt/css/cfx_cssvaluelist.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFX_CSSStyleSheetTest : public testing::Test {
 public:
  void SetUp() override {
    sheet_ = std::make_unique<CFX_CSSStyleSheet>();
    decl_ = nullptr;
  }

  void TearDown() override { decl_ = nullptr; }

  void VerifyLoadFails(WideStringView buf) {
    DCHECK(sheet_);
    EXPECT_FALSE(sheet_->LoadBuffer(buf));
  }

  void LoadAndVerifyRuleCount(WideStringView buf, size_t rule_count) {
    DCHECK(sheet_);
    EXPECT_TRUE(sheet_->LoadBuffer(buf));
    EXPECT_EQ(sheet_->CountRules(), rule_count);
  }

  void LoadAndVerifyDecl(WideStringView buf,
                         const std::vector<WideString>& selectors,
                         size_t decl_count) {
    LoadAndVerifyRuleCount(buf, 1);
    CFX_CSSStyleRule* style = sheet_->GetRule(0);
    ASSERT_TRUE(style);
    EXPECT_EQ(selectors.size(), style->CountSelectorLists());

    for (size_t i = 0; i < selectors.size(); i++) {
      uint32_t hash = FX_HashCode_GetLoweredW(selectors[i].AsStringView());
      EXPECT_EQ(hash, style->GetSelectorList(i)->name_hash());
    }

    decl_ = style->GetDeclaration();
    ASSERT_TRUE(decl_);
    EXPECT_EQ(decl_->PropertyCountForTesting(), decl_count);
  }

  void VerifyFloat(CFX_CSSProperty prop, float val, CFX_CSSNumber::Unit unit) {
    DCHECK(decl_);

    bool important;
    RetainPtr<CFX_CSSValue> v = decl_->GetProperty(prop, &important);
    EXPECT_EQ(v->GetType(), CFX_CSSValue::PrimitiveType::kNumber);
    EXPECT_EQ(v.AsRaw<CFX_CSSNumberValue>()->unit(), unit);
    EXPECT_EQ(v.AsRaw<CFX_CSSNumberValue>()->value(), val);
  }

  void VerifyEnum(CFX_CSSProperty prop, CFX_CSSPropertyValue val) {
    DCHECK(decl_);

    bool important;
    RetainPtr<CFX_CSSValue> v = decl_->GetProperty(prop, &important);
    EXPECT_EQ(v->GetType(), CFX_CSSValue::PrimitiveType::kEnum);
    EXPECT_EQ(v.AsRaw<CFX_CSSEnumValue>()->Value(), val);
  }

  void VerifyList(CFX_CSSProperty prop,
                  std::vector<CFX_CSSPropertyValue> expected_values) {
    DCHECK(decl_);

    bool important;
    RetainPtr<CFX_CSSValueList> list =
        decl_->GetProperty(prop, &important).As<CFX_CSSValueList>();
    ASSERT_TRUE(list);
    const std::vector<RetainPtr<CFX_CSSValue>>& values = list->values();
    ASSERT_EQ(values.size(), expected_values.size());

    for (size_t i = 0; i < expected_values.size(); ++i) {
      const auto& val = values[i];
      EXPECT_EQ(val->GetType(), CFX_CSSValue::PrimitiveType::kEnum);
      EXPECT_EQ(val.AsRaw<CFX_CSSEnumValue>()->Value(), expected_values[i]);
    }
  }

  static bool HasSelector(CFX_CSSStyleRule* style, WideStringView selector) {
    uint32_t hash = FX_HashCode_GetLoweredW(selector);
    for (size_t i = 0; i < style->CountSelectorLists(); ++i) {
      if (style->GetSelectorList(i)->name_hash() == hash)
        return true;
    }
    return false;
  }

  std::unique_ptr<CFX_CSSStyleSheet> sheet_;
  CFX_CSSDeclaration* decl_;
};

TEST_F(CFX_CSSStyleSheetTest, ParseEmpty) {
  LoadAndVerifyRuleCount(L"", 0);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBlankEmpty) {
  LoadAndVerifyRuleCount(L"  \n\r\t", 0);
}

TEST_F(CFX_CSSStyleSheetTest, ParseStrayClose1) {
  VerifyLoadFails(L"}");
}

TEST_F(CFX_CSSStyleSheetTest, ParseStrayClose2) {
  LoadAndVerifyRuleCount(L"foo }", 0);
}

TEST_F(CFX_CSSStyleSheetTest, ParseStrayClose3) {
  VerifyLoadFails(L"foo {a: b}}");
}

TEST_F(CFX_CSSStyleSheetTest, ParseEmptySelector) {
  VerifyLoadFails(L"{a: b}");
}

TEST_F(CFX_CSSStyleSheetTest, ParseEmptyBody) {
  LoadAndVerifyRuleCount(L"foo {}", 0);
}

TEST_F(CFX_CSSStyleSheetTest, ParseMultipleSelectors) {
  const wchar_t* buf =
      L"a { border: 10px; }\n"
      L"bcdef { text-decoration: underline; }\n"
      L"* { padding: 0; }\n";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  ASSERT_EQ(3u, sheet_->CountRules());

  CFX_CSSStyleRule* style = sheet_->GetRule(0);
  ASSERT_TRUE(style);
  EXPECT_EQ(1u, style->CountSelectorLists());
  EXPECT_TRUE(HasSelector(style, L"a"));

  decl_ = style->GetDeclaration();
  ASSERT_TRUE(decl_);
  EXPECT_EQ(4u, decl_->PropertyCountForTesting());

  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);

  style = sheet_->GetRule(1);
  ASSERT_TRUE(style);
  EXPECT_EQ(1u, style->CountSelectorLists());
  EXPECT_TRUE(HasSelector(style, L"bcdef"));
  EXPECT_FALSE(HasSelector(style, L"bcde"));

  decl_ = style->GetDeclaration();
  ASSERT_TRUE(decl_);
  EXPECT_EQ(1u, decl_->PropertyCountForTesting());
  VerifyList(CFX_CSSProperty::TextDecoration,
             {CFX_CSSPropertyValue::Underline});

  style = sheet_->GetRule(2);
  ASSERT_TRUE(style);
  EXPECT_EQ(1u, style->CountSelectorLists());
  EXPECT_TRUE(HasSelector(style, L"*"));

  decl_ = style->GetDeclaration();
  ASSERT_TRUE(decl_);
  EXPECT_EQ(4u, decl_->PropertyCountForTesting());
  VerifyFloat(CFX_CSSProperty::PaddingLeft, 0.0f, CFX_CSSNumber::Unit::kNumber);
  VerifyFloat(CFX_CSSProperty::PaddingRight, 0.0f,
              CFX_CSSNumber::Unit::kNumber);
  VerifyFloat(CFX_CSSProperty::PaddingTop, 0.0f, CFX_CSSNumber::Unit::kNumber);
  VerifyFloat(CFX_CSSProperty::PaddingBottom, 0.0f,
              CFX_CSSNumber::Unit::kNumber);
}

TEST_F(CFX_CSSStyleSheetTest, ParseChildSelectors) {
  const wchar_t* buf = L"a b c { border: 10px; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(1u, sheet_->CountRules());

  CFX_CSSStyleRule* style = sheet_->GetRule(0);
  ASSERT_TRUE(style);
  EXPECT_EQ(1u, style->CountSelectorLists());

  const auto* sel = style->GetSelectorList(0);
  ASSERT_TRUE(sel);
  EXPECT_EQ(FX_HashCode_GetLoweredW(L"c"), sel->name_hash());

  sel = sel->next_selector();
  ASSERT_TRUE(sel);
  EXPECT_EQ(FX_HashCode_GetLoweredW(L"b"), sel->name_hash());

  sel = sel->next_selector();
  ASSERT_TRUE(sel);
  EXPECT_EQ(FX_HashCode_GetLoweredW(L"a"), sel->name_hash());

  sel = sel->next_selector();
  EXPECT_FALSE(sel);

  decl_ = style->GetDeclaration();
  ASSERT_TRUE(decl_);
  EXPECT_EQ(4u, decl_->PropertyCountForTesting());

  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 10.0f,
              CFX_CSSNumber::Unit::kPixels);
}

TEST_F(CFX_CSSStyleSheetTest, ParseUnhandledSelectors) {
  const wchar_t* buf = L"a > b { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(0u, sheet_->CountRules());

  buf = L"a[first] { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(0u, sheet_->CountRules());

  buf = L"a+b { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(0u, sheet_->CountRules());

  buf = L"a ^ b { padding: 0; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(0u, sheet_->CountRules());
}

TEST_F(CFX_CSSStyleSheetTest, ParseMultipleSelectorsCombined) {
  LoadAndVerifyDecl(L"a, b, c { border: 5px; }", {L"a", L"b", L"c"}, 4);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorder) {
  LoadAndVerifyDecl(L"a { border: 5px; }", {L"a"}, 4);
  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderFull) {
  LoadAndVerifyDecl(L"a { border: 5px solid red; }", {L"a"}, 4);
  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 5.0,
              CFX_CSSNumber::Unit::kPixels);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderLeft) {
  LoadAndVerifyDecl(L"a { border-left: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderLeftThick) {
  LoadAndVerifyDecl(L"a { border-left: thick; }", {L"a"}, 1);
  VerifyEnum(CFX_CSSProperty::BorderLeftWidth, CFX_CSSPropertyValue::Thick);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderRight) {
  LoadAndVerifyDecl(L"a { border-right: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderTop) {
  LoadAndVerifyDecl(L"a { border-top: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderBottom) {
  LoadAndVerifyDecl(L"a { border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithCommentsInSelector) {
  LoadAndVerifyDecl(L"/**{*/a/**}*/ { border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithCommentsInProperty) {
  LoadAndVerifyDecl(L"a { /*}*/border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithCommentsInValue) {
  LoadAndVerifyDecl(L"a { border-bottom: /*;*/2.5pc;/* color:red;*/ }", {L"a"},
                    1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumber::Unit::kPicas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithUnterminatedCommentInSelector) {
  LoadAndVerifyRuleCount(L"a/* { border-bottom: 2.5pc; }", 0);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithUnterminatedCommentInProperty) {
  LoadAndVerifyRuleCount(L"a { /*border-bottom: 2.5pc; }", 1);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithUnterminatedCommentInValue) {
  LoadAndVerifyRuleCount(L"a { border-bottom: /*2.5pc; }", 1);
}
