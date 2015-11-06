// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "third_party/zlib_v128/zlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// Note: Some of these return Z_* status codes from zlib.h.
void* FPDFAPI_FlateInit(void* (*alloc_func)(void*, unsigned int, unsigned int),
                        void (*free_func)(void*, void*));
void FPDFAPI_FlateInput(void* context,
                        const unsigned char* src_buf,
                        unsigned int src_size);
int FPDFAPI_FlateOutput(void* context,
                        unsigned char* dest_buf,
                        unsigned int dest_size);
int FPDFAPI_FlateGetAvailIn(void* context);
int FPDFAPI_FlateGetAvailOut(void* context);
void FPDFAPI_FlateEnd(void* context);

#ifdef __cplusplus
}  // extern "C"
#endif
