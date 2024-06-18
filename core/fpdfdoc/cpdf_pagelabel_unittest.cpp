// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_pagelabel.h"

#include <memory>
#include <optional>
#include <utility>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::Eq;
using testing::Optional;

namespace {

struct NumValue {
  const char* style;
  const char* prefix;
  std::optional<int> starting_number;
};

void AddNumKeyValue(CPDF_Array* nums, int key, const NumValue& value) {
  nums->AppendNew<CPDF_Number>(key);
  auto page_label_dict = nums->AppendNew<CPDF_Dictionary>();
  page_label_dict->SetNewFor<CPDF_Name>("Type", "PageLabel");
  if (value.style) {
    page_label_dict->SetNewFor<CPDF_Name>("S", value.style);
  }
  if (value.prefix) {
    page_label_dict->SetNewFor<CPDF_String>("P", value.prefix);
  }
  if (value.starting_number.has_value()) {
    page_label_dict->SetNewFor<CPDF_Number>("St",
                                            value.starting_number.value());
  }
}

void AddLimitsArray(CPDF_Dictionary* node, int least, int greatest) {
  auto limits = node->SetNewFor<CPDF_Array>("Limits");
  limits->AppendNew<CPDF_Number>(least);
  limits->AppendNew<CPDF_Number>(greatest);
}

// Set up a number tree with 3 levels and 5 nodes per diagram below. The number
// tree is intended for use as the /PageLabels entry in a Catalog. The keys in
// the leaf nodes are page indices, and the values are page label dictionaries.
// See ISO 32000-1:2008 spec, table 159.
//
// The values are in the format: |S,P,St|
//
// S: Style (name)
// P: Prefix (string)
// St: Starting number (integer) Must be >= 1 if specified.
//
// All 3 fields are optional.
//
//   [page_labels_root]
//     |
//     |
//     |
//   [kid1]
//     |
//     +------------+
//     |            |
//   [grand_kid2] [grand_kid3]
//     |          {8000: |,"x",|}
//     |
//     +--------------------+
//     |                    |
//   [great_grand_kid4]    [great_grand_kid5]
//   {0: |"R",,|}          {3000: |"r",,|}
//   {100: |"A","abc",5|}  {5000: |"a",,|}
//   {900: |"D",,999|}
//
void FillPageLabelsTreeDict(CPDF_Dictionary* page_labels_root) {
  constexpr char kKids[] = "Kids";
  constexpr char kNums[] = "Nums";

  auto page_labels_root_kids = page_labels_root->SetNewFor<CPDF_Array>(kKids);
  auto kid1 = page_labels_root_kids->AppendNew<CPDF_Dictionary>();

  AddLimitsArray(kid1.Get(), 0, 8000);
  auto kids1_kids = kid1->SetNewFor<CPDF_Array>(kKids);
  auto grand_kid2 = kids1_kids->AppendNew<CPDF_Dictionary>();
  auto grand_kid3 = kids1_kids->AppendNew<CPDF_Dictionary>();

  AddLimitsArray(grand_kid2.Get(), 0, 5000);
  auto grand_kid2_kids = grand_kid2->SetNewFor<CPDF_Array>(kKids);
  auto great_grand_kid4 = grand_kid2_kids->AppendNew<CPDF_Dictionary>();
  auto great_grand_kid5 = grand_kid2_kids->AppendNew<CPDF_Dictionary>();

  AddLimitsArray(grand_kid3.Get(), 8000, 8000);
  auto nums = grand_kid3->SetNewFor<CPDF_Array>(kNums);
  AddNumKeyValue(
      nums.Get(), 8000,
      {.style = nullptr, .prefix = "x", .starting_number = std::nullopt});

  AddLimitsArray(great_grand_kid4.Get(), 0, 900);
  nums = great_grand_kid4->SetNewFor<CPDF_Array>(kNums);
  AddNumKeyValue(
      nums.Get(), 0,
      {.style = "R", .prefix = nullptr, .starting_number = std::nullopt});
  AddNumKeyValue(nums.Get(), 100,
                 {.style = "A", .prefix = "abc", .starting_number = 5});
  AddNumKeyValue(nums.Get(), 900,
                 {.style = "D", .prefix = nullptr, .starting_number = 999});

  AddLimitsArray(great_grand_kid5.Get(), 3000, 5000);
  nums = great_grand_kid5->SetNewFor<CPDF_Array>(kNums);
  AddNumKeyValue(
      nums.Get(), 3000,
      {.style = "r", .prefix = nullptr, .starting_number = std::nullopt});
  AddNumKeyValue(
      nums.Get(), 5000,
      {.style = "a", .prefix = nullptr, .starting_number = std::nullopt});
}

}  // namespace

