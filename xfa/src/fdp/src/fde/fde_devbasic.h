// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_DEVICE_BASIC_IMP
#define _FDE_DEVICE_BASIC_IMP
struct FDE_HATCHDATA {
    FX_INT32	iWidth;
    FX_INT32	iHeight;
    FX_BYTE		MaskBits[64];
};
typedef FDE_HATCHDATA const * FDE_LPCHATCHDATA;
FDE_LPCHATCHDATA FDE_DEVGetHatchData(FX_INT32 iHatchStyle);
#endif
