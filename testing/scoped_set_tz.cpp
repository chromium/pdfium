// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/scoped_set_tz.h"

#include <stdlib.h>
#include <time.h>

#include "build/build_config.h"
#include "third_party/base/check_op.h"

namespace {

constexpr char kTZ[] = "TZ";

#if BUILDFLAG(IS_WIN)
#define SETENV(name, value) _putenv_s(name, value)
#define TZSET _tzset
#define UNSETENV(name) _putenv_s(name, "")
#else
#define SETENV(name, value) setenv(name, value, 1)
#define TZSET tzset
#define UNSETENV(name) unsetenv(name)
#endif

}  // namespace

ScopedSetTZ::ScopedSetTZ(const std::string& tz) {
  const char* old_tz = getenv(kTZ);
  if (old_tz)
    old_tz_ = old_tz;

  CHECK_EQ(0, SETENV(kTZ, tz.c_str()));
  TZSET();
}

ScopedSetTZ::~ScopedSetTZ() {
  if (old_tz_.has_value())
    CHECK_EQ(0, SETENV(kTZ, old_tz_.value().c_str()));
  else
    CHECK_EQ(0, UNSETENV(kTZ));
  TZSET();
}
