// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/allocator_shim_config.h"

#include "base/allocator/partition_allocator/partition_alloc_buildflags.h"
#include "base/allocator/partition_allocator/shim/allocator_shim.h"

namespace pdfium {

void ConfigurePartitionAllocShimPartitionForTest() {
#if BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
#if BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
  allocator_shim::ConfigurePartitions(
      allocator_shim::EnableBrp(true),
      allocator_shim::EnableMemoryTagging(false),
      allocator_shim::SplitMainPartition(true),
      allocator_shim::UseDedicatedAlignedPartition(true), 0,
      allocator_shim::BucketDistribution::kNeutral);
#endif  // BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
#endif  // BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
}

}  // namespace pdfium
