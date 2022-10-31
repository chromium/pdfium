// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FUZZERS_XFA_PROCESS_STATE_H_
#define TESTING_FUZZERS_XFA_PROCESS_STATE_H_

#if !defined(PDF_ENABLE_XFA)
#error "XFA only"
#endif

#include "fxjs/gc/heap.h"

namespace v8 {
class Isolate;
class Platform;
}  // namespace v8

class XFAProcessState {
 public:
  XFAProcessState(v8::Platform* platform, v8::Isolate* isolate);
  ~XFAProcessState();

  cppgc::Heap* GetHeap() const;
  void ForceGCAndPump();

 private:
  v8::Platform* const platform_;
  v8::Isolate* const isolate_;
  FXGCScopedHeap heap_;
};

#endif  // TESTING_FUZZERS_XFA_PROCESS_STATE_H_
