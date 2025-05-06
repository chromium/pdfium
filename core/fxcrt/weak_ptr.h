// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_WEAK_PTR_H_
#define CORE_FXCRT_WEAK_PTR_H_

#include <stdint.h>

#include <memory>
#include <utility>

#include "core/fxcrt/retain_ptr.h"

namespace fxcrt {

template <class T, class D = std::default_delete<T>>
class WeakPtr {
 public:
  WeakPtr() = default;
  WeakPtr(const WeakPtr& that) : handle_(that.handle_) {}
  WeakPtr(WeakPtr&& that) noexcept { Swap(that); }
  explicit WeakPtr(std::unique_ptr<T, D> pObj)
      : handle_(new Handle(std::move(pObj))) {}

  // Deliberately implicit to allow passing nullptr.
  // NOLINTNEXTLINE(runtime/explicit)
  WeakPtr(std::nullptr_t arg) {}

  explicit operator bool() const { return handle_ && !!handle_->Get(); }
  bool HasOneRef() const { return handle_ && handle_->HasOneRef(); }
  T* operator->() { return handle_->Get(); }
  const T* operator->() const { return handle_->Get(); }
  WeakPtr& operator=(const WeakPtr& that) {
    handle_ = that.handle_;
    return *this;
  }
  friend inline bool operator==(const WeakPtr& lhs, const WeakPtr& rhs) {
    return lhs.handle_ == rhs.handle_;
  }

  T* Get() const { return handle_ ? handle_->Get() : nullptr; }
  void DeleteObject() {
    if (handle_) {
      handle_->Clear();
      handle_.Reset();
    }
  }
  void Reset() { handle_.Reset(); }
  void Reset(std::unique_ptr<T, D> pObj) {
    handle_.Reset(new Handle(std::move(pObj)));
  }
  void Swap(WeakPtr& that) { handle_.Swap(that.handle_); }

 private:
  class Handle {
   public:
    explicit Handle(std::unique_ptr<T, D> ptr) : obj_(std::move(ptr)) {}

    void Reset(std::unique_ptr<T, D> ptr) { obj_ = std::move(ptr); }
    void Clear() {   // Now you're all weak ptrs ...
      obj_.reset();  // unique_ptr nulls first before invoking delete.
    }
    T* Get() const { return obj_.get(); }
    T* Retain() {
      ++count_;
      return obj_.get();
    }
    void Release() {
      if (--count_ == 0) {
        delete this;
      }
    }
    bool HasOneRef() const { return count_ == 1; }

   private:
    ~Handle() = default;

    intptr_t count_ = 0;
    std::unique_ptr<T, D> obj_;
  };

  RetainPtr<Handle> handle_;
};

}  // namespace fxcrt

using fxcrt::WeakPtr;

#endif  // CORE_FXCRT_WEAK_PTR_H_
