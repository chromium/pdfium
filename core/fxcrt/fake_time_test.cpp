// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fake_time_test.h"

#include "core/fxcrt/fx_extension.h"

void FakeTimeTest::SetUp() {
  // Arbitrary, picked descending digits, 2020-04-23 15:05:21.
  FXSYS_SetTimeFunction([]() -> time_t { return 1587654321; });
  FXSYS_SetLocaltimeFunction([](const time_t* t) { return gmtime(t); });
}

void FakeTimeTest::TearDown() {
  FXSYS_SetTimeFunction(nullptr);
  FXSYS_SetLocaltimeFunction(nullptr);
}
