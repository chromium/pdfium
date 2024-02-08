// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_SCOPED_SET_TZ_H_
#define TESTING_SCOPED_SET_TZ_H_

#include <optional>
#include <string>

#include "core/fxcrt/fx_memory.h"

class ScopedSetTZ {
 public:
  FX_STACK_ALLOCATED();

  explicit ScopedSetTZ(const std::string& tz);
  ScopedSetTZ(const ScopedSetTZ&) = delete;
  ScopedSetTZ& operator=(const ScopedSetTZ&) = delete;
  ~ScopedSetTZ();

 private:
  std::optional<std::string> old_tz_;
};

#endif  // TESTING_SCOPED_SET_TZ_H_
