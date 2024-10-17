// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void AddNameKeyValue(CPDF_Array* names, const char* key, int value) {
  names->AppendNew<CPDF_String>(key);
  names->AppendNew<CPDF_Number>(value);
}

void CheckNameKeyValue(const CPDF_Array* names,
                       int pair_index,
                       const char* key,
                       int value) {
  ASSERT_TRUE(names);
  EXPECT_EQ(key, names->GetByteStringAt(pair_index * 2));
  EXPECT_EQ(value, names->GetIntegerAt(pair_index * 2 + 1));
}

void AddLimitsArray(CPDF_Dictionary* node,
                    const char* least,
                    const char* greatest) {
  auto limits = node->SetNewFor<CPDF_Array>("Limits");
  limits->AppendNew<CPDF_String>(least);
  limits->AppendNew<CPDF_String>(greatest);
}

void CheckLimitsArray(const CPDF_Dictionary* node,
                      const char* least,
                      const char* greatest) {
  ASSERT_TRUE(node);
  RetainPtr<const CPDF_Array> limits = node->GetArrayFor("Limits");
  ASSERT_TRUE(limits);
  EXPECT_EQ(2u, limits->size());
  RetainPtr<const CPDF_String> left = limits->GetStringAt(0);
  ASSERT_TRUE(left);
  RetainPtr<const CPDF_String> right = limits->GetStringAt(1);
  ASSERT_TRUE(right);
  EXPECT_EQ(least, left->GetString());
  EXPECT_EQ(greatest, right->GetString());
}

// Set up a name tree with 3 levels and 5 nodes, per diagram:
//
//   [root]
//     |
//     |
//     |
//   [pKid1]
//     |
//     +------------+
//     |            |
//   [pGrandKid2] [pGrandKid3]
//     |          {9.txt: 999}
//     |
//     +-----------------+
//     |                 |
//   [pGreatGrandKid4] [pGreatGrandKid5]
//   {1.txt: 111}      {3.txt: 333}
//   {2.txt: 222}      {5.txt: 555}
//
void FillNameTreeDict(CPDF_Dictionary* pRootDict) {
  auto pRootKids = pRootDict->SetNewFor<CPDF_Array>("Kids");
  auto pKid1 = pRootKids->AppendNew<CPDF_Dictionary>();

  // Make the lower and upper limit out of order on purpose.
  AddLimitsArray(pKid1.Get(), "9.txt", "1.txt");
  auto pKids1Kids = pKid1->SetNewFor<CPDF_Array>("Kids");
  auto pGrandKid2 = pKids1Kids->AppendNew<CPDF_Dictionary>();
  auto pGrandKid3 = pKids1Kids->AppendNew<CPDF_Dictionary>();

  AddLimitsArray(pGrandKid2.Get(), "1.txt", "5.txt");
  auto pGrandKid2Kids = pGrandKid2->SetNewFor<CPDF_Array>("Kids");
  auto pGreatGrandKid4 = pGrandKid2Kids->AppendNew<CPDF_Dictionary>();
  auto pGreatGrandKid5 = pGrandKid2Kids->AppendNew<CPDF_Dictionary>();

  AddLimitsArray(pGrandKid3.Get(), "9.txt", "9.txt");
  auto pNames = pGrandKid3->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames.Get(), "9.txt", 999);

  // Make the lower and upper limit out of order on purpose.
  AddLimitsArray(pGreatGrandKid4.Get(), "2.txt", "1.txt");
  pNames = pGreatGrandKid4->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames.Get(), "1.txt", 111);
  AddNameKeyValue(pNames.Get(), "2.txt", 222);

  AddLimitsArray(pGreatGrandKid5.Get(), "3.txt", "5.txt");
  pNames = pGreatGrandKid5->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames.Get(), "3.txt", 333);
  AddNameKeyValue(pNames.Get(), "5.txt", 555);
}

}  // namespace

TEST(CPDFNameTreeTest, GetUnicodeNameWithBOM) {
  // Set up the root dictionary with a Names array.
  auto pRootDict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pNames = pRootDict->SetNewFor<CPDF_Array>("Names");

  // Add the key "1" (with BOM) and value 100 into the array.
  constexpr uint8_t kData[] = {0xFE, 0xFF, 0x00, 0x31};
  pNames->AppendNew<CPDF_String>(kData, CPDF_String::DataType::kIsHex);
  pNames->AppendNew<CPDF_Number>(100);

  // Check that the key is as expected.
  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(pRootDict.Get());
  WideString stored_name;
  name_tree->LookupValueAndName(0, &stored_name);
  EXPECT_EQ(L"1", stored_name);

  // Check that the correct value object can be obtained by looking up "1".
  RetainPtr<const CPDF_Number> pNumber = ToNumber(name_tree->LookupValue(L"1"));
  ASSERT_TRUE(pNumber);
  EXPECT_EQ(100, pNumber->GetInteger());
}