class PageLabelTest : public TestWithPageModule {
 public:
  PageLabelTest() = default;
  ~PageLabelTest() override = default;

  void SetUp() override {
    TestWithPageModule::SetUp();

    auto root_dict = pdfium::MakeRetain<CPDF_Dictionary>();
    root_dict->SetNewFor<CPDF_Dictionary>("Pages");
    auto page_labels_dict = root_dict->SetNewFor<CPDF_Dictionary>("PageLabels");
    FillPageLabelsTreeDict(page_labels_dict.Get());

    doc_ = std::make_unique<CPDF_TestDocument>();
    doc_->SetRoot(std::move(root_dict));
    for (int page_index = 0; page_index < 10001; ++page_index) {
      ASSERT_TRUE(doc_->CreateNewPage(page_index));
    }

    page_label_ = std::make_unique<CPDF_PageLabel>(doc_.get());
  }

  void TearDown() override {
    page_label_.reset();
    doc_.reset();

    TestWithPageModule::TearDown();
  }

  const CPDF_PageLabel* page_label() const { return page_label_.get(); }

 private:
  std::unique_ptr<CPDF_TestDocument> doc_;
  std::unique_ptr<CPDF_PageLabel> page_label_;
};

TEST_F(PageLabelTest, GetLabel) {
  EXPECT_THAT(page_label()->GetLabel(-1), Eq(std::nullopt));
  EXPECT_THAT(page_label()->GetLabel(0), Optional(WideString(L"I")));
  EXPECT_THAT(page_label()->GetLabel(1), Optional(WideString(L"II")));
  EXPECT_THAT(page_label()->GetLabel(37), Optional(WideString(L"XXXVIII")));
  EXPECT_THAT(page_label()->GetLabel(99), Optional(WideString(L"C")));
  EXPECT_THAT(page_label()->GetLabel(100), Optional(WideString(L"abcE")));
  EXPECT_THAT(page_label()->GetLabel(101), Optional(WideString(L"abcF")));
  EXPECT_THAT(page_label()->GetLabel(525),
              Optional(WideString(L"abcNNNNNNNNNNNNNNNNN")));
  EXPECT_THAT(page_label()->GetLabel(899),
              Optional(WideString(L"abcXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")));
  EXPECT_THAT(page_label()->GetLabel(900), Optional(WideString(L"999")));
  EXPECT_THAT(page_label()->GetLabel(901), Optional(WideString(L"1000")));
  EXPECT_THAT(page_label()->GetLabel(1234), Optional(WideString(L"1333")));
  EXPECT_THAT(page_label()->GetLabel(2999), Optional(WideString(L"3098")));
  EXPECT_THAT(page_label()->GetLabel(3000), Optional(WideString(L"i")));
  EXPECT_THAT(page_label()->GetLabel(3001), Optional(WideString(L"ii")));
  EXPECT_THAT(page_label()->GetLabel(3579), Optional(WideString(L"dlxxx")));
  EXPECT_THAT(page_label()->GetLabel(4999), Optional(WideString(L"mm")));
  EXPECT_THAT(page_label()->GetLabel(5000), Optional(WideString(L"a")));
  EXPECT_THAT(page_label()->GetLabel(5001), Optional(WideString(L"b")));
  EXPECT_THAT(page_label()->GetLabel(7654),
              Optional(WideString(
                  L"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
                  L"cccccccccccccccccccccccccccccccccccccccccccc")));
  EXPECT_THAT(
      page_label()->GetLabel(7999),
      Optional(WideString(
          L"jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj"
          L"jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj")));
  EXPECT_THAT(page_label()->GetLabel(8000), Optional(WideString(L"x")));
  EXPECT_THAT(page_label()->GetLabel(8001), Optional(WideString(L"x")));
  EXPECT_THAT(page_label()->GetLabel(10000), Optional(WideString(L"x")));
  EXPECT_THAT(page_label()->GetLabel(10001), Eq(std::nullopt));
}

TEST_F(PageLabelTest, GetLabelPerf) {
  for (int i = 0; i < 10001; ++i) {
    page_label()->GetLabel(i);
  }
}
