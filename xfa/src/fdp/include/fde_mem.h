// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_MEM
#define _FDE_MEM

#include "../../../../core/include/fxcrt/fx_memory.h"

#ifdef __cplusplus
extern "C" {
#endif
#define FDE_Alloc(size) FX_Alloc(uint8_t, size)
#define FDE_Realloc(ptr, newSize) FX_Realloc(uint8_t, ptr, newSize)
#define FDE_Free(ptr) FX_Free(ptr)
#ifdef __cplusplus
}
#define FDE_NewWith FXTARGET_NewWith
#define FDE_DeleteWith FXTARGET_DeleteWith
#endif
#endif
