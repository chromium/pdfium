// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_dictionary.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(DictionaryTest, LockerGetters) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Dictionary>("A");
  dict->SetNewFor<CPDF_Array>("B");

  CPDF_DictionaryLocker locked_dict(std::move(dict));
  EXPECT_TRUE(locked_dict.GetObjectFor("A"));
  EXPECT_FALSE(locked_dict.GetArrayFor("A"));
  EXPECT_TRUE(locked_dict.GetDictFor("A"));
  EXPECT_TRUE(locked_dict.GetObjectFor("B"));
  EXPECT_TRUE(locked_dict.GetArrayFor("B"));
  EXPECT_FALSE(locked_dict.GetDictFor("B"));
}
