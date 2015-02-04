// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FX_SAFE_TYPES_H_
#define FX_SAFE_TYPES_H_

#include <stdlib.h>  // For size_t.

#include "../../../third_party/numerics/safe_math.h"

typedef base::CheckedNumeric<FX_DWORD> FX_SAFE_DWORD;
typedef base::CheckedNumeric<FX_INT32> FX_SAFE_INT32;
typedef base::CheckedNumeric<size_t> FX_SAFE_SIZE_T;
typedef base::CheckedNumeric<FX_FILESIZE> FX_SAFE_FILESIZE;

#endif  // FX_SAFE_TYPES_H_
