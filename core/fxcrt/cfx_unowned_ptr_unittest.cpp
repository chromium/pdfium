// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_unowned_ptr.h"

#include <utility>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"

namespace {

class Clink {
 public:
  CFX_UnownedPtr<Clink> next_ = nullptr;
};

void DeleteDangling() {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  delete ptr1;
  delete ptr2;
}

void AssignDangling() {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  delete ptr1;
  ptr2->next_ = nullptr;
  delete ptr2;
}

void ReleaseDangling() {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  delete ptr1;
  ptr2->next_.Release();
  delete ptr2;
}

}  // namespace

TEST(fxcrt, UnownedPtrOk) {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  delete ptr2;
  delete ptr1;
}

TEST(fxcrt, UnownedPtrNotOk) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  EXPECT_DEATH(DeleteDangling(), "");
#else
  DeleteDangling();
#endif
}

TEST(fxcrt, UnownedAssignOk) {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  ptr2->next_ = nullptr;
  delete ptr2;
  delete ptr1;
}

TEST(fxcrt, UnownedAssignNotOk) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  EXPECT_DEATH(AssignDangling(), "");
#else
  AssignDangling();
#endif
}

TEST(fxcrt, UnownedReleaseOk) {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  ptr2->next_.Release();
  delete ptr1;
  delete ptr2;
}

TEST(fxcrt, UnownedReleaseNotOk) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  EXPECT_DEATH(ReleaseDangling(), "");
#else
  ReleaseDangling();
#endif
}

TEST(fxcrt, UnownedOperatorEQ) {
  int foo;
  CFX_UnownedPtr<int> ptr1;
  EXPECT_TRUE(ptr1 == ptr1);

  CFX_UnownedPtr<int> ptr2;
  EXPECT_TRUE(ptr1 == ptr2);

  CFX_UnownedPtr<int> ptr3(&foo);
  EXPECT_TRUE(&foo == ptr3);
  EXPECT_TRUE(ptr3 == &foo);
  EXPECT_FALSE(ptr1 == ptr3);

  ptr1 = &foo;
  EXPECT_TRUE(ptr1 == ptr3);
}

TEST(fxcrt, UnownedOperatorNE) {
  int foo;
  CFX_UnownedPtr<int> ptr1;
  EXPECT_FALSE(ptr1 != ptr1);

  CFX_UnownedPtr<int> ptr2;
  EXPECT_FALSE(ptr1 != ptr2);

  CFX_UnownedPtr<int> ptr3(&foo);
  EXPECT_FALSE(&foo != ptr3);
  EXPECT_FALSE(ptr3 != &foo);
  EXPECT_TRUE(ptr1 != ptr3);

  ptr1 = &foo;
  EXPECT_FALSE(ptr1 != ptr3);
}

TEST(fxcrt, UnownedOperatorLT) {
  int foos[2];
  CFX_UnownedPtr<int> ptr1(&foos[0]);
  CFX_UnownedPtr<int> ptr2(&foos[1]);

  EXPECT_FALSE(ptr1 < ptr1);
  EXPECT_TRUE(ptr1 < ptr2);
  EXPECT_FALSE(ptr2 < ptr1);
}
