// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_INCLUDE_CFX_COUNT_REF_H_
#define CORE_FXCRT_INCLUDE_CFX_COUNT_REF_H_

#include "core/fxcrt/include/cfx_retain_ptr.h"
#include "core/fxcrt/include/fx_system.h"

template <class ObjClass>
class CFX_CountRef {
 public:
  CFX_CountRef() {}
  CFX_CountRef(const CFX_CountRef& other) : m_pObject(other.m_pObject) {}
  ~CFX_CountRef() {}

  template <typename... Args>
  ObjClass* New(Args... params) {
    m_pObject.Reset(new CountedObj(params...));
    return m_pObject.Get();
  }

  CFX_CountRef& operator=(const CFX_CountRef& that) {
    if (*this != that)
      m_pObject = that.m_pObject;
    return *this;
  }

  void Clear() { m_pObject.Reset(); }
  ObjClass* GetObject() { return m_pObject.Get(); }
  const ObjClass* GetObject() const { return m_pObject.Get(); }

  template <typename... Args>
  void MakePrivateCopy(Args... params) {
    if (!m_pObject)
      m_pObject.Reset(new CountedObj(params...));
    else if (!m_pObject->HasOneRef())
      m_pObject.Reset(new CountedObj(*m_pObject));
  }

  bool operator==(const CFX_CountRef& that) const {
    return m_pObject == that.m_pObject;
  }
  bool operator!=(const CFX_CountRef& that) const { return !(*this == that); }
  operator bool() const { return m_pObject; }
  ObjClass& operator*() const { return *m_pObject.Get(); }
  ObjClass* operator->() const { return m_pObject.Get(); }

 private:
  class CountedObj : public ObjClass {
   public:
    template <typename... Args>
    CountedObj(Args... params) : ObjClass(params...), m_RefCount(0) {}

    CountedObj(const CountedObj& src) : ObjClass(src), m_RefCount(0) {}

    bool HasOneRef() const { return m_RefCount == 1; }
    void Retain() { m_RefCount++; }
    void Release() {
      if (--m_RefCount <= 0)
        delete this;
    }

   private:
    intptr_t m_RefCount;
  };

  CFX_RetainPtr<CountedObj> m_pObject;
};

#endif  // CORE_FXCRT_INCLUDE_CFX_COUNT_REF_H_
