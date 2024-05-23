// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/allocator_shim_config.h"

#include "core/fxcrt/check.h"
#include "partition_alloc/dangling_raw_ptr_checks.h"
#include "partition_alloc/partition_alloc_buildflags.h"
#include "partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h"

namespace pdfium {

void ConfigurePartitionAllocShimPartitionForTest() {
#if PA_BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
#if PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
  partition_alloc::SetDanglingRawPtrDetectedFn([](uintptr_t) { CHECK(0); });
#endif  // PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
  allocator_shim::ConfigurePartitionsForTesting();
#endif  // PA_BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
#endif  // PA_BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
}

}  // namespace pdfium
