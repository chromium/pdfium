// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_folder.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>

#include "build/build_config.h"
#include "core/fxcrt/unowned_ptr.h"

#if defined(OS_WIN)
#error "built on wrong platform"
#endif

struct FX_FolderHandle {
  ByteString m_Path;
  UnownedPtr<DIR> m_Dir;
};

FX_FolderHandle* FX_OpenFolder(const char* path) {
  auto handle = std::make_unique<FX_FolderHandle>();
  DIR* dir = opendir(path);
  if (!dir)
    return nullptr;

  handle->m_Path = path;
  handle->m_Dir = dir;
  return handle.release();
}

bool FX_GetNextFile(FX_FolderHandle* handle,
                    ByteString* filename,
                    bool* bFolder) {
  if (!handle)
    return false;

  struct dirent* de = readdir(handle->m_Dir);
  if (!de)
    return false;

  ByteString fullpath = handle->m_Path + "/" + de->d_name;
  struct stat deStat;
  if (stat(fullpath.c_str(), &deStat) < 0)
    return false;

  *filename = de->d_name;
  *bFolder = S_ISDIR(deStat.st_mode);
  return true;
}

void FX_CloseFolder(FX_FolderHandle* handle) {
  if (!handle)
    return;

  closedir(handle->m_Dir.Release());
  delete handle;
}
