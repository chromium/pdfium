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

// Used with std::vector<> to put purely numeric vectors into
// the same "general" parition used by FX_Alloc(). Otherwise,
// replacing FX_Alloc/FX_Free pairs with std::vector<> may undo
// some of the nice segregation that we get from partition alloc.
template <class T>
struct FxAllocAllocator {
 public:
  static_assert(std::is_arithmetic<T>::value,
                "Only numeric types allowed in this partition");

  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template <class U>
  struct rebind {
    using other = FxAllocAllocator<U>;
  };

  FxAllocAllocator() noexcept = default;
  FxAllocAllocator(const FxAllocAllocator& other) noexcept = default;
  ~FxAllocAllocator() = default;

  template <typename U>
  FxAllocAllocator(const FxAllocAllocator<U>& other) noexcept {}

  pointer address(reference x) const noexcept { return &x; }
  const_pointer address(const_reference x) const noexcept { return &x; }
  pointer allocate(size_type n, const void* hint = 0) {
    return static_cast<pointer>(FX_AllocOrDie(n, sizeof(value_type)));
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
  bool operator==(const FxAllocAllocator& that) { return true; }
  bool operator!=(const FxAllocAllocator& that) { return false; }
};

#endif  // CORE_FXCRT_FX_MEMORY_WRAPPERS_H_
