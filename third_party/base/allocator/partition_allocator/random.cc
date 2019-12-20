// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/base/allocator/partition_allocator/random.h"

#include "build/build_config.h"
#include "third_party/base/allocator/partition_allocator/spin_lock.h"
#include "third_party/base/no_destructor.h"

#if defined(OS_WIN)
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace pdfium {
namespace base {

// This is the same PRNG as used by tcmalloc for mapping address randomness;
// see http://burtleburtle.net/bob/rand/smallprng.html.
struct RandomContext {
  subtle::SpinLock lock;
  bool initialized;
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;
};

namespace {

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

uint32_t RandomValueInternal(RandomContext* x) {
  uint32_t e = x->a - rot(x->b, 27);
  x->a = x->b ^ rot(x->c, 17);
  x->b = x->c + x->d;
  x->c = x->d + e;
  x->d = e + x->a;
  return x->d;
}

#undef rot

RandomContext* GetRandomContext() {
  static NoDestructor<RandomContext> g_random_context;
  RandomContext* x = g_random_context.get();
  subtle::SpinLock::Guard guard(x->lock);
  if (UNLIKELY(!x->initialized)) {
    x->initialized = true;
    char c;
    uint32_t seed = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&c));
    uint32_t pid;
    uint32_t usec;
#if defined(OS_WIN)
    pid = GetCurrentProcessId();
    SYSTEMTIME st;
    GetSystemTime(&st);
    usec = static_cast<uint32_t>(st.wMilliseconds * 1000);
#else
    pid = static_cast<uint32_t>(getpid());
    struct timeval tv;
    gettimeofday(&tv, 0);
    usec = static_cast<uint32_t>(tv.tv_usec);
#endif
    seed ^= pid;
    seed ^= usec;
    x->a = 0xf1ea5eed;
    x->b = x->c = x->d = seed;
    for (int i = 0; i < 20; ++i) {
      RandomValueInternal(x);
    }
  }
  return x;
}

}  // namespace

uint32_t RandomValue() {
  RandomContext* x = GetRandomContext();
  subtle::SpinLock::Guard guard(x->lock);
  return RandomValueInternal(x);
}

void SetMmapSeedForTesting(int64_t seed) {
  RandomContext* x = GetRandomContext();
  subtle::SpinLock::Guard guard(x->lock);
  x->a = x->b = static_cast<uint32_t>(seed);
  x->c = x->d = static_cast<uint32_t>(seed >> 32);
  x->initialized = true;
}

}  // namespace base
}  // namespace pdfium
