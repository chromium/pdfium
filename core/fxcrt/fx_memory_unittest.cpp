// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memory.h"

#include <limits>
#include <memory>

#include "build/build_config.h"
#include "core/fxcrt/compiler_specific.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(PDF_USE_PARTITION_ALLOC)
#include "partition_alloc/partition_address_space.h"
#endif

namespace {

constexpr size_t kMaxByteAlloc = std::numeric_limits<size_t>::max();
constexpr size_t kMaxIntAlloc = kMaxByteAlloc / sizeof(int);
constexpr size_t kOverflowIntAlloc = kMaxIntAlloc + 100;
constexpr size_t kWidth = 640;
constexpr size_t kOverflowIntAlloc2D = kMaxIntAlloc / kWidth + 10;
constexpr size_t kCloseToMaxIntAlloc = kMaxIntAlloc - 100;
constexpr size_t kCloseToMaxByteAlloc = kMaxByteAlloc - 100;

}  // namespace

TEST(fxcrt, FXAllocZero) {
  uint8_t* ptr = FX_Alloc(uint8_t, 0);
  uint8_t* ptr2 = FX_Alloc(uint8_t, 0);
  EXPECT_TRUE(ptr);      // Malloc(0) is distinguishable from OOM.
  EXPECT_NE(ptr, ptr2);  // Each malloc(0) is distinguishable.
  FX_Free(ptr2);
  FX_Free(ptr);
}

TEST(fxcrt, FXAllocOOM) {
  EXPECT_DEATH_IF_SUPPORTED((void)FX_Alloc(int, kCloseToMaxIntAlloc), "");

  int* ptr = FX_Alloc(int, 1);
  EXPECT_TRUE(ptr);
  EXPECT_DEATH_IF_SUPPORTED((void)FX_Realloc(int, ptr, kCloseToMaxIntAlloc),
                            "");
  FX_Free(ptr);
}

TEST(fxcrt, FXAllocOverflow) {
  // |ptr| needs to be defined and used to avoid Clang optimizes away the
  // FX_Alloc() statement overzealously for optimized builds.
  int* ptr = nullptr;
  EXPECT_DEATH_IF_SUPPORTED(ptr = FX_Alloc(int, kOverflowIntAlloc), "") << ptr;

  ptr = FX_Alloc(int, 1);
  EXPECT_TRUE(ptr);
  EXPECT_DEATH_IF_SUPPORTED((void)FX_Realloc(int, ptr, kOverflowIntAlloc), "");
  FX_Free(ptr);
}

TEST(fxcrt, FXAllocOverflow2D) {
  // |ptr| needs to be defined and used to avoid Clang optimizes away the
  // FX_Alloc() statement overzealously for optimized builds.
  int* ptr = nullptr;
  EXPECT_DEATH_IF_SUPPORTED(ptr = FX_Alloc2D(int, kWidth, kOverflowIntAlloc2D),
                            "")
      << ptr;
}

TEST(fxcrt, FXTryAllocOOM) {
  EXPECT_FALSE(FX_TryAlloc(int, kCloseToMaxIntAlloc));

  int* ptr = FX_Alloc(int, 1);
  EXPECT_TRUE(ptr);
  EXPECT_FALSE(FX_TryRealloc(int, ptr, kCloseToMaxIntAlloc));
  FX_Free(ptr);
}

TEST(fxcrt, FXTryAllocUninit) {
  int* ptr = FX_TryAllocUninit(int, 4);
  EXPECT_TRUE(ptr);
  FX_Free(ptr);

  ptr = FX_TryAllocUninit2D(int, 4, 4);
  EXPECT_TRUE(ptr);
  FX_Free(ptr);
}

TEST(fxcrt, FXTryAllocUninitOOM) {
  EXPECT_FALSE(FX_TryAllocUninit(int, kCloseToMaxIntAlloc));
  EXPECT_FALSE(FX_TryAllocUninit2D(int, kWidth, kOverflowIntAlloc2D));
}

