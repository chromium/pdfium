// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_basic.h"

extern "C" {

void*	FXMEM_DefaultAlloc(size_t byte_size, int flags)
{
    return malloc(byte_size);
}
void*	FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags)
{
    return realloc(pointer, new_size);
}
void	FXMEM_DefaultFree(void* pointer, int flags)
{
    free(pointer);
}

}  // extern "C"
