// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/unowned_ptr.h"

#include <utility>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace fxcrt {
namespace {

class Clink {
 public:
  UnownedPtr<Clink> next_ = nullptr;
};

void DeleteDangling() {
  auto ptr2 = pdfium::MakeUnique<Clink>();
  {
    auto ptr1 = pdfium::MakeUnique<Clink>();
    ptr2->next_ = ptr1.get();
  }
}

void ResetDangling() {
  auto ptr2 = pdfium::MakeUnique<Clink>();
  {
    auto ptr1 = pdfium::MakeUnique<Clink>();
    ptr2->next_.Reset(ptr1.get());
  }
  ptr2->next_.Reset();
}

void AssignDangling() {
  auto ptr2 = pdfium::MakeUnique<Clink>();
  {
    auto ptr1 = pdfium::MakeUnique<Clink>();
    ptr2->next_ = ptr1.get();
  }
  ptr2->next_ = nullptr;
}

void ReleaseDangling() {
  auto ptr2 = pdfium::MakeUnique<Clink>();
  {
    auto ptr1 = pdfium::MakeUnique<Clink>();
    ptr2->next_ = ptr1.get();
  }
  ptr2->next_.Release();
}

}  // namespace

TEST(UnownedPtr, PtrOk) {
  auto ptr1 = pdfium::MakeUnique<Clink>();
  {
    auto ptr2 = pdfium::MakeUnique<Clink>();
    ptr2->next_ = ptr1.get();
  }
}

TEST(UnownedPtr, PtrNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(DeleteDangling(), "");
#else
  DeleteDangling();
#endif
}

TEST(UnownedPtr, ResetOk) {
  auto ptr1 = pdfium::MakeUnique<Clink>();
  {
    auto ptr2 = pdfium::MakeUnique<Clink>();
    ptr2->next_.Reset(ptr1.get());
    ptr2->next_.Reset(nullptr);
  }
}

TEST(UnownedPtr, ResetNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(ResetDangling(), "");
#else
  ResetDangling();
#endif
}

TEST(UnownedPtr, AssignOk) {
  auto ptr1 = pdfium::MakeUnique<Clink>();
  {
    auto ptr2 = pdfium::MakeUnique<Clink>();
    ptr2->next_ = ptr1.get();
    ptr2->next_ = nullptr;
  }
}

TEST(UnownedPtr, AssignNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(AssignDangling(), "");
#else
  AssignDangling();
#endif
}

TEST(UnownedPtr, ReleaseOk) {
  auto ptr2 = pdfium::MakeUnique<Clink>();
  {
    auto ptr1 = pdfium::MakeUnique<Clink>();
    ptr2->next_ = ptr1.get();
    ptr2->next_.Release();
  }
}

TEST(UnownedPtr, ReleaseNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(ReleaseDangling(), "");
#else
  ReleaseDangling();
#endif
}

TEST(UnownedPtr, OperatorEQ) {
  int foo;
  UnownedPtr<int> ptr1;
  EXPECT_TRUE(ptr1 == ptr1);

  UnownedPtr<int> ptr2;
  EXPECT_TRUE(ptr1 == ptr2);

  UnownedPtr<int> ptr3(&foo);
  EXPECT_TRUE(&foo == ptr3);
  EXPECT_TRUE(ptr3 == &foo);
  EXPECT_FALSE(ptr1 == ptr3);

  ptr1 = &foo;
  EXPECT_TRUE(ptr1 == ptr3);
}

TEST(UnownedPtr, OperatorNE) {
  int foo;
  UnownedPtr<int> ptr1;
  EXPECT_FALSE(ptr1 != ptr1);

  UnownedPtr<int> ptr2;
  EXPECT_FALSE(ptr1 != ptr2);

  UnownedPtr<int> ptr3(&foo);
  EXPECT_FALSE(&foo != ptr3);
  EXPECT_FALSE(ptr3 != &foo);
  EXPECT_TRUE(ptr1 != ptr3);

  ptr1 = &foo;
  EXPECT_FALSE(ptr1 != ptr3);
}

TEST(UnownedPtr, OperatorLT) {
  int foos[2];
  UnownedPtr<int> ptr1(&foos[0]);
  UnownedPtr<int> ptr2(&foos[1]);

  EXPECT_FALSE(ptr1 < ptr1);
  EXPECT_TRUE(ptr1 < ptr2);
  EXPECT_FALSE(ptr2 < ptr1);
}

}  // namespace fxcrt
