// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_RETAIN_PTR_H_
#define CORE_FXCRT_RETAIN_PTR_H_

#include <stdint.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"

namespace fxcrt {

// Used with std::unique_ptr to Release() objects that can't be deleted.
template <class T>
struct ReleaseDeleter {
  inline void operator()(T* ptr) const { ptr->Release(); }
};

// Analogous to base's scoped_refptr.
template <class T>
class TRIVIAL_ABI RetainPtr {
 public:
  RetainPtr() noexcept = default;

  // Deliberately implicit to allow returning nullptrs.
  // NOLINTNEXTLINE(runtime/explicit)
  RetainPtr(std::nullptr_t ptr) {}

  explicit RetainPtr(T* pObj) noexcept : obj_(pObj) {
    if (obj_) {
      obj_->Retain();
    }
  }

  // Copy-construct a RetainPtr.
  // Required in addition to copy conversion constructor below.
  RetainPtr(const RetainPtr& that) noexcept : RetainPtr(that.Get()) {}

  // Move-construct a RetainPtr. After construction, |that| will be NULL.
  // Required in addition to move conversion constructor below.
  RetainPtr(RetainPtr&& that) noexcept { Unleak(that.Leak()); }

  // Copy conversion constructor.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  RetainPtr(const RetainPtr<U>& that) : RetainPtr(that.Get()) {}

  // Move-conversion constructor.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  RetainPtr(RetainPtr<U>&& that) noexcept {
    Unleak(that.Leak());
  }

  // Assign a RetainPtr from nullptr;
  RetainPtr& operator=(std::nullptr_t) noexcept {
    Reset();
    return *this;
  }

  // Copy-assign a RetainPtr.
  // Required in addition to copy conversion assignment below.
  RetainPtr& operator=(const RetainPtr& that) {
    if (*this != that) {
      Reset(that.Get());
    }
    return *this;
  }

  // Move-assign a RetainPtr. After assignment, |that| will be NULL.
  // Required in addition to move conversion assignment below.
  RetainPtr& operator=(RetainPtr&& that) noexcept {
    Unleak(that.Leak());
    return *this;
  }

  // Copy-convert assign a RetainPtr.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  RetainPtr& operator=(const RetainPtr<U>& that) {
    if (*this != that) {
      Reset(that.Get());
    }
    return *this;
  }

  // Move-convert assign a RetainPtr. After assignment, |that| will be NULL.
  template <class U>
    requires(std::is_convertible_v<U*, T*>)
  RetainPtr& operator=(RetainPtr<U>&& that) noexcept {
    Unleak(that.Leak());
    return *this;
  }

  ~RetainPtr() = default;

  template <class U>
  U* AsRaw() const {
    return static_cast<U*>(Get());
  }

  template <class U>
  RetainPtr<U> As() const {
    return RetainPtr<U>(AsRaw<U>());
  }

  void Reset(T* obj = nullptr) {
    if (obj) {
      obj->Retain();
    }
    obj_.reset(obj);
  }

  operator T*() const noexcept { return Get(); }
  T* Get() const noexcept { return obj_.get(); }

  void Swap(RetainPtr& that) { obj_.swap(that.obj_); }

  // Useful for passing notion of object ownership across a C API.
  T* Leak() { return obj_.release(); }
  void Unleak(T* ptr) { obj_.reset(ptr); }

  friend inline bool operator==(const RetainPtr& lhs, const RetainPtr& rhs) {
    return lhs.Get() == rhs.Get();
  }

  template <typename U>
  friend inline bool operator==(const RetainPtr& lhs, const U& rhs) {
    return lhs.Get() == rhs;
  }

  bool operator<(const RetainPtr& that) const {
    return std::less<T*>()(Get(), that.Get());
  }

  explicit operator bool() const { return !!obj_; }
  T& operator*() const { return *obj_; }
  T* operator->() const { return obj_.get(); }

 private:
  std::unique_ptr<T, ReleaseDeleter<T>> obj_;
};

// Trivial implementation - internal ref count with virtual destructor.
class Retainable {
 public:
  Retainable() = default;

  bool HasOneRef() const { return ref_count_ == 1; }

 protected:
  virtual ~Retainable() = default;

 private:
  template <typename U>
  friend struct ReleaseDeleter;

  template <typename U>
  friend class RetainPtr;

  Retainable(const Retainable& that) = delete;
  Retainable& operator=(const Retainable& that) = delete;

  // These need to be const methods operating on a mutable member so that
  // RetainPtr<const T> can be used for an object that is otherwise const
  // apart from the internal ref-counting.
  void Retain() const {
    ++ref_count_;
    CHECK(ref_count_ > 0);
  }
  void Release() const {
    CHECK(ref_count_ > 0);
    if (--ref_count_ == 0) {
      delete this;
    }
  }

  mutable uintptr_t ref_count_ = 0;
  static_assert(std::is_unsigned<decltype(ref_count_)>::value,
                "ref_count_ must be an unsigned type for overflow check"
                "to work properly in Retain()");
};

}  // namespace fxcrt

using fxcrt::ReleaseDeleter;
using fxcrt::Retainable;
using fxcrt::RetainPtr;

namespace pdfium {

// Helper to make a RetainPtr along the lines of std::make_unique<>().
// Arguments are forwarded to T's constructor. Classes managed by RetainPtr
// should have protected (or private) constructors, and should friend this
// function.
template <typename T, typename... Args>
RetainPtr<T> MakeRetain(Args&&... args) {
  return RetainPtr<T>(new T(std::forward<Args>(args)...));
}

// Type-deducing wrapper to make a RetainPtr from an ordinary pointer,
// since equivalent constructor is explicit.
template <typename T>
RetainPtr<T> WrapRetain(T* that) {
  return RetainPtr<T>(that);
}

}  // namespace pdfium

// Macro to allow construction via MakeRetain<>() only, when used
// with a private constructor in a class.
#define CONSTRUCT_VIA_MAKE_RETAIN         \
  template <typename T, typename... Args> \
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args)

#endif  // CORE_FXCRT_RETAIN_PTR_H_
