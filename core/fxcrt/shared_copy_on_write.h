// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_SHARED_COPY_ON_WRITE_H_
#define CORE_FXCRT_SHARED_COPY_ON_WRITE_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"

namespace fxcrt {

// A shared pointer to an object with Copy on Write semantics that makes it
// appear as if all instances were independent. |ObjClass| must implement the
// requirements of |Retainable| from retain_ptr.h, and must also provide a
// Clone() method. Often this will just call MakeRetain<>(*this) but will need
// to be virtual if |ObjClass| is subclassed.
template <class ObjClass>
class SharedCopyOnWrite {
 public:
  SharedCopyOnWrite() = default;
  SharedCopyOnWrite(const SharedCopyOnWrite& other) : object_(other.object_) {}
  ~SharedCopyOnWrite() = default;

  template <typename... Args>
  ObjClass* Emplace(Args... params) {
    object_ = pdfium::MakeRetain<ObjClass>(params...);
    return object_.Get();
  }

  SharedCopyOnWrite& operator=(const SharedCopyOnWrite& that) {
    if (*this != that) {
      object_ = that.object_;
    }
    return *this;
  }

  void SetNull() { object_.Reset(); }
  const ObjClass* GetObject() const { return object_.Get(); }

  template <typename... Args>
  ObjClass* GetPrivateCopy(Args... params) {
    if (!object_) {
      return Emplace(params...);
    }
    if (!object_->HasOneRef()) {
      object_ = object_->Clone();
    }
    return object_.Get();
  }

  friend inline bool operator==(const SharedCopyOnWrite& lhs,
                                const SharedCopyOnWrite& rhs) {
    return lhs.object_ == rhs.object_;
  }

  explicit operator bool() const { return !!object_; }

 private:
  RetainPtr<ObjClass> object_;
};

}  // namespace fxcrt

using fxcrt::SharedCopyOnWrite;

#endif  // CORE_FXCRT_SHARED_COPY_ON_WRITE_H_