TEST(CPDFNameTreeTest, GetFromTreeWithLimitsArrayWith4Items) {
  // After creating a name tree, mutate a /Limits array so it has excess
  // elements.
  auto pRootDict = pdfium::MakeRetain<CPDF_Dictionary>();
  FillNameTreeDict(pRootDict.Get());
  RetainPtr<CPDF_Dictionary> pKid1 =
      pRootDict->GetMutableArrayFor("Kids")->GetMutableDictAt(0);
  RetainPtr<CPDF_Dictionary> pGrandKid3 =
      pKid1->GetMutableArrayFor("Kids")->GetMutableDictAt(1);
  RetainPtr<CPDF_Array> pLimits = pGrandKid3->GetMutableArrayFor("Limits");
  ASSERT_EQ(2u, pLimits->size());
  pLimits->AppendNew<CPDF_Number>(5);
  pLimits->AppendNew<CPDF_Number>(6);
  ASSERT_EQ(4u, pLimits->size());
  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(pRootDict.Get());

  RetainPtr<const CPDF_Number> pNumber =
      ToNumber(name_tree->LookupValue(L"9.txt"));
  ASSERT_TRUE(pNumber);
  EXPECT_EQ(999, pNumber->GetInteger());
  CheckLimitsArray(pKid1.Get(), "1.txt", "9.txt");
  CheckLimitsArray(pGrandKid3.Get(), "9.txt", "9.txt");
}

TEST(CPDFNameTreeTest, AddIntoNames) {
  // Set up a name tree with a single Names array.
  auto pRootDict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pNames = pRootDict->SetNewFor<CPDF_Array>("Names");
  AddNameKeyValue(pNames.Get(), "2.txt", 222);
  AddNameKeyValue(pNames.Get(), "7.txt", 777);

  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(pRootDict.Get());

  // Insert a name that already exists in the names array.
  EXPECT_FALSE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(111),
                                          L"2.txt"));

  // Insert in the beginning of the names array.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(111),
                                         L"1.txt"));

  // Insert in the middle of the names array.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(555),
                                         L"5.txt"));

  // Insert at the end of the names array.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(999),
                                         L"9.txt"));

  // Check that the names array has the expected key-value pairs.
  CheckNameKeyValue(pNames.Get(), 0, "1.txt", 111);
  CheckNameKeyValue(pNames.Get(), 1, "2.txt", 222);
  CheckNameKeyValue(pNames.Get(), 2, "5.txt", 555);
  CheckNameKeyValue(pNames.Get(), 3, "7.txt", 777);
  CheckNameKeyValue(pNames.Get(), 4, "9.txt", 999);
}

TEST(CPDFNameTreeTest, AddIntoEmptyNames) {
  // Set up a name tree with an empty Names array.
  auto pRootDict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pNames = pRootDict->SetNewFor<CPDF_Array>("Names");

  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(pRootDict.Get());

  // Insert a name should work.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(111),
                                         L"2.txt"));

  // Insert a name that already exists in the names array.
  EXPECT_FALSE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(111),
                                          L"2.txt"));

  // Insert in the beginning of the names array.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(111),
                                         L"1.txt"));

  // Insert in the middle of the names array.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(555),
                                         L"5.txt"));

  // Insert at the end of the names array.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(999),
                                         L"9.txt"));

  // Check that the names array has the expected key-value pairs.
  CheckNameKeyValue(pNames.Get(), 0, "1.txt", 111);
  CheckNameKeyValue(pNames.Get(), 1, "2.txt", 111);
  CheckNameKeyValue(pNames.Get(), 2, "5.txt", 555);
  CheckNameKeyValue(pNames.Get(), 3, "9.txt", 999);
}

