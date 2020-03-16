// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssstylesheet.h"

#include <memory>
#include <vector>

#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/css/cfx_cssenumvalue.h"
#include "core/fxcrt/css/cfx_cssnumbervalue.h"
#include "core/fxcrt/css/cfx_cssstylerule.h"
#include "core/fxcrt/css/cfx_cssvaluelist.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

class CFX_CSSStyleSheetTest : public testing::Test {
 public:
  void SetUp() override {
    sheet_ = pdfium::MakeUnique<CFX_CSSStyleSheet>();
    decl_ = nullptr;
  }

  void TearDown() override { decl_ = nullptr; }

  void VerifyLoadFails(WideStringView buf) {
    ASSERT(sheet_);
    EXPECT_FALSE(sheet_->LoadBuffer(buf));
  }

  void LoadAndVerifyRuleCount(WideStringView buf, size_t rule_count) {
    ASSERT(sheet_);
    EXPECT_TRUE(sheet_->LoadBuffer(buf));
    EXPECT_EQ(sheet_->CountRules(), rule_count);
  }

  void LoadAndVerifyDecl(WideStringView buf,
                         const std::vector<WideString>& selectors,
                         size_t decl_count) {
    LoadAndVerifyRuleCount(buf, 1);
    CFX_CSSStyleRule* style = sheet_->GetRule(0);
    EXPECT_EQ(selectors.size(), style->CountSelectorLists());

    for (size_t i = 0; i < selectors.size(); i++) {
      uint32_t hash = FX_HashCode_GetW(selectors[i].AsStringView(), true);
      EXPECT_EQ(hash, style->GetSelectorList(i)->GetNameHash());
    }

    decl_ = style->GetDeclaration();
    EXPECT_EQ(decl_->PropertyCountForTesting(), decl_count);
  }

  void VerifyFloat(CFX_CSSProperty prop, float val, CFX_CSSNumberType type) {
    ASSERT(decl_);

    bool important;
    RetainPtr<CFX_CSSValue> v = decl_->GetProperty(prop, &important);
    EXPECT_EQ(v->GetType(), CFX_CSSPrimitiveType::Number);
    EXPECT_EQ(v.As<CFX_CSSNumberValue>()->Kind(), type);
    EXPECT_EQ(v.As<CFX_CSSNumberValue>()->Value(), val);
  }

  void VerifyEnum(CFX_CSSProperty prop, CFX_CSSPropertyValue val) {
    ASSERT(decl_);

    bool important;
    RetainPtr<CFX_CSSValue> v = decl_->GetProperty(prop, &important);
    EXPECT_EQ(v->GetType(), CFX_CSSPrimitiveType::Enum);
    EXPECT_EQ(v.As<CFX_CSSEnumValue>()->Value(), val);
  }

  void VerifyList(CFX_CSSProperty prop,
                  std::vector<CFX_CSSPropertyValue> values) {
    ASSERT(decl_);

    bool important;
    RetainPtr<CFX_CSSValueList> list =
        decl_->GetProperty(prop, &important).As<CFX_CSSValueList>();
    EXPECT_EQ(list->CountValues(), pdfium::CollectionSize<int32_t>(values));

    for (size_t i = 0; i < values.size(); i++) {
      RetainPtr<CFX_CSSValue> val = list->GetValue(i);
      EXPECT_EQ(val->GetType(), CFX_CSSPrimitiveType::Enum);
      EXPECT_EQ(val.As<CFX_CSSEnumValue>()->Value(), values[i]);
    }
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
      L"a { border: 10px; }\nb { text-decoration: underline; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(2u, sheet_->CountRules());

  CFX_CSSStyleRule* style = sheet_->GetRule(0);
  EXPECT_EQ(1u, style->CountSelectorLists());

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
  EXPECT_EQ(4u, decl_->PropertyCountForTesting());

  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 10.0,
              CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 10.0,
              CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 10.0, CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 10.0,
              CFX_CSSNumberType::Pixels);

  style = sheet_->GetRule(1);
  EXPECT_EQ(1u, style->CountSelectorLists());

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
  EXPECT_EQ(1u, decl_->PropertyCountForTesting());
  VerifyList(CFX_CSSProperty::TextDecoration,
             {CFX_CSSPropertyValue::Underline});
}

TEST_F(CFX_CSSStyleSheetTest, ParseChildSelectors) {
  const wchar_t* buf = L"a b c { border: 10px; }";
  EXPECT_TRUE(sheet_->LoadBuffer(buf));
  EXPECT_EQ(1u, sheet_->CountRules());

  CFX_CSSStyleRule* style = sheet_->GetRule(0);
  EXPECT_EQ(1u, style->CountSelectorLists());

  auto* sel = style->GetSelectorList(0);
  ASSERT_TRUE(sel);
  EXPECT_EQ(FX_HashCode_GetW(L"c", true), sel->GetNameHash());

  sel = sel->GetNextSelector();
  ASSERT_TRUE(sel);
  EXPECT_EQ(FX_HashCode_GetW(L"b", true), sel->GetNameHash());

  sel = sel->GetNextSelector();
  ASSERT_TRUE(sel);
  EXPECT_EQ(FX_HashCode_GetW(L"a", true), sel->GetNameHash());

  sel = sel->GetNextSelector();
  EXPECT_FALSE(sel);

  decl_ = style->GetDeclaration();
  EXPECT_EQ(4u, decl_->PropertyCountForTesting());

  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 10.0,
              CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 10.0,
              CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 10.0, CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 10.0,
              CFX_CSSNumberType::Pixels);
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
  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 5.0, CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 5.0,
              CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 5.0, CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 5.0,
              CFX_CSSNumberType::Pixels);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderFull) {
  LoadAndVerifyDecl(L"a { border: 5px solid red; }", {L"a"}, 4);
  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 5.0, CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 5.0,
              CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 5.0, CFX_CSSNumberType::Pixels);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 5.0,
              CFX_CSSNumberType::Pixels);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderLeft) {
  LoadAndVerifyDecl(L"a { border-left: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderLeftWidth, 2.5, CFX_CSSNumberType::Picas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderLeftThick) {
  LoadAndVerifyDecl(L"a { border-left: thick; }", {L"a"}, 1);
  VerifyEnum(CFX_CSSProperty::BorderLeftWidth, CFX_CSSPropertyValue::Thick);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderRight) {
  LoadAndVerifyDecl(L"a { border-right: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderRightWidth, 2.5, CFX_CSSNumberType::Picas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderTop) {
  LoadAndVerifyDecl(L"a { border-top: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderTopWidth, 2.5, CFX_CSSNumberType::Picas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseBorderBottom) {
  LoadAndVerifyDecl(L"a { border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumberType::Picas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithCommentsInSelector) {
  LoadAndVerifyDecl(L"/**{*/a/**}*/ { border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumberType::Picas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithCommentsInProperty) {
  LoadAndVerifyDecl(L"a { /*}*/border-bottom: 2.5pc; }", {L"a"}, 1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumberType::Picas);
}

TEST_F(CFX_CSSStyleSheetTest, ParseWithCommentsInValue) {
  LoadAndVerifyDecl(L"a { border-bottom: /*;*/2.5pc;/* color:red;*/ }", {L"a"},
                    1);
  VerifyFloat(CFX_CSSProperty::BorderBottomWidth, 2.5,
              CFX_CSSNumberType::Picas);
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
