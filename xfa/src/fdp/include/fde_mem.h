// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_MEM
#define _FDE_MEM
#ifdef __cplusplus
extern "C" {
#endif
#define FDE_Alloc(size)				FX_Alloc(FX_BYTE, size)
#define FDE_Realloc(ptr, newSize)	FX_Realloc(FX_BYTE, ptr, newSize)
#define FDE_Free(ptr)				FX_Free(ptr)
#define FDE_New			FXTARGET_New
#define FDE_Delete		FXTARGET_Delete
#define FDE_NewWith		FXTARGET_NewWith
#define FDE_DeleteWith	FXTARGET_DeleteWith

#ifdef __cplusplus
};
#endif
#endif
