// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_INCLUDE_CFX_OBSERVABLE_H_
#define CORE_FXCRT_INCLUDE_CFX_OBSERVABLE_H_

#include <set>

#include "core/fxcrt/include/fx_system.h"
#include "third_party/base/stl_util.h"

template <class T>
class CFX_Observable {
 public:
  class Observer {
   public:
    Observer() : m_pWatchedPtr(nullptr) {}
    Observer(T** pWatchedPtr) : m_pWatchedPtr(pWatchedPtr) {
      if (m_pWatchedPtr)
        (*m_pWatchedPtr)->AddObserver(this);
    }
    Observer(const Observer& that) = delete;
    ~Observer() {
      if (m_pWatchedPtr)
        (*m_pWatchedPtr)->RemoveObserver(this);
    }
    void SetWatchedPtr(T** pWatchedPtr) {
      if (m_pWatchedPtr)
        (*m_pWatchedPtr)->RemoveObserver(this);
      m_pWatchedPtr = pWatchedPtr;
      if (m_pWatchedPtr)
        (*m_pWatchedPtr)->AddObserver(this);
    }
    void OnDestroy() {
      ASSERT(m_pWatchedPtr);
      *m_pWatchedPtr = nullptr;
      m_pWatchedPtr = nullptr;
    }
    Observer& operator=(const Observer& that) = delete;

   private:
    T** m_pWatchedPtr;
  };

  CFX_Observable() {}
  CFX_Observable(const CFX_Observable& that) = delete;
  ~CFX_Observable() { NotifyObservers(); }
  void AddObserver(Observer* pObserver) {
    ASSERT(!pdfium::ContainsKey(m_Observers, pObserver));
    m_Observers.insert(pObserver);
  }
  void RemoveObserver(Observer* pObserver) {
    ASSERT(pdfium::ContainsKey(m_Observers, pObserver));
    m_Observers.erase(pObserver);
  }
  void NotifyObservers() {
    for (auto* pObserver : m_Observers)
      pObserver->OnDestroy();
    m_Observers.clear();
  }
  CFX_Observable& operator=(const CFX_Observable& that) = delete;

 private:
  std::set<Observer*> m_Observers;
};

#endif  // CORE_FXCRT_INCLUDE_CFX_OBSERVABLE_H_
