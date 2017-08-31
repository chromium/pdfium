// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "core/fxcrt/fx_stream.h"
#include "third_party/base/ptr_util.h"

FX_FileHandle* FX_OpenFolder(const char* path) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  auto pData = pdfium::MakeUnique<CFindFileDataA>();
  pData->m_Handle = FindFirstFileExA((CFX_ByteString(path) + "/*.*").c_str(),
                                     FindExInfoStandard, &pData->m_FindData,
                                     FindExSearchNameMatch, nullptr, 0);
  if (pData->m_Handle == INVALID_HANDLE_VALUE)
    return nullptr;

  pData->m_bEnd = false;
  return pData.release();
#else
  return opendir(path);
#endif
}

bool FX_GetNextFile(FX_FileHandle* handle,
                    CFX_ByteString* filename,
                    bool* bFolder) {
  if (!handle)
    return false;

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  if (handle->m_bEnd)
    return false;

  *filename = handle->m_FindData.cFileName;
  *bFolder =
      (handle->m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  if (!FindNextFileA(handle->m_Handle, &handle->m_FindData))
    handle->m_bEnd = true;
  return true;
#else
  struct dirent* de = readdir(handle);
  if (!de)
    return false;
  *filename = de->d_name;
  *bFolder = de->d_type == DT_DIR;
  return true;
#endif
}

void FX_CloseFolder(FX_FileHandle* handle) {
  if (!handle)
    return;

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  FindClose(handle->m_Handle);
  delete handle;
#else
  closedir(handle);
#endif
}

uint32_t GetBits32(const uint8_t* pData, int bitpos, int nbits) {
  ASSERT(0 < nbits && nbits <= 32);
  const uint8_t* dataPtr = &pData[bitpos / 8];
  int bitShift;
  int bitMask;
  int dstShift;
  int bitCount = bitpos & 0x07;
  if (nbits < 8 && nbits + bitCount <= 8) {
    bitShift = 8 - nbits - bitCount;
    bitMask = (1 << nbits) - 1;
    dstShift = 0;
  } else {
    bitShift = 0;
    int bitOffset = 8 - bitCount;
    bitMask = (1 << std::min(bitOffset, nbits)) - 1;
    dstShift = nbits - bitOffset;
  }
  uint32_t result =
      static_cast<uint32_t>((*dataPtr++ >> bitShift & bitMask) << dstShift);
  while (dstShift >= 8) {
    dstShift -= 8;
    result |= *dataPtr++ << dstShift;
  }
  if (dstShift > 0) {
    bitShift = 8 - dstShift;
    bitMask = (1 << dstShift) - 1;
    result |= *dataPtr++ >> bitShift & bitMask;
  }
  return result;
}
