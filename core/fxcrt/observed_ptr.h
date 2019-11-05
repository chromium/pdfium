// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_OBSERVED_PTR_H_
#define CORE_FXCRT_OBSERVED_PTR_H_

#include <set>

#include "core/fxcrt/fx_system.h"
#include "third_party/base/stl_util.h"

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
  ~Observable();
  void AddObserver(ObserverIface* pObserver) {
    ASSERT(!pdfium::ContainsKey(m_Observers, pObserver));
    m_Observers.insert(pObserver);
  }
  void RemoveObserver(ObserverIface* pObserver) {
    ASSERT(pdfium::ContainsKey(m_Observers, pObserver));
    m_Observers.erase(pObserver);
  }
  void NotifyObservers() {
    for (auto* pObserver : m_Observers)
      pObserver->OnObservableDestroyed();
    m_Observers.clear();
  }
  Observable& operator=(const Observable& that) = delete;

 protected:
  size_t ActiveObserversForTesting() const { return m_Observers.size(); }

 private:
  std::set<ObserverIface*> m_Observers;
};

// Simple case of a self-nulling pointer.
template <typename T>
class ObservedPtr final : public Observable::ObserverIface {
 public:
  ObservedPtr() = default;
  explicit ObservedPtr(T* pObservable) : m_pObservable(pObservable) {
    if (m_pObservable)
      m_pObservable->AddObserver(this);
  }
  ObservedPtr(const ObservedPtr& that) : ObservedPtr(that.Get()) {}
  ~ObservedPtr() override {
    if (m_pObservable)
      m_pObservable->RemoveObserver(this);
  }
  void Reset(T* pObservable = nullptr) {
    if (m_pObservable)
      m_pObservable->RemoveObserver(this);
    m_pObservable = pObservable;
    if (m_pObservable)
      m_pObservable->AddObserver(this);
  }
  void OnObservableDestroyed() override {
    ASSERT(m_pObservable);
    m_pObservable = nullptr;
  }
  bool HasObservable() const { return !!m_pObservable; }
  ObservedPtr& operator=(const ObservedPtr& that) {
    Reset(that.Get());
    return *this;
  }
  bool operator==(const ObservedPtr& that) const {
    return m_pObservable == that.m_pObservable;
  }
  bool operator!=(const ObservedPtr& that) const { return !(*this == that); }

  template <typename U>
  bool operator==(const U* that) const {
    return Get() == that;
  }

  template <typename U>
  bool operator!=(const U* that) const {
    return !(*this == that);
  }

  explicit operator bool() const { return HasObservable(); }
  T* Get() const { return m_pObservable; }
  T& operator*() const { return *m_pObservable; }
  T* operator->() const { return m_pObservable; }

 private:
  T* m_pObservable = nullptr;
};

template <typename T, typename U>
inline bool operator==(const U* lhs, const ObservedPtr<T>& rhs) {
  return rhs == lhs;
}

template <typename T, typename U>
inline bool operator!=(const U* lhs, const ObservedPtr<T>& rhs) {
  return rhs != lhs;
}

}  // namespace fxcrt

using fxcrt::Observable;
using fxcrt::ObservedPtr;

#endif  // CORE_FXCRT_OBSERVED_PTR_H_
