// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_folder.h"

#include <direct.h>

#include <memory>

#include "build/build_config.h"

#if !defined(OS_WIN)
#error "built on wrong platform"
#endif

struct FX_FolderHandle {
  HANDLE m_Handle;
  bool m_bReachedEnd;
  WIN32_FIND_DATAA m_FindData;
};

FX_FolderHandle* FX_OpenFolder(const char* path) {
  auto handle = std::make_unique<FX_FolderHandle>();
  handle->m_Handle =
      FindFirstFileExA((ByteString(path) + "/*.*").c_str(), FindExInfoStandard,
                       &handle->m_FindData, FindExSearchNameMatch, nullptr, 0);
  if (handle->m_Handle == INVALID_HANDLE_VALUE)
    return nullptr;

  handle->m_bReachedEnd = false;
  return handle.release();
}

bool FX_GetNextFile(FX_FolderHandle* handle,
                    ByteString* filename,
                    bool* bFolder) {
  if (!handle)
    return false;

  if (handle->m_bReachedEnd)
    return false;

  *filename = handle->m_FindData.cFileName;
  *bFolder =
      (handle->m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  if (!FindNextFileA(handle->m_Handle, &handle->m_FindData))
    handle->m_bReachedEnd = true;
  return true;
}

void FX_CloseFolder(FX_FolderHandle* handle) {
  if (!handle)
    return;

  FindClose(handle->m_Handle);
  delete handle;
}
