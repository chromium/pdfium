// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_CFX_UNOWNED_PTR_H_
#define CORE_FXCRT_CFX_UNOWNED_PTR_H_

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "core/fxcrt/fx_memory.h"

template <class T>
class CFX_UnownedPtr {
 public:
  CFX_UnownedPtr() {}
  CFX_UnownedPtr(const CFX_UnownedPtr& that) : CFX_UnownedPtr(that.Get()) {}

  template <typename U>
  explicit CFX_UnownedPtr(U* pObj) : m_pObj(pObj) {}

  // Deliberately implicit to allow returning nullptrs.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_UnownedPtr(std::nullptr_t ptr) {}

#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  ~CFX_UnownedPtr() { Probe(); }
#endif

  CFX_UnownedPtr& operator=(T* that) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
    Probe();
#endif
    m_pObj = that;
    return *this;
  }
  CFX_UnownedPtr& operator=(const CFX_UnownedPtr& that) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
    Probe();
#endif
    if (*this != that)
      m_pObj = that.Get();
    return *this;
  }

  bool operator==(const CFX_UnownedPtr& that) const {
    return Get() == that.Get();
  }
  bool operator==(const T* that) const { return Get() == that; }
  bool operator!=(const CFX_UnownedPtr& that) const { return !(*this == that); }
  bool operator!=(const T* that) const { return !(*this == that); }
  bool operator<(const CFX_UnownedPtr& that) const {
    return std::less<T*>()(Get(), that.Get());
  }

  T* Get() const { return m_pObj; }
  explicit operator bool() const { return !!m_pObj; }
  T& operator*() const { return *m_pObj; }
  T* operator->() const { return m_pObj; }

 private:
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  void Probe() {
    if (m_pObj)
      reinterpret_cast<volatile uint8_t*>(m_pObj)[0];
  }
#endif

  T* m_pObj = nullptr;
};

#endif  // CORE_FXCRT_CFX_UNOWNED_PTR_H_
