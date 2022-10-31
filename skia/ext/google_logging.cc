// Copyright 2012 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file provides integration with Google-style "base/logging.h" assertions
// for Skia SkASSERT. If you don't want this, you can link with another file
// that provides integration with the logging of your choice.

#include <stdarg.h>
#include <stdio.h>

#include "third_party/skia/include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_WIN) && !defined(__clang__)
#include <stdlib.h>
#endif

void SkDebugf_FileLine(const char* file, int line, const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  fprintf(stderr, "%s:%d ", file, line);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

#if defined(SK_BUILD_FOR_WIN) && !defined(__clang__)

void SkDebugf_FileLineOnly(const char* file, int line) {
  fprintf(stderr, "%s:%d\n", file, line);
}

void SkAbort_FileLine(const char* file, int line, const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  fprintf(stderr, "%s:%d ", file, line);
  vfprintf(stderr, format, ap);
  va_end(ap);

  sk_abort_no_print();
  // Extra safety abort().
  abort();
}

#endif  // defined(SK_BUILD_FOR_WIN) && !defined(__clang__)
