// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_dictionary.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(DictionaryTest, LockerGetters) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Dictionary>("the-dictionary");
  dict->SetNewFor<CPDF_Array>("the-array");
  dict->SetNewFor<CPDF_Stream>("the-stream");
  dict->SetNewFor<CPDF_Number>("the-number", 42);

  CPDF_DictionaryLocker locked_dict(std::move(dict));
  EXPECT_TRUE(locked_dict.GetObjectFor("the-dictionary"));
  EXPECT_FALSE(locked_dict.GetArrayFor("the-dictionary"));
  EXPECT_TRUE(locked_dict.GetDictFor("the-dictionary"));
  EXPECT_FALSE(locked_dict.GetStreamFor("the-dictionary"));
  EXPECT_FALSE(locked_dict.GetNumberFor("the-dictionary"));

  EXPECT_TRUE(locked_dict.GetObjectFor("the-array"));
  EXPECT_TRUE(locked_dict.GetArrayFor("the-array"));
  EXPECT_FALSE(locked_dict.GetDictFor("the-array"));
  EXPECT_FALSE(locked_dict.GetStreamFor("the-array"));
  EXPECT_FALSE(locked_dict.GetNumberFor("the-array"));

  EXPECT_TRUE(locked_dict.GetObjectFor("the-stream"));
  EXPECT_FALSE(locked_dict.GetArrayFor("the-stream"));
  EXPECT_FALSE(locked_dict.GetDictFor("the-stream"));
  EXPECT_TRUE(locked_dict.GetStreamFor("the-stream"));
  EXPECT_FALSE(locked_dict.GetNumberFor("the-stream"));

  EXPECT_TRUE(locked_dict.GetObjectFor("the-number"));
  EXPECT_FALSE(locked_dict.GetArrayFor("the-number"));
  EXPECT_FALSE(locked_dict.GetDictFor("the-number"));
  EXPECT_FALSE(locked_dict.GetStreamFor("the-number"));
  EXPECT_TRUE(locked_dict.GetNumberFor("the-number"));

  EXPECT_FALSE(locked_dict.GetObjectFor("nonesuch"));
  EXPECT_FALSE(locked_dict.GetArrayFor("nonesuch"));
  EXPECT_FALSE(locked_dict.GetDictFor("nonesuch"));
  EXPECT_FALSE(locked_dict.GetStreamFor("nonesuch"));
  EXPECT_FALSE(locked_dict.GetNumberFor("nonesuch"));
}
