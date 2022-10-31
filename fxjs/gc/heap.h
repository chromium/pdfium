// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_HEAP_H_
#define FXJS_GC_HEAP_H_

#include <memory>

#include "v8/include/cppgc/allocation.h"

namespace cppgc {
class Heap;
}  // namespace cppgc

namespace v8 {
class Isolate;
class Platform;
}  // namespace v8

struct FXGCHeapDeleter {
  void operator()(cppgc::Heap* heap);
};

using FXGCScopedHeap = std::unique_ptr<cppgc::Heap, FXGCHeapDeleter>;

void FXGC_Initialize(v8::Platform* platform, v8::Isolate* isolate);
void FXGC_Release();
FXGCScopedHeap FXGC_CreateHeap();
void FXGC_ForceGarbageCollection(cppgc::Heap* heap);

#define CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED \
  template <typename T>                      \
  friend class cppgc::MakeGarbageCollectedTrait

#endif  // FXJS_GC_HEAP_H_
