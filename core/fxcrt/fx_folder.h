// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_FOLDER_H_
#define CORE_FXCRT_FX_FOLDER_H_

#include "core/fxcrt/fx_string.h"

struct FX_FolderHandle;

FX_FolderHandle* FX_OpenFolder(const char* path);
bool FX_GetNextFile(FX_FolderHandle* handle,
                    ByteString* filename,
                    bool* bFolder);
void FX_CloseFolder(FX_FolderHandle* handle);

// Used with std::unique_ptr to automatically call FX_CloseFolder().
struct FxFolderHandleCloser {
  inline void operator()(FX_FolderHandle* h) const { FX_CloseFolder(h); }
};

#endif  // CORE_FXCRT_FX_FOLDER_H_
