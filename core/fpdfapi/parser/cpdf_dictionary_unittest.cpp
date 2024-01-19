// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_dictionary.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(DictionaryTest, Iterators) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Dictionary>("the-dictionary");
  dict->SetNewFor<CPDF_Array>("the-array");
  dict->SetNewFor<CPDF_Number>("the-number", 42);

  CPDF_DictionaryLocker locked_dict(dict);
  auto it = locked_dict.begin();
  EXPECT_NE(it, locked_dict.end());
  EXPECT_EQ(it->first, ByteString("the-array"));
  EXPECT_TRUE(it->second->IsArray());

  ++it;
  EXPECT_NE(it, locked_dict.end());
  EXPECT_EQ(it->first, ByteString("the-dictionary"));
  EXPECT_TRUE(it->second->IsDictionary());

  ++it;
  EXPECT_NE(it, locked_dict.end());
  EXPECT_EQ(it->first, ByteString("the-number"));
  EXPECT_TRUE(it->second->IsNumber());

  ++it;
  EXPECT_EQ(it, locked_dict.end());
}
