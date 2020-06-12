// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_HEAP_H_
#define FXJS_GC_HEAP_H_

#include <memory>

#include "v8/include/cppgc/heap.h"
#include "v8/include/libplatform/libplatform.h"

void FXGC_Initialize(v8::Platform* platform);
void FXGC_Release();

std::unique_ptr<cppgc::Heap> FXGC_CreateHeap();
void FXGC_ReleaseHeap(std::unique_ptr<cppgc::Heap> heap);

#endif  // FXJS_GC_HEAP_H_
