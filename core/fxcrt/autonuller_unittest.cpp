// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/autonuller.h"
#include "core/fxcrt/unowned_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, AutoNuller) {
  int x = 5;
  int* ptr;
  {
    AutoNuller<int*> nuller(&ptr);
    ptr = &x;
    EXPECT_EQ(&x, ptr);
  }
  EXPECT_FALSE(ptr);
}

TEST(fxcrt, AutoNullerAbandon) {
  int x = 5;
  int* ptr;
  {
    AutoNuller<int*> nuller(&ptr);
    ptr = &x;
    EXPECT_EQ(&x, ptr);
    nuller.AbandonNullification();
  }
  EXPECT_EQ(&x, ptr);
}

TEST(fxcrt, AutoNullerUnownedPtr) {
  int x = 5;
  UnownedPtr<int> ptr;
  {
    AutoNuller<UnownedPtr<int>> nuller(&ptr);
    ptr = &x;
    EXPECT_EQ(&x, ptr);
  }
  EXPECT_FALSE(ptr);
}

TEST(fxcrt, AutoNullerUnownedPtrAbandon) {
  int x = 5;
  UnownedPtr<int> ptr;
  {
    AutoNuller<UnownedPtr<int>> nuller(&ptr);
    ptr = &x;
    EXPECT_EQ(&x, ptr);
    nuller.AbandonNullification();
  }
  EXPECT_EQ(&x, ptr);
}
