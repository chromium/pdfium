// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_UTILS_FILE_UTIL_H_
#define TESTING_UTILS_FILE_UTIL_H_

#include <stdlib.h>

#include <memory>

#include "testing/free_deleter.h"

// Reads the entire contents of a file into a newly alloc'd buffer.
std::unique_ptr<char, pdfium::FreeDeleter> GetFileContents(const char* filename,
                                                           size_t* retlen);

#endif  // TESTING_UTILS_FILE_UTIL_H_
