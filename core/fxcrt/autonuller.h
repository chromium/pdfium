// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_AUTONULLER_H_
#define CORE_FXCRT_AUTONULLER_H_

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/unowned_ptr.h"

namespace fxcrt {

template <typename T>
class AutoNuller {
 public:
  FX_STACK_ALLOCATED();

  explicit AutoNuller(T* location) : location_(location) {}
  ~AutoNuller() {
    if (location_) {
      *location_ = nullptr;
    }
  }
  void AbandonNullification() { location_ = nullptr; }

 private:
  UnownedPtr<T> location_;
};

}  // namespace fxcrt

using fxcrt::AutoNuller;

#endif  // CORE_FXCRT_AUTONULLER_H_
