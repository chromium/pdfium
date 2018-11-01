// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/cfx_globaldata.h"

#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

namespace {

class TestDelegate : public CFX_GlobalData::Delegate {
 public:
  ~TestDelegate() override {}

  bool StoreBuffer(pdfium::span<const uint8_t> buffer) override {
    last_buffer_ = std::vector<uint8_t>(buffer.begin(), buffer.end());
    return true;
  }
  Optional<pdfium::span<uint8_t>> LoadBuffer() override {
    return pdfium::span<uint8_t>(last_buffer_);
  }
  void BufferDone() override {
    last_buffer_ = std::vector<uint8_t>();  // Catch misuse after done.
  }

  std::vector<uint8_t> last_buffer_;
};

}  // namespace

TEST(CFXGlobalData, StoreReload) {
  TestDelegate delegate;
  CFX_GlobalData* pInstance = CFX_GlobalData::GetRetainedInstance(&delegate);
  pInstance->SetGlobalVariableNumber("double", 2.0);
  pInstance->SetGlobalVariableString("string", "clams");
  pInstance->SetGlobalVariablePersistent("double", true);
  pInstance->SetGlobalVariablePersistent("string", true);
  ASSERT_TRUE(pInstance->Release());

  pInstance = CFX_GlobalData::GetRetainedInstance(&delegate);
  auto* element = pInstance->GetAt(0);
  ASSERT_TRUE(element);
  EXPECT_EQ("double", element->data.sKey);
  EXPECT_EQ(CFX_KeyValue::DataType::NUMBER, element->data.nType);
  EXPECT_EQ(2.0, element->data.dData);

  element = pInstance->GetAt(1);
  ASSERT_TRUE(element);
  EXPECT_EQ("string", element->data.sKey);
  EXPECT_EQ(CFX_KeyValue::DataType::STRING, element->data.nType);
  EXPECT_EQ("clams", element->data.sData);
  ASSERT_TRUE(pInstance->Release());
}
