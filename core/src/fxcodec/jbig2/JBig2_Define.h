// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_DEFINE_H_
#define _JBIG2_DEFINE_H_
#include "../../../include/fxcrt/fx_system.h"
#define JBIG2_memset	FXSYS_memset
#define JBIG2_memcmp	FXSYS_memcmp
#define JBIG2_memcpy	FXSYS_memcpy
#include "JBig2_Object.h"
#define JBIG2_OOB			1
typedef struct {
    int32_t width,
             height;
    int32_t x,
             y;
    uint8_t flags;
} JBig2RegionInfo;
typedef struct {
    int32_t codelen;
    int32_t code;
} JBig2HuffmanCode;
extern "C" {
    void _FaxG4Decode(void *pModule, const uint8_t* src_buf, FX_DWORD src_size, int* pbitpos, uint8_t* dest_buf, int width, int height, int pitch = 0);
};
#define JBIG2_MAX_REFERRED_SEGMENT_COUNT		64
#define JBIG2_MAX_EXPORT_SYSMBOLS				65535
#define JBIG2_MAX_NEW_SYSMBOLS					65535
#define JBIG2_MAX_PATTERN_INDEX					65535
#define JBIG2_MAX_IMAGE_SIZE					65535
#endif
