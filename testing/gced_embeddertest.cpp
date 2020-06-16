// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gced_embeddertest.h"

#include "public/fpdfview.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

void GCedEmbedderTest::SetUp() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = static_cast<v8::ArrayBuffer::Allocator*>(
      FPDF_GetArrayBufferAllocatorSharedInstance());
  isolate_.reset(v8::Isolate::New(params));
  EmbedderTest::SetExternalIsolate(isolate_.get());
  EmbedderTest::SetUp();
}

void GCedEmbedderTest::TearDown() {
  EmbedderTest::TearDown();
  isolate_.reset();
}

void GCedEmbedderTest::PumpPlatformMessageLoop() {
  v8::Platform* platform = EmbedderTestEnvironment::GetInstance()->platform();
  while (v8::platform::PumpMessageLoop(platform, isolate_.get()))
    continue;
}

void GCedEmbedderTest::IsolateDeleter::operator()(v8::Isolate* ptr) {
  ptr->Dispose();
}
