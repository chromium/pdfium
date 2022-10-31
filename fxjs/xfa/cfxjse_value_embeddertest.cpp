// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/xfa/cfxjse_value.h"

#include <memory>
#include <utility>
#include <vector>

#include "fxjs/xfa/cfxjse_engine.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CFXJSE_ValueEmbedderTest : public XFAJSEmbedderTest {};

TEST_F(CFXJSE_ValueEmbedderTest, Empty) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  auto pValue = std::make_unique<CFXJSE_Value>();
  EXPECT_TRUE(pValue->IsEmpty());
  EXPECT_FALSE(pValue->IsUndefined(isolate()));
  EXPECT_FALSE(pValue->IsNull(isolate()));
  EXPECT_FALSE(pValue->IsBoolean(isolate()));
  EXPECT_FALSE(pValue->IsString(isolate()));
  EXPECT_FALSE(pValue->IsNumber(isolate()));
  EXPECT_FALSE(pValue->IsObject(isolate()));
  EXPECT_FALSE(pValue->IsArray(isolate()));
  EXPECT_FALSE(pValue->IsFunction(isolate()));
}

TEST_F(CFXJSE_ValueEmbedderTest, EmptyArrayInsert) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  // Test inserting empty values into arrays.
  auto pValue = std::make_unique<CFXJSE_Value>();
  std::vector<std::unique_ptr<CFXJSE_Value>> vec;
  vec.push_back(std::move(pValue));

  CFXJSE_Value array;
  array.SetArray(isolate(), vec);
  EXPECT_TRUE(array.IsArray(isolate()));
}
