// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_OBSERVED_PTR_H_
#define CORE_FXCRT_OBSERVED_PTR_H_

#include <stddef.h>

#include <set>

#include "core/fxcrt/check.h"
#include "core/fxcrt/unowned_ptr_exclusion.h"

namespace fxcrt {

class Observable {
 public:
  // General-purpose interface for more complicated cleanup.
  class ObserverIface {
   public:
    virtual ~ObserverIface() = default;
    virtual void OnObservableDestroyed() = 0;
  };

  Observable();
  Observable(const Observable& that) = delete;
  Observable& operator=(const Observable& that) = delete;
  ~Observable();

  void AddObserver(ObserverIface* pObserver);
  void RemoveObserver(ObserverIface* pObserver);
  void NotifyObservers();

 protected:
  size_t ActiveObserversForTesting() const { return observers_.size(); }

 private:
  std::set<ObserverIface*> observers_;
};

// Simple case of a self-nulling pointer.
// Generally, pass ObservedPtr<> by non-const reference since this saves
// considerable work compared to pass by value.
// NOTE: Once an UnownedPtr<> is made from an underlying reference, the
// best practice is to only refer to that object by the UnownedPtr<> and
// not the original reference.
template <typename T>
class ObservedPtr final : public Observable::ObserverIface {
 public:
  ObservedPtr() = default;
  explicit ObservedPtr(T* pObservable) : observable_(pObservable) {
    if (observable_) {
      observable_->AddObserver(this);
    }
  }
  ObservedPtr(const ObservedPtr& that) : ObservedPtr(that.Get()) {}
  ~ObservedPtr() override {
    if (observable_) {
      observable_->RemoveObserver(this);
    }
  }
  void Reset(T* pObservable = nullptr) {
    if (observable_) {
      observable_->RemoveObserver(this);
    }
    observable_ = pObservable;
    if (observable_) {
      observable_->AddObserver(this);
    }
  }
  void OnObservableDestroyed() override {
    DCHECK(observable_);
    observable_ = nullptr;
  }
  bool HasObservable() const { return !!observable_; }
  ObservedPtr& operator=(const ObservedPtr& that) {
    Reset(that.Get());
    return *this;
  }
  friend inline bool operator==(const ObservedPtr& lhs,
                                const ObservedPtr& rhs) {
    return lhs.observable_ == rhs.observable_;
  }

  template <typename U>
  friend inline bool operator==(const ObservedPtr& lhs, const U* rhs) {
    return lhs.observable_ == rhs;
  }

  explicit operator bool() const { return HasObservable(); }
  T* Get() const { return observable_; }
  T& operator*() const { return *observable_; }
  T* operator->() const { return observable_; }

 private:
  UNOWNED_PTR_EXCLUSION T* observable_ = nullptr;
};

}  // namespace fxcrt

using fxcrt::Observable;
using fxcrt::ObservedPtr;

#endif  // CORE_FXCRT_OBSERVED_PTR_H_
