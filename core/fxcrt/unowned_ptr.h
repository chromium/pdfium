// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_UNOWNED_PTR_H_
#define CORE_FXCRT_UNOWNED_PTR_H_

// UnownedPtr is a smart pointer class that behaves very much like a
// standard C-style pointer. The advantages of using it over native T*
// pointers are:
//
// 1. It documents the nature of the pointer with no need to add a comment
//    explaining that is it // Not owned.
//
// 2. An attempt to delete an unowned ptr will fail to compile rather
//    than silently succeeding, since it is a class and not a raw pointer.
//
// 3. It is initialized to nullptr by default.
//
// When implemented via PartitionAlloc, additional properties apply.
//
// 4. When built using one of the dangling pointer detectors, the class
//    detects that the object being pointed to remains alive.
//
// 5. When built against PartitionAlloc's BRP feature, it provides the same
//    UaF protections as base::raw_ptr<T>
//
// Hence, when using UnownedPtr, no dangling pointers are ever permitted,
// even if they are not de-referenced after becoming dangling. The style of
// programming required is that the lifetime an object containing an
// UnownedPtr must be strictly less than the object to which it points.
//
// The same checks are also performed at assignment time to prove that the
// old value was not a dangling pointer.
//
// The array indexing operator[] is not supported on an unowned ptr,
// because an unowned ptr expresses a one to one relationship with some
// other heap object. Use pdfium::span<> for the cases where indexing
// into an unowned array is desired, which performs the same checks.

#include "build/build_config.h"
#include "core/fxcrt/compiler_specific.h"

#if defined(PDF_USE_PARTITION_ALLOC)
#include "partition_alloc/partition_alloc_buildflags.h"
#include "partition_alloc/pointers/raw_ptr.h"

#if !PA_BUILDFLAG(USE_PARTITION_ALLOC)
#error "pdf_use_partition_alloc=true requires use_partition_alloc=true"
#endif

#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS) || \
    PA_BUILDFLAG(USE_RAW_PTR_ASAN_UNOWNED_IMPL)
#define UNOWNED_PTR_DANGLING_CHECKS
#endif

static_assert(raw_ptr<int>::kZeroOnConstruct, "Unsafe build arguments");
static_assert(raw_ptr<int>::kZeroOnMove, "Unsafe build arguments");

template <typename T>
using UnownedPtr = raw_ptr<T>;

#else  // defined(PDF_USE_PARTITION_ALLOC)

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "core/fxcrt/unowned_ptr_exclusion.h"

namespace fxcrt {

template <class T>
class TRIVIAL_ABI GSL_POINTER UnownedPtr {
 public:
  constexpr UnownedPtr() noexcept = default;

  // Deliberately implicit to allow returning nullptrs.
  // NOLINTNEXTLINE(runtime/explicit)
  constexpr UnownedPtr(std::nullptr_t ptr) {}

  explicit constexpr UnownedPtr(T* pObj) noexcept : obj_(pObj) {}

  // Copy-construct an UnownedPtr.
  // Required in addition to copy conversion constructor below.
  constexpr UnownedPtr(const UnownedPtr& that) noexcept = default;

  // Move-construct an UnownedPtr. After construction, |that| will be NULL.
  // Required in addition to move conversion constructor below.
  constexpr UnownedPtr(UnownedPtr&& that) noexcept
      : obj_(that.ExtractAsDangling()) {}

  // Copy-conversion constructor.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  UnownedPtr(const UnownedPtr<U>& that) : obj_(static_cast<U*>(that)) {}

  // Move-conversion constructor.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  UnownedPtr(UnownedPtr<U>&& that) noexcept : obj_(that.ExtractAsDangling()) {}

  // Assign an UnownedPtr from nullptr.
  UnownedPtr& operator=(std::nullptr_t) noexcept {
    obj_ = nullptr;
    return *this;
  }

  // Assign an UnownedPtr from a raw ptr.
  UnownedPtr& operator=(T* that) noexcept {
    obj_ = that;
    return *this;
  }

  // Copy-assign an UnownedPtr.
  // Required in addition to copy conversion assignment below.
  UnownedPtr& operator=(const UnownedPtr& that) noexcept = default;

  // Move-assign an UnownedPtr. After assignment, |that| will be NULL.
  // Required in addition to move conversion assignment below.
  UnownedPtr& operator=(UnownedPtr&& that) noexcept {
    if (*this != that) {
      obj_ = that.ExtractAsDangling();
    }
    return *this;
  }

  // Copy-convert assignment.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  UnownedPtr& operator=(const UnownedPtr<U>& that) noexcept {
    if (*this != that) {
      obj_ = static_cast<U*>(that);
    }
    return *this;
  }

  // Move-convert assignment. After assignment, |that| will be NULL.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  UnownedPtr& operator=(UnownedPtr<U>&& that) noexcept {
    if (*this != that) {
      obj_ = that.ExtractAsDangling();
    }
    return *this;
  }

  ~UnownedPtr() { obj_ = nullptr; }

  friend inline bool operator==(const UnownedPtr& lhs, std::nullptr_t rhs) {
    return lhs.obj_ == nullptr;
  }
  friend inline bool operator==(const UnownedPtr& lhs, const UnownedPtr& rhs) {
    return lhs.obj_ == rhs.obj_;
  }
  bool operator<(const UnownedPtr& that) const {
    return std::less<T*>()(obj_, static_cast<T*>(that));
  }

  operator T*() const noexcept { return obj_; }
  T* get() const noexcept { return obj_; }

  T* ExtractAsDangling() { return std::exchange(obj_, nullptr); }
  void ClearAndDelete() { delete std::exchange(obj_, nullptr); }

  explicit operator bool() const { return !!obj_; }
  T& operator*() const { return *obj_; }
  T* operator->() const { return obj_; }

 private:
  UNOWNED_PTR_EXCLUSION T* obj_ = nullptr;
};

}  // namespace fxcrt

using fxcrt::UnownedPtr;

#endif  // defined(PDF_USE_PARTITION_ALLOC)

namespace pdfium {

// Type-deducing wrapper to make an UnownedPtr from an ordinary pointer,
// since equivalent constructor is explicit.
template <typename T>
UnownedPtr<T> WrapUnowned(T* that) {
  return UnownedPtr<T>(that);
}

}  // namespace pdfium

#endif  // CORE_FXCRT_UNOWNED_PTR_H_
