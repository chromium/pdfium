// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/xfa_process_state.h"

#include "fxjs/gc/heap.h"
#include "v8/include/libplatform/libplatform.h"

XFAProcessState::XFAProcessState(v8::Platform* platform, v8::Isolate* isolate)
    : platform_(platform), isolate_(isolate), heap_(FXGC_CreateHeap()) {}

XFAProcessState::~XFAProcessState() {
  FXGC_ForceGarbageCollection(heap_.get());
}

cppgc::Heap* XFAProcessState::GetHeap() const {
  return heap_.get();
}

void XFAProcessState::ForceGCAndPump() {
  FXGC_ForceGarbageCollection(heap_.get());
  while (v8::platform::PumpMessageLoop(platform_, isolate_))
    continue;
}
