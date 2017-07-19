// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_nametree, GetUnicodeNameWithBOM) {
  // Set up the root dictionary with a Names array.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pNames = pRootDict->SetNewFor<CPDF_Array>("Names");

  // Add the key "1" (with BOM) and value 100 into the array.
  std::ostringstream buf;
  buf << static_cast<unsigned char>(254) << static_cast<unsigned char>(255)
      << static_cast<unsigned char>(0) << static_cast<unsigned char>(49);
  pNames->AddNew<CPDF_String>(CFX_ByteString(buf), true);
  pNames->AddNew<CPDF_Number>(100);

  // Check that the key is as expected.
  CPDF_NameTree nameTree(pRootDict.get());
  CFX_WideString storedName;
  nameTree.LookupValueAndName(0, &storedName);
  EXPECT_STREQ(L"1", storedName.c_str());

  // Check that the correct value object can be obtained by looking up "1".
  CFX_WideString matchName = L"1";
  CPDF_Object* pObj = nameTree.LookupValue(matchName);
  ASSERT_TRUE(pObj->IsNumber());
  EXPECT_EQ(100, pObj->AsNumber()->GetInteger());
}