#if !defined(COMPILER_GCC)
TEST(fxcrt, FXTryAllocOverflow) {
  // |ptr| needs to be defined and used to avoid Clang optimizes away the
  // calloc() statement overzealously for optimized builds.
  int* ptr = (int*)calloc(sizeof(int), kOverflowIntAlloc);
  EXPECT_FALSE(ptr) << ptr;

  ptr = FX_Alloc(int, 1);
  EXPECT_TRUE(ptr);
  *ptr = 1492;  // Arbitrary sentinel.
  EXPECT_FALSE(FX_TryRealloc(int, ptr, kOverflowIntAlloc));
  EXPECT_EQ(1492, *ptr);
  FX_Free(ptr);
}
#endif

TEST(fxcrt, FXMEMDefaultOOM) {
  EXPECT_FALSE(FXMEM_DefaultAlloc(kCloseToMaxByteAlloc));

  void* ptr = FXMEM_DefaultAlloc(1);
  EXPECT_TRUE(ptr);
  EXPECT_FALSE(FXMEM_DefaultRealloc(ptr, kCloseToMaxByteAlloc));
  FXMEM_DefaultFree(ptr);
}

TEST(fxcrt, AllocZeroesMemory) {
  uint8_t* ptr = FX_Alloc(uint8_t, 32);
  ASSERT_TRUE(ptr);
  for (size_t i = 0; i < 32; ++i) {
    // SAFETY: required for testing, length and loop bounds 32.
    EXPECT_EQ(0, UNSAFE_BUFFERS(ptr[i]));
  }
  FX_Free(ptr);
}

TEST(fxcrt, FXAlign) {
  static_assert(std::numeric_limits<size_t>::max() % 2 == 1,
                "numeric limit must be odd for this test");

  size_t s0 = 0;
  size_t s1 = 1;
  size_t s2 = 2;
  size_t sbig = std::numeric_limits<size_t>::max() - 2;
  EXPECT_EQ(0u, FxAlignToBoundary<2>(s0));
  EXPECT_EQ(2u, FxAlignToBoundary<2>(s1));
  EXPECT_EQ(2u, FxAlignToBoundary<2>(s2));
  EXPECT_EQ(std::numeric_limits<size_t>::max() - 1, FxAlignToBoundary<2>(sbig));

  int i0 = 0;
  int i511 = 511;
  int i512 = 512;
  int ineg = -513;
  EXPECT_EQ(0, FxAlignToBoundary<512>(i0));
  EXPECT_EQ(512, FxAlignToBoundary<512>(i511));
  EXPECT_EQ(512, FxAlignToBoundary<512>(i512));
  EXPECT_EQ(-512, FxAlignToBoundary<512>(ineg));
}

#if defined(PDF_USE_PARTITION_ALLOC)
#if PA_BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC) && \
    PA_BUILDFLAG(HAS_64_BIT_POINTERS)
TEST(FxMemory, NewOperatorResultIsPA) {
  auto obj = std::make_unique<double>(4.0);
  EXPECT_TRUE(partition_alloc::IsManagedByPartitionAlloc(
      reinterpret_cast<uintptr_t>(obj.get())));
#if PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
  EXPECT_TRUE(partition_alloc::IsManagedByPartitionAllocBRPPool(
      reinterpret_cast<uintptr_t>(obj.get())));
#endif  // PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
}

TEST(FxMemory, MallocResultIsPA) {
  void* obj = malloc(16);
  EXPECT_TRUE(partition_alloc::IsManagedByPartitionAlloc(
      reinterpret_cast<uintptr_t>(obj)));
#if PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
  EXPECT_TRUE(partition_alloc::IsManagedByPartitionAllocBRPPool(
      reinterpret_cast<uintptr_t>(obj)));
#endif  // PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
  free(obj);
}

TEST(FxMemory, StackObjectIsNotPA) {
  int x = 3;
  EXPECT_FALSE(partition_alloc::IsManagedByPartitionAlloc(
      reinterpret_cast<uintptr_t>(&x)));
#if PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
  EXPECT_FALSE(partition_alloc::IsManagedByPartitionAllocBRPPool(
      reinterpret_cast<uintptr_t>(&x)));
#endif  // PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
}
#endif  // PA_BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC) &&
        // PA_BUILDFLAG(HAS_64_BIT_POINTERS)
#endif  // defined(PDF_USE_PARTITION_ALLOC)
