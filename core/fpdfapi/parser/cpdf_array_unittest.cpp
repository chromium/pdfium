// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_array.h"

#include <iterator>
#include <memory>
#include <utility>

#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(ArrayTest, GetBooleanAt) {
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  arr->AppendNew<CPDF_Boolean>(true);
  arr->AppendNew<CPDF_Boolean>(false);
  arr->AppendNew<CPDF_Number>(100);
  arr->AppendNew<CPDF_Number>(0);

  ASSERT_EQ(4u, arr->size());
  EXPECT_TRUE(arr->GetBooleanAt(0, true));
  EXPECT_TRUE(arr->GetBooleanAt(0, false));
  EXPECT_FALSE(arr->GetBooleanAt(1, true));
  EXPECT_FALSE(arr->GetBooleanAt(1, false));
  EXPECT_TRUE(arr->GetBooleanAt(2, true));
  EXPECT_FALSE(arr->GetBooleanAt(2, false));
  EXPECT_TRUE(arr->GetBooleanAt(3, true));
  EXPECT_FALSE(arr->GetBooleanAt(3, false));
  EXPECT_TRUE(arr->GetBooleanAt(99, true));
  EXPECT_FALSE(arr->GetBooleanAt(99, false));
}

TEST(ArrayTest, RemoveAt) {
  {
    const int elems[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto arr = pdfium::MakeRetain<CPDF_Array>();
    for (size_t i = 0; i < std::size(elems); ++i)
      arr->AppendNew<CPDF_Number>(elems[i]);
    for (size_t i = 0; i < 3; ++i)
      arr->RemoveAt(3);
    const int expected[] = {1, 2, 3, 7, 8, 9, 10};
    ASSERT_EQ(std::size(expected), arr->size());
    for (size_t i = 0; i < std::size(expected); ++i)
      EXPECT_EQ(expected[i], arr->GetIntegerAt(i));
    arr->RemoveAt(4);
    arr->RemoveAt(4);
    const int expected2[] = {1, 2, 3, 7, 10};
    ASSERT_EQ(std::size(expected2), arr->size());
    for (size_t i = 0; i < std::size(expected2); ++i)
      EXPECT_EQ(expected2[i], arr->GetIntegerAt(i));
  }
  {
    // When the range is out of bound, RemoveAt() has no effect.
    const int elems[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto arr = pdfium::MakeRetain<CPDF_Array>();
    for (size_t i = 0; i < std::size(elems); ++i)
      arr->AppendNew<CPDF_Number>(elems[i]);
    arr->RemoveAt(11);
    EXPECT_EQ(std::size(elems), arr->size());
  }
}

TEST(ArrayTest, Clear) {
  const int elems[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  EXPECT_EQ(0U, arr->size());
  for (size_t i = 0; i < std::size(elems); ++i)
    arr->AppendNew<CPDF_Number>(elems[i]);
  EXPECT_EQ(std::size(elems), arr->size());
  arr->Clear();
  EXPECT_EQ(0U, arr->size());
}

TEST(ArrayTest, SetAtBeyond) {
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  EXPECT_FALSE(arr->SetNewAt<CPDF_Number>(0, 0));
  EXPECT_TRUE(arr->InsertNewAt<CPDF_Number>(0, 0));
  EXPECT_FALSE(arr->SetNewAt<CPDF_Number>(1, 0));
}

TEST(ArrayTest, InsertAt) {
  const int elems[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  for (size_t i = 0; i < std::size(elems); ++i)
    arr->InsertNewAt<CPDF_Number>(i, elems[i]);
  ASSERT_EQ(std::size(elems), arr->size());
  for (size_t i = 0; i < std::size(elems); ++i)
    EXPECT_EQ(elems[i], arr->GetIntegerAt(i));
  arr->InsertNewAt<CPDF_Number>(3, 33);
  arr->InsertNewAt<CPDF_Number>(6, 55);
  arr->InsertNewAt<CPDF_Number>(12, 12);
  const int expected[] = {1, 2, 3, 33, 4, 5, 55, 6, 7, 8, 9, 10, 12};
  ASSERT_EQ(std::size(expected), arr->size());
  for (size_t i = 0; i < std::size(expected); ++i)
    EXPECT_EQ(expected[i], arr->GetIntegerAt(i));
}

TEST(ArrayTest, InsertAtBeyond) {
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  EXPECT_FALSE(arr->InsertNewAt<CPDF_Number>(1, 0));
  EXPECT_TRUE(arr->InsertNewAt<CPDF_Number>(0, 0));
  EXPECT_FALSE(arr->InsertNewAt<CPDF_Number>(2, 0));
}

TEST(ArrayTest, Clone) {
  {
    // Basic case.
    const int elems[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto arr = pdfium::MakeRetain<CPDF_Array>();
    for (size_t i = 0; i < std::size(elems); ++i)
      arr->InsertNewAt<CPDF_Number>(i, elems[i]);
    RetainPtr<CPDF_Array> arr2 = ToArray(arr->Clone());
    ASSERT_EQ(arr->size(), arr2->size());
    for (size_t i = 0; i < std::size(elems); ++i) {
      // Clone() always create new objects.
      EXPECT_NE(arr->GetObjectAt(i), arr2->GetObjectAt(i));
      EXPECT_EQ(arr->GetIntegerAt(i), arr2->GetIntegerAt(i));
    }
  }
  {
    // Clone() with and without dereferencing reference objects.
    static const size_t kNumOfRows = 3;
    static const size_t kNumOfRowElems = 5;
    const int elems[kNumOfRows][kNumOfRowElems] = {
        {1, 2, 3, 4, 5}, {10, 9, 8, 7, 6}, {11, 12, 13, 14, 15}};
    auto arr = pdfium::MakeRetain<CPDF_Array>();
    // Indirect references to indirect objects.
    auto obj_holder = std::make_unique<CPDF_IndirectObjectHolder>();
    for (size_t i = 0; i < kNumOfRows; ++i) {
      auto arr_elem = pdfium::MakeRetain<CPDF_Array>();
      for (size_t j = 0; j < kNumOfRowElems; ++j) {
        auto obj = pdfium::MakeRetain<CPDF_Number>(elems[i][j]);
        // Starts object number from 1.
        int obj_num = i * kNumOfRowElems + j + 1;
        obj_holder->ReplaceIndirectObjectIfHigherGeneration(obj_num,
                                                            std::move(obj));
        arr_elem->InsertNewAt<CPDF_Reference>(j, obj_holder.get(), obj_num);
      }
      arr->InsertAt(i, std::move(arr_elem));
    }
    ASSERT_EQ(kNumOfRows, arr->size());
    // Not dereferencing reference objects means just creating new references
    // instead of new copies of direct objects.
    RetainPtr<CPDF_Array> arr1 = ToArray(arr->Clone());
    ASSERT_EQ(arr->size(), arr1->size());
    // Dereferencing reference objects creates new copies of direct objects.
    RetainPtr<CPDF_Array> arr2 = ToArray(arr->CloneDirectObject());
    ASSERT_EQ(arr->size(), arr2->size());
    for (size_t i = 0; i < kNumOfRows; ++i) {
      const CPDF_Array* arr_elem = arr->GetObjectAt(i)->AsArray();
      const CPDF_Array* arr1_elem = arr1->GetObjectAt(i)->AsArray();
      const CPDF_Array* arr2_elem = arr2->GetObjectAt(i)->AsArray();
      EXPECT_NE(arr_elem, arr1_elem);
      EXPECT_NE(arr_elem, arr2_elem);
      for (size_t j = 0; j < kNumOfRowElems; ++j) {
        auto elem_obj = arr_elem->GetObjectAt(j);
        auto elem_obj1 = arr1_elem->GetObjectAt(j);
        auto elem_obj2 = arr2_elem->GetObjectAt(j);
        // Results from not deferencing reference objects.
        EXPECT_NE(elem_obj, elem_obj1);
        EXPECT_TRUE(elem_obj1->IsReference());
        EXPECT_EQ(elem_obj->GetDirect(), elem_obj1->GetDirect());
        EXPECT_EQ(elem_obj->GetInteger(), elem_obj1->GetInteger());
        // Results from deferencing reference objects.
        EXPECT_NE(elem_obj, elem_obj2);
        EXPECT_TRUE(elem_obj2->IsNumber());
        EXPECT_NE(elem_obj->GetDirect(), elem_obj2);
        EXPECT_EQ(elem_obj->GetObjNum(), elem_obj2->GetObjNum());
        EXPECT_EQ(elem_obj->GetInteger(), elem_obj2->GetInteger());
      }
    }
    arr.Reset();
    ASSERT_EQ(kNumOfRows, arr1->size());
    for (size_t i = 0; i < kNumOfRows; ++i) {
      for (size_t j = 0; j < kNumOfRowElems; ++j) {
        // Results from not deferencing reference objects.
        auto elem_obj1 = arr1->GetObjectAt(i)->AsArray()->GetObjectAt(j);
        EXPECT_TRUE(elem_obj1->IsReference());
        EXPECT_EQ(elems[i][j], elem_obj1->GetInteger());
        // Results from deferencing reference objects.
        EXPECT_EQ(elems[i][j],
                  arr2->GetObjectAt(i)->AsArray()->GetIntegerAt(j));
      }
    }
  }
}

TEST(ArrayTest, Find) {
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  auto dict0 = pdfium::MakeRetain<CPDF_Dictionary>();
  auto dict1 = pdfium::MakeRetain<CPDF_Dictionary>();
  auto dict2 = pdfium::MakeRetain<CPDF_Dictionary>();
  arr->Append(dict0);
  arr->Append(dict1);

  absl::optional<size_t> maybe_found = arr->Find(nullptr);
  EXPECT_FALSE(maybe_found.has_value());

  maybe_found = arr->Find(dict0.Get());
  ASSERT_TRUE(maybe_found.has_value());
  EXPECT_EQ(0u, maybe_found.value());

  maybe_found = arr->Find(dict1.Get());
  ASSERT_TRUE(maybe_found.has_value());
  EXPECT_EQ(1u, maybe_found.value());

  maybe_found = arr->Find(dict2.Get());
  EXPECT_FALSE(maybe_found.has_value());
}

TEST(ArrayTest, Contains) {
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  auto dict0 = pdfium::MakeRetain<CPDF_Dictionary>();
  auto dict1 = pdfium::MakeRetain<CPDF_Dictionary>();
  auto dict2 = pdfium::MakeRetain<CPDF_Dictionary>();
  arr->Append(dict0);
  arr->Append(dict1);
  EXPECT_TRUE(arr->Contains(dict0.Get()));
  EXPECT_TRUE(arr->Contains(dict1.Get()));
  EXPECT_FALSE(arr->Contains(dict2.Get()));
}

TEST(ArrayTest, Iterator) {
  const int elems[] = {-23, -11,     3,         455,   2345877,
                       0,   7895330, -12564334, 10000, -100000};
  auto arr = pdfium::MakeRetain<CPDF_Array>();
  for (size_t i = 0; i < std::size(elems); ++i)
    arr->InsertNewAt<CPDF_Number>(i, elems[i]);

  size_t index = 0;
  CPDF_ArrayLocker locker(arr);
  for (const auto& it : locker)
    EXPECT_EQ(elems[index++], it->AsNumber()->GetInteger());
  EXPECT_EQ(std::size(elems), index);
}
