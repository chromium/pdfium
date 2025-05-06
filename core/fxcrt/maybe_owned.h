// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_MAYBE_OWNED_H_
#define CORE_FXCRT_MAYBE_OWNED_H_

#include <memory>
#include <utility>
#include <variant>

#include "core/fxcrt/unowned_ptr.h"

namespace fxcrt {

// A template that can hold either owned or unowned references, and cleans up
// appropriately.  Possibly the most pernicious anti-pattern imaginable, but
// it crops up throughout the codebase due to a desire to avoid copying-in
// objects or data.
template <typename T, typename D = std::default_delete<T>>
class MaybeOwned {
 public:
  using OwnedType = std::unique_ptr<T, D>;
  using UnownedType = UnownedPtr<T>;

  MaybeOwned() = default;
  explicit MaybeOwned(T* ptr) : ptr_(UnownedType(ptr)) {}
  explicit MaybeOwned(const UnownedType& ptr) : ptr_(ptr) {}
  explicit MaybeOwned(OwnedType ptr) : ptr_(std::move(ptr)) {}

  MaybeOwned(const MaybeOwned& that) = delete;
  MaybeOwned(MaybeOwned&& that) noexcept = default;

  MaybeOwned& operator=(const MaybeOwned& that) = delete;
  MaybeOwned& operator=(MaybeOwned&& that) noexcept = default;

  ~MaybeOwned() = default;

  void Reset(T* ptr = nullptr) { ptr_ = UnownedType(ptr); }
  void Reset(OwnedType ptr) { ptr_ = std::move(ptr); }

  bool IsOwned() const { return std::holds_alternative<OwnedType>(ptr_); }

  // Helpful for untangling a collection of intertwined MaybeOwned<>.
  void ResetIfUnowned() {
    if (!IsOwned()) {
      Reset();
    }
  }

  T* Get() const& {
    return std::visit([](const auto& obj) { return obj.get(); }, ptr_);
  }
  T* Get() && {
    auto local_variable_preventing_move_elision = std::move(ptr_);
    return std::visit([](const auto& obj) { return obj.get(); },
                      local_variable_preventing_move_elision);
  }

  // Downgrades to unowned, caller takes ownership.
  OwnedType Release() {
    auto result = std::move(std::get<OwnedType>(ptr_));
    ptr_ = UnownedType(result.get());
    return result;
  }

  // Downgrades to empty, caller takes ownership.
  OwnedType ReleaseAndClear() {
    auto result = std::move(std::get<OwnedType>(ptr_));
    ptr_ = UnownedType();
    return result;
  }

  MaybeOwned& operator=(T* ptr) {
    Reset(ptr);
    return *this;
  }
  MaybeOwned& operator=(const UnownedType& ptr) {
    Reset(ptr);
    return *this;
  }
  MaybeOwned& operator=(OwnedType ptr) {
    Reset(std::move(ptr));
    return *this;
  }

  friend inline bool operator==(const MaybeOwned& lhs, const MaybeOwned& rhs) {
    return lhs.Get() == rhs.Get();
  }
  friend inline bool operator==(const MaybeOwned& lhs, const OwnedType& rhs) {
    return lhs.Get() == rhs.get();
  }
  friend inline bool operator==(const MaybeOwned& lhs, T* rhs) {
    return lhs.Get() == rhs;
  }

  explicit operator bool() const { return !!Get(); }
  T& operator*() const { return *Get(); }
  T* operator->() const { return Get(); }

 private:
  std::variant<UnownedType, OwnedType> ptr_;
};

}  // namespace fxcrt

using fxcrt::MaybeOwned;

#endif  // CORE_FXCRT_MAYBE_OWNED_H_
