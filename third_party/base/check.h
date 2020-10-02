// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_CHECK_H_
#define THIRD_PARTY_BASE_CHECK_H_

#include <assert.h>

#include "build/build_config.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/immediate_crash.h"

#define CHECK(condition)          \
  do {                            \
    if (UNLIKELY(!(condition))) { \
      IMMEDIATE_CRASH();          \
    }                             \
  } while (0)

#if defined(NDEBUG) && !defined(DCHECK_ALWAYS_ON)
#define DCHECK_IS_ON() 0
#else
#define DCHECK_IS_ON() 1
#endif

// Debug mode: Use assert() for better diagnostics
// Release mode, DCHECK_ALWAYS_ON: Use CHECK() since assert() is a no-op.
// Release mode, no DCHECK_ALWAYS_ON: Use assert(), which is a no-op.
#if defined(NDEBUG) && defined(DCHECK_ALWAYS_ON)
#define DCHECK CHECK
#else
#define DCHECK assert
#endif

#endif  // THIRD_PARTY_BASE_CHECK_H_
