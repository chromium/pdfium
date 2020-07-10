// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gced_embeddertest.h"

#include "public/fpdfview.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

void GCedEmbedderTest::PumpPlatformMessageLoop() {
  v8::Platform* platform = EmbedderTestEnvironment::GetInstance()->platform();
  while (v8::platform::PumpMessageLoop(platform, isolate()))
    continue;
}
