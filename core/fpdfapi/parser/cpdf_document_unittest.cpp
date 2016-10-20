// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_document.h"

#include <memory>

#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/fx_memory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

using ScopedDictionary =
    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>>;

}  // namespace

TEST(cpdf_document, UseCachedPageObjNumIfHaveNotPagesDict) {
  // ObjNum can be added in CPDF_DataAvail::IsPageAvail, and PagesDict
  // can be not exists in this case.
  // (case, when hint table is used to page check in CPDF_DataAvail).
  std::unique_ptr<CPDF_Parser> parser(new CPDF_Parser());
  CPDF_Document document(std::move(parser));
  ScopedDictionary dict(new CPDF_Dictionary());
  const int page_count = 100;
  dict->SetIntegerFor("N", page_count);
  document.LoadLinearizedDoc(dict.get());
  ASSERT_EQ(page_count, document.GetPageCount());
  CPDF_Object* page_stub = new CPDF_Dictionary();
  const uint32_t obj_num = document.AddIndirectObject(page_stub);
  const int test_page_num = 33;

  EXPECT_FALSE(document.IsPageLoaded(test_page_num));
  EXPECT_EQ(nullptr, document.GetPage(test_page_num));

  document.SetPageObjNum(test_page_num, obj_num);
  EXPECT_TRUE(document.IsPageLoaded(test_page_num));
  EXPECT_EQ(page_stub, document.GetPage(test_page_num));
}
