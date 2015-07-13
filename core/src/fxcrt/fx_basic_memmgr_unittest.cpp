// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "testing/gtest/include/gtest/gtest.h"
#include "../../include/fxcrt/fx_memory.h"

namespace {

const size_t kMaxByteAlloc = std::numeric_limits<size_t>::max();
const size_t kMaxIntAlloc = kMaxByteAlloc / sizeof(int);
const size_t kOverflowIntAlloc = kMaxIntAlloc + 100;

}  // namespace

TEST(fxcrt, FX_AllocOOM) {
    EXPECT_DEATH_IF_SUPPORTED(FX_Alloc(int, kMaxIntAlloc), "");
    EXPECT_DEATH_IF_SUPPORTED(FX_Alloc(int, kOverflowIntAlloc), "");

    int* ptr = FX_Alloc(int, 1);
    EXPECT_TRUE(ptr);
    EXPECT_DEATH_IF_SUPPORTED(FX_Realloc(int, ptr, kMaxIntAlloc), "");
    EXPECT_DEATH_IF_SUPPORTED(FX_Realloc(int, ptr, kOverflowIntAlloc), "");
    FX_Free(ptr);
}

TEST(fxcrt, FX_TryAllocOOM) {
    EXPECT_FALSE(FX_TryAlloc(int, kMaxIntAlloc));
    EXPECT_FALSE(FX_TryAlloc(int, kOverflowIntAlloc));

    int* ptr = FX_Alloc(int, 1);
    EXPECT_TRUE(ptr);
    EXPECT_FALSE(FX_TryRealloc(int, ptr, kMaxIntAlloc));
    EXPECT_FALSE(FX_TryRealloc(int, ptr, kOverflowIntAlloc));
    FX_Free(ptr);
}

TEST(fxcrt, FXMEM_DefaultOOM) {
    EXPECT_FALSE(FXMEM_DefaultAlloc(kMaxByteAlloc, 0));

    void* ptr = FXMEM_DefaultAlloc(1, 0);
    EXPECT_TRUE(ptr);
    EXPECT_FALSE(FXMEM_DefaultRealloc(ptr, kMaxByteAlloc, 0));
    FXMEM_DefaultFree(ptr, 0);
}
