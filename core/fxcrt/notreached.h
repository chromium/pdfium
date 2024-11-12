// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_NOTREACHED_H_
#define CORE_FXCRT_NOTREACHED_H_

#include <assert.h>

#include "core/fxcrt/check.h"

// NOTREACHED() annotates paths that are supposed to be unreachable. They crash
// if they are ever hit.
#define NOTREACHED() CHECK(false)

#endif  // CORE_FXCRT_NOTREACHED_H_
