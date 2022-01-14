// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_FX_MEMORY_WRAPPERS_H_
#define CORE_FXCRT_FX_MEMORY_WRAPPERS_H_

#include <limits>
#include <type_traits>
#include <utility>

#include "core/fxcrt/fx_memory.h"

// Used with std::unique_ptr to FX_Free raw memory.
struct FxFreeDeleter {
  inline void operator()(void* ptr) const { FX_Free(ptr); }
};

// Allocators for mapping STL containers onto Partition Alloc.
// Otherwise, replacing e.g. the FX_AllocUninit/FX_Free pairs with STL may
// undo some of the nice segregation that we get from PartitionAlloc.
template <class T, void* F(size_t, size_t)>
struct FxPartitionAllocAllocator {
 public:
#if !defined(COMPILER_MSVC) || defined(NDEBUG)
  static_assert(std::is_arithmetic<T>::value,
                "Only numeric types allowed in this partition");
#endif

  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template <class U>
  struct rebind {
    using other = FxPartitionAllocAllocator<U, F>;
  };

  FxPartitionAllocAllocator() noexcept = default;
  FxPartitionAllocAllocator(const FxPartitionAllocAllocator& other) noexcept =
      default;
  ~FxPartitionAllocAllocator() = default;

  template <typename U>
  FxPartitionAllocAllocator(
      const FxPartitionAllocAllocator<U, F>& other) noexcept {}

  pointer address(reference x) const noexcept { return &x; }
  const_pointer address(const_reference x) const noexcept { return &x; }
  pointer allocate(size_type n, const void* hint = 0) {
    return static_cast<pointer>(F(n, sizeof(value_type)));
  }
  void deallocate(pointer p, size_type n) { FX_Free(p); }
  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(value_type);
  }

  template <class U, class... Args>
  void construct(U* p, Args&&... args) {
    new (reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
  }

  template <class U>
  void destroy(U* p) {
    p->~U();
  }

  // There's no state, so they are all the same,
  bool operator==(const FxPartitionAllocAllocator& that) { return true; }
  bool operator!=(const FxPartitionAllocAllocator& that) { return false; }
};

// Used to put backing store for std::vector<> and such
// into the general partition.
template <typename T>
using FxAllocAllocator =
    FxPartitionAllocAllocator<T, pdfium::internal::AllocOrDie>;

// Used to put backing store for std::string<> and std::ostringstream<>
// into the string partition.
template <typename T>
using FxStringAllocator =
    FxPartitionAllocAllocator<T, pdfium::internal::StringAllocOrDie>;

#endif  // CORE_FXCRT_FX_MEMORY_WRAPPERS_H_
