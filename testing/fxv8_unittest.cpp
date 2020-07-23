// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fxv8_unittest.h"

#include "fxjs/cfx_v8.h"
#include "v8/include/v8.h"

void FXV8UnitTest::V8IsolateDeleter::operator()(v8::Isolate* ptr) const {
  ptr->Dispose();
}

FXV8UnitTest::FXV8UnitTest() = default;

FXV8UnitTest::~FXV8UnitTest() = default;

void FXV8UnitTest::SetUp() {
  array_buffer_allocator_ = std::make_unique<CFX_V8ArrayBufferAllocator>();

  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = array_buffer_allocator_.get();
  isolate_.reset(v8::Isolate::New(params));
}
