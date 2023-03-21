// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_NOTREACHED_H_
#define THIRD_PARTY_BASE_NOTREACHED_H_

#include <assert.h>

#include "third_party/base/check.h"

// TODO(crbug.com/pdfium/2008): Migrate NOTREACHED() callers to
// NOTREACHED_NORETURN() which is [[noreturn]] and always FATAL. Once that's
// done, rename NOTREACHED_NORETURN() back to NOTREACHED() and remove the
// non-FATAL version.
#define NOTREACHED() DCHECK(false)

// NOTREACHED_NORETURN() annotates paths that are supposed to be unreachable.
// They crash if they are ever hit.
// TODO(crbug.com/pdfium/2008): Rename back to NOTREACHED() once there are no
// callers of the old non-CHECK-fatal macro.
#define NOTREACHED_NORETURN() CHECK(false)

#endif  // THIRD_PARTY_BASE_NOTREACHED_H_