TEST(CPDFNameTreeTest, AddIntoKids) {
  auto pRootDict = pdfium::MakeRetain<CPDF_Dictionary>();
  FillNameTreeDict(pRootDict.Get());
  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(pRootDict.Get());

  // Check that adding an existing name would fail.
  EXPECT_FALSE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(444),
                                          L"9.txt"));

  // Add a name within the limits of a leaf node.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(444),
                                         L"4.txt"));
  ASSERT_TRUE(name_tree->LookupValue(L"4.txt"));
  EXPECT_EQ(444, name_tree->LookupValue(L"4.txt")->GetInteger());

  // Add a name that requires changing the limits of two bottom levels.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(666),
                                         L"6.txt"));
  ASSERT_TRUE(name_tree->LookupValue(L"6.txt"));
  EXPECT_EQ(666, name_tree->LookupValue(L"6.txt")->GetInteger());

  // Add a name that requires changing the limits of two top levels.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(99),
                                         L"99.txt"));
  ASSERT_TRUE(name_tree->LookupValue(L"99.txt"));
  EXPECT_EQ(99, name_tree->LookupValue(L"99.txt")->GetInteger());

  // Add a name that requires changing the lower limit of all levels.
  EXPECT_TRUE(name_tree->AddValueAndName(pdfium::MakeRetain<CPDF_Number>(-5),
                                         L"0.txt"));
  ASSERT_TRUE(name_tree->LookupValue(L"0.txt"));
  EXPECT_EQ(-5, name_tree->LookupValue(L"0.txt")->GetInteger());

  // Check that the node on the first level has the expected limits.
  RetainPtr<const CPDF_Dictionary> pKid1 =
      name_tree->GetRootForTesting()->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pKid1);
  CheckLimitsArray(pKid1.Get(), "0.txt", "99.txt");

  // Check that the nodes on the second level has the expected limits and names.
  RetainPtr<const CPDF_Dictionary> pGrandKid2 =
      pKid1->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pGrandKid2);
  CheckLimitsArray(pGrandKid2.Get(), "0.txt", "6.txt");

  RetainPtr<const CPDF_Dictionary> pGrandKid3 =
      pKid1->GetArrayFor("Kids")->GetDictAt(1);
  ASSERT_TRUE(pGrandKid3);
  CheckLimitsArray(pGrandKid3.Get(), "9.txt", "99.txt");
  RetainPtr<const CPDF_Array> pNames = pGrandKid3->GetArrayFor("Names");
  CheckNameKeyValue(pNames.Get(), 0, "9.txt", 999);
  CheckNameKeyValue(pNames.Get(), 1, "99.txt", 99);

  // Check that the nodes on the third level has the expected limits and names.
  RetainPtr<const CPDF_Dictionary> pGreatGrandKid4 =
      pGrandKid2->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pGreatGrandKid4);
  CheckLimitsArray(pGreatGrandKid4.Get(), "0.txt", "2.txt");
  pNames = pGreatGrandKid4->GetArrayFor("Names");
  CheckNameKeyValue(pNames.Get(), 0, "0.txt", -5);
  CheckNameKeyValue(pNames.Get(), 1, "1.txt", 111);
  CheckNameKeyValue(pNames.Get(), 2, "2.txt", 222);

  RetainPtr<const CPDF_Dictionary> pGreatGrandKid5 =
      pGrandKid2->GetArrayFor("Kids")->GetDictAt(1);
  ASSERT_TRUE(pGreatGrandKid5);
  CheckLimitsArray(pGreatGrandKid5.Get(), "3.txt", "6.txt");
  pNames = pGreatGrandKid5->GetArrayFor("Names");
  CheckNameKeyValue(pNames.Get(), 0, "3.txt", 333);
  CheckNameKeyValue(pNames.Get(), 1, "4.txt", 444);
  CheckNameKeyValue(pNames.Get(), 2, "5.txt", 555);
  CheckNameKeyValue(pNames.Get(), 3, "6.txt", 666);
}

