// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_ALLOCATOR_SHIM_CONFIG_H_
#define TESTING_ALLOCATOR_SHIM_CONFIG_H_

#if !defined(PDF_USE_PARTITION_ALLOC)
#error "Included under the wrong build options"
#endif

namespace pdfium {

void ConfigurePartitionAllocShimPartitionForTest();

}  // namespace pdfium

#endif  // TESTING_ALLOCATOR_SHIM_CONFIG_H_
