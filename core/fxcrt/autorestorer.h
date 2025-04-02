// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_AUTORESTORER_H_
#define CORE_FXCRT_AUTORESTORER_H_

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/unowned_ptr.h"

namespace fxcrt {

template <typename T>
class AutoRestorer {
 public:
  FX_STACK_ALLOCATED();

  explicit AutoRestorer(T* location)
      : location_(location), old_value_(*location) {}
  ~AutoRestorer() {
    if (location_) {
      *location_ = old_value_;
    }
  }
  void AbandonRestoration() { location_ = nullptr; }

 private:
  UnownedPtr<T> location_;
  const T old_value_;
};

}  // namespace fxcrt

using fxcrt::AutoRestorer;

#endif  // CORE_FXCRT_AUTORESTORER_H_
