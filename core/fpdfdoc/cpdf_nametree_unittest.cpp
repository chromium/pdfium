// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void AddNameKeyValue(CPDF_Array* pNames, const char* key, const int value) {
  pNames->AddNew<CPDF_String>(key, false);
  pNames->AddNew<CPDF_Number>(value);
}

void CheckNameKeyValue(CPDF_Array* pNames,
                       const int index,
                       const char* key,
                       const int value) {
  EXPECT_STREQ(key, pNames->GetStringAt(index * 2).c_str());
  EXPECT_EQ(value, pNames->GetIntegerAt(index * 2 + 1));
}

void AddLimitsArray(CPDF_Dictionary* pNode,
                    const char* least,
                    const char* greatest) {
  CPDF_Array* pLimits = pNode->SetNewFor<CPDF_Array>("Limits");
  pLimits->AddNew<CPDF_String>(least, false);
  pLimits->AddNew<CPDF_String>(greatest, false);
}

void CheckLimitsArray(CPDF_Dictionary* pNode,
                      const char* least,
                      const char* greatest) {
  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  ASSERT_TRUE(pLimits);
  EXPECT_STREQ(least, pLimits->GetStringAt(0).c_str());
  EXPECT_STREQ(greatest, pLimits->GetStringAt(1).c_str());
}

}  // namespace

TEST(cpdf_nametree, GetUnicodeNameWithBOM) {
  // Set up the root dictionary with a Names array.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pNames = pRootDict->SetNewFor<CPDF_Array>("Names");

  // Add the key "1" (with BOM) and value 100 into the array.
  std::ostringstream buf;
  constexpr char kData[] = "\xFE\xFF\x00\x31";
  for (size_t i = 0; i < sizeof(kData); ++i)
    buf.put(kData[i]);
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

TEST(cpdf_nametree, AddIntoNames) {
  // Set up a name tree with a single Names array.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pNames = pRootDict->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames, "2.txt", 222);
  AddNameKeyValue(pNames, "7.txt", 777);

  CPDF_NameTree nameTree(pRootDict.get());
  pNames = nameTree.GetRoot()->GetArrayFor("Names");

  // Insert a name that already exists in the names array.
  EXPECT_FALSE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(111), L"2.txt"));

  // Insert in the beginning of the names array.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(111), L"1.txt"));

  // Insert in the middle of the names array.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(555), L"5.txt"));

  // Insert at the end of the names array.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(999), L"9.txt"));

  // Check that the names array has the expected key-value pairs.
  CheckNameKeyValue(pNames, 0, "1.txt", 111);
  CheckNameKeyValue(pNames, 1, "2.txt", 222);
  CheckNameKeyValue(pNames, 2, "5.txt", 555);
  CheckNameKeyValue(pNames, 3, "7.txt", 777);
  CheckNameKeyValue(pNames, 4, "9.txt", 999);
}

TEST(cpdf_nametree, AddIntoKids) {
  // Set up a name tree with five nodes of three levels.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pKids = pRootDict->SetNewFor<CPDF_Array>("Kids");
  CPDF_Dictionary* pKid1 = pKids->AddNew<CPDF_Dictionary>();

  AddLimitsArray(pKid1, "1.txt", "9.txt");
  pKids = pKid1->SetNewFor<CPDF_Array>("Kids");
  CPDF_Dictionary* pKid2 = pKids->AddNew<CPDF_Dictionary>();
  CPDF_Dictionary* pKid3 = pKids->AddNew<CPDF_Dictionary>();

  AddLimitsArray(pKid2, "1.txt", "5.txt");
  pKids = pKid2->SetNewFor<CPDF_Array>("Kids");
  CPDF_Dictionary* pKid4 = pKids->AddNew<CPDF_Dictionary>();
  CPDF_Dictionary* pKid5 = pKids->AddNew<CPDF_Dictionary>();

  AddLimitsArray(pKid3, "9.txt", "9.txt");
  CPDF_Array* pNames = pKid3->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames, "9.txt", 999);

  AddLimitsArray(pKid4, "1.txt", "2.txt");
  pNames = pKid4->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames, "1.txt", 111);
  AddNameKeyValue(pNames, "2.txt", 222);

  AddLimitsArray(pKid5, "3.txt", "5.txt");
  pNames = pKid5->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames, "3.txt", 333);
  AddNameKeyValue(pNames, "5.txt", 555);

  CPDF_NameTree nameTree(pRootDict.get());

  // Check that adding an existing name would fail.
  EXPECT_FALSE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(444), L"9.txt"));

  // Add a name within the limits of a leaf node.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(444), L"4.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"4.txt"));
  EXPECT_EQ(444, nameTree.LookupValue(L"4.txt")->GetInteger());

  // Add a name that requires changing the limits of two bottom levels.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(666), L"6.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"6.txt"));
  EXPECT_EQ(666, nameTree.LookupValue(L"6.txt")->GetInteger());

  // Add a name that requires changing the limits of two top levels.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(99), L"99.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"99.txt"));
  EXPECT_EQ(99, nameTree.LookupValue(L"99.txt")->GetInteger());

  // Add a name that requires changing the lower limit of all levels.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(-5), L"0.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"0.txt"));
  EXPECT_EQ(-5, nameTree.LookupValue(L"0.txt")->GetInteger());

  // Check that the node on the first level has the expected limits.
  pKid1 = nameTree.GetRoot()->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pKid1);
  CheckLimitsArray(pKid1, "0.txt", "99.txt");

  // Check that the nodes on the second level has the expected limits and names.
  pKid2 = pKid1->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pKid2);
  CheckLimitsArray(pKid2, "0.txt", "6.txt");

  pKid3 = pKid1->GetArrayFor("Kids")->GetDictAt(1);
  ASSERT_TRUE(pKid3);
  CheckLimitsArray(pKid3, "9.txt", "99.txt");
  pNames = pKid3->GetArrayFor("Names");
  ASSERT_TRUE(pNames);
  CheckNameKeyValue(pNames, 0, "9.txt", 999);
  CheckNameKeyValue(pNames, 1, "99.txt", 99);

  // Check that the nodes on the third level has the expected limits and names.
  pKid4 = pKid2->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pKid4);
  CheckLimitsArray(pKid4, "0.txt", "2.txt");
  pNames = pKid4->GetArrayFor("Names");
  ASSERT_TRUE(pNames);
  CheckNameKeyValue(pNames, 0, "0.txt", -5);
  CheckNameKeyValue(pNames, 1, "1.txt", 111);
  CheckNameKeyValue(pNames, 2, "2.txt", 222);

  pKid5 = pKid2->GetArrayFor("Kids")->GetDictAt(1);
  ASSERT_TRUE(pKid5);
  CheckLimitsArray(pKid5, "3.txt", "6.txt");
  pNames = pKid5->GetArrayFor("Names");
  ASSERT_TRUE(pNames);
  CheckNameKeyValue(pNames, 0, "3.txt", 333);
  CheckNameKeyValue(pNames, 1, "4.txt", 444);
  CheckNameKeyValue(pNames, 2, "5.txt", 555);
  CheckNameKeyValue(pNames, 3, "6.txt", 666);
}
