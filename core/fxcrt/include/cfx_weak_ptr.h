// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_INCLUDE_CFX_WEAK_PTR_H_
#define CORE_FXCRT_INCLUDE_CFX_WEAK_PTR_H_

#include "core/fxcrt/include/fx_system.h"

template <class T>
class CFX_WeakPtr {
 public:
  class Handle {
   public:
    explicit Handle(T* ptr) : m_nCount(1), m_pObj(ptr) {}
    void reset(T* ptr) {  // CAUTION: tosses prior ref counts.
      m_nCount = 1;
      m_pObj = ptr;
    }
    void clear() {  // Now you're all weak ptrs ...
      // Guard against accidental re-entry.
      T* pObj = m_pObj;
      m_pObj = nullptr;
      delete pObj;
    }
    T* get() const { return m_pObj; }
    T* AddRef() {
      ASSERT(m_pObj);
      ++m_nCount;
      return m_pObj;
    }
    void RemoveRef() {
      if (m_nCount)
        --m_nCount;
    }
    size_t use_count() const { return m_nCount; }

   protected:
    size_t m_nCount;
    T* m_pObj;
  };

  // TODO(tsepez): implement weak pointer operations themselves.
};

#endif  // CORE_FXCRT_INCLUDE_CFX_WEAK_PTR_H_
