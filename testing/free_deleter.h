// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FREE_DELETER_H_
#define TESTING_FREE_DELETER_H_

#include <stdlib.h>

namespace pdfium {

// Used with std::unique_ptr to free() objects that can't be deleted.
struct FreeDeleter {
  inline void operator()(void* ptr) const { free(ptr); }
};

}  // namespace pdfium

#endif  // TESTING_FREE_DELETER_H_
