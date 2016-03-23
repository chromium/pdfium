// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_FGAS_SYSTEM_H_
#define XFA_FGAS_CRT_FGAS_SYSTEM_H_

#include "core/fxcrt/include/fx_system.h"

#define FX_RAD2DEG(r) ((r)*180.0f / FX_PI)
#define FX_DEG2RAD(a) ((a)*FX_PI / 180.0f)

FX_FLOAT FX_wcstof(const FX_WCHAR* pwsStr,
                   int32_t iLength = -1,
                   int32_t* pUsedLen = NULL);
int32_t FX_wcsnicmp(const FX_WCHAR* s1, const FX_WCHAR* s2, size_t count);

int32_t FX_filelength(FXSYS_FILE* file);
FX_BOOL FX_fsetsize(FXSYS_FILE* file, int32_t size);

#endif  // XFA_FGAS_CRT_FGAS_SYSTEM_H_
