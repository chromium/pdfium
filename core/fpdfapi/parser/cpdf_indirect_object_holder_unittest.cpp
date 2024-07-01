// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_null.h"
#include "core/fxcrt/check.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class MockIndirectObjectHolder final : public CPDF_IndirectObjectHolder {
 public:
  MockIndirectObjectHolder() = default;
  ~MockIndirectObjectHolder() override = default;

  MOCK_METHOD(RetainPtr<CPDF_Object>, ParseIndirectObject, (uint32_t objnum));
};

}  // namespace

TEST(IndirectObjectHolderTest, RecursiveParseOfSameObject) {
  MockIndirectObjectHolder mock_holder;
  // ParseIndirectObject should not be called again on recursively same object
  // parse request.
  EXPECT_CALL(mock_holder, ParseIndirectObject(::testing::_))
      .WillOnce(::testing::WithArg<0>(::testing::Invoke(
          [&mock_holder](uint32_t objnum) -> RetainPtr<CPDF_Object> {
            RetainPtr<const CPDF_Object> same_parse =
                mock_holder.GetOrParseIndirectObject(objnum);
            CHECK(!same_parse);
            return pdfium::MakeRetain<CPDF_Null>();
          })));

  EXPECT_TRUE(mock_holder.GetOrParseIndirectObject(1000));
}

TEST(IndirectObjectHolderTest, GetObjectMethods) {
  static constexpr uint32_t kObjNum = 1000;
  MockIndirectObjectHolder mock_holder;

  EXPECT_CALL(mock_holder, ParseIndirectObject(::testing::_)).Times(0);
  EXPECT_FALSE(mock_holder.GetIndirectObject(kObjNum));
  ::testing::Mock::VerifyAndClearExpectations(&mock_holder);

  EXPECT_CALL(mock_holder, ParseIndirectObject(::testing::_))
      .WillOnce(::testing::WithArg<0>(
          ::testing::Invoke([](uint32_t objnum) -> RetainPtr<CPDF_Object> {
            return pdfium::MakeRetain<CPDF_Null>();
          })));
  EXPECT_TRUE(mock_holder.GetOrParseIndirectObject(kObjNum));
  ::testing::Mock::VerifyAndClearExpectations(&mock_holder);

  EXPECT_CALL(mock_holder, ParseIndirectObject(::testing::_)).Times(0);
  ASSERT_TRUE(mock_holder.GetIndirectObject(kObjNum));
  ::testing::Mock::VerifyAndClearExpectations(&mock_holder);

  EXPECT_EQ(kObjNum, mock_holder.GetIndirectObject(kObjNum)->GetObjNum());
}

TEST(IndirectObjectHolderTest, ParseInvalidObjNum) {
  MockIndirectObjectHolder mock_holder;

  EXPECT_CALL(mock_holder, ParseIndirectObject(::testing::_)).Times(0);
  EXPECT_FALSE(
      mock_holder.GetOrParseIndirectObject(CPDF_Object::kInvalidObjNum));
}

TEST(IndirectObjectHolderTest, ReplaceObjectWithInvalidObjNum) {
  MockIndirectObjectHolder mock_holder;

  EXPECT_CALL(mock_holder, ParseIndirectObject(::testing::_)).Times(0);
  EXPECT_FALSE(mock_holder.ReplaceIndirectObjectIfHigherGeneration(
      CPDF_Object::kInvalidObjNum, pdfium::MakeRetain<CPDF_Null>()));
}

TEST(IndirectObjectHolderTest, TemplateNewMethods) {
  MockIndirectObjectHolder mock_holder;

  auto pDict = mock_holder.NewIndirect<CPDF_Dictionary>();
  auto pArray = mock_holder.NewIndirect<CPDF_Array>();
  mock_holder.DeleteIndirectObject(pDict->GetObjNum());
  mock_holder.DeleteIndirectObject(pArray->GetObjNum());

  // No longer UAF since NewIndirect<> returns retained objects.
  EXPECT_TRUE(pDict->IsDictionary());
  EXPECT_TRUE(pArray->IsArray());
}
