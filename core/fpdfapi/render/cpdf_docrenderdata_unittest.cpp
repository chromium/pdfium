// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/render/cpdf_docrenderdata.h"

#include <memory>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/render/cpdf_transferfunc.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace {

constexpr uint8_t kExpectedType2FunctionSamples[] = {
    26, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13};

std::unique_ptr<CPDF_Dictionary> CreateType2FunctionDict() {
  auto func_dict = pdfium::MakeUnique<CPDF_Dictionary>();
  func_dict->SetNewFor<CPDF_Number>("FunctionType", 2);
  func_dict->SetNewFor<CPDF_Number>("N", 1);

  CPDF_Array* domain_array = func_dict->SetNewFor<CPDF_Array>("Domain");
  domain_array->AddNew<CPDF_Number>(0);
  domain_array->AddNew<CPDF_Number>(1);

  CPDF_Array* c0_array = func_dict->SetNewFor<CPDF_Array>("C0");
  c0_array->AddNew<CPDF_Number>(0.1f);
  c0_array->AddNew<CPDF_Number>(0.2f);
  c0_array->AddNew<CPDF_Number>(0.8f);

  CPDF_Array* c1_array = func_dict->SetNewFor<CPDF_Array>("C1");
  c1_array->AddNew<CPDF_Number>(0.05f);
  c1_array->AddNew<CPDF_Number>(0.01f);
  c1_array->AddNew<CPDF_Number>(0.4f);

  return func_dict;
}

class TestDocRenderData : public CPDF_DocRenderData {
 public:
  TestDocRenderData() : CPDF_DocRenderData(nullptr) {}

  RetainPtr<CPDF_TransferFunc> CreateTransferFuncForTesting(
      const CPDF_Object* pObj) const {
    return CreateTransferFunc(pObj);
  }
};

TEST(CPDF_DocRenderDataTest, TransferFunctionOne) {
  std::unique_ptr<CPDF_Dictionary> func_dict = CreateType2FunctionDict();

  TestDocRenderData render_data;
  auto func = render_data.CreateTransferFuncForTesting(func_dict.get());
  ASSERT_TRUE(func);
  EXPECT_FALSE(func->GetIdentity());

  auto r_samples = func->GetSamplesR();
  auto g_samples = func->GetSamplesG();
  auto b_samples = func->GetSamplesB();
  ASSERT_EQ(FX_ArraySize(kExpectedType2FunctionSamples), r_samples.size());
  ASSERT_EQ(FX_ArraySize(kExpectedType2FunctionSamples), g_samples.size());
  ASSERT_EQ(FX_ArraySize(kExpectedType2FunctionSamples), b_samples.size());

  for (size_t i = 0; i < FX_ArraySize(kExpectedType2FunctionSamples); ++i) {
    EXPECT_EQ(kExpectedType2FunctionSamples[i], r_samples[i]);
    EXPECT_EQ(kExpectedType2FunctionSamples[i], g_samples[i]);
    EXPECT_EQ(kExpectedType2FunctionSamples[i], b_samples[i]);
  }

  EXPECT_EQ(0x000d0d0du, func->TranslateColor(0x00ffffff));
  EXPECT_EQ(0x000d1a1au, func->TranslateColor(0x00ff0000));
  EXPECT_EQ(0x001a0d1au, func->TranslateColor(0x0000ff00));
  EXPECT_EQ(0x001a1a0du, func->TranslateColor(0x000000ff));
  EXPECT_EQ(0x000f0f0fu, func->TranslateColor(0x00cccccc));
  EXPECT_EQ(0x00191715u, func->TranslateColor(0x00123456));
  EXPECT_EQ(0x000d0d0du, func->TranslateColor(0xffffffff));
  EXPECT_EQ(0x001a1a1au, func->TranslateColor(0xff000000));
  EXPECT_EQ(0x000d0d0du, func->TranslateColor(0xccffffff));
  EXPECT_EQ(0x001a1a1au, func->TranslateColor(0x99000000));
}

}  // namespace