TEST(CPDFNameTreeTest, DeleteFromKids) {
  auto pRootDict = pdfium::MakeRetain<CPDF_Dictionary>();
  FillNameTreeDict(pRootDict.Get());
  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(pRootDict.Get());

  // Retrieve the kid dictionaries.
  RetainPtr<const CPDF_Dictionary> pKid1 =
      name_tree->GetRootForTesting()->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pKid1);
  RetainPtr<const CPDF_Dictionary> pGrandKid2 =
      pKid1->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pGrandKid2);
  RetainPtr<const CPDF_Dictionary> pGrandKid3 =
      pKid1->GetArrayFor("Kids")->GetDictAt(1);
  ASSERT_TRUE(pGrandKid3);
  RetainPtr<const CPDF_Dictionary> pGreatGrandKid4 =
      pGrandKid2->GetArrayFor("Kids")->GetDictAt(0);
  ASSERT_TRUE(pGreatGrandKid4);
  RetainPtr<const CPDF_Dictionary> pGreatGrandKid5 =
      pGrandKid2->GetArrayFor("Kids")->GetDictAt(1);
  ASSERT_TRUE(pGreatGrandKid5);

  // Check that deleting an out-of-bound index would fail.
  EXPECT_FALSE(name_tree->DeleteValueAndName(5));

  // Delete the name "9.txt", and check that its node gets deleted and its
  // parent node's limits get updated.
  WideString csName;
  ASSERT_TRUE(name_tree->LookupValue(L"9.txt"));
  EXPECT_EQ(999, name_tree->LookupValue(L"9.txt")->GetInteger());
  EXPECT_TRUE(name_tree->LookupValueAndName(4, &csName));
  EXPECT_EQ(L"9.txt", csName);
  EXPECT_EQ(2u, pKid1->GetArrayFor("Kids")->size());
  EXPECT_TRUE(name_tree->DeleteValueAndName(4));
  EXPECT_EQ(1u, pKid1->GetArrayFor("Kids")->size());
  CheckLimitsArray(pKid1.Get(), "1.txt", "5.txt");

  // Delete the name "2.txt", and check that its node does not get deleted, its
  // node's limits get updated, and no other limits get updated.
  ASSERT_TRUE(name_tree->LookupValue(L"2.txt"));
  EXPECT_EQ(222, name_tree->LookupValue(L"2.txt")->GetInteger());
  EXPECT_TRUE(name_tree->LookupValueAndName(1, &csName));
  EXPECT_EQ(L"2.txt", csName);
  EXPECT_EQ(4u, pGreatGrandKid4->GetArrayFor("Names")->size());
  EXPECT_TRUE(name_tree->DeleteValueAndName(1));
  EXPECT_EQ(2u, pGreatGrandKid4->GetArrayFor("Names")->size());
  CheckLimitsArray(pGreatGrandKid4.Get(), "1.txt", "1.txt");
  CheckLimitsArray(pGrandKid2.Get(), "1.txt", "5.txt");
  CheckLimitsArray(pKid1.Get(), "1.txt", "5.txt");

  // Delete the name "1.txt", and check that its node gets deleted, and its
  // parent's and gradparent's limits get updated.
  ASSERT_TRUE(name_tree->LookupValue(L"1.txt"));
  EXPECT_EQ(111, name_tree->LookupValue(L"1.txt")->GetInteger());
  EXPECT_TRUE(name_tree->LookupValueAndName(0, &csName));
  EXPECT_EQ(L"1.txt", csName);
  EXPECT_EQ(2u, pGrandKid2->GetArrayFor("Kids")->size());
  EXPECT_TRUE(name_tree->DeleteValueAndName(0));
  EXPECT_EQ(1u, pGrandKid2->GetArrayFor("Kids")->size());
  CheckLimitsArray(pGrandKid2.Get(), "3.txt", "5.txt");
  CheckLimitsArray(pKid1.Get(), "3.txt", "5.txt");

  // Delete the name "3.txt", and check that its node does not get deleted, and
  // its node's, its parent's, and its grandparent's limits get updated.
  ASSERT_TRUE(name_tree->LookupValue(L"3.txt"));
  EXPECT_EQ(333, name_tree->LookupValue(L"3.txt")->GetInteger());
  EXPECT_TRUE(name_tree->LookupValueAndName(0, &csName));
  EXPECT_EQ(L"3.txt", csName);
  EXPECT_EQ(4u, pGreatGrandKid5->GetArrayFor("Names")->size());
  EXPECT_TRUE(name_tree->DeleteValueAndName(0));
  EXPECT_EQ(2u, pGreatGrandKid5->GetArrayFor("Names")->size());
  CheckLimitsArray(pGreatGrandKid5.Get(), "5.txt", "5.txt");
  CheckLimitsArray(pGrandKid2.Get(), "5.txt", "5.txt");
  CheckLimitsArray(pKid1.Get(), "5.txt", "5.txt");

  // Delete the name "5.txt", and check that all nodes in the tree get deleted
  // since they are now all empty.
  ASSERT_TRUE(name_tree->LookupValue(L"5.txt"));
  EXPECT_EQ(555, name_tree->LookupValue(L"5.txt")->GetInteger());
  EXPECT_TRUE(name_tree->LookupValueAndName(0, &csName));
  EXPECT_EQ(L"5.txt", csName);
  EXPECT_EQ(1u, name_tree->GetRootForTesting()->GetArrayFor("Kids")->size());
  EXPECT_TRUE(name_tree->DeleteValueAndName(0));
  EXPECT_EQ(0u, name_tree->GetRootForTesting()->GetArrayFor("Kids")->size());

  // Check that the tree is now empty.
  EXPECT_EQ(0u, name_tree->GetCount());
  EXPECT_FALSE(name_tree->LookupValueAndName(0, &csName));
  EXPECT_FALSE(name_tree->DeleteValueAndName(0));
}
