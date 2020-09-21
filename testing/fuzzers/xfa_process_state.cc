// Copyright 2020 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/xfa_process_state.h"

#include "fxjs/gc/heap.h"

XFAProcessState::XFAProcessState(v8::Platform* platform, v8::Isolate* isolate)
    : platform_(platform), isolate_(isolate), heap_(FXGC_CreateHeap()) {}

XFAProcessState::~XFAProcessState() = default;

cppgc::Heap* XFAProcessState::GetHeap() const {
  return heap_.get();
}

void XFAProcessState::MaybeForceGCAndPump() {
  if (++iterations_ > 1000) {
    FXGC_ForceGarbageCollection(heap_.get());
    iterations_ = 0;
  }
  while (v8::platform::PumpMessageLoop(platform_, isolate_))
    continue;
}
